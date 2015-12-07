#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>

#include "support.h"
#include "acclog.h"
#include "defaults.h"

char *access_log = NULL;
pthread_mutex_t *access_lock;

/* ========================================================================= */
int InitAccessLog(char *filename)
{
   access_log = filename;

   /* Don't bother doing other work if we will never log */
   if ( NULL == access_log )
      return(0);

   if ( NULL == ( access_lock = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t)) ) )
   {
      error_msg("ERROR: Failed to allocate memory for the access log lock.");
      return(1);
   }

   /* Static initialization fails to compile on Linux */
   /* access_lock = PTHREAD_MUTEX_INITIALIZER; */

   pthread_mutex_init(access_lock, NULL);
   
   return(0);
}

/* ========================================================================= */
int LogAccess(int sock, int tree, int format, int hrv)
{
   struct sockaddr_in addr;
   socklen_t len;
   char addrstr[INET6_ADDRSTRLEN];
   pthread_t tid;
#ifndef PORT_Linux
   struct timespec timeo;
#endif
   FILE *f;
   time_t when;

   if ( NULL == access_log )
      return(0);

   len = sizeof(addr);

   if ( getpeername(sock, (struct sockaddr *) &addr, &len) )
   {
      perror("getpeername():");
      strcpy(addrstr, "UNKNOWN");
   }
   else
      inet_ntop(AF_INET, &addr.sin_addr, addrstr, INET6_ADDRSTRLEN);

   tid = pthread_self();
   time(&when);

#ifndef PORT_Linux
   timeo.tv_sec = DEFAULT_ACCESS_LOG_TIMEOUT;
#endif

   /* ===================================== */
   if ( 0 == strcmp(access_log, "stdout") )
   {
#ifdef PORT_Linux
      pthread_mutex_lock(access_lock);
#else
      if ( pthread_mutex_timedlock(access_lock, &timeo) )
      {
         /* Do not try to error log - it is likely hung too. */
         return(1);
      }
#endif

      printf("%lld TID: 0x%lx; SD %d; From: %s; Tree: %d; Fmt: %d; RV: %d;\n",
             (long long)when,
             (unsigned long)tid,
             sock,
             addrstr,
             tree,
             format,
             hrv);

      pthread_mutex_unlock(access_lock);

      fflush(stdout);
      return(0);
   }

   /* ===================================== */
   if ( 0 == strcmp(access_log, "stderr") )
   {
#ifdef PORT_Linux
      pthread_mutex_lock(access_lock);
#else
      if ( pthread_mutex_timedlock(access_lock, &timeo) )
      {
         /* Do not try to error log - it is likely hung too. */
         return(1);
      }
#endif

      fprintf(stderr, "%lld TID: 0x%lx; SD %d; From: %s; Tree: %d; Fmt: %d; RV: %d;\n",
              (long long)when,
              (unsigned long)tid,
              sock,
              addrstr,
              tree,
              format,
              hrv);

      pthread_mutex_unlock(access_lock);
      return(0);
   }

   /* ... fall through ... */

   /* ===================================== */
   /* if ( a regular file )                 */
#ifdef PORT_Linux
   pthread_mutex_lock(access_lock);
#else
   if ( pthread_mutex_timedlock(access_lock, &timeo) )
   {
      /* Do not try to error log - it is likely hung too. */
      return(1);
   }
#endif

   if ( NULL == ( f = fopen(access_log, "a+") ) )
   {
      error_msg("ERROR: Unable to open ACCESS_FILE for access logging.");
      pthread_mutex_unlock(access_lock);
      return(0);
   }

   fprintf(f, "%lld TID: 0x%lx; SD %d; From: %s; Tree: %d; Fmt: %d; RV: %d;\n",
           (long long)when,
           (unsigned long)tid,
           sock,
           addrstr,
           tree,
           format,
           hrv);

   fclose(f);

   pthread_mutex_unlock(access_lock);
   
   return(0);
}




