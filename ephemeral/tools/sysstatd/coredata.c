#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include "support.h"
#include "coredata.h"
#include "version.h"

static char *version_string = NULL;
static char *uname_s = NULL;
static char *uname_p = NULL;
static char *uname_r = NULL;
static char *uname_n = NULL;

/* This is the counter (and lock) for the number of service requests */
extern unsigned long customers_served;
extern pthread_mutex_t cs_lock;

/* ========================================================================= */
int init_version(char *verstr)
{
   if ( NULL == ( version_string = mkstring(verstr) ) )
   {
      error_msg("ERROR: Failed to allocate simple version string.");
      return(1);
   }

   return(0);
}

long GetVersion(char *status, unsigned long size)
{
   char *end;

   if ( size > strlen(version_string) )
   {
      end = strcpy(status, version_string);
      return((long)(end - status));
   }

   return(0);
}

/* ========================================================================= */
int init_uname_s(void)
{
   FILE *cmd;
   char results[32];

   if (NULL == (cmd = popen("uname -s", "r")))
   {
      if ( NULL == (uname_s = mkstring("NO_UNAME")) )
      {
         error_msg("ERROR: Failed to malloc string memory.");
         return(1);
      }

      return(0); /* This is a problem, but not enough to end it all */
   }

   if( fgets(results, 32, cmd) )
   {
      chomp(results);
      if ( NULL == (uname_s = mkstring(results)) )
      {
         error_msg("ERROR: Failed to malloc string memory.");
         return(1);
      }
   }
   else
   {
      if ( NULL == (uname_s = mkstring("UNKNOWN")) )
      {
         error_msg("ERROR: Failed to malloc string memory.");
         return(1);
      }

      return(0); /* This is a problem, but not enough to end it all */
   }
      
   pclose(cmd);
   return(0);
}

long GetUname_s(char *status, unsigned long size)
{
   char *end;

   if ( size > strlen(uname_s) )
   {
      end = strcpy(status, uname_s);
      return((long)(end - status));
   }

   return(0);
}

/* ========================================================================= */
int init_uname_p(void)
{
   FILE *cmd;
   char results[32];

   if (NULL == (cmd = popen("uname -p", "r")))
   {
      if ( NULL == (uname_p = mkstring("NO_UNAME")) )
      {
         error_msg("ERROR: Failed to malloc string memory.");
         return(1);
      }

      return(0);
   }

   if( fgets(results, 32, cmd) )
   {
      chomp(results);
      if ( NULL == (uname_p = mkstring(results)) )
      {
         error_msg("ERROR: Failed to malloc string memory.");
         return(1);
      }
   }
   else
   {
      if ( NULL == (uname_p = mkstring("UNKNOWN")) )
      {
         error_msg("ERROR: Failed to malloc string memory.");
         return(1);
      }

      return(0);
   }
      
   pclose(cmd);
   return(0);
}

long GetUname_p(char *status, unsigned long size)
{
   char *end;

   if ( size > strlen(uname_p) )
   {
      end = strcpy(status, uname_p);
      return((long)(end - status));
   }

   return(0);
}

/* ========================================================================= */
int init_uname_r(void)
{
   FILE *cmd;
   char results[32];

   if (NULL == (cmd = popen("uname -r", "r")))
   {
      if ( NULL == (uname_r = mkstring("NO_UNAME")) )
      {
         error_msg("ERROR: Failed to malloc string memory.");
         return(1);
      }

      return(0);
   }

   if( fgets(results, 32, cmd) )
   {
      chomp(results);
      if ( NULL == (uname_r = mkstring(results)) )
      {
         error_msg("ERROR: Failed to malloc string memory.");
         return(1);
      }
   }
   else
   {
      if ( NULL == (uname_r = mkstring("UNKNOWN")) )
      {
         error_msg("ERROR: Failed to malloc string memory.");
         return(1);
      }

      return(0);
   }
      
   pclose(cmd);
   return(0);
}

long GetUname_r(char *status, unsigned long size)
{
   char *end;

   if ( size > strlen(uname_r) )
   {
      end = strcpy(status, uname_r);
      return((long)(end - status));
   }

   return(0);
}

/* ========================================================================= */
int init_uname_n(void)
{
   FILE *cmd;
   char results[32];

   if (NULL == (cmd = popen("uname -n", "r")))
   {
      if ( NULL == (uname_n = mkstring("NO_UNAME")) )
      {
         error_msg("ERROR: Failed to malloc string memory.");
         return(1);
      }

      return(0);
   }

   if( fgets(results, 32, cmd) )
   {
      chomp(results);
      if ( NULL == (uname_n = mkstring(results)) )
      {
         error_msg("ERROR: Failed to malloc string memory.");
         return(1);
      }
   }
   else
   {
      if ( NULL == (uname_n = mkstring("UNKNOWN")) )
      {
         error_msg("ERROR: Failed to malloc string memory.");
         return(1);
      }

      return(0);
   }
      
   pclose(cmd);
   return(0);
}

long GetUname_n(char *status, unsigned long size)
{
   char *end;

   if ( size > strlen(uname_n) )
   {
      end = strcpy(status, uname_n);
      return((long)(end - status));
   }

   return(0);
}

/* ========================================================================= */
long GetTimestamp(char *status, unsigned long size)
{
   char timestamp[64];
   struct tm st;
   char *end;
   time_t t;

   time(&t);

   /* Note: I use gmtime() because:
            1. It is more appropriate to use a universal time for such a
               tool as this.
            2. I don't want to have to figure the TZ and apply it. (I am
            lazy.) */
   gmtime_r(&t, &st);

   /* SEE: https://en.wikipedia.org/wiki/ISO_8601 */
   sprintf(timestamp, "%d-%02d-%02dT%02d:%02d:%02dZ",
           st.tm_year + 1900,
           st.tm_mon,
           st.tm_mday,
           st.tm_hour,
           st.tm_min,
           st.tm_sec);


   /* This is the old code that I was using
   ctime_r(&t, timestamp);
   chomp(timestamp);
   */

   if ( size > strlen(timestamp) )
   {
      end = strcpy(status, timestamp);
      return((long)(end - status));
   }

   return(0);
}

/* ========================================================================= */
long GetServed(char *status, unsigned long size)
{
   unsigned long served;

   /* Way more than what should be required */
   if ( size < 7 )
      return(0);

   /* The copy local is less costly than holding the lock through the printing
      of the variable. It does not matter much, but slightly cleaner. */
   pthread_mutex_lock(&cs_lock);
   served = customers_served;
   pthread_mutex_unlock(&cs_lock);

   return(sprintf(status, "%lu", served));
}

/* ========================================================================= */
int InitCoreData(void)
{
   if ( init_version(VERSION_STRING) )
      return(1);

   if ( init_uname_s() )
      return(1);

   if ( init_uname_p() )
      return(1);

   if ( init_uname_r() )
      return(1);

   if ( init_uname_n() )
      return(1);

   return(0);
}

