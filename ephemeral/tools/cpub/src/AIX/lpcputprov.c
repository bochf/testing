#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <libperfstat.h>

#include "../mainhelp.h"
#include "lpcputprov.h"

static perfstat_cpu_total_t *this_cpu;
static perfstat_cpu_total_t *last_cpu;

/* The items we support */
static const char *diname[] = {"pswitch",   "syscall",   "sysread", \
                               "syswrite",  "sysfork",   "sysexec", \
                               "devintrs",  "softintrs", "sema",    \
                               "runque",    "swpque",    "bread",   \
                               "bwrite",    "lread",     "lwrite",  \
                               "phread",    "phwrite",   "iget",    \
                               "namei",     "dirblk",    "msg",     \
                               "lbolt",     "iowait",    "physio",  \
                               "twait",                             \
                               NULL};

static const int ditype[]  =  {PDT_UINT64,   PDT_UINT64,   PDT_UINT64,   \
                               PDT_UINT64,   PDT_UINT64,   PDT_UINT64,   \
                               PDT_UINT64,   PDT_UINT64,   PDT_UINT64,   \
                               PDT_UINT64,   PDT_UINT64,   PDT_UINT64,   \
                               PDT_UINT64,   PDT_UINT64,   PDT_UINT64,   \
                               PDT_UINT64,   PDT_UINT64,   PDT_UINT64,   \
                               PDT_UINT64,   PDT_UINT64,   PDT_UINT64,   \
                               PDT_INT64,    PDT_INT16,    PDT_INT16,    \
                               PDT_INT64
                               };

static struct provider *this_provider;

/* ========================================================================= */
int init_pcputdata(void)
{
   if ( NULL == (this_cpu = (perfstat_cpu_total_t *)malloc(sizeof(perfstat_cpu_total_t))) )
      return(1);

   if ( NULL == (last_cpu = (perfstat_cpu_total_t *)malloc(sizeof(perfstat_cpu_total_t))) )
      return(1);

   if ( 1 != perfstat_cpu_total (NULL, this_cpu, sizeof(perfstat_cpu_total_t), 1))
   {
      error_msg("ERROR: Unable to retrieve perfstat data.");
      return(1);
   }

   return(0);
}

/* ========================================================================= */
int lpcput_update(int interval)
{
   perfstat_cpu_total_t *temp_cpu;
   struct pitem *update_pi;
   int rv;

   /* rotate this and last */
   temp_cpu = this_cpu;
   this_cpu = last_cpu;
   last_cpu = temp_cpu;

   if ( 1 != perfstat_cpu_total (NULL, this_cpu, sizeof(perfstat_cpu_total_t), 1))
   {
      error_msg("ERROR: Unable to retrieve perfstat data.");
      return(1);
   }

   rv = 0;
   update_pi = this_provider->ui_list;
   while( update_pi )
   {

      rv += CalcData(update_pi,
                     (void *)((unsigned long)last_cpu + (unsigned long)update_pi->sioffset),
                     (void *)((unsigned long)this_cpu + (unsigned long)update_pi->sioffset),
                     interval);

      update_pi = update_pi->next_ui;
   }

   return(rv);
}

/* ========================================================================= */
int lpcput_listquads(int dflag)
{
   int icount = 0;

   while ( diname[icount] )
   {
      DumpQuadData(dflag, ditype[icount]);
      printf("perfstat.cputotal::%s\n", diname[icount]);

      icount++;
   }

   return(icount);
}

/* ========================================================================= */
struct pitem *lpcput_itemenable(int data_size, int data_type, unsigned long sioffset, struct qparts *qp)
{
   struct pitem *this_pi;
   void *data_ptr;

   /* Validate input */
   if (( NULL == data_ptr) || ( NULL == qp ))
      return(NULL);

   if ( data_type == PDT_UNKNOWN )
      return(NULL);

   /* Allocate space specifically for the destination data */
   if ( NULL == (data_ptr = malloc(data_size)) )
      return(NULL);

   /* Allocate pitem */
   if ( NULL == (this_pi = NewPItem(qp, data_type, data_ptr)) )
      return(NULL);

   /* Set offset to data */
   this_pi->sioffset = sioffset;

   /* Parse iargs */
   if ( ShouldDiff(qp) )
      this_pi->munge_flag |= MUNGE_DIFF;

   if ( ShouldPSAvg(qp) )
      this_pi->munge_flag |= MUNGE_PSAVG;

   /* Insert into ui list */
   this_pi->next_ui = this_provider->ui_list;
   this_provider->ui_list = this_pi;

   return(this_pi);
}

/* ========================================================================= */
#define IF_MATCH_ENABLE_UINT64(VARNAME); if ( 0 == strcmp(qp->iname, #VARNAME ) )     \
                                         { return(lpcput_itemenable(sizeof(uint64_t), \
                                                                    PDT_UINT64,       \
                                                                    (unsigned long)&this_cpu->VARNAME - (unsigned long)this_cpu, \
                                                                    qp)); } 

#define IF_MATCH_ENABLE_INT64(VARNAME); if ( 0 == strcmp(qp->iname, #VARNAME ) )     \
                                        { return(lpcput_itemenable(sizeof(int64_t),  \
                                                                    PDT_INT64,       \
                                                                    (unsigned long)&this_cpu->VARNAME - (unsigned long)this_cpu, \
                                                                    qp)); } 

#define IF_MATCH_ENABLE_INT32(VARNAME); if ( 0 == strcmp(qp->iname, #VARNAME ) )     \
                                        { return(lpcput_itemenable(sizeof(int32_t),  \
                                                                    PDT_INT32,       \
                                                                    (unsigned long)&this_cpu->VARNAME - (unsigned long)this_cpu, \
                                                                    qp)); } 

#define IF_MATCH_ENABLE_INT16(VARNAME); if ( 0 == strcmp(qp->iname, #VARNAME ) )     \
                                        { return(lpcput_itemenable(sizeof(int16_t),  \
                                                                    PDT_INT16,       \
                                                                    (unsigned long)&this_cpu->VARNAME - (unsigned long)this_cpu, \
                                                                    qp)); } 

struct pitem *lpcput_enablepitem(struct qparts *qp)
{
    struct pitem *this_pitem;

   /* Make sure we were called with the right quad (provider) name */
   if ( 0 != strcmp(qp->pname, "perfstat.cputotal") )
      return(NULL);

   /* See LPCPURegister() notes - Allocate ONLY when we have to. */
   if ( NULL == this_cpu )
   {
      if(init_pcputdata())
      {
         error_msg("ERROR: Failed to allocate memory for perfstat.cputotal provider.");
         return(NULL);
      }
   }

   /* This provider is now "hot" */
   this_provider->update_required = 1;

   IF_MATCH_ENABLE_UINT64(pswitch);
   IF_MATCH_ENABLE_UINT64(syscall);
	IF_MATCH_ENABLE_UINT64(sysread);
	IF_MATCH_ENABLE_UINT64(syswrite);
	IF_MATCH_ENABLE_UINT64(sysfork);
	IF_MATCH_ENABLE_UINT64(sysexec);
	IF_MATCH_ENABLE_UINT64(devintrs);
	IF_MATCH_ENABLE_UINT64(softintrs);
	IF_MATCH_ENABLE_UINT64(sema);
   IF_MATCH_ENABLE_UINT64(runque);
	IF_MATCH_ENABLE_UINT64(swpque);
	IF_MATCH_ENABLE_UINT64(bread);
	IF_MATCH_ENABLE_UINT64(bwrite);
	IF_MATCH_ENABLE_UINT64(lread);
	IF_MATCH_ENABLE_UINT64(lwrite);
	IF_MATCH_ENABLE_UINT64(phread);
	IF_MATCH_ENABLE_UINT64(phwrite);
	IF_MATCH_ENABLE_UINT64(iget);
	IF_MATCH_ENABLE_UINT64(namei);
	IF_MATCH_ENABLE_UINT64(dirblk);
	IF_MATCH_ENABLE_UINT64(msg);
   IF_MATCH_ENABLE_INT64(lbolt);
   IF_MATCH_ENABLE_INT16(iowait);
   IF_MATCH_ENABLE_INT16(physio);
	IF_MATCH_ENABLE_INT64(twait);

   /* The item was not found */
   return(NULL);
}

/* ========================================================================= */
int LPCPUTRegister(struct proot *p)
{

   /* Set this to NULL. The data for this will not be allocated until
      lpcput_emablepitem() is called. It will use this variable specifically
      to see if the operation has been done or not. The point here is to NOT
      do the allocations and the first request unless it is requred. */
   this_cpu = NULL;

   /* Some "compile time" tests to insure our data types are correct */
   assert(sizeof(u_longlong_t) == sizeof(uint64_t));
   assert(sizeof(short) == sizeof(int16_t));
   /* This seems a bit non-intuitive... but it is what it is */
   assert(sizeof(time_t) == sizeof(int64_t));


   /* this_provider is global/static to this module. */
   if ( NULL == (this_provider = RegisterProvider(p, "perfstat.cputotal",
                                                  lpcput_update,
                                                  lpcput_listquads,
                                                  lpcput_enablepitem)) )
   {
      return(1);
   }

   return(0);

}
