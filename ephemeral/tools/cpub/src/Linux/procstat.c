#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>

#include "../mainhelp.h"
#include "procstat.h"
#include "linproviders.h"

/* ToDos for this provider
   [ ] The additional items need to be parsed. (Interrupts?)
*/


/* ========================================================================= */
/* How the data structures work:
   psdataf - holds all references to all stats. The read of /proc/stat fills
        this struct.
   cpudata - holds a single read of stats for a single CPU. Two of these must
        exist for each CPU (one for this, and one for the previous).
   cpu_data_bin - This is what is put in dstruct. It is a
        "bin" of all CPU stats for a givven CPU. It only references items in
        psdataf. So, it is a "view" on the data.

   A choice was made to put the data in simple arrays/structures and then
   represent it in more specific "views" that are related to the items
   requested by the user. As a result, the data is rotated when it is
   inserted but then *again* in the views.
*/

/* cpudata - holds one line of cpu data from the stat file. */
struct cpudata
{
   int cpun;        /* CPU number                    */

   uint64_t user;   /* User                          */
   uint64_t nice;   /* User, but niced               */
   uint64_t sys;    /* System                        */
   uint64_t idle;   /* Idle                          */
   uint64_t iowait; /* I/O Wait                      */
   uint64_t irq;    /* IRQ (servicing)               */
   uint64_t sirq;   /* Soft IRQ                      */
   uint64_t steal;  /* Time given to (other) virtual */    
   uint64_t guest;  /* Guest (virtualized)           */
   uint64_t gnice;  /* Guest nice (virtualized)      */
   uint64_t total;  /* THIS IS NOT "REAL" DATA. This is a derived
                       value of this AND previous. It is used for pctg
                       calculations when MORE THAN ONE pctg calculation
                       is done per CPU. */
};

/* psdataf is a representation of the entire /proc/stat file */
struct psdataf /* proc stat data file/fields */
{
   int cpucnt;

   /* Line for total cpu: [cpu <user> <nice>...] */
   struct cpudata *thistotal;
   struct cpudata *lasttotal;

   /* Line(s) for each cpu: [cpu0 <user> <nice>...] */
   struct cpudata *thiscpuX;
   struct cpudata *lastcpuX;

   /* NOTE - Additional items can go here (later). Also note that
             when items are added they MUST be bzero()'d like the
             above values. Search for the bzero() call to see where. 
   */
};

/* cpu_data_bin is a "bin of cpu data". It holds all the info related
   to a CPU. This is a way to grab the stats required for this CPU,
   but not to create a multi-level structure to hold it all. Instead
   this struct just reaches in and references what it needs.
*/
#define DTYPE_RAW 0
#define DTYPE_PCT 1
struct cpu_data_bin
{
   int dtype;             /* <--- Operation [raw|pct] for this request */
   struct cpudata *thisd; /* <--- pointer to data in psdataf */
   struct cpudata *lastd; /* <--- pointer to data in psdataf */
};

/* ========================================================================= */
/* Prototypes */
int cpus_in_buff(struct buffer *b);
struct psdataf *new_stat_struct(int cpu_cnt);
int update_stat_data(struct buffer *buf, struct psdataf *psd);
/* The three published interfaces */
int procstat_update(int interval);
int procstat_list(int dflag);
struct pitem *procstat_enablepitem(struct qparts *qp);

/* ========================================================================= */
/* These items are "local" to this "class" */
static struct provider *this_provider;
static struct psdataf *this_data;
static struct buffer *this_buf;

/* ========================================================================= */
int ProcStatRegister(struct proot *p)
{
   this_data = NULL;
   this_buf = NULL;

   this_provider = RegisterProvider(p,               /* The provider "root" structure */
                                    "proc.stat",  /* The name of the provider Q1   */
                                    procstat_update,
                                    procstat_list,
                                    procstat_enablepitem);

   /* Check status */
   if ( NULL == this_provider )
      return(1);

   return(0);

}

/* ========================================================================= */
int procstat_list(int dflag)
{
   char *pscitem[] = { "user",   "nice", "system",  "idle",
                       "iowait", "irq",  "softirq", "steal",
                       "guest",  "guest_nice" };
   int pscicnt = 10;
   int icount = 0;
   int i;

   i = 0;
   while ( i < pscicnt )
   {
      DumpQuadData(dflag, PDT_INT64);
      printf("proc.stat::cputotal.raw.%s\n", pscitem[i]);

      icount++;
      i++;
   }

   i = 0;
   while ( i < pscicnt )
   {
      DumpQuadData(dflag, PDT_INT64);
      printf("proc.stat:*:cpu.raw.%s\n", pscitem[i]);

      icount++;
      i++;
   }

   i = 0;
   while ( i < pscicnt )
   {
      DumpQuadData(dflag, PDT_INT16);
      printf("proc.stat::cputotal.pct.%s\n", pscitem[i]);

      icount++;
      i++;
   }

   i = 0;
   while ( i < pscicnt )
   {
      DumpQuadData(dflag, PDT_INT16);
      printf("proc.stat:*:cpu.pct.%s\n", pscitem[i]);

      icount++;
      i++;
   }

   /* NOTE: (Future) Need to handle cxct, btime, processes..... */

   return(icount);
}


/* ========================================================================= */
int procstat_update(int interval)
{
   struct cpudata *cpudtemp;
   struct pitem *this_pitem;
   struct cpu_data_bin *this_bin;
   uint64_t total;  /* Total (used for pct */
   uint64_t tthis;  /* Temp this */
   uint64_t tlast;  /* Temp last */
   uint64_t *tptr;  /* Temp pointer */

   /* NOTE: Because we use a "data bin" we don't care if this is a per-CPU
            stat or the total stat. In either case the updates happen 
            against the "full" view, and the per-item is provided by the
            "data bin" where individual calculations are done. */

   /* First, update stats from /proc/stat */
   if(update_stat_data(this_buf, this_data)) /* <---- This rotates the main set of data */
      return(1);

   /* Next, walk the update list and update items */
   this_pitem = this_provider->ui_list;
   while ( this_pitem )
   {
      /* Get a "local" reference to the data. */
      this_bin = (struct cpu_data_bin *)this_pitem->dstruct;

      /* Ok.... Because we use the "data bin" as a view, and not an
         actual container in the main struct. (More on *why* later.)
         The data gets rotated in the main struct, but the view never
         got updated (rotated). So the data needs to be rotated in 
         the view too. The view is required for EACH pitem because
         there can be multiple pitems for a single CPU - or even for
         a single stat on a CPU (RAW and PCT). The view holds this
         info, so there must be an individual for each pitem. Also
         note that the "main view" is rotated above in the
         update_stat_data() function. This rotation keeps them in
         sync. */
      cpudtemp = this_bin->lastd;
      this_bin->lastd = this_bin->thisd;
      this_bin->thisd = cpudtemp;


      if ( DTYPE_PCT == this_bin->dtype )
      {
         /* Save the total value. Likely it will be calculated over and over.
            This saves some of that ugly calculation at the cost of a simple
            compare. */

         if ( 0 == this_bin->lastd->total ) /* First run problem (should only happen once) */
         {
            this_bin->lastd->total = this_bin->lastd->user   +
                                     this_bin->lastd->nice   +
                                     this_bin->lastd->sys    +
                                     this_bin->lastd->idle   +
                                     this_bin->lastd->iowait +
                                     this_bin->lastd->irq    +
                                     this_bin->lastd->sirq   +
                                     this_bin->lastd->steal  +
                                     this_bin->lastd->guest  +
                                     this_bin->lastd->gnice;


         }

         if ( 0 == this_bin->thisd->total )
         {
            this_bin->thisd->total = this_bin->thisd->user   +
                                     this_bin->thisd->nice   +
                                     this_bin->thisd->sys    +
                                     this_bin->thisd->idle   +
                                     this_bin->thisd->iowait +
                                     this_bin->thisd->irq    +
                                     this_bin->thisd->sirq   +
                                     this_bin->thisd->steal  +
                                     this_bin->thisd->guest  +
                                     this_bin->thisd->gnice;
         }


         total = this_bin->thisd->total - this_bin->lastd->total;

         /* This was the old way - calculating total each time - in its entirety
         total = (this_bin->thisd->user   - this_bin->lastd->user) +
                 (this_bin->thisd->nice   - this_bin->lastd->nice) +
                 (this_bin->thisd->sys    - this_bin->lastd->sys) +
                 (this_bin->thisd->idle   - this_bin->lastd->idle) +
                 (this_bin->thisd->iowait - this_bin->lastd->iowait) +
                 (this_bin->thisd->irq    - this_bin->lastd->irq) +
                 (this_bin->thisd->sirq   - this_bin->lastd->sirq) +
                 (this_bin->thisd->steal  - this_bin->lastd->steal) +
                 (this_bin->thisd->guest  - this_bin->lastd->guest) +
                 (this_bin->thisd->gnice  - this_bin->lastd->gnice);
         */

         /* ISSUE: This works, but could likely be simplified */
         tptr = (uint64_t *)((unsigned long)this_bin->thisd + (unsigned long)this_pitem->sioffset);
         tthis = *tptr;
         tptr = (uint64_t *)((unsigned long)this_bin->lastd + (unsigned long)this_pitem->sioffset);
         tlast = *tptr;

         if ( total > 0 )
            total = ((tthis - tlast) * 100) / total; /* re-using total here */
         else
            total = 0; /* incase it somehow went negative */

         *((int16_t *)this_pitem->data_ptr) = (int16_t)total;
      }


      if ( DTYPE_RAW == this_bin->dtype )
      {

         CalcData(this_pitem,
                  (uint64_t *)((unsigned long)this_bin->lastd + (unsigned long)this_pitem->sioffset),
                  (uint64_t *)((unsigned long)this_bin->thisd + (unsigned long)this_pitem->sioffset),
                  interval);
      }

      this_pitem = this_pitem->next_ui;
   }

   return(0);
}

/* ========================================================================= */
/* Two tiny helper functions just for procstat_enablepitem() */
int is_raw_stat(char *iname)
{
   /* Check the converse first. This solves the problem of a user 
      entering both .pct. and .raw. */
   if ( StrStr(iname, ".pct.") )
      return(0);

   if ( StrStr(iname, ".raw.") )
      return(1);

   return(0);
}

int is_pct_stat(char *iname)
{
   /* Check the converse first. This solves the problem of a user 
      entering both .pct. and .raw. */
   if ( StrStr(iname, ".raw.") )
      return(0);

   if ( StrStr(iname, ".pct.") )
      return(1);

   return(0);
}

/* ========================================================================= */
/* These are defines for procstat_enablepitem() */

#define CMPEN_CPU_RAW(INAME); if (StrStr(qp->iname, #INAME))                                                                               \
                              {                                                                                                            \
                                 data_ptr = malloc(sizeof(uint64_t));                                                                      \
                                 if ( NULL == (this_pitem = NewPItem(qp, PDT_UINT64, data_ptr)) ) { return(NULL); }                        \
                                 this_pitem->sioffset = (unsigned long)&this_data->thistotal->INAME - (unsigned long)this_data->thistotal; \
                              }

#define CMPEN_CPU_PCT(INAME); if (StrStr(qp->iname, #INAME))                                                                               \
                              {                                                                                                            \
                                 data_ptr = malloc(sizeof(uint16_t));                                                                      \
                                 if ( NULL == (this_pitem = NewPItem(qp, PDT_UINT16, data_ptr)) ) { return(NULL); }                        \
                                 this_pitem->sioffset = (unsigned long)&this_data->thistotal->INAME - (unsigned long)this_data->thistotal; \
                              }

/* ========================================================================= */
struct pitem *procstat_enablepitem(struct qparts *qp)
{
   char hdr_temp[MAX_QPART_LEN]; /* Temp holder of the header */
   char *hdr_ptr;                /* Holds the location of the header info we need */
   int cpu_cnt;
   int requested_cpu;

   void *data_ptr;
   struct cpu_data_bin *dstruct;
   struct pitem *this_pitem;
   struct pitem *base_pitem; /* Used to return a list */
   struct pitem *last_pitem; /* Used to build the returned list */
   int dtype;

   /* Make sure we were called with the right quad (provider) name */
   if ( 0 != strcmp(qp->pname, "proc.stat") )
      return(NULL);

   /* Discussion:
      I am going to go ahead and set up the provider to collect data.
      Therotically this should be done after we establish that the input 
      from the user is good. (Such as - they did not ask for data on a CPU
      that does not exist.) But, since I know that the code will exit on
      any failure, then I will just do the work and bail if it comes to 
      that. Furthermore... I need to know how many CPUs there are in order
      to properly judge the input anyways.
   */

   /* Flag update for the provider */
   this_provider->update_required = 1;

   /* Setup the read buffer - if not already done. */
   if ( NULL == this_buf )
   {
      if ( NULL == (this_buf = InitReadBuff("/proc/stat", 25)) )
      {
         error_msg("ERROR: Unable to setup /proc/stat buffer.");
         return(NULL);
      }
   }

   if ( NULL == this_data )
   {
      if ( 1 > (cpu_cnt = cpus_in_buff(this_buf)) )
      {
         error_msg("ERROR: Unable to enumerate CPUs.");
         return(NULL);
      }

      if ( NULL == (this_data = new_stat_struct(cpu_cnt)) )
      {
         error_msg("ERROR: Unable to allocate memory for CPU stats.");
         return(NULL);
      }


      if(update_stat_data(this_buf, this_data))
      {
         error_msg("ERROR: Unable to retrieve data from /proc/stat.");
         return(NULL);
      }

      /* What is going on here?!
         bzero() the data. Do this so that the first iteration will not
         be all 0s. NULL the data so that we are looking at all 0s
         for the first set of data - and the resulting data will be the
         CPU times since boot. bzero() also insures that the values are
         all 0, and not random data that may make for some interesting
         values.
      */
      bzero(this_data->thistotal, sizeof(struct cpudata));
      bzero(this_data->lasttotal, sizeof(struct cpudata));

      bzero(this_data->thiscpuX, cpu_cnt * sizeof(struct cpudata));
      bzero(this_data->lastcpuX, cpu_cnt * sizeof(struct cpudata));
      
      /* NOTE - Additional items can go here (later) */
   }

   /* Now start parsing the quad. */

   /* ============================== */
   /* Total CPU stat */
   if ( qp->iname == StrStr(qp->iname, "cputotal.") )
   {
      /* This is a total CPU stat */

      if ( qp->pargs[0] != 0 )
      {
         /* A provider arg was used - with an item that does not support it! */
         error_msg("ERROR: The proc.stat cputotal option does not support a CPU argument.");
         return(NULL);
      }

      /* Break down into raw and pct - then parse individual items */

      if ( is_raw_stat(qp->iname) )
      {
         this_pitem = NULL;

         CMPEN_CPU_RAW(user);
         CMPEN_CPU_RAW(nice);
         CMPEN_CPU_RAW(sys);
         CMPEN_CPU_RAW(idle);
         CMPEN_CPU_RAW(iowait);
         CMPEN_CPU_RAW(irq);
         CMPEN_CPU_RAW(sirq);
         CMPEN_CPU_RAW(steal);
         CMPEN_CPU_RAW(guest);
         CMPEN_CPU_RAW(gnice);
         
         if ( this_pitem )
         {
            if ( NULL == (dstruct = malloc(sizeof(struct cpu_data_bin))) )
               return(NULL);
            dstruct->dtype = DTYPE_RAW;
            dstruct->thisd = this_data->thistotal;
            dstruct->lastd = this_data->lasttotal;
            this_pitem->dstruct = (void *)dstruct;

            this_pitem->next_ui = this_provider->ui_list;
            this_provider->ui_list = this_pitem;

            return(this_pitem);
         }

         /* This is a fall-through to return(NULL) */
      }

      if ( is_pct_stat(qp->iname) )
      {
         if ( ( ShouldDiff(qp) ) || ( ShouldPSAvg(qp) ) )
         {
            error_msg("ERROR: diff and/or psavg is not supported with an explicit pct.");
            return(NULL);
         }

         CMPEN_CPU_PCT(user);
         CMPEN_CPU_PCT(nice);
         CMPEN_CPU_PCT(sys);
         CMPEN_CPU_PCT(idle);
         CMPEN_CPU_PCT(iowait);
         CMPEN_CPU_PCT(irq);
         CMPEN_CPU_PCT(sirq);
         CMPEN_CPU_PCT(steal);
         CMPEN_CPU_PCT(guest);
         CMPEN_CPU_PCT(gnice);

         if ( this_pitem )
         {
            if ( NULL == (dstruct = malloc(sizeof(struct cpu_data_bin))) )
               return(NULL);

            dstruct->dtype = DTYPE_PCT;
            dstruct->thisd = this_data->thistotal;
            dstruct->lastd = this_data->lasttotal;

            this_pitem->dstruct = (void *)dstruct;

            this_pitem->next_ui = this_provider->ui_list;
            this_provider->ui_list = this_pitem;

            return(this_pitem);
         }

         /* This is a fall-through to return(NULL) */
      }

      /* If we are here, then the input was not parsed ---> ERROR CONDITION */
   }


   /* ============================== */
   /* Per-CPU (either specific or all) */
   if ( qp->iname == StrStr(qp->iname, "cpu.") )
   {
      /* This is a per-CPU stat */

      if ( qp->pargs[0] == 0 )
      {
         /* A provider arg was not used - with an item that requires it! */
         error_msg("ERROR: The per-cpu option requires a CPU argument.");
         return(NULL);
      }

      /* ============================== */
      /* For all CPUs (wildcard) */
      if (( qp->pargs[0] == '*' ) && ( qp->pargs[1] == 0 ))
      {
         dtype = DTYPE_PCT;

         if(is_raw_stat(qp->iname))
            dtype = DTYPE_RAW;

         requested_cpu = 0;
         base_pitem = NULL;
         last_pitem = NULL; /* About a compiler warning */
         while ( requested_cpu < this_data->cpucnt )
         {
            this_pitem = NULL;

            if ( DTYPE_RAW == dtype )
            {
               CMPEN_CPU_RAW(user);
               CMPEN_CPU_RAW(nice);
               CMPEN_CPU_RAW(sys);
               CMPEN_CPU_RAW(idle);
               CMPEN_CPU_RAW(iowait);
               CMPEN_CPU_RAW(irq);
               CMPEN_CPU_RAW(sirq);
               CMPEN_CPU_RAW(steal);
               CMPEN_CPU_RAW(guest);
               CMPEN_CPU_RAW(gnice);
            }
            else
            {
               CMPEN_CPU_PCT(user);
               CMPEN_CPU_PCT(nice);
               CMPEN_CPU_PCT(sys);
               CMPEN_CPU_PCT(idle);
               CMPEN_CPU_PCT(iowait);
               CMPEN_CPU_PCT(irq);
               CMPEN_CPU_PCT(sirq);
               CMPEN_CPU_PCT(steal);
               CMPEN_CPU_PCT(guest);
               CMPEN_CPU_PCT(gnice);
            }

            
            if ( NULL == this_pitem )
               return(NULL);

            if ( NULL == (dstruct = malloc(sizeof(struct cpu_data_bin))) )
                  return(NULL);

            dstruct->dtype = dtype;

            dstruct->thisd = this_data->thiscpuX + requested_cpu;
            dstruct->lastd = this_data->lastcpuX + requested_cpu;

            this_pitem->dstruct = (void *)dstruct;

            /* Linked list magic for the output item */
            if ( NULL == base_pitem )
            {
               base_pitem = this_pitem;
            }
            else
            {
               /* last_pitem is set at the bottom of the loop. It is not set when base_pitem is NULL */
               last_pitem->next_opi = this_pitem;
            }

            /* Set the header - if none was requested */
            if ( NULL == qp->cfpn->header )
            {
               strcpy(hdr_temp, this_pitem->header); /* Copy the existing header */
               hdr_ptr = hdr_temp + 3; /* We *know* that the string begins with "cpu." */
               sprintf(this_pitem->header, "%d%s", requested_cpu, hdr_ptr);
            }

            /* Put this in the update item list */
            this_pitem->next_ui = this_provider->ui_list;
            this_provider->ui_list = this_pitem;

            last_pitem = this_pitem;

            requested_cpu++;
         }

         /* Return the top of the list */
         return(base_pitem);
      }

      /* ============================== */
      /* Per-CPU for a single CPU */
      if ( is_pnumeric(qp->pargs) )
      {
         requested_cpu = atoi(qp->pargs);

         if ( requested_cpu >= this_data->cpucnt ) 
         {
            error_msg("ERROR: The requested proc.stat CPU (%d) is not on this system.", requested_cpu);
            return(NULL);
         }

         this_pitem = NULL;

         if (is_raw_stat(qp->iname))
         {
            dtype = DTYPE_RAW;

            CMPEN_CPU_RAW(user);
            CMPEN_CPU_RAW(nice);
            CMPEN_CPU_RAW(sys);
            CMPEN_CPU_RAW(idle);
            CMPEN_CPU_RAW(iowait);
            CMPEN_CPU_RAW(irq);
            CMPEN_CPU_RAW(sirq);
            CMPEN_CPU_RAW(steal);
            CMPEN_CPU_RAW(guest);
            CMPEN_CPU_RAW(gnice);
         }
         else
         {
            dtype = DTYPE_PCT;

            CMPEN_CPU_PCT(user);
            CMPEN_CPU_PCT(nice);
            CMPEN_CPU_PCT(sys);
            CMPEN_CPU_PCT(idle);
            CMPEN_CPU_PCT(iowait);
            CMPEN_CPU_PCT(irq);
            CMPEN_CPU_PCT(sirq);
            CMPEN_CPU_PCT(steal);
            CMPEN_CPU_PCT(guest);
            CMPEN_CPU_PCT(gnice);
         }
         
         if ( this_pitem )
         {
            if ( NULL == (dstruct = malloc(sizeof(struct cpu_data_bin))) )
               return(NULL);
            dstruct->dtype = dtype;

            dstruct->thisd = this_data->thiscpuX + requested_cpu;
            dstruct->lastd = this_data->lastcpuX + requested_cpu;

            this_pitem->dstruct = (void *)dstruct;

            this_pitem->next_ui = this_provider->ui_list;
            this_provider->ui_list = this_pitem;

            return(this_pitem);
         }
      }

      /* If we are here, then the input was not parsed (or error) ---> ERROR CONDITION */
   }


   return(NULL);
}

/* ========================================================================= */
/* Returns 0 on error - 0 is ALWAYS invalid */
int cpus_in_buff(struct buffer *b)
{
   char line[8]; /* We only need the first set of chars */
   int cpu_cnt = -1; /* Dont count the first cpu... line (it is total) */

   if ( NULL == b )
      return(0);

   if(FillReadBuff(b))
      return(0);

   while ( GetNextLine(line, b, 8) )
   {
      if (( line[0] == 'c' ) && ( line[1] == 'p' ) && ( line[2] == 'u' ))
         cpu_cnt++;
   }

   return(cpu_cnt);
}

/* ========================================================================= */
struct psdataf *new_stat_struct(int cpu_cnt)
{
   struct psdataf *d;

   if ( cpu_cnt < 1 )
      return(NULL);

   if ( NULL == (d = (struct psdataf *)malloc(sizeof(struct psdataf))) )
      return(NULL);

   if ( NULL == (d->thistotal = (struct cpudata *)malloc(sizeof(struct cpudata))) )
      return(NULL);

   if ( NULL == (d->lasttotal = (struct cpudata *)malloc(sizeof(struct cpudata))) )
      return(NULL);

   if ( NULL == (d->thiscpuX = (struct cpudata *)malloc(sizeof(struct cpudata) * cpu_cnt)) )
      return(NULL);

   if ( NULL == (d->lastcpuX = (struct cpudata *)malloc(sizeof(struct cpudata) * cpu_cnt)) )
      return(NULL);

   d->cpucnt = cpu_cnt;

   return(d);
}

/* ========================================================================= */
int update_stat_data(struct buffer *buf, struct psdataf *psd)
{
   struct cpudata *cpudtemp;
   char line[128];

   int cpuN;

   unsigned long long user;
   unsigned long long nice;
   unsigned long long sys;
   unsigned long long idle;
   unsigned long long iowait;
   unsigned long long irq;
   unsigned long long sirq;
   unsigned long long steal;
   unsigned long long guest;
   unsigned long long gnice;

   /* Rotate all stats */
   cpudtemp = psd->lasttotal;
   psd->lasttotal = psd->thistotal;
   psd->thistotal = cpudtemp;

   cpudtemp = psd->lastcpuX;
   psd->lastcpuX = psd->thiscpuX;
   psd->thiscpuX = cpudtemp;


   /* NOTE: The sscanf() return value is only checked for the base value
            of 5. This varies by release. The default behavior (on the
            previous releases) is to return 0s for values that are not
            supported. This is done silently. */

   if( FillReadBuff(buf) )
      return(1);

   while ( GetNextLine(line, buf, 128) )
   {
      if (( line[0] == 'c' ) && ( line[1] == 'p' ) && ( line[2] == 'u' ) && ( line[3] == ' ' ))
      {
         user = 0;
         nice = 0;
         sys = 0;
         idle = 0;
         iowait = 0;
         irq = 0;
         sirq = 0;
         steal = 0;
         guest = 0;
         gnice = 0;

         sscanf(line, "cpu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                &user,
                &nice,
                &sys,
                &idle,
                &iowait,
                &irq,
                &sirq,
                &steal,
                &guest,
                &gnice);

         psd->thistotal->user = user;
         psd->thistotal->nice = nice;
         psd->thistotal->sys = sys;
         psd->thistotal->idle = idle;
         psd->thistotal->iowait = iowait;
         psd->thistotal->irq = irq;
         psd->thistotal->sirq = sirq;
         psd->thistotal->steal = steal;
         psd->thistotal->guest = guest;
         psd->thistotal->total = 0; /* This means that the value was reset - NOT calculated at this time */
         
      }


      if (( line[0] == 'c' ) && ( line[1] == 'p' ) && ( line[2] == 'u' ) && ( line[3] != ' ' ))
      {
         user = 0;
         nice = 0;
         sys = 0;
         idle = 0;
         iowait = 0;
         irq = 0;
         sirq = 0;
         steal = 0;
         guest = 0;
         gnice = 0;

         sscanf(line, "cpu%d %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                &cpuN,
                &user,
                &nice,
                &sys,
                &idle,
                &iowait,
                &irq,
                &sirq,
                &steal,
                &guest,
                &gnice);

         psd->thiscpuX[cpuN].cpun = cpuN;
         psd->thiscpuX[cpuN].user = user;
         psd->thiscpuX[cpuN].nice = nice;
         psd->thiscpuX[cpuN].sys = sys;
         psd->thiscpuX[cpuN].idle = idle;
         psd->thiscpuX[cpuN].iowait = iowait;
         psd->thiscpuX[cpuN].irq = irq;
         psd->thiscpuX[cpuN].sirq = sirq;
         psd->thiscpuX[cpuN].steal = steal;
         psd->thiscpuX[cpuN].guest = guest;
         psd->thiscpuX[cpuN].gnice = gnice;
         psd->thiscpuX[cpuN].total = 0; /* This means that the value was reset - NOT calculated at this time */
      }
   } 

   /* ISSUE/NOTE: This is designed to plug in the other lines here */

   return(0);

};


