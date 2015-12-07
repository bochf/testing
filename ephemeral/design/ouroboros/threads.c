#ifdef PORT_Linux
/* In order to get O_DIRECT. From the man page for open()
   "...One may have to define the _GNU_SOURCE  macro to get its definition."
*/
#define _GNU_SOURCE
#endif

/* #define DIO_DEBUG */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <fcntl.h>

#include "threads.h"


/* ========================================================================= */
void *source_loop(void *v)
{
   int go;
   struct ringbuf *rb = (struct ringbuf *)v;
   struct rbobject *rbo = NULL;
   ssize_t got;
   int qdone;

   go = 1;
   while(go)
   {
      while ( NextHeadItem(rb, &rbo) )
         sched_yield();

      got = read(rb->sfd, rbo->buf, rb->rbo_size);

      if ( got > 0 )
      {
         rbo->data_size = got;
         InsertHeadItem(rb);
      }

      if ( got == 0 )
      {
         close(rb->sfd);

         qdone = 0;
         while(SetQDone(rb))
         {
            /* These "timeout" variables are arbitrary. They might be better
               if they were based on the Q size? */
            switch(qdone)
            {
            case 1000:
               PostEvent(rb->tevt, EVENT_FAIL_QFLUSH);
               go = 0;
               break;
            case 100:
               PostEvent(rb->tevt, EVENT_SLOW_QFLUSH);
               /* Fall-thru */
            default:
               qdone++;
               sched_yield();
               break;
            }
         }

         go=0; /* This will drop us out of the loop and out of the thread. */
      }

      if ( got < 0 )
      {
         if ( errno != EINTR )
         {
#ifdef DIO_DEBUG
            fprintf(stderr, " errno = %d; ", errno);
            perror("read()");
#endif
            PostEvent(rb->tevt, EVENT_READ_ERROR);
            go = 0;
         }
      }
      
   }

   /* I don't think this is right */
   return((void *)0);
}

/* ========================================================================= */
void *dest_loop(void *v)
{
   int go;
   struct ringbuf *rb = (struct ringbuf *)v;
   struct rbobject *rbo = NULL;
   ssize_t put;

   go = 1;
   while(go)
   {
      while ( NextTailItem(rb, &rbo) )
         sched_yield();

      if ( rbo->qend )
      {
         close(rb->dfd);

         PostEvent(rb->tevt, EVENT_WRITE_DONE);
         
         pthread_exit((void *)0);
      }


      put = write(rb->dfd, rbo->buf, rbo->data_size);

      if ( put == rbo->data_size )
      {
         /* This means that it was written */
         rbo->data_size = 0;

         ReleaseTailItem(rb);
      }
      else
      {
         if ( errno != EINTR )
         {
#ifdef DIO_DEBUG
            perror("write()");
#endif

            PostEvent(rb->tevt, EVENT_WRITE_ERROR);
            go = 0;
         }
      }

   }

   /* I don't think this is right */
   return((void *)0);
}

/* ========================================================================= */
int LaunchThreads(struct options *o, struct ringbuf *rb)
{
   pthread_t t;

   int r_oflag = O_RDONLY;
   int w_oflag = O_WRONLY | O_CREAT | O_EXCL;

   if ( o->direct & DIRECT_READ )
      r_oflag |= ( O_DIRECT );

   if ( o->direct & DIRECT_WRITE )
      w_oflag |= ( O_DIRECT | O_SYNC );

   /* Open the source file */
   if ( 0 > (rb->sfd = open(rb->sfn, r_oflag)) )
   {
      fprintf(stderr, "ERROR: Unable to open source file.\n");
      perror("     ");

      return(1);
   }

   /* Open the destination file */
   if ( 0 > ( rb->dfd = open(rb->dfn, w_oflag, 0777) ) )
   {
      fprintf(stderr, "ERROR: Unable to open destination file.\n");
      perror("     ");
      return(1);
   }

   if( pthread_create(&t, NULL, source_loop, (void *)rb) )
   {
      fprintf(stderr, "ERROR: Unable to launch the source thread.\n");
      return(1);
   }

   if( pthread_create(&t, NULL, dest_loop, (void *)rb) )
   {
      fprintf(stderr, "ERROR: Unable to launch the destination thread.\n");
      return(1);
   }

   return(0);
}
