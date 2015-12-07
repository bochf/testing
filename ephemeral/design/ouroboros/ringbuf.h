#ifndef RINGBUF_H
#define RINGBUF_H

#include <pthread.h>

#include "options.h"
#include "events.h"



struct rbobject
{
   char *buf;               /* The data buffer                               */
   unsigned long data_size; /* How much was read into the buffer             */

   int qend;                /* This is the last item in the queue            */

   /* Doubly linked circular list */
   struct rbobject *next;
   struct rbobject *last;
};

struct ringbuf
{
   pthread_mutex_t qlock;   /* One lock protects all Q operations            */

   struct rbobject *head;   /* Head of the ring buffer                       */
   struct rbobject *tail;   /* Tail of the ring buffer                       */

   int qdone;               /* This Q is done. Stop trying to read it.       */

   unsigned long long qfull;  /* # of times the Q was filled (on insert)     */
   unsigned long long qempty; /* # of times the Q was empty (on pull)        */
   unsigned long long hgets;  /* # of hgets calls                            */
   unsigned long long tgets;  /* # of tgets calls                            */
   unsigned long long hgot;   /* # of successful hgets calls                 */
   unsigned long long tgot;   /* # of successful tgets calls                 */
   

   /* Stuff unique to this implementation                                    */
   int sfd;                 /* Source file descriptor                        */
   int dfd;                 /* Destination file descriptor                   */

   char *sfn;               /* Source file name                              */
   char *dfn;               /* Destination file name                         */

   unsigned long rbo_size;  /* The size of each of the buffers               */

   /* The event framwork */
   struct tevent *tevt;
};


/* ========================================================================= */
struct ringbuf *InitRingBuffer(struct options *o);




int SetQDone(struct ringbuf *rb);

int NextHeadItem(struct ringbuf *rb, struct rbobject **rbo);

int InsertHeadItem(struct ringbuf *rb);

int NextTailItem(struct ringbuf *rb, struct rbobject **rbo);

int ReleaseTailItem(struct ringbuf *rb);


#endif
