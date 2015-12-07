#include <stdio.h>
#include <stdlib.h>
#include <sys/sysinfo.h>

#include "linbasic.h"
#include "../support.h"

static struct sysinfo* si;
char *uptime_str;
char *procnt_str;

/* ========================================================================= */
int InitBasic(void)
{
   if ( NULL == (si = (struct sysinfo *)malloc(sizeof(struct sysinfo))) )
   {
      error_msg("ERROR: Failed to allocate memory initializing tree.");
      return(1);
   }

   if ( NULL == (uptime_str = (char *)malloc(32)) )
   {
      free(si); /* Free memory to increase chance of printing error */
      error_msg("ERROR: Failed to allocate memory initializing tree.");
      return(1);
   }

   if ( NULL == (procnt_str = (char *)malloc(32)) )
   {
      free(si);
      free(uptime_str);
      error_msg("ERROR: Failed to allocate memory initializing tree.");
      return(1);
   }

   return(0);
}

/* ========================================================================= */
int RefreshBasic(void)
{
   int rv;
   
   if ( 0 != (rv = sysinfo(si)) )
   {
      error_msg("ERROR: failed call to sysinfo().");
   }

   return(rv);
}

/* ========================================================================= */
long GetUptimeSecs(char *status, unsigned long size)
{
   if ( size > 20 ) /* A reasonable guess as to size */
   {
      return(sprintf(status, "%ld", si->uptime));
   }

   return(0);
}

/* ========================================================================= */
long GetProcCount(char *status, unsigned long size)
{
   if ( size > 8 ) /* A reasonable guess as to size */
   {
      return(sprintf(status, "%u", si->procs));
   }

   return(0);
}

/* See this:
http://stackoverflow.com/questions/1540627/what-api-do-i-call-to-get-the-system-uptime
*/



