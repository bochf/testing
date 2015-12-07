#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "support.h"
#include "httpd.h"
#include "urlparse.h"
#include "iobuf.h"
#include "proctree.h"
#include "acclog.h"

/* ========================================================================= */
void *httpd_worker(void *vp);
int init_thread_data(struct httpdsrv *h);
int launch_threads(struct httpdsrv *h);
int request_assign_thread(struct httpdsrv *h, int s);
int init_thread_data(struct httpdsrv *h);
int launch_threads(struct httpdsrv *h);
void *httpd_worker(void *vp);
int send_options_header(int s, int rc);

#define MAX_URI_LENGTH 1024

struct query *read_request(struct query *q, int s, struct iobuf *b);

#define METHOD_NONE    0
/* Supported */
#define METHOD_GET     1
/* STUB: Plan on some support */
#define METHOD_POST    2
#define METHOD_OPTIONS 3
#define METHOD_HEAD    4
/* No plans to support */
#define METHOD_PUT     9
#define METHOD_OTHER   10

int parse_status_line(int *method, char *uri, char *ibuf);

#define CTYPE_TEXT 0 /* text/html */
#define CTYPE_JSON 1 /* application/json; charset=utf-8 */
#define CTYPE_YAML 2 /* text/x-yaml */

int send_header(int s, int rc, int ctype, unsigned long len);

#ifdef DEBUGGERY
#include "apuesrc.h"
#endif

/* This is the counter (and lock) for the number of service requests */
extern unsigned long customers_served;
extern pthread_mutex_t cs_lock;

/* ========================================================================= */
struct httpdsrv *HTTPDInit(struct options *o)
{
   struct httpdsrv *h;

   if ( NULL == (h = (struct httpdsrv *)malloc(sizeof(struct httpdsrv))) )
   {
      error_msg("ERROR: Failed to allocate memory setting up httpd server.");
      return(NULL);
   }

   /* Build out all the trees */
   if ( NULL == (h->trees = PlantForest(RAINFOREST, o)) )
      return(NULL);
   
   /* Initialize some values */
   h->lsock = -1;

   /* Transfer over the options struct - it will be used for a number of 
      configuration items in the httpd worker threads. */
   h->o = o;
   
   /* Set the run flag to non-zero */
   h->trun = 1;

   /* Set up all the thread data */
   if (init_thread_data(h))
      return(NULL);

   if (InitAccessLog(o->cf_accessfile))
      return(NULL);

   return(h);
}

/* ========================================================================= */
int HTTPDStart(struct httpdsrv *h)
{
   char gaistr[128];
   struct addrinfo hint;
   struct addrinfo *ailist, *aiuse;
   int err;
   int hc;

   /* Start the worker threads */
   if(launch_threads(h))
      return(1);

   assert(NULL != h);

   memset(&hint, 0, sizeof(struct addrinfo));

   hint.ai_flags = AI_PASSIVE;
   hint.ai_family = AF_INET;
   hint.ai_socktype = SOCK_STREAM;
   hint.ai_protocol = IPPROTO_TCP;
   hint.ai_addrlen = 0;
   hint.ai_canonname = NULL;
   hint.ai_addr = NULL;
   hint.ai_next = NULL;

   if (0 != ( err = getaddrinfo(h->o->cf_host, h->o->cf_port, &hint, &ailist) ))
   {
      error_msg("ERROR: Failed call to getaddrinfo() for local connection info.");
      strcpy(gaistr, gai_strerror(err));
      chomp(gaistr);
      error_msg("       %s", gaistr);
      return(1);
   }

   if ( NULL == (aiuse = ailist) )
   {
      error_msg("ERROR: Empty list from getaddrinfo.");
      return(1);
   }

   /* Set up the socket */
   if ( 0 > (h->lsock = socket(aiuse->ai_family, aiuse->ai_socktype , aiuse->ai_protocol)) )
   {
      error_msg("ERROR: Failed to establish a socket.");
      return(1);
   }


   if ( 0 > bind(h->lsock, aiuse->ai_addr, aiuse->ai_addrlen) )
   {
      error_msg("ERROR: Failed to bind to established address.");
      return(1);
   }


   if ( 0 > listen(h->lsock, 0) )
   {
      error_msg("ERROR: HTTPd server failed to listen().");
      return(1);
   }

   while ( h->trun )
   {
      if ( 0 < ( hc = accept(h->lsock, NULL, NULL) ) )
      {
         /* This is a potential logging point.
            Why we do NOT log here:
            - Even though it is a non-contensious point (in the parent thread
              with no locking issues), it does have the potential to hold up
              connections with slow I/O to a file.
            - There is no request data - only the remote connection data
              that we can get elsewhere.
            - There is no timing data (how long it took to handle the request)
              as we have not determined what the requestor wants.
            - There is no status of the completed request.
         */
         request_assign_thread(h, hc);
      }
   }

   close(h->lsock);

   return(0);
}

/* ========================================================================= */
int request_assign_thread(struct httpdsrv *h, int s)
{
   int rv;

   pthread_mutex_lock(h->qlock);
   /* NOTE: We do NOT check to see if the last queued item was retrieved.
            This could be done for reporting, but this is a single item
            "queue" and is not designed for high volume requests. We do
            save the value and return it so that it can be checked elsewhere.*/
   rv = h->squeue;
   h->squeue = s;
   pthread_mutex_unlock(h->qlock);
   pthread_cond_signal(h->qready);

   return(rv);
}

/* ========================================================================= */
int init_thread_data(struct httpdsrv *h)
{
   if ( NULL == (h->tid_list = (pthread_t *)malloc(sizeof(pthread_t) * h->o->cf_hst)) )
   {
      error_msg("ERROR: Failed to allocate memory for tid list.");
      return(1);
   }

   /* Init socket, mutex, and cond var */
   h->trun = 1;   /* Run, don't exit */
   h->squeue = -1; /* The "Q" is empty */

   if ( NULL == ( h->qlock = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t)) ) )
   {
      error_msg("ERROR: Failed to allocate worker mutex.");
      return(1);
   }

   if ( NULL == ( h->qready = (pthread_cond_t *)malloc(sizeof(pthread_cond_t)) ) )
   {
      error_msg("ERROR: Failed to allocate worker cond variable.");
      return(1);
   }

   if ( pthread_mutex_init(h->qlock, NULL) )
   {
      error_msg("ERROR: Failed to initialize worker mutex.");
      return(1);
   }

   if ( pthread_cond_init(h->qready, NULL) )
   {
      error_msg("ERROR: Failed to initialize worker cond variable.");
      return(1);
   }

   return(0);
}

/* ========================================================================= */
int launch_threads(struct httpdsrv *h)
{
   int tcnt;
   pthread_t *tid;

   tcnt = 0;
   while ( tcnt < h->o->cf_hst )
   {
      /* Simplify a bit for readability */
      tid = &h->tid_list[tcnt];

      if ( pthread_create (tid, NULL, httpd_worker, (void *)h) )
      {
         error_msg("ERROR: Failed to launch HTTPd worker thread.");
         return(1);
      }

      tcnt++;
   }

   return(0);
}

/* ========================================================================= */
void *httpd_worker(void *vp)
{
   struct httpdsrv *h;
   int s;
   struct iobuf *ibuf; /* input buffer */
   struct iobuf *obuf; /* output buffer */
   struct query *q = NULL;
   int httprc = 200; /* Set to an initial value for compiler sake */

   /* Bring in the big data structure */
   h = (struct httpdsrv *)vp;

   /* Allocate memory for the input buffer */

   if ( NULL == (ibuf = NewIOBuf(h->o->cf_ibufsz)) )
   {
      error_msg("ERROR: Worker thread failed to allocate buffer memory.");
      pthread_exit((void *)1);
   }


   if ( NULL == (obuf = NewIOBuf(h->o->cf_obufsz)) )
   {
      error_msg("ERROR: Worker thread failed to allocate buffer memory.");
      pthread_exit((void *)1);
   }

   while ( h->trun )
   {   
      /* This is our holding pattern location */
      pthread_mutex_lock(h->qlock);
      while ( h->squeue == -1 )
         pthread_cond_wait(h->qready, h->qlock);
      s = h->squeue;
      h->squeue = -1;
      pthread_mutex_unlock(h->qlock);

      /* Passing a -2 to all our threads will shut them all down */
      if ( s == -2 )
         pthread_exit((void *)0);

      /* Read what the client sent */
      ResetIOBuf(ibuf);
      if (NULL == (q = read_request(q, s, ibuf)))
      {
         error_msg("ERROR: Worker thread failed to allocate query memory.\n");
         pthread_exit((void *)1);
      }

      /* Go ahead and set up the send - as long as the socket is good */
      if ( q->parse_failure != UP_DEAD_SOCKET )
      {
         /* The query could still be bad. But we will consider this a customer
            served - even if it may have been an error. */
         pthread_mutex_lock(&cs_lock);
         customers_served++;
         pthread_mutex_unlock(&cs_lock);
         
         /* Conditionally handle what method the user/client requested */
         switch(q->method)
         {
         case METHOD_GET:
         case METHOD_POST:
            RenderTree(obuf, q, h->trees, h->o, &httprc);
            send_header(s, httprc, 0, obuf->eod);

            if ( 200 == httprc )
               send(s, obuf->buf, obuf->eod, 0);
            break;
         case METHOD_HEAD: /* Like GET but no data */
            RenderTree(obuf, q, h->trees, h->o, &httprc);
            send_header(s, httprc, 0, obuf->eod);
            break;
         case METHOD_OPTIONS:
            send_options_header(s, 200);
            break;
         default:
            send_options_header(s, 405);
            break;
         }

         /* LogAccess() is dependent on an open socket to get connection
            information. It must be called before the close() on the socket. */
         LogAccess(s, q->tree, q->format, httprc);
      }

      close(s);
   } /* while ( continue_to_run ) */

   return((void *)0);
}

/* ========================================================================= */
struct query *read_request(struct query *q, int s, struct iobuf *b)
{
   size_t offset; 
   ssize_t got;
   int recvmore;
   char uri[MAX_URI_LENGTH];
   int method;

   offset = 0;
   recvmore = 1;
   while ( recvmore )
   {
      got = recv(s, b->buf + offset, b->size - offset, 0);

      if ( got < 0 )
      {
         if ( errno != EINTR )
         {
            if ( q )
               q->parse_failure |= UP_DEAD_SOCKET;
            else
            {
               /* NOTE:
                    ParseURLString() will allocate and initialize q if it is NULL.
                    So... it is possible that q might be NULL here. If it is, then
                    allocate it so we can pass the specific error. */
               if ( NULL == (q = (struct query *)malloc(sizeof(struct query))) )
               {
                  error_msg("ERROR: Failed to allocate memory for a query.");
                  pthread_exit((void *)1);
               }

               q->parse_failure = UP_DEAD_SOCKET;
            }

            error_msg("ERROR: Dropped connection on recv() call.\n");
            return(q);
         }
      }
      else
      {
         offset += got;
         b->eod += got;
         
         if ( got == 0 )
         {
            /* This should never happen */
            if ( offset > 10 ) /* Arbitrary number *somewhat* large enough to hold a request */
            {
               if (( b->buf[offset - 4] == '\r' ) &&
                   ( b->buf[offset - 3] == '\n' ) &&
                   ( b->buf[offset - 2] == '\r' ) &&
                   ( b->buf[offset - 1] == '\n' ))
               {
                  recvmore = 0;

                  parse_status_line(&method, uri, b->buf);
               }
            } /* If some data ( > 10 ) */
         }
         else
         {
            if ( offset > 10 ) /* Arbitrary number *somewhat* large enough to hold a request */
            {
               if (( b->buf[offset - 4] == '\r' ) &&
                   ( b->buf[offset - 3] == '\n' ) &&
                   ( b->buf[offset - 2] == '\r' ) &&
                   ( b->buf[offset - 1] == '\n' ))
               {
                  recvmore = 0;

                  parse_status_line(&method, uri, b->buf);
               }

            } /* If some data ( > 10 ) */
         } /* if ( got == 0 ) else */
      } /* if ( got < 0 ) else */
   } /* while recvmore */


   b->buf[b->eod] = 0; /* Terminate for safety */





   /* STUB: Enable this to see the http request header
      




      DEBUG_STUB("%s", b->buf);




   */

   switch(method)
   {
   case METHOD_GET:
      q = ParseURLString(q, uri);
      q->method = METHOD_GET;
      break;
   case METHOD_HEAD:
      q = ParseURLString(q, uri);
      q->method = METHOD_HEAD;
      break;
   case METHOD_OPTIONS:
      q = ParseURLString(q, NULL);         /* This insures that q has been allocated */
      q->method = METHOD_OPTIONS;
      break;
   case METHOD_POST:
      q = ParseURLString(q, uri);
      q->method = METHOD_POST;
      break;
   default:
      error_msg("ERROR: Unsupported request method.\n");
      q = ParseURLString(q, NULL);         /* This insures that q has been allocated */
      q->parse_failure |= UP_BAD_METHOD;
      break;
   }

   return(q);
}

/* ========================================================================= */
int parse_status_line(int *method, char *uri, char *ibuf)
{
   int i, j;

   /* Set the method to "no method" */
   *method = METHOD_NONE;

   /* Skip over leading white space or leading <cr><lf>. This
      is explicitly mentioned in RFC 2616 */
   while (( *ibuf == ' ' ) || ( *ibuf == '\r' ) || ( *ibuf == '\n' ))
      ibuf++;

   if ( (( ibuf[0] == 'G' ) || ( ibuf[0] == 'g' )) &&
        (( ibuf[1] == 'E' ) || ( ibuf[1] == 'e' )) &&
        (( ibuf[2] == 'T' ) || ( ibuf[2] == 't' )) &&
         ( ibuf[3] == ' ' ) )
   {
      *method = METHOD_GET;

      i = 4;
      j = 0;
      while (( ibuf[i] != ' ' ) && ( ibuf[i] != 0 ))
         uri[j++] = ibuf[i++];

      uri[j] = 0;
      
      return(0);
   }

   if ( (( ibuf[0] == 'H' ) || ( ibuf[0] == 'h' )) &&
        (( ibuf[1] == 'E' ) || ( ibuf[1] == 'e' )) &&
        (( ibuf[2] == 'A' ) || ( ibuf[2] == 'a' )) &&
        (( ibuf[3] == 'D' ) || ( ibuf[3] == 'd' )) &&
         ( ibuf[4] == ' ' ) )
   {
      *method = METHOD_HEAD;

      i = 5;
      j = 0;
      while (( ibuf[i] != ' ' ) && ( ibuf[i] != 0 ))
         uri[j++] = ibuf[i++];

      uri[j] = 0;
      
      return(0);
   }

   if ( (( ibuf[0] == 'O' ) || ( ibuf[0] == 'o' )) &&
        (( ibuf[1] == 'P' ) || ( ibuf[1] == 'p' )) &&
        (( ibuf[2] == 'T' ) || ( ibuf[2] == 't' )) &&
        (( ibuf[3] == 'I' ) || ( ibuf[3] == 'i' )) &&
        (( ibuf[4] == 'O' ) || ( ibuf[4] == 'o' )) &&
        (( ibuf[5] == 'N' ) || ( ibuf[5] == 'n' )) &&
        (( ibuf[6] == 'S' ) || ( ibuf[6] == 's' )) &&
         ( ibuf[7] == ' ' ) )
   {
      *method = METHOD_OPTIONS;

      i = 5;
      j = 0;
      while (( ibuf[i] != ' ' ) && ( ibuf[i] != 0 ))
         uri[j++] = ibuf[i++];

      uri[j] = 0;
      
      return(0);
   }

   if ( (( ibuf[0] == 'P' ) || ( ibuf[0] == 'p' )) &&
        (( ibuf[1] == 'O' ) || ( ibuf[1] == 'o' )) &&
        (( ibuf[2] == 'S' ) || ( ibuf[2] == 's' )) &&
        (( ibuf[3] == 'T' ) || ( ibuf[2] == 't' )) &&
         ( ibuf[4] == ' ' ) )
   {
      *method = METHOD_POST;

      i = 5;
      j = 0;
      while (( ibuf[i] != ' ' ) && ( ibuf[i] != 0 ))
         uri[j++] = ibuf[i++];

      uri[j] = 0;
      
      return(0);
   }

   /* I am not going to parse out other methods because they are not 
      supported. What is the point of parsing them out, only to then
      not handle them. If we don't find a pattern here then it might
      as well be some hack. Drop it. */
   return(1);
}


/* ========================================================================= */
int send_header(int s, int rc, int ctype, unsigned long len)
{
   char buf[1024];
   char rmsg[32];
   char ctypestr[32];

   buf[0] = 0;

   /* Initialize for safety - this will be overwritten */
   rmsg[0] = 0;
   ctypestr[0] = 0;

   switch ( rc )
   {
   case 200:
      strcpy(rmsg, "OK");
      break;
   case 404:
      strcpy(rmsg, "Not found");
      len = 0;
      break;
   case 405:
      strcpy(rmsg, "Method Not Allowed");
      len = 0;
      break;
   case 500:
      strcpy(rmsg, "Internal Error");
      len = 0;
      break;
   case 501:
      strcpy(rmsg, "Not Implemented");
      len = 0;
      break;
   default:
      strcpy(rmsg, "UnknownRC");
      len = 0;
      break;
   }

   /* NOTE: text types are limited in interpretation to simple text.
            I believe it is ISO-8859-1 (for text) but application
            suggests it can have an alternate char set (although it
            defaults to UTF-8 which we are compatible). I use
            application for json, but explicitly mention char set 
            because it is text. UTF-8 is compatible with C and there
            is no BOM or other header info required. The text based
            types always assume basic char types (that are followed). */
   switch ( ctype )
   {
   case CTYPE_JSON:
      strcpy(ctypestr, "application/json; charset=utf-8");
      break;
   case CTYPE_YAML:
      strcpy(ctypestr, "text/x-yaml");
      break;
   default:
      strcpy(ctypestr, "text/html");
      break;
   }

   sprintf(buf, "HTTP/1.1 %d %s\r\nCache-Control: no-cache\r\nContent-Type: %s\r\nContent-Length: %lu\r\n\r\n", rc, rmsg, ctypestr, len);

   /* STUB: Check return values */
   /* STUB: Make more robust */
   send(s, buf, strlen(buf), 0); 

   return(0);
}

/* ========================================================================= */
/* rc should ONLY be 200 or a 405                                            */
int send_options_header(int s, int rc)
{
   char buf[1024];
   char rmsg[32];

   buf[0] = 0;

   /* Initialize for safety - this will be overwritten */
   rmsg[0] = 0;

   switch ( rc )
   {
   case 200:
      strcpy(rmsg, "OK");
      break;
   case 405:
      strcpy(rmsg, "Method Not Allowed");
      break;
   case 501:
   default:
      rc = 501;
      strcpy(rmsg, "Not Implemented");
      break;
   }

   sprintf(buf, "HTTP/1.1 %d %s\r\nCache-Control: no-cache\r\nAllow: GET\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n", rc, rmsg);

   /* STUB: Check return values */
   /* STUB: Make more robust */
   send(s, buf, strlen(buf), 0); 

   return(0);
}





