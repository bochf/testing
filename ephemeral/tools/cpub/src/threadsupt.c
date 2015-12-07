#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <signal.h>
#include <time.h>

#include "threadsupt.h"
#include "mainhelp.h"


extern int go;
extern int tt;

/* ========================================================================= */
void *sighandler(void *arg)
{
   int                sig_caught;    /* signal caught       */
   sigset_t           signal_mask;
   timer_t            timerid;
   struct itimerspec  value;
   struct sighandler_arg *tc;
   unsigned long      ms_sleep;
   unsigned long      sec_sleep;

   tc = (struct sighandler_arg *)arg;

   /* Create a timer id */
   timer_create(CLOCK_REALTIME, NULL, &timerid);

   /* Set invalid values */
   ms_sleep = 0;
   sec_sleep = 0;

   /* Set the timer values */
   if ( tc->ms_sleept >= 1000 )
   {
      ms_sleep = tc->ms_sleept % 1000;
      tc->ms_sleept = tc->ms_sleept - ms_sleep;
      sec_sleep = tc->ms_sleept / 1000;
   }
   else
   {
      sec_sleep = 0;
      ms_sleep = tc->ms_sleept;
   }

   if ((ms_sleep == 0) && (sec_sleep == 0))
   {
      /* This really can't happen, it is checked elsewhere */
      go = 0;
      return((void *)0);
   }

   /* The next timer to set */
   value.it_interval.tv_sec = sec_sleep;  
   value.it_interval.tv_nsec = ms_sleep * 1000;

   /* This timer            */
   value.it_value.tv_sec = sec_sleep;     
   value.it_value.tv_nsec = ms_sleep * 1000;

   /* Actually set the timer */
   timer_settime(timerid, TIMER_ABSTIME, &value, NULL);

   /* Define the signals that WILL be caught (listened for) */
   sigemptyset (&signal_mask);
   sigaddset (&signal_mask, SIGINT);
   sigaddset (&signal_mask, SIGTERM);
   sigaddset (&signal_mask, SIGALRM);

   /* Release the hounds */
   pthread_mutex_unlock(&tc->mutex);

   /* Use a similar loop here */
   while( go )
   {
      /* Look for signals */
      if ( 0 == sigwait (&signal_mask, &sig_caught) )
      {
         switch (sig_caught)
         {
         case SIGINT:     /* process SIGINT  */
            go = 0; /* Terminating condition */
            tt = 1; /* gotta set tt "event" and signal to evaluate go */
            pthread_cond_signal(&tc->cond);
            break;
         case SIGTERM:    /* process SIGTERM */
            go = 0;
            tt = 1;
            pthread_cond_signal(&tc->cond);
            break;
         case SIGALRM:    /* process SIGALRM */
            tt = 1;
            pthread_cond_signal(&tc->cond);
            break;
         default:         /* should normally not happen */
            error_msg("ERROR: Unexpected signal %d\n", sig_caught);
            break;
         }
         
      }
      else
      {
         /* Only one thing causes this - invalid input, so this should be
            considered more of an assert() kind of operation here. */
         error_msg("ERROR: sigwait() failure.\n");
         return((void *)0);
      }
   }

   return((void *)0);
}

/* ========================================================================= */
struct sighandler_arg *SpawnTimerThread(unsigned long ms_sleep)
{
   struct sighandler_arg *tc;
   sigset_t signal_mask;
   pthread_t  sig_thr_id;      /* signal handler thread ID */

   /* Allocate the struct for cond/mutex */
   if ( NULL == (tc = (struct sighandler_arg *)malloc(sizeof(struct sighandler_arg))) )
      return(NULL);

   /* Initialize both members of the struct before use */
   pthread_mutex_init(&tc->mutex, NULL);
   pthread_cond_init(&tc->cond, NULL);
   tc->ms_sleept = ms_sleep;

   /* Lock - so that main will know when thread is ready to roll */
   pthread_mutex_lock(&tc->mutex);

   /* NOTE: This function will set up and be the central location for
      modifications to signal handling for the framework. So... it is
      all done here. */

   /* Create a signal mask of what we will NOT listen for */
   sigemptyset (&signal_mask);
   sigaddset (&signal_mask, SIGINT);
   sigaddset (&signal_mask, SIGTERM);
   sigaddset (&signal_mask, SIGALRM);
   sigaddset (&signal_mask, SIGHUP);

   /* Block all signals */
   if(pthread_sigmask (SIG_BLOCK, &signal_mask, NULL))
   {
      error_msg("ERROR: Attempt to block signals with pthread_sigmask() failed.\n");
      return(NULL);
   }

   /* Create the timer/signal thread */
   if ( pthread_create (&sig_thr_id, NULL, sighandler, (void *)tc) )
   {
      error_msg("ERROR: Failed to launch the timer thread.\n");
      return(NULL);
   }

   return(tc);
}

/* ========================================================================= */
void *worker(void *arg)
{
   /* Local references - for convienence */
   struct worker_arg *mydata;
   pthread_mutex_t *datalock;
   pthread_cond_t *runcond;
   struct provider *this_prov;


   /* Activate the local references */
   mydata = (struct worker_arg *)arg;
   datalock = &mydata->datalock;
   runcond = &mydata->runcond;
   this_prov = mydata->prov;

   while(go)
   {
      pthread_mutex_lock(datalock);
      pthread_cond_wait(runcond, datalock);

#ifdef PROPOSED_CHANGE
      if(this_prov->refresh(mydata->sinterval))
      {
         error_msg("ERROR: %s failed to collect stats.", this_prov->name);

         mydata->data_ready = -1;
      }
      else
         mydata->data_ready = 1;
#else
      this_prov->refresh(mydata->sinterval);

      mydata->data_ready = 1;
#endif
      pthread_mutex_unlock(datalock);
   }

   return((void *)NULL);
}

/* ========================================================================= */
void *writer(void *arg)
{
   struct writer_arg *mydata;
   pthread_mutex_t *readygo;
   pthread_mutex_t *writelock;
   pthread_cond_t *writecond;
   int tindex;
   int readycnt;
   struct worker_arg *wkra_root;
   struct worker_arg *wkra_this;

   mydata = (struct writer_arg *)arg;
   writelock = &mydata->writelock;
   writecond = &mydata->writecond;
   readygo = &mydata->readygo;
   wkra_root = mydata->worker_list;

   pthread_mutex_unlock(readygo);

   while(go)
   {
      pthread_mutex_lock(writelock);
      pthread_cond_wait(writecond, writelock);

      mydata->wcount++;

      /* Loop through work product until they are all done  */
      /* RAKE: This is a "spin" until all workers are done. */
      /* Without it, the writer may (and has) beat the
         workers to the data. */
      readycnt = 0;
      while ( readycnt < mydata->wtcount )
      {
         readycnt = 0;
         tindex = 0;
         while ( tindex < mydata->wtcount )
         {
            wkra_this = &wkra_root[tindex];

            pthread_mutex_lock(&wkra_this->datalock);
#ifdef PROPOSED_CHANGE
            switch ( wkra_this->data_ready )
            {
            case 1:
               readycnt++;
               break;
            case -1:
               /* Call writer cleanup */
               exit(1);
               break;
            case 0:
            default:
               break;
            }
#else
            if ( wkra_this->data_ready )
               readycnt++;
#endif
            pthread_mutex_unlock(&wkra_this->datalock);

            tindex++;
         }
      }


      /* Ok, now all worker thread data is ready (print it) */
      /* The condition we are in:
         - All workers are done
         - The readygo lock is held here (workers cannot be restarted)
         - All (worker/change) activity on the data is quiesced
      */
      mydata->do_write(mydata->writer_data);


      /* Walk through the (active) providers, mark their data as being written */
      /* NO LOCKS ARE TAKEN!
         Why!?! - Because the writer holds the readygo lock. The writer
         has determined that all worker(provider) threads are done, and
         they cannot move again until the readygo lock is released. So there
         is effectively a lock on the data - and there is no need to take
         another. Furthermore - this is really a bitwise operation and is
         essentially atomic anyways. */
      tindex = 0;
      while ( tindex < mydata->wtcount )
      {
         wkra_this = &wkra_root[tindex];
         /* pthread_mutex_lock(&wkra_this->datalock); */
         wkra_this->data_ready = 0; /* Invalidate it - it has been written */
         /* pthread_mutex_unlock(&wkra_this->datalock); */
         
         tindex++;
      }

      pthread_mutex_unlock(writelock);  /* Done writing */
      pthread_mutex_unlock(readygo);    /* The ready part of readygo */
   }

   return((void *)NULL);

}

/* ========================================================================= */
struct thread_root *InitThreadRoot(int wcount, int sinterval)
{
   struct thread_root *tr;
   int tcnt;

   /* NOTE: Don't bother cleaning up the malloc()s. Just return NULL - the
            program will exit shortly thereafter. */

   /* Validate input - No errors reported, just input sanity checking */
   if (wcount < 1)
      return(NULL);

   /* Allocate memory for the core struct */
   if (NULL == (tr = (struct thread_root *)malloc(sizeof(struct thread_root))))
   {
      fprintf(stderr, "ERROR: Failed to allocate memory for thread structures.\n");
      return(NULL);
   }

   /* Allocate memory for the thread ID list */
   if (NULL == (tr->tlist = (pthread_t *)malloc(sizeof(pthread_t) * (wcount + 1))))
   {
      fprintf(stderr, "ERROR: Failed to allocate memory for thread structures.\n");
      return(NULL);
   }

   /* Allocate memory for the writer argument */
   if (NULL == (tr->wrarg = (struct writer_arg *)malloc(sizeof(struct writer_arg))))
   {
      fprintf(stderr, "ERROR: Failed to allocate memory for thread structures.\n");
      return(NULL);
   }

   /* Allocate memory for the worker arguments */
   if (NULL == (tr->wkalist = (struct worker_arg *)malloc(sizeof(struct worker_arg) * wcount)))
   {
      fprintf(stderr, "ERROR: Failed to allocate memory for thread structures.\n");
      return(NULL);
   }

   /* Allocate memory for the slot makrers */
   if (NULL == (tr->slot = (int *)malloc(sizeof(int) * (wcount + 1))))
   {
      fprintf(stderr, "ERROR: Failed to allocate memory for thread structures.\n");
      return(NULL);
   }

   /* Initialize what writer thread data we can */
   pthread_mutex_init(&tr->wrarg->readygo, NULL);
   pthread_mutex_init(&tr->wrarg->writelock, NULL);
   pthread_cond_init(&tr->wrarg->writecond, NULL);
   tr->wrarg->wcount = 0;                           /* No writes yet */
   tr->wrarg->wtcount = wcount;                     /* Expected number of worker threads */
   tr->wrarg->writer_data = NULL;
   tr->wrarg->do_write = NULL;
   tr->wrarg->worker_list = tr->wkalist;
   tr->slot[0] = 0;

   /* Initialize the same for the worker thread data */
   tcnt = 0;
   while ( tcnt < wcount )
   {
      pthread_mutex_init(&tr->wkalist[tcnt].datalock, NULL);
      pthread_cond_init(&tr->wkalist[tcnt].runcond, NULL);
      tr->wkalist[tcnt].tnum = tcnt;
      tr->wkalist[tcnt].data_ready = 0;
      tr->wkalist[tcnt].prov = NULL;
      tr->wkalist[tcnt].sinterval = sinterval;
      /* slot is an array that is a member of the thread_root struct */
      tr->slot[tcnt + 1] = 0;

      tcnt++;
   }

   /* At this point the thread structures are initialized but...
      - do not have thread references initialized (threads have not been created)
      - do not have data references initialized
   */

   return(tr);
}

/* ========================================================================= */
int SpawnWorkerThread(struct thread_root *tr, struct provider *prov)
{
   int index;
   struct worker_arg *wa;

   /* Find an open slot */
   index = 1; /* Skip slot 0. That is used for the writer */
   while ( tr->slot[index] != 0 )
      index++;

   /* Mark the slot as used */
   tr->slot[index] = 1;

   /* Set the provider reference in this thread argument. REMEMBER: The wkalist
      index is one-off from the slot and thread indexes */
   wa = &tr->wkalist[index -  1];
   wa->prov = prov;


   /* Now the argument list is good, lets pass it to the thread creation */
   return(pthread_create(&tr->tlist[index], NULL, worker, (void *)wa));

}

/* ========================================================================= */
int SpawnWriterThread(struct thread_root *tr)
{
   struct writer_arg *wa;

   wa = tr->wrarg;

   /* Make sure that the SetWriterData() function was called */
   if ( wa->writer_data == NULL )
      return(1);

   if ( wa->do_write == NULL )
      return(1);

   /* Now the argument list is good, lets pass it to the thread creation */
   return(pthread_create(&tr->tlist[0], NULL, writer, (void *)wa));

}


/* ========================================================================= */
int SetWriterData(struct thread_root *tr,
                  void *data,
                  int (*do_write)(void *data_handle),
                  int (*writer_finish)(void *data_handle))
{
   struct writer_arg *wa;

   /* An internal developer issue */
   assert ( tr != NULL );
      
   /* If data is null, then the writer's init() failed */
   if ( NULL == data )
      return(1);

   /* Make a local reference - makes some of the work a bit clearer */
   wa = tr->wrarg;

   /* Record the location of the data */
   wa->writer_data = data; 

   /* Record the location of the funcitons */   
   wa->do_write = do_write;
   wa->writer_finish = writer_finish;

   return(0);
}

#ifdef HOLD_OFF_ON_SIGS_FOR_NOW
/* ========================================================================= */
int sig_setup(int want_signals)
{
   sigset_t signal_set;


   if ( want_signals )
   {
      sigemptyset(&signal_set);


   }
   else
   {
      sigfillset(&signal_set);
      sigaddset(&signal_set, SIGINT);
      sigaddset(&signal_set, SIGTERM);
      pthread_sigmask(SIG_BLOCK, &signal_set, NULL);

   }

   return(0);
}
#endif
