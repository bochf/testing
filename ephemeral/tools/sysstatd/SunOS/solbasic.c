#include <stdio.h>
#include <stdlib.h>
#include <time.h>
/* STUB: #include <sys/systemcfg.h> */
#include <assert.h>
#include <kstat.h>
#include <string.h>

#include "solbasic.h"
#include "../support.h"

static int cpu_pcores = -1;
static int cpu_lcores = -1;
static int cpu_threads = -1;
static int cpu_sockets = -1;
static time_t boot_time = 0;

/* ========================================================================= */
/* This is used for the various CPU discovery/counting functions immediately 
   below this struct declaration.                                            */
struct cpu_arch
{
   long *chip_id;
   int  *chip_id_cnt; /* Effectively not used! */
   long *core_id;
   int  *core_id_cnt; /* Effectively not used! */
   unsigned int size;
};

struct cpu_arch *init_cpu_arch(unsigned int ccount);
int free_cpu_arch(struct cpu_arch *ca);
int insert_cpu_core_id(struct cpu_arch *ca, long core_id);
int insert_cpu_chip_id(struct cpu_arch *ca, long chip_id);
int get_ca_socket_count(struct cpu_arch *ca);
int get_ca_pcore_count(struct cpu_arch *ca);
int get_ca_thread_count(struct cpu_arch *ca);
int get_ca_lcore_count(struct cpu_arch *ca);

/* ========================================================================= */
/* Other prototypes                                                          */
long get_single_kstat(char *module, int instance, char *sname, char *iname, char *status);

/* ========================================================================= */
int InitBasic(void)
{
   kstat_ctl_t *kc;
   kstat_t *kr;
   kstat_named_t *kf;
   int i;

   /*** Get the system boot time ***/
   if ( NULL == (kc = kstat_open()) )
      return(-1);

   if (NULL == (kr = kstat_lookup(kc, "unix", 0, "system_misc")))
   {
      kstat_close(kc);
      return(-1);
   }

  if (-1 == kstat_read(kc, kr, NULL))
   {
      kstat_close(kc);
      return(-1);
   }

   kf = KSTAT_NAMED_PTR(kr);

   i = 0;
   while ( i < kr->ks_ndata )
   {
      if ( 0 == strcmp(kf[i].name, "boot_time") )
      {
         switch ( kf[i].data_type )
         {
         case KSTAT_DATA_UINT32:
            boot_time = (time_t)kf[i].value.ui32; /* This is what it is on tested system */
            break;
         case KSTAT_DATA_INT32:
            boot_time = (time_t)kf[i].value.i32;  /* This is what I expect (32bit) */
            break;
         case KSTAT_DATA_INT64:
            boot_time = (time_t)kf[i].value.i64; /* What I expect on 64 bit */
            break;
         case KSTAT_DATA_UINT64:
            boot_time = (time_t)kf[i].value.ui64;
            break;
         }

         /* If we found it, then artifically jump i to terminating condition. */
         i = kr->ks_ndata;
      }

      i++;
   }

   kstat_close(kc);

   return(0);
}

/* ========================================================================= */
int RefreshBasic(void)
{
   int rv = 0;
   kstat_ctl_t *kc;
   kstat_t *kr;
   kstat_named_t *kf;
   int i;
   struct cpu_arch *ca;
   unsigned int ncpus;

   /* The plan:
      - Allocate arrays based on core count
      - Walk cpu list counting core_id and chip_id
      - Assign values to core and socket count
      - Derive thread count
      - Free memory
      - Exit success
   */

   /*** Get the *logical* CPU count ***/
   if ( NULL == (kc = kstat_open()) )
      return(-1);

   if (NULL == (kr = kstat_lookup(kc, "unix", -1, "system_misc")))
   {
      kstat_close(kc);
      return(-1);
   }

  if (-1 == kstat_read(kc, kr, NULL))
   {
      kstat_close(kc);
      return(-1);
   }

   kf = KSTAT_NAMED_PTR(kr);

   i = 0;
   while ( i < kr->ks_ndata )
   {
      if ( 0 == strcmp(kf[i].name, "ncpus") )
      {
         ncpus = kf[i].value.ui32;

         /* If we found it, then artifically jump i to terminating condition. */
         i = kr->ks_ndata;
      }

      i++;
   }

   kstat_close(kc);

   /*** Initialize the counter structure ***/
   if ( NULL == (ca = init_cpu_arch(ncpus)) )
      return(-1);

   /*** Read in CPU data ***/
   if ( NULL == (kc = kstat_open()) )
      return(-1);

   kr = kc->kc_chain;
   while ( kr != NULL )
   {
      if ( kr->ks_type == KSTAT_TYPE_NAMED )
      {
         if (-1 == kstat_read(kc, kr, NULL))
         {
            kstat_close(kc);
            return(-1);
         }

         if ( 0 == strcmp(kr->ks_module, "cpu_info") )
         {
            kf = KSTAT_NAMED_PTR(kr);

            i = 0;
            while ( i < kr->ks_ndata )
            {
               if ( 0 == strcmp(kf[i].name, "core_id") )
               {
                  insert_cpu_core_id(ca, (long)kf[i].value.i64);
               }
               
               if ( 0 == strcmp(kf[i].name, "chip_id") )
               {
                  insert_cpu_chip_id(ca, (long)kf[i].value.i64);
               }
               
               i++;
            }
         }
      }

      kr = kr->ks_next;
   }

   kstat_close(kc);

   /* This was used for debuggery - saved in the event that this code is reused
   printf("Sockets  = %d\n", get_ca_socket_count(ca));
   printf("PhyCores = %d\n", get_ca_pcore_count(ca));
   printf("LogCores = %d\n", get_ca_lcore_count(ca));
   printf("Thr/Core = %d\n", get_ca_thread_count(ca));
   */

   cpu_pcores = get_ca_pcore_count(ca);
   cpu_lcores = get_ca_lcore_count(ca);
   cpu_threads = get_ca_thread_count(ca);
   cpu_sockets = get_ca_socket_count(ca);

   free_cpu_arch(ca);

   return(rv);
}

/* ========================================================================= */
long GetUptimeSecs(char *status, unsigned long size)
{
   time_t t;

   time(&t);

   if ( size > 20 ) /* A reasonable guess as to size */
   {
      return(sprintf(status, "%ld", t - boot_time));
   }

   return(0);
}

/* ========================================================================= */
long GetProcCount(char *status, unsigned long size)
{
   if ( size > 8 ) /* A reasonable guess as to size */
   {
      return(get_single_kstat("unix", 0, "system_misc", "nproc", status));
   }

   return(0);
}

/* ========================================================================= */
long GetCPUSocketCount(char *status, unsigned long size)
{
   /* A reasonable limitation on length */
   if ( size < 8 )
      return(0);

   return(sprintf(status, "%d", cpu_sockets));
}

/* ========================================================================= */
long GetCPUCoreCount(char *status, unsigned long size)
{
   /* A reasonable limitation on length */
   if ( size < 8 )
      return(0);

   return(sprintf(status, "%d", cpu_pcores));
}

/* ========================================================================= */
long GetCPULogicalCount(char *status, unsigned long size)
{
   /* A reasonable limitation on length */
   if ( size < 8 )
      return(0);

   return(sprintf(status, "%d", cpu_lcores));
}

/* ========================================================================= */
long GetCPUThreadCount(char *status, unsigned long size)
{
   /* A reasonable limitation on length */
   if ( size < 8 )
      return(0);

   return(sprintf(status, "%d", cpu_threads));
}

#ifdef STUB_REMOVE
/* ========================================================================= */
long GetCPUMHz(char *status, unsigned long size)
{
   kstat_ctl_t *kc;
   kstat_t *kr;
   kstat_named_t *kf;
   int i;
   long rv;

   /* A reasonable limitation on length */
   if ( size < 6 )
      return(0);

   if ( NULL == (kc = kstat_open()) )
      return(sprintf(status, "%d", -1));

   if (NULL == (kr = kstat_lookup(kc, "cpu_info", -1, NULL)))
   {
      kstat_close(kc);
      return(sprintf(status, "%d", -1));
   }

  if (-1 == kstat_read(kc, kr, NULL))
   {
      kstat_close(kc);
      return(sprintf(status, "%d", -1));
   }

   kf = KSTAT_NAMED_PTR(kr);

   i = 0;
   while ( i < kr->ks_ndata )
   {
      if ( 0 == strcmp(kf[i].name, "clock_MHz") )
      {
         if ( kf[i].data_type != KSTAT_DATA_INT64 )
         {
            rv = sprintf(status, "T%d",  kf[i].data_type);
            kstat_close(kc);
            return(rv);
         }

         /* It is known that kf[i].data_type is KSTAT_DATA_INT64 */
         rv = sprintf(status, "%ld", (long)kf[i].value.i64);
         kstat_close(kc);
         return(rv);
      }
      i++;

   }
   kstat_close(kc);
   return(sprintf(status, "%d", -1));
}
#endif

/* ========================================================================= */
long GetCPUMHz(char *status, unsigned long size)
{
   /* A reasonable limitation on length */
   if ( size < 8 )
      return(0);

   return(get_single_kstat("cpu_info", -1, NULL, "clock_MHz", status));
}

/* ========================================================================= */
long get_single_kstat(char *module, int instance, char *sname, char *iname, char *status)
{
   kstat_ctl_t *kc;
   kstat_t *kr;
   kstat_named_t *kf;
   int i;
   long rv;

   if ( NULL == (kc = kstat_open()) )
      return(sprintf(status, "FAIL"));

   if (NULL == (kr = kstat_lookup(kc, module, instance, sname)))
   {
      kstat_close(kc);
      return(sprintf(status, "FAIL"));
   }

  if (-1 == kstat_read(kc, kr, NULL))
   {
      kstat_close(kc);
      return(sprintf(status, "FAIL"));
   }

   kf = KSTAT_NAMED_PTR(kr);

   i = 0;
   while ( i < kr->ks_ndata )
   {
      if ( 0 == strcmp(kf[i].name, iname) )
      {
         switch(kf[i].data_type)
         {
         case KSTAT_DATA_INT32:
            rv = sprintf(status, "%d", kf[i].value.i32);
            break;
         case KSTAT_DATA_UINT32:
            rv = sprintf(status, "%u", kf[i].value.ui32);
            break;
         case KSTAT_DATA_INT64:
            rv = sprintf(status, "%ld", (long)kf[i].value.i64);
            break;
         case KSTAT_DATA_UINT64:
            rv = sprintf(status, "%lu", (unsigned long)kf[i].value.ui64);
            break;
         case KSTAT_DATA_CHAR:
            rv = sprintf(status, "%s", kf[i].value.c); /* Is this right!? */
            break;
         case KSTAT_DATA_STRING:
            /* See: KSTAT_NAMED_STR_PTR(kf) */
            rv = sprintf(status, "%s", kf[i].value.str.addr.ptr);
            break;
         default:
            rv = sprintf(status, "FAIL");
         }

         kstat_close(kc);
         return(rv);
      }

      i++;
   }

   kstat_close(kc);
   return(sprintf(status, "%s", "FAIL"));
}

/* ========================================================================= */
int free_cpu_arch(struct cpu_arch *ca)
{
   if ( NULL == ca )
      return(0);

   if ( NULL != ca->chip_id )
      free(ca->chip_id);

   if ( NULL != ca->chip_id_cnt )
      free(ca->chip_id_cnt);

   if ( NULL != ca->core_id )
      free(ca->core_id);

   if ( NULL != ca->core_id_cnt )
      free(ca->core_id_cnt);

   free(ca);
   
   return(0);
}

/* ========================================================================= */
struct cpu_arch *init_cpu_arch(unsigned int ccount)
{
   struct cpu_arch *ca;
   unsigned int i;

   if ( NULL == (ca = (struct cpu_arch *)malloc(sizeof(struct cpu_arch))) )
      return(NULL);

   /* Set these to NULL. That makes our failure cleanup easier */
   ca->chip_id = NULL;
   ca->chip_id_cnt = NULL;
   ca->core_id = NULL;
   ca->core_id_cnt = NULL;
   /* Initialize this */
   ca->size = ccount;

   if ( NULL == (ca->chip_id = (long *)malloc(sizeof(long) * ccount)) )
   {
      free_cpu_arch(ca);
      return(NULL);
   }

   if ( NULL == (ca->core_id = (long *)malloc(sizeof(long) * ccount)) )
   {
      free_cpu_arch(ca);
      return(NULL);
   }

   if ( NULL == (ca->chip_id_cnt = (int *)malloc(sizeof(int) * ccount)) )
   {
      free_cpu_arch(ca);
      return(NULL);
   }

   if ( NULL == (ca->core_id_cnt = (int *)malloc(sizeof(int) * ccount)) )
   {
      free_cpu_arch(ca);
      return(NULL);
   }

   /* Initialize all values */
   i = 0;
   while ( i < ccount )
   {
      ca->chip_id[i]     = -1;
      ca->chip_id_cnt[i] = 0;
      ca->core_id[i]     = -1;
      ca->core_id_cnt[i] = 0;
      i++;
   }

   return(ca);
}

/* ========================================================================= */
int insert_cpu_core_id(struct cpu_arch *ca, long core_id)
{
   int i;

   assert(NULL != ca);

   i = 0;
   while(i < ca->size)
   {
      if ( ca->core_id[i] == core_id )
      {
         ca->core_id_cnt[i]++;
         return(1);
      }

      if ( ca->core_id[i] == -1 )
      {
         ca->core_id[i] = core_id;
         ca->core_id_cnt[i] = 1;
         return(1);
      }

      i++;
   }

   return(0);
}

/* ========================================================================= */
int insert_cpu_chip_id(struct cpu_arch *ca, long chip_id)
{
   int i;

   assert(NULL != ca);

   i = 0;
   while(i < ca->size)
   {
      if ( ca->chip_id[i] == chip_id )
      {
         ca->chip_id_cnt[i]++;
         return(1);
      }

      if ( ca->chip_id[i] == -1 )
      {
         ca->chip_id[i] = chip_id;
         ca->chip_id_cnt[i] = 1;
         return(1);
      }

      i++;
   }

   return(0);
}

/* ========================================================================= */
int get_ca_socket_count(struct cpu_arch *ca)
{
   int i;

   i = 0;
   while ( i < ca->size )
   {
      if ( -1 == ca->chip_id[i] )
         return(i);

      i++;
   }

   return((int)ca->size);
}

/* ========================================================================= */
int get_ca_pcore_count(struct cpu_arch *ca)
{
   int i;

   i = 0;
   while ( i < ca->size )
   {
      if ( -1 == ca->core_id[i] )
         return(i);

      i++;
   }

   return((int)ca->size);
}

/* ========================================================================= */
int get_ca_thread_count(struct cpu_arch *ca)
{
   int i;
   int pcore;

   i = 0;
   while ( i < ca->size )
   {
      if ( -1 == ca->core_id[i] )
      {
         pcore = i;
         break;
      }

      i++;
   }

   return((int)ca->size / pcore);
}

/* ========================================================================= */
int get_ca_lcore_count(struct cpu_arch *ca)
{
   return((int)ca->size);
}
