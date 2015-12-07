#ifdef PORT_Linux
/* Required for useconds_t type */
#define _XOPEN_SOURCE 600
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "options.h"
#include "providers.h"
#include "coreprov.h"
#include "configfile.h"

#include "mainhelp.h"
#include "Writers/libwriters.h"
#include "Common/cmnproviders.h"

#include "threadsupt.h"
#include "danger.h"
#include "version.h"

#ifdef PORT_AIX
#include "AIX/aixproviders.h"
#endif

#ifdef PORT_Linux
#include "Linux/linproviders.h"
#endif

#ifdef PORT_SunOS
#include "SunOS/solproviders.h"
#endif

int help(void);
int about(void);
int make_daemon(void);

/* App globals */
int go;                /* Global to keep us running - used by sighandler */
int tt;                /* Timer tick - used to signify tick - sighandler */
int errorto;           /* Bitmask of where to log to (see mainhelp.h)    */

/* ========================================================================= */
int main(int argc, char *argv[])
{
   struct options *clo;            /* Command line options                   */
   struct proot *p;                /* Base of all provider/pitem data        */
   struct config *cfg;             /* Config file input (parsed to struct)   */
   struct prov_name *pn;
   struct thread_root *tr;
   struct provider *hot_provider;
   struct provider *this_provider; /* Used to walk the provider list */
   struct sighandler_arg *timerc;  /* This is the timer cond variable */

   int first_iteration;
   int sdivisor;      /* Divisor calculated for per-second averaging         */
   int stemp;         /* Temp variable used for sdivisor calculations        */
   int mfail; /* Used in multiple error/fail situations */

   /* Used for simple stat notification (if verbose) */
   int provcnt;
   int apcnt; /* Active provider count */
   int prrv; /* provider registration rv (used to check registrations) */

   /* Used to print (reformat) parts of error messages */
   char errstr[80];

   /* Used for the writer */
   int (*writer_finish)(void *data_handle);
   void *writer_data;
   int writer_state;

   /* Times used for the run_for config file option */
   time_t stop_at;
   time_t time_now;


   int tc;
   /* Used for $ARG substitution */
   int ascount; /* Used as basic stats on substitution counts - for reporting */
   char *argsub; 
   char *argval;
   int   argnum;
   char *before_arg;
   char *after_arg;
   char *rebuilt_arg;
   int i, j;

   /* Set default error message destination to stderr */
   errorto = ERROR_MSG_STDERR;

   if ( NULL == (clo = ReadOptions(argc, argv)) )
      return(1);

   if ( clo->bHelp )
      return(help());

   if ( clo->bAbout )
      return(about());

   if ( clo->bError )
      return(1);

   /* Initialize our provider root */
   p = Providers();

   /* Register the provider(s) */
   prrv =  CoreRegister(p); 
   prrv += AllOSRegister(p);
   prrv += CmnPRegister(p);

   if ( prrv )
   {
      error_msg("ERROR: Failed to register a provider.");
      return(1);
   }

   /* Handle all the potential list options (-l,-p,-d) */
   /* Dump providers (if asked) */
   if ( clo->bListProv )
   {
      this_provider = p->prov_list;
      while( this_provider )
      {
         printf("%s\n", this_provider->name);
         this_provider = this_provider->next;
      }
      fflush(stdout);
      return(0);
   }

   /* Dump data items for a provider (if asked) */
   if ( clo->bListPD )
   {
      this_provider = p->prov_list;
      while( this_provider )
      {
         if ( 0 == strcmp(this_provider->name, clo->provider) )
         {
            /* Match found */
            printf("%s\n", this_provider->name);
            this_provider->listavail(clo->bListTypes);
            fflush(stdout);
            return(0);
         }
         this_provider = this_provider->next;
      }
      error_msg("ERROR: No provider matches \"%s\".", clo->provider);
      fflush(stdout);
      return(1);
   }

   /* Lis all data items from all providers (if asked) */
   if ( clo->bListAllD )
   {
      DumpPItemList(p, clo->bListTypes);
      return(0);
   }

   /* We are in it for the long haul - set go to true */
   go = 1;
   tt = 0; /* No ticks at this time */

   if ( NULL == clo->confile )
   {
      error_msg("ERROR: No config file was specified. Unable to continue.");
      return(1);
   }

   /* Read in the config file */
   if ( NULL == (cfg = ReadConfigFile(clo->confile)) )
      return(1);

   /* Validate some DANGER! conditions */
   mfail = 0;
   if (( 0 == clo->bDanger ) && ( DANGER_NACK == cfg->danger_mode ))
   {
#ifdef MIN_RUN_EVERY
      /* Test for *slightly* less than a second for those trying to
         introduce skew */
      if ( cfg->run_every < MIN_RUN_EVERY )
      {
         error_msg("DANGER: A sampling period of less than a second was specified without -D!");
         mfail = 1;
      }
#endif

#ifdef MAX_QUAD_COUNT
      /* This really should be for the number of providers, not quads */
      if ( cfg->quad_count >= MAX_QUAD_COUNT )
      {
         error_msg("DANGER: Exceeded safe registered quad count. Acknowledge the danger with -D!");
         mfail = 1;
      }
#endif
   }


   /* Do some substitutions ($ARGx for arg item)
      Note - this is not exceptionally strong but it is what is supported
      at this time.

      What is supported:
      - Only ONE $ARGx per line
      - Only $ARG1 (really 0) through $ARG9
      - Only in a quad string
   */
   ascount = 0;
   pn = cfg->pnlist;
   while ( pn )
   {
      if ( NULL != ( argsub = strstr(pn->quad, "$ARG") ) )
      {
         /* Good news! This is supported */

         /* quad is a string, not a quad! So all insertions are "complex" */
         /* First - Save the leading part (if any) */
         before_arg = NULL;
         if ( argsub != pn->quad )
            before_arg = pn->quad;

         argnum = -1;
         if (( argsub[4] >= '0' ) && ( argsub[4] <= '9' ))
            argnum = argsub[4] - '0';

         after_arg = NULL;
         if (( argsub[5] != 0 ) && ( argnum >= 0 ))
            after_arg = argsub + 5;

         /* Validate that things are correct */
         if ( argnum >= 0 )
         {
            if ( NULL == (argval = GetConfArgument(clo, argnum)) )
            {
               error_msg("ERROR: Argument not supplied for \"$ARG%d\".", argnum);
               mfail = 1;
            }      
            else
            {
               if ( NULL == (rebuilt_arg = (char *)malloc(strlen(pn->quad) + strlen(argval) + 1)) )
               {
                  /* Shine this noise - just exit on the extreme rare case of failrue */
                  error_msg("ERROR: Failed to allocate memory for arg replacement string.");
                  return(1);
               }

               rebuilt_arg[0] = 0;

               i = 0;
               j = 0;
               if ( before_arg )
               {
                  while(before_arg[j] != '$')
                     rebuilt_arg[i++] = before_arg[j++];
               }

               j = 0;
               while(argval[j] != 0)
                  rebuilt_arg[i++] = argval[j++];

               if ( after_arg )
               {
                  j = 0;
                  while(after_arg[j] != 0)
                     rebuilt_arg[i++] = after_arg[j++];
               }

               rebuilt_arg[i] = 0;

               free(pn->quad);
               pn->quad = rebuilt_arg;

               ascount++; /* Keep track of how many successful substitutions done */
            }
         }
         else
         {
            dotdotdot_string(errstr, pn->quad, 35);
            error_msg("ERROR: Failed to parse argument in quad \"%s\".", errstr);
            mfail = 1;
         }
      }

      pn = pn->next;
   }

   /* Activate the data points (but not threads) */
   pn = cfg->pnlist;
   while ( pn )
   {
      if(EnableDataPoint(p, pn))
      {
         dotdotdot_string(errstr, pn->quad, 45);
         error_msg("ERROR: Unable to register \"%s\".", errstr);
         mfail = 1;
      }

      pn = pn->next;
   }

   /* mfail lets errors build up (a bit) before the app bails. This way the
      user can (potentially) diagnose more than one error at a time. */
   if ( mfail )
      return(1);
   
   /* Count the number of active providers */
   if ( 1 > (apcnt = GetActiveProviderCount(p)) )
   {
      error_msg("ERROR: No providers are registered for updates.");
      return(1);
   }

   /* See "danger.h" */
#ifdef MAX_ACTIVE_PROVIDERS
   /* Check again for danger conditions */
   if (( 0 == clo->bDanger ) && ( DANGER_NACK == cfg->danger_mode ))
   {
      if ( apcnt >= MAX_ACTIVE_PROVIDERS )
      {
         error_msg("DANGER: Exceeded safe max provider count. Acknowledge the danger with -D!");
         return(1);
      }      
   }
#endif

   /* Calculate the divisior for per-second averaging */

   if ( cfg->run_every < 950 )
   {
      /* 950 shall be considered 1 second - for those looking to 
         introduce drift. Anything less than that, then no averaging
         is going to happen. (although... concievably.... we could 
         go negative and consider this a multiplier.).    */
      sdivisor = 1;
   }
   else
   {
      stemp = cfg->run_every % 1000;

      if ( stemp >= 950 )
      {
         sdivisor = cfg->run_every + ( 1000 - stemp );
         sdivisor = sdivisor / 1000;
      }
      else if ( stemp <= 50 )
      {
         sdivisor = cfg->run_every - stemp;
         sdivisor = sdivisor / 1000;
      }
      else
      {
         /* Not going to average */
         sdivisor = 1;
      }
   }

   if ( clo->bVerbose )
   {
      provcnt = 0;
      this_provider = p->prov_list;
      while( this_provider )
      {
         provcnt++;
         this_provider = this_provider->next;
      }

      /* This MUST go to stderr - because someone might be capturing stdout */
      fprintf(stderr, "Registered %d providers, %d active.\n", provcnt, apcnt);
      fprintf(stderr, "Requested (pre-enabled) %d quads.\n", cfg->quad_count);
      fprintf(stderr, "Per-Second averaging factor is %d.\n", sdivisor);
      fprintf(stderr, "Completed %d $ARGx substitutions to the config file.\n", ascount);
   }



   /* Set up our thread structures (but not the threads themselves) */
   if ( NULL == ( tr = InitThreadRoot(apcnt, sdivisor) ) )
      return(1);

   /* Commentary: The "hooks" to install the writer modules are not as
         simple as a "point registration". The init must be called, the
         write() must be registered (and then called by a thread) and the
         finish() method must be called on exit - after likely being 
         registered. So the three interfaces are well defined... the 
         problem is that they are defined in very different places/ways.
         Additionally, the idea of calling init() inline is not so brilliant
         as the data then has to be fished out of some struct later in
         main() (as it is about to exit) to call the finish() method.
   */
   /* Start with bad values */
   writer_state = -1;
   writer_data = NULL;

   /* Make the bad values good */
   switch ( cfg->send_to )
   {
   case WM_STDOUT_TYPE:
      writer_data = InitStdoutWM(p, cfg);
      writer_state = SetWriterData(tr, writer_data, StdoutWMWrite, StdoutWMFinish);
      writer_finish = StdoutWMFinish;
      break;
   case WM_FILE_TYPE:
      writer_data = InitFileWM(p, cfg);
      writer_state = SetWriterData(tr, writer_data, WriteFileWM, FinishFileWM);
      writer_finish = FinishFileWM;
      break;
   case WM_ALARM_TYPE:
      writer_data = InitAlarmWM(p, cfg);
      writer_state = SetWriterData(tr, writer_data, WriteAlarmWM, FinishAlarmWM);
      writer_finish = FinishAlarmWM;
      break;
   case WM_BCD_TYPE:
      writer_data = InitBCDWM(p, cfg);
      writer_state = SetWriterData(tr, writer_data, WriteBCDWM, FinishBCDWM);
      writer_finish = FinishBCDWM;
      break;
   default:
      error_msg("ERROR: Internal error send_to type %d unknown.", cfg->send_to);
      break;
   }

   /* Look for bad values */
   if ( NULL == writer_data )
   {
      error_msg("ERROR: Failed to initialize writer module data.");
      return(1);
   }

   if ( 0 != writer_state )
   {
      error_msg("ERROR: Failed to set writer module data.");
      return(1);
   }

   /* This is where we will daemonize. Because:
      - The command line has been parsed (checked for errors)
      - The config file has been parsed (checked for errors)
      - The providers have been registered / validated (checked for errors)
      - The writer has looked at all the options (checked for errors)
      - Only after all of this can we fork() with clarity
      > Also note - we MUST fork() before the thread creation or they will
        be stripped away in the fork().
   */
   if ( cfg->run_method == RM_DAEMON )
   {
      make_daemon();
      /* Set default error message destination to syslog */
      errorto = ERROR_MSG_SYSLOG;
   }

   /* ========== NOW POTENTIALLY A DAEMON ========== */

   /* Lock this - makes the timer hang until the worker is ready */
   pthread_mutex_lock(&tr->wrarg->readygo);

   /* ========== JUST ONE THREAD, ABOUT TO BE MANY ========== */

   /* Spawn the timer thread first - because the launch code also sets
      the signal handling settings that all threads will inherit. If
      this is going to be moved, then the signal handling mask needs
      to be removed from this function. */
   if ( NULL == (timerc = SpawnTimerThread(cfg->run_every)) )
      return(1);

   /* Spawn worker threads */
   hot_provider = GetNextActiveProvider(p, NULL);
   while ( hot_provider )
   {
      if ( SpawnWorkerThread(tr, hot_provider) )
         return(1);

      hot_provider = GetNextActiveProvider(p, hot_provider);
   }

   /* Spawn writer thread LAST - It will release readygo */
   /* The point here is to give te workers every chance to get started
      and wait until the very last minute to spawn the writer thread.
      Because it is the writer thread that will release the readygo
      lock and let things start. While this is not *perfect* (the
      workers *could* be still starting - but that is exceptionally
      unlikely), it is fairly solid (as witnessed in practice). */
   if ( SpawnWriterThread(tr) )
      return(1);

   /* If the user said run for x time, then set that up here */
   stop_at = time(NULL);
   stop_at--;            /* Effectively an assert(), but more about
                            making the compiler happy. */

   if ( 0 < cfg->run_for )
   {
      stop_at = time(NULL);
      stop_at += cfg->run_for;
   }


   /* This is the timer loop that activates all the other threads */
   first_iteration = 1;
   while(go)
   {
      /* Check for run_for time expiration */
      if ( 0 < cfg->run_for )
      {
         time_now = time(NULL);
         if ( time_now >= stop_at )
         {
            go = 0;
            continue;
         }
      }

      /* The first iteration is used to avoid printing data on start
         that is typically incomplete anyways. (Incomplete = the diff
         values are not properly timed) Also, the first printing is
         timed VERY CLOSE to the second iteration. So it is ok to 
         skip this. Unfortunately, the way the loop is structured the
         timer (event) has to happen at the BOTTOM so that we can exit
         out (without some kind of break) when we get our event. The
         break *may* have been the more elegant answer here....
      */
      if ( first_iteration )
         first_iteration = 0;
      else
      {
         pthread_mutex_lock(&tr->wrarg->readygo); /* Gotta take lock to go */

         tc = 0;
         while ( tc < apcnt )
         {
            pthread_cond_signal(&tr->wkalist[tc].runcond);
            tc++;
         }

         pthread_cond_signal(&tr->wrarg->writecond);
      }

      /* What is going on here:
         - There is a mutex/cond semaphore that we block on waiting
           for some sort of signal. Typically this will be a simple
           alarm timer, but it may be a Ctrl-C or similarly induced
           signal. The key difference will be what the value of the
           global "go" is. If it is a timer, then go will be 1. If
           it is an exit-inducing event, then go will be 0. Either
           way, go will be evaluated at the top of the loop.
         - No, go is not protected by a mutex. No, this is not seen
           as a problem.
         - The mutex here is only for access to the cond variable.
           It protects nothing (other than the cond).
         - The cond here is about responding to the signal induced
           event loop. And as expressed earlier... there are really
           only two events we care about - a timer tick and a quit
           signal.
       */

      /* Standard cond wait block */
      pthread_mutex_lock(&timerc->mutex);
      while( tt == 0 ) /* Use tt because timer may have already fired */
         pthread_cond_wait(&timerc->cond, &timerc->mutex);
      tt = 0; /* Immediately clear */
      pthread_mutex_unlock(&timerc->mutex);
   }

   /* Take the writer lock (to insure that it is done writing)
      and then finish up the write - whatever that may be. The
      writer_finish() method was registered at about the same time the
      write() method.
   */
   pthread_mutex_lock(&tr->wrarg->writelock);
   writer_finish(tr->wrarg->writer_data);
   return(0);
}


/* ========================================================================= */
int help(void)
{
   printf("cpub - A comprehensive stat collection tool.\n");
   printf("  Usage: cpub -a | -h | <-l | -p | -d <prov>> | <config_file>\n");
   printf("  Options:\n");
   printf("    -a   Show about information and exit\n");
   printf("    -h   Show help information and exit\n");
   printf("    -D   Run Dangerously (used to override usage warnings)\n");
   printf("    -p   List all data providers and exit\n");
   printf("    -l   List all data items (all providers) and exit\n");
   printf("    -d P List all data items (for provider P) and exit\n");
   printf("    -t   Display data types in -l and -d options\n");
   printf("    -v   Print some informational stats on load\n");

   fflush(stdout);

   return(0);
}


/* ========================================================================= */
int about(void)
{
   printf("cpub - A comprehensive stat collection tool.\n");
   printf("  Version: %s\n", VERSION_STRING);
   printf("     Ottmar Mergenthaler <etaion.shrdlu@linotype.com>\n");
   printf("     William Favorite <wfavorite2@bloomberg.net>\n");

   fflush(stdout);

   return(0);
}

/* ========================================================================= */
int make_daemon(void)
{
   int i;

   /* Do the first fork */
   if(fork() != 0)
      exit(0);
    
   /* We are now running the child */
   /* Become the session leader */
   setsid();
  
   /* Fork again */
   if(fork() != 0)
      exit(0);
    
   /* Set ourself to a base set of settings */
   chdir("/");
   /* It may be appropriate to chdir("/bb/pm") here... even if it fails. */
   umask(0);

   /* Close any files that may be open */
   i = 0;
   while(i <= 256)
      close(i++);

   /* Ignore HUPs */
   /* This is done anyways by the timer thread setup */
   /* NOT NEEDED: signal(SIGHUP, SIG_IGN); */

   return(0);
}
