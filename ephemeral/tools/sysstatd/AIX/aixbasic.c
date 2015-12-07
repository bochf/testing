#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/systemcfg.h>
#include <libperfstat.h>
#include <sys/rset.h>
#include <string.h>

#include "aixbasic.h"
#include "../support.h"

extern struct _system_configuration;

perfstat_cpu_total_t *cput;

/* ========================================================================= */
int InitBasic(void)
{
   if ( NULL == (cput = (perfstat_cpu_total_t *)malloc(sizeof(perfstat_cpu_total_t))) )
   {
      error_msg("ERROR: Failed to allocate memory initializing tree.");
      return(1);
   }

   return(0);
}

/* ========================================================================= */
int RefreshBasic(void)
{
   int rv = 0;
   
   if ( 1 != perfstat_cpu_total(NULL, cput, sizeof(perfstat_cpu_total_t), 1) )
   {
      error_msg("ERROR: failed call to perfstat_cpu_total().");
      rv = 1;
   }

   return(rv);
}

/* ========================================================================= */
long GetUptimeSecs(char *status, unsigned long size)
{
   if ( size > 20 ) /* A reasonable guess as to size */
   {
      return(sprintf(status, "%ld", cput->lbolt / 100));
   }

   return(0);
}

/* ========================================================================= */
long GetProcCount(char *status, unsigned long size)
{
   int procnt;

   procnt = perfstat_process(NULL, NULL, sizeof(perfstat_process_t), 0);

   /* procnt might be -1 if there was an error. What can we do but pass
      it on. So... I don't even bother to check. */

   if ( size > 8 ) /* A reasonable guess as to size */
   {
      return(sprintf(status, "%d", procnt));
   }

   return(0);
}

/* ========================================================================= */
long GetCPUSocketCount(char *status, unsigned long size)
{
   int sradcnt;

   /* A reasonable limitation on length */
   if ( size < 8 )
      return(0);

   /* Once again, not much we can do with a -1 failure but to pass it on. */
   sradcnt = rs_getinfo(NULL, R_SRADSDL, 0);

   return(sprintf(status, "%d", sradcnt));
}

/* ========================================================================= */
long GetCPUCoreCount(char *status, unsigned long size)
{
   int smt;

   /* A reasonable limitation on length */
   if ( size < 8 )
      return(0);

   smt = 1;

   if ( _system_configuration.smt_status & 0x2 )
   {
      smt = _system_configuration.smt_threads;
   }

   return(sprintf(status, "%d", _system_configuration.ncpus / smt));
}

/* ========================================================================= */
long GetCPULogicalCount(char *status, unsigned long size)
{
   /* A reasonable limitation on length */
   if ( size < 8 )
      return(0);

   return(sprintf(status, "%d", cput->ncpus));
}

/* ========================================================================= */
long GetCPUThreadCount(char *status, unsigned long size)
{
   /* A reasonable limitation on length */
   if ( size < 8 )
      return(0);

   if ( 0 == _system_configuration.smt_status & 0x2 )
   {
      return(sprintf(status, "1"));
   }

   return(sprintf(status, "%d", _system_configuration.smt_threads));
}

/* ========================================================================= */
long GetCPUMHz(char *status, unsigned long size)
{
   /* A reasonable limitation on length */
   if ( size < 20 )
      return(0);

   return(sprintf(status, "%llu", cput->processorHZ / 1000000));
}








