#include <stdlib.h>
#include <stdio.h>

#include "ringbuf.h"

/* ========================================================================= */
struct ringbuf *InitRingBuffer(struct options *o)
{
   struct ringbuf *rb;
   struct rbobject *this_rbo;
   struct rbobject *last_rbo;
   unsigned int icount;

   if ( NULL == (rb = (struct ringbuf *)malloc(sizeof(struct ringbuf))) )
   {
      fprintf(stderr, "ERROR: Failed to initialize the base ringbuf structure.\n");
      return(NULL);
   }

   rb->head = NULL;
   rb->tail = NULL;
   rb->qdone = 0;
   rb->sfd = -1;
   rb->dfd = -1;
   rb->sfn = o->source;
   rb->dfn = o->dest;
   rb->rbo_size = o->buf_bsize;
   
   /* Reset (initialize) all stats */
   rb->qfull = 0;
   rb->qempty = 0;
   rb->hgets = 0;
   rb->tgets = 0;
   rb->hgot = 0;
   rb->tgot = 0;

   /* Initialize the event framework */
   if ( NULL == (rb->tevt = InitEventFramework()) )
      return(NULL);

   /* Initialize the big lock */
   pthread_mutex_init(&rb->qlock, NULL);


   last_rbo = NULL;
   icount = o->buf_rsize;
   while ( icount )
   {
      if ( NULL == (this_rbo = (struct rbobject *)malloc(sizeof(struct rbobject))) )
      {
         fprintf(stderr, "ERROR: Failed to create ring buffer object.\n");
         /* Shine trying to fix this, just return and exit */
         return(NULL);
      }

      if ( NULL == (this_rbo->buf = (char *)malloc(o->buf_bsize)) )
      {
         fprintf(stderr, "ERROR: Failed to allocate a buffer.\n");
         return(NULL);
      }

      this_rbo->next = NULL;
      this_rbo->last = last_rbo;
      this_rbo->qend = 0;
      if ( last_rbo )
         last_rbo->next = this_rbo;
      this_rbo->data_size = 0;

      /*** Now insert it into ring */
      this_rbo->next = rb->head;
      rb->head = this_rbo;
      if ( NULL == rb->tail )
         rb->tail = this_rbo;

      last_rbo = this_rbo;
      icount--;
   }

   /*** Now close the loop ***/
   /* Stitch it up */
   rb->tail->last = rb->head;
   rb->head->next = rb->tail;
   /* Close the head & tail */
   rb->tail = rb->head;
   /* STUB: I am going to try this... The tail == NULL to denote that the
      STUB:    queue is empty. */
   rb->tail = NULL;

   return(rb);
}


/* ========================================================================= */
int SetQDone(struct ringbuf *rb)
{
   /* Take the big lock */
   pthread_mutex_lock(&rb->qlock);

   if ( NULL == rb->tail )
   {
      /* The Q is empty */
      rb->head->qend = 1;
      rb->tail = rb->head;

      /* Free the big lock */
      pthread_mutex_unlock(&rb->qlock);

      return(0);
   }

   if ( rb->head->next == rb->tail )
   {
      /* The Q is full */

      /* Free the big lock */
      pthread_mutex_unlock(&rb->qlock);

      return(1);
   }

   rb->head->next->qend = 1;
   rb->head = rb->head->next;

   /* Free the big lock */
   pthread_mutex_unlock(&rb->qlock);

   return(0);
}









/* ========================================================================= */
int NextHeadItem(struct ringbuf *rb, struct rbobject **rbo)
{
   /* Take the big lock */
   pthread_mutex_lock(&rb->qlock);

   rb->hgets++; /* Update stats */

   if ( NULL == rb->tail )
   {
      /* The Q is empty */
      *rbo = rb->head;

      rb->hgot++; /* Update stats */

      /* Free the big lock */
      pthread_mutex_unlock(&rb->qlock);

      return(0);
   }

   if ( rb->head->next == rb->tail )
   {
      /* The Q is full */
      *rbo = NULL;

      rb->qfull++; /* Update stats */

      /* Free the big lock */
      pthread_mutex_unlock(&rb->qlock);

      return(1);
   }

   *rbo = rb->head->next;

   rb->hgot++; /* Update stats */

   /* Free the big lock */
   pthread_mutex_unlock(&rb->qlock);

   return(0);
}

/* ========================================================================= */
int InsertHeadItem(struct ringbuf *rb)
{
   /* Take the big lock */
   pthread_mutex_lock(&rb->qlock);

   if ( NULL == rb->tail )
   {
      rb->tail = rb->head;

      /* Free the big lock */
      pthread_mutex_unlock(&rb->qlock);

      return(0);
   }

   if ( rb->head->next == rb->tail )
   {
      /* Free the big lock */
      pthread_mutex_unlock(&rb->qlock);

      return(1);
   }

   rb->head = rb->head->next;

   /* Free the big lock */
   pthread_mutex_unlock(&rb->qlock);

   return(0);
}

/* ========================================================================= */
int NextTailItem(struct ringbuf *rb, struct rbobject **rbo)
{
   /* Take the big lock */
   pthread_mutex_lock(&rb->qlock);

   rb->tgets++; /* Update stats */

   if ( NULL == rb->tail )
   {
      /* The Q is empty */
      *rbo = NULL;

      rb->qempty++; /* Update stats */

      /* Free the big lock */
      pthread_mutex_unlock(&rb->qlock);

      return(1);
   }

   *rbo = rb->tail;

   rb->tgot++; /* Update stats */

   /* Free the big lock */
   pthread_mutex_unlock(&rb->qlock);

   return(0);
}

/* ========================================================================= */
int ReleaseTailItem(struct ringbuf *rb)
{
   /* Take the big lock */
   pthread_mutex_lock(&rb->qlock);

   if ( NULL == rb->tail )
   {
      /* Free the big lock */
      pthread_mutex_unlock(&rb->qlock);

      return(1);
   }

   if ( rb->tail == rb->head )
   {
      /* Only a single item on the Q. Mark the Q empty. */
      rb->tail = NULL;
   }
   else
   {
      /* Move the tail pointer up an item. */
      rb->tail = rb->tail->next;
   }

   /* Free the big lock */
   pthread_mutex_unlock(&rb->qlock);

   return(0);
}

