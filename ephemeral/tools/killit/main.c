#include <stdio.h>

#include "options.h"
#include "pwhelp.h"
#include "pshelp.h"
#include "bbenv.h"
#include "version.h"

/* ========================================================================= */
int usage(void)
{
   printf("killit - Kill PIDs based on a search criteria\n");
   printf("  Version: %s\n", VERSION_STRING);
   printf("  Usage: killit [-l|-k] [-u <uname>]\n");
   printf("  Options:\n");
   printf("    -l         List only, do not kill (default, exclusive of -k)\n");
   printf("    -k         Kill the processes (exclusive of -l) [STUBBED]\n");
   printf("    -r         Rake/repeat until done\n");
   printf("    -t <pid>   Process a tree (exclusive of -u) [PROPOSED]\n");
   printf("    -u <uname> Filter by a user name\n");
   printf("    -v         Verbose\n");

   return(0);
}


/* ========================================================================= */
int main(int argc, char *argv[])
{
   struct options *o;
   struct filter_on *f;
   struct pid_data p;
   int rake;
   int pcnt;

   if (NULL == (o = ParseOptions(argc, argv)))
   {
      /* Let the called function handle the error */
      return(1);
   }

   if (o->opt_error)
      return(1);

   if (o->opt_help)
   {
      usage();
      return(0);
   }

   /*** Check to insure that this is a dev system ***/
   if ( o->opt_verbose )
   {
      printf("Insuring this is a dev system...");
      fflush(stdout);
   }
   
   if(IsBBDevSystem())
   {
      if ( o->opt_verbose )
         printf("OK.\n");
   }
   else
   {
      if ( o->opt_verbose )
         printf("Failed.\n");

      fprintf(stderr, "ERROR: This is not a dev system. Stopping.\n");
      return(1);
   }

   /*** Create the filter - if "filtering" or not ***/
   if (NULL == (f = CreateFilter()))
   {
      fprintf(stderr, "ERROR: Unable to allocate memory. Badness is afoot.\n");
      return(1);
   }

   if (o->opt_ufilter)
   {
      if (o->opt_verbose)
         fprintf(stderr, "Filtering based on name (%s).\n", o->ufilter_name);

      if(FilterOnUName(f, o->ufilter_name))
      {
         fprintf(stderr, "ERROR: Unable to retrieve id for %s.\n", o->ufilter_name);
         return(1);
      }
   }
   
   if (o->opt_debug)
   {
      if ( 0 == f->uid )
         printf("No filtering on UID.\n");
      else
         printf("Filtering on UID %d.\n", f->uid);  
   }

   /*** Begin processing ***/
   rake = 1;
   while ( rake )
   {
      FilterReset(f);

      pcnt = 0;
      while ( GetPIDData(&p, f) )
      {
         pcnt++;

         switch(o->opt_action)
         {
         case ACTION_LIST:
            printf("%d", p.pid);
            if ( o->opt_debug )
               printf(" [%d] [%s]", p.uid, p.pi_comm);
            printf("\n");
            break;
         case ACTION_KILL:
            if ( o->opt_verbose )
            {
               printf("Sending %d signal to PID %d\n", o->kill_sig, p.pid);
               fflush(stdout);
            }
            printf("  DEBUG: kill(%d, %d);\n", p.pid, o->kill_sig);
            break;
         }
      }
      
      if ( 0 == pcnt )
         rake = 0;

      if ( 0 == o->opt_rake )
         rake = 0;
   }


   return(0);
}


