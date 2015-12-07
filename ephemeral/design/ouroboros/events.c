#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "events.h"


/* ========================================================================= */
struct tevent *InitEventFramework(void)
{
   struct tevent *e;

   if ( NULL == (e = (struct tevent *)malloc(sizeof(struct tevent))) )
   {
      fprintf(stderr, "ERROR: Failed to allocate memory for event framework.\n");
      return(NULL);
   }

   /* Set the event to "no event" */
   e->event = EVENT_NORMAL_OPS;

   /* STUB: Check return values */
   pthread_mutex_init(&e->elock, NULL);
   pthread_cond_init(&e->econd, NULL);
   
   return(e);
}

/* ========================================================================= */
int PostEvent(struct tevent *t, int event)
{
   int last_event;
   
   pthread_mutex_lock(&t->elock);

   last_event = t->event;  /* Save off last event (to see if one was missed) */
   t->event = event;       /* Set the new event                              */

   pthread_mutex_unlock(&t->elock);

   pthread_cond_signal(&t->econd);

   return(last_event);
}

/* ========================================================================= */
int GetNextEvent(struct tevent *t)
{
   int this_event;
   
   pthread_mutex_lock(&t->elock);

   while ( t->event == EVENT_NORMAL_OPS )
      pthread_cond_wait(&t->econd, &t->elock);

   this_event = t->event;       /* Save off the current event */
   t->event = EVENT_NORMAL_OPS; /* Clear the event            */

   pthread_mutex_unlock(&t->elock);

   return(this_event);
}
