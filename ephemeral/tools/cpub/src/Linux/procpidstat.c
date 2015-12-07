#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <sys/types.h>

#include "../mainhelp.h"
#include "procpidstat.h"
#include "linproviders.h"


struct pps_data
{
   pid_t pid;
   char *comm;
   char *state;
   pid_t ppid;
   pid_t pgrp;
   pid_t session;  /* array.c --> sid */
   int32_t tty_nr;
   int32_t tpgid;  /* array.c --> tty_pgrp */
   uint32_t flags;
   uint64_t minflt;
   uint64_t cminflt;
   uint64_t majflt;
   uint64_t cmajflt;
   uint64_t utime;
   uint64_t stime;
   int64_t cutime;
   int64_t cstime;
   int64_t priority;
   int64_t nice;
   int32_t num_threads;
   int32_t itrealvalue; /* Hard set to 0 */
   uint64_t starttime;
   uint64_t vsize;
   int64_t rss;
   uint64_t rsslim;
   uint64_t startcode;
   uint64_t endcode;
   uint64_t startstack;
   uint64_t kstkesp;
   uint64_t kstkeip;
   uint64_t signal;
   uint64_t blocked;
   uint64_t sigignore;
   uint64_t sigcatch;
   uint64_t wchan;
   uint64_t nswap; /* Hard set to 0 */
   uint64_t cnswap; /* Hard set to 0 */
   int32_t exit_signal;
   int32_t processor;
   uint32_t rt_priority;
   uint32_t policy;
   uint64_t delayacct_blkio_ticks;
   uint64_t guest_time;
   int64_t cguest_time;
};


#define BUFFER_ALLOCATION 1024   /* Size of the file buffer allocation */
                                 /* Sampling tells me that this is closer to 256 */
#define PROC_FILENAME_LEN 32     /* Size of the proc file name */
#define COMM_MALLOC_SZ    64     /* How much to pre-allocate for comm usage */

struct pps_listentry
{
   pid_t pid;

   char *filename;
   struct buffer *rbuf;
   
   struct pps_data *this_pps;
   struct pps_data *last_pps;

   struct pps_listentry *next;
};

static const char *piname[] = { "pid",             \
                                "comm",            \
                                "state",           \
                                "ppid",            \
                                "pgrp",            \
                                "session",         \
                                "tty_nr",          \
                                "tpgid",           \
                                "flags",           \
                                "minflt",          \
                                "cminflt",         \
                                "majflt",          \
                                "cmajflt",         \
                                "utime",           \
                                "stime",           \
                                "cutime",          \
                                "cstime",          \
                                "priority",        \
                                "nice",            \
                                "num_threads",     \
                                "itrealvalue",     \
                                "starttime",       \
                                "vsize",           \
                                "rss",             \
                                "rsslim",          \
                                "startcode",       \
                                "endcode",         \
                                "startstack",      \
                                "kstkesp",         \
                                "kstkeip",         \
                                "signal",          \
                                "blocked",         \
                                "sigignore",       \
                                "sigcatch",        \
                                "wchan",           \
                                "nswap",           \
                                "cnswap",          \
                                "exit_signal",     \
                                "processor",       \
                                "rt_priority",     \
                                "policy",          \
                                "delayacct_blkio_ticks", \
                                "guest_time",      \
                                "cguest_time",     \
                                NULL};

static const int pitype[]  =  {PDT_INT32,       \
                               PDT_STRING,      \
                               PDT_STRING,      \
                               PDT_INT32,       \
                               PDT_INT32,       \
                               PDT_INT32,       \
                               PDT_INT32,       \
                               PDT_INT32,       \
                               PDT_UINT32,      \
                               PDT_UINT64,      \
                               PDT_UINT64,      \
                               PDT_UINT64,      \
                               PDT_UINT64,      \
                               PDT_UINT64,      \
                               PDT_UINT64,      \
                               PDT_INT64,       \
                               PDT_INT64,       \
                               PDT_INT64,       \
                               PDT_INT64,       \
                               PDT_INT32,       \
                               PDT_INT32,       \
                               PDT_UINT64,      \
                               PDT_UINT64,      \
                               PDT_INT64,       \
                               PDT_UINT64,      \
                               PDT_UINT64,      \
                               PDT_UINT64,      \
                               PDT_UINT64,      \
                               PDT_UINT64,      \
                               PDT_UINT64,      \
                               PDT_UINT64,      \
                               PDT_UINT64,      \
                               PDT_UINT64,      \
                               PDT_UINT64,      \
                               PDT_UINT64,      \
                               PDT_UINT64,      \
                               PDT_UINT64,      \
                               PDT_INT32,       \
                               PDT_INT32,       \
                               PDT_UINT32,      \
                               PDT_UINT32,      \
                               PDT_UINT64,      \
                               PDT_UINT64,      \
                               PDT_INT64};
 


static struct provider *myself;
static struct pps_listentry *pidlist;    /* List of PROCESS (not pitems) that
                                            must be updated. */

/* ========================================================================= */
int pps_list(int dflag);
struct pitem *pps_enablepitem(struct qparts *qp);
int pps_update(int interval);

pid_t str_to_pid(char *pidstr);
struct pps_listentry *pps_insert_pid(char *pargs);
int parse_pid_stat(struct pps_listentry *thispid);




/* ========================================================================= */
int ProcPidStatRegister(struct proot *p)
{
   /* Minimal type comparisons - most are self-defined */
   assert(sizeof(pid_t) == sizeof(int32_t));

   /* NULL out the pidlist - no members yet */
   pidlist = NULL;


   if ( NULL == (myself = RegisterProvider(p, "proc.pid.stat",
                                           pps_update,
                                           pps_list,
                                           pps_enablepitem)) )
   {
      /* The above section is indented differently. This bracket is only
         to assist in making this error condition block more clear */
      return(1);
   }

   return(0);

}

/* ========================================================================= */
int pps_list(int dflag)
{
   int icount = 0;

   while ( piname[icount] )
   {
      DumpQuadData(dflag, pitype[icount]);
      printf("proc.pid.stat:*:%s\n", piname[icount]);

      icount++;
   }

   return(icount);
}


/* ========================================================================= */
#define COMP_AND_ENABLE_S32(INAME); if ( 0 == strcmp(qp->iname, #INAME) ) \
                                    {                                                                                                      \
                                       data_ptr = malloc(sizeof(int32_t));                                                                 \
                                       if ( NULL == (this_pitem = NewPItem(qp, PDT_INT32, data_ptr)) ) { return(NULL); }                   \
                                       this_pitem->sioffset = (unsigned long)&thispid->this_pps->INAME - (unsigned long)thispid->this_pps; \
                                       this_pitem->dstruct = (void *)thispid;                                                              \
                                       goto itemfound;                                                                                     \
                                    }

#define COMP_AND_ENABLE_U32(INAME); if ( 0 == strcmp(qp->iname, #INAME) ) \
                                    {                                                                                                      \
                                       data_ptr = malloc(sizeof(uint32_t));                                                                \
                                       if ( NULL == (this_pitem = NewPItem(qp, PDT_UINT32, data_ptr)) ) { return(NULL); }                  \
                                       this_pitem->sioffset = (unsigned long)&thispid->this_pps->INAME - (unsigned long)thispid->this_pps; \
                                       this_pitem->dstruct = (void *)thispid;                                                              \
                                       goto itemfound;                                                                                     \
                                    }

#define COMP_AND_ENABLE_S64(INAME); if ( 0 == strcmp(qp->iname, #INAME) ) \
                                    {                                                                                                      \
                                       data_ptr = malloc(sizeof(int64_t));                                                                 \
                                       if ( NULL == (this_pitem = NewPItem(qp, PDT_INT64, data_ptr)) ) { return(NULL); }                   \
                                       this_pitem->sioffset = (unsigned long)&thispid->this_pps->INAME - (unsigned long)thispid->this_pps; \
                                       this_pitem->dstruct = (void *)thispid;                                                              \
                                       goto itemfound;                                                                                     \
                                    }

#define COMP_AND_ENABLE_U64(INAME); if ( 0 == strcmp(qp->iname, #INAME) ) \
                                    {                                                                                                      \
                                       data_ptr = malloc(sizeof(uint64_t));                                                                \
                                       if ( NULL == (this_pitem = NewPItem(qp, PDT_UINT64, data_ptr)) ) { return(NULL); }                  \
                                       this_pitem->sioffset = (unsigned long)&thispid->this_pps->INAME - (unsigned long)thispid->this_pps; \
                                       this_pitem->dstruct = (void *)thispid;                                                              \
                                       goto itemfound;                                                                                     \
                                    }

/*                                  NOTE: On the use of DEADBEEF. This is because the update() function will point the data_ptr to
                                    the string data on each update. You cannot diff, psavg, or other.... so just point to the most
                                    recent string. */
#define COMP_AND_ENABLE_STRING(INAME); if ( 0 == strcmp(qp->iname, #INAME) ) \
                                    {                                                                                                      \
                                       if ( NULL == (this_pitem = NewPItem(qp, PDT_STRING, (void *)0xDEADBEEF)) ) { return(NULL); }        \
                                       this_pitem->sioffset = (unsigned long)&thispid->this_pps->INAME - (unsigned long)thispid->this_pps; \
                                       this_pitem->dstruct = (void *)thispid;                                                              \
                                       goto itemfound;                                                                                     \
                                    }

#define COMP_AND_ENABLE_4B_STR(INAME); if ( 0 == strcmp(qp->iname, #INAME) ) \
                                    {                                                                                                      \
                                       if ( NULL == (this_pitem = NewPItem(qp, PDT_STRING, (void *)0xDEADBEEF)) ) { return(NULL); }        \
                                       this_pitem->sioffset = (unsigned long)&thispid->this_pps->INAME - (unsigned long)thispid->this_pps; \
                                       this_pitem->dstruct = (void *)thispid;                                                              \
                                       goto itemfound;                                                                                     \
                                    }


/* ========================================================================= */
struct pitem *pps_enablepitem(struct qparts *qp)
{
   struct pitem *this_pitem;
   struct pps_listentry *thispid;
   void *data_ptr;

   /* Make sure we were called with the right quad (provider) name */
   if ( 0 != strcmp(qp->pname, "proc.pid.stat") )
      return(NULL);

   /* Flag update for the provider - Go ahead and do it. Because if we fail
      later for any reason, we sill simply exit. */
   myself->update_required = 1;

   /* Insert or find and return pointer to proc_data struct for this PID */
   /* pps_insert_pid() will query the data (if this is a new pid). This
      will insure that this is a valid PID and the results are obtainable. */
   if ( NULL == (thispid = pps_insert_pid(qp->pargs)) )
      return(NULL);

   COMP_AND_ENABLE_S32(pid);
   COMP_AND_ENABLE_STRING(comm);
   COMP_AND_ENABLE_4B_STR(state);
   COMP_AND_ENABLE_S32(ppid);
   COMP_AND_ENABLE_S32(pgrp);
   COMP_AND_ENABLE_S32(session);
   COMP_AND_ENABLE_S32(tty_nr); /* Controlling terminal ***/
   COMP_AND_ENABLE_S32(tpgid); /* tpgid - PID of foreground process group of the controlling terminal... */
   COMP_AND_ENABLE_S32(flags);
   COMP_AND_ENABLE_U64(minflt);
   COMP_AND_ENABLE_U64(cminflt);
   COMP_AND_ENABLE_U64(majflt);
   COMP_AND_ENABLE_U64(cmajflt);
   COMP_AND_ENABLE_U64(utime);
   COMP_AND_ENABLE_U64(stime);
   COMP_AND_ENABLE_S64(cutime);
   COMP_AND_ENABLE_S64(cstime);
   COMP_AND_ENABLE_S64(priority);
   COMP_AND_ENABLE_S64(nice);
   COMP_AND_ENABLE_S32(num_threads);
   COMP_AND_ENABLE_S32(itrealvalue);
   COMP_AND_ENABLE_U64(starttime);
   COMP_AND_ENABLE_U64(vsize); /* in bytes */
   COMP_AND_ENABLE_S64(rss); /* in pages */
   COMP_AND_ENABLE_U64(rsslim); /* in bytes */
   COMP_AND_ENABLE_U64(startcode);
   COMP_AND_ENABLE_U64(endcode);
   COMP_AND_ENABLE_U64(startstack);
   COMP_AND_ENABLE_U64(kstkesp);
   COMP_AND_ENABLE_U64(kstkeip);
   COMP_AND_ENABLE_U64(signal);    /* Use status instead */
   COMP_AND_ENABLE_U64(blocked);   /* Use status instead */
   COMP_AND_ENABLE_U64(sigignore); /* Use status instead */
   COMP_AND_ENABLE_U64(sigcatch);  /* Use status instead */
   COMP_AND_ENABLE_U64(wchan);
   COMP_AND_ENABLE_U64(nswap);     /* (not maintained) */
   COMP_AND_ENABLE_U64(cnswap);    /* (not maintained) */
   COMP_AND_ENABLE_S32(exit_signal);
   COMP_AND_ENABLE_S32(processor);
   COMP_AND_ENABLE_U32(rt_priority);
   COMP_AND_ENABLE_U32(policy);
   COMP_AND_ENABLE_U64(delayacct_blkio_ticks);
   COMP_AND_ENABLE_U64(guest_time);
   COMP_AND_ENABLE_S64(cguest_time);


   /* Item was not found - We fell through to the basement!
      Returning without clearing up the registration of the proc_data item
      would be a problem - but this non-0 return is going to kill us all
      so - who cares. */
   return(NULL);

 itemfound:
   /* The item *was* found */

   /* Insert into ui list */
   InsertUpdateItem(myself, this_pitem);

   /* Pull the data as a test */
   /* At least 42 items should be read from this file */
   if ( 42 > parse_pid_stat(thispid) )
      return(NULL);

   return(this_pitem);
}

/* ========================================================================= */
pid_t str_to_pid(char *pidstr)
{
   pid_t pid;

   if ( NULL == pidstr )
      return(0);

   if ( is_pnumeric(pidstr) )
   {
      pid = atol(pidstr);
   }
   else
      return(0);

   return (pid);
}

/* ========================================================================= */
struct pps_listentry *pps_insert_pid(char *pargs)
{
   struct pps_listentry *thispid;
   pid_t pid;

   if ( 0 == (pid = str_to_pid(pargs)) )
   {
      fprintf(stderr, "ERROR: Unable to parse 2nd quad \"%s\" into a PID.\n", pargs);
      return(NULL);
   }

   /* See if that PID is already registered */
   thispid = pidlist;
   while(thispid)
   {
      if ( thispid->pid == pid )
         return(thispid);

      thispid = thispid->next;
   }

   /* No match was found - create a new one */
   if ( NULL == (thispid = (struct pps_listentry *)malloc(sizeof(struct pps_listentry))) )
   {
      error_msg("ERROR: Unable to allocate memory for pps_listentry structure.\n");
      return(NULL);
   }

   if ( NULL == (thispid->filename = (char *)malloc(PROC_FILENAME_LEN)) )
   {
      error_msg("Unable to allocate memory for proc filename.");
      return(NULL);
   }

   /* Drop in the filename */
   sprintf(thispid->filename, "/proc/%d/stat", pid);

   if ( NULL == (thispid->rbuf = InitProcBuff(BUFFER_ALLOCATION)) )
   {
      error_msg("Unable to allocate memory for read buffer.");
      return(NULL);
   }

   if ( NULL == (thispid->last_pps = (struct pps_data *)malloc(sizeof(struct pps_data))) )
   {
      error_msg("ERROR: Unable to allocate memory for pps_data structure.\n");
      return(NULL);
   }

   if ( NULL == (thispid->this_pps = (struct pps_data *)malloc(sizeof(struct pps_data))) )
   {
      error_msg("ERROR: Unable to allocate memory for pps_data structure.\n");
      return(NULL);
   }

   thispid->last_pps->comm = (char *)malloc(COMM_MALLOC_SZ);
   thispid->this_pps->comm = thispid->last_pps->comm; 
   
   thispid->last_pps->state = (char *)malloc(4);
   thispid->this_pps->state = thispid->last_pps->state; 
   
   thispid->pid = pid;

   /* Check that we can actually pull the data before putting into the list */
   if ( 42 > parse_pid_stat(thispid) )
   {
      error_msg("ERROR: Unable to retrieve stats for PID %d.", thispid->pid);
      return(NULL);
   }

   /* Insert into the list */
   thispid->next = pidlist;
   pidlist = thispid;

   /* Return a reference to what was created */
   return(thispid);
}

/* ========================================================================= */
#define RATCHET_FAIL();  data = next;                                             \
                         while (( *next != 0 ) && ( *next != ' ' ) && ( *next != '\n' )) { next++; } \
                         if ( *next == 0 ) { return(0); }                         \
                         if ( *next == ' ' )  { *next = 0; next++; }              \
                         if ( *next == '\n' ) { *next = 0; }

#define RATCHET_DATA();  data = next;                                             \
                         while (( *next != 0 ) && ( *next != ' ' ) && ( *next != '\n' )) { next++; } \
                         if ( *next == 0 ) { goto data_done; }          \
                         if ( *next == ' ' )  { *next = 0; next++; }              \
                         if ( *next == '\n' ) { *next = 0; }

#define READ_I__DATA(MEMBER); temp_pps->MEMBER = atoi(data); rv++;  /* fprintf(stderr, "DEBUG: Ireading(%d, %s)\n", rv, #MEMBER); */
#define READ_L__DATA(MEMBER); temp_pps->MEMBER = atol(data); rv++;  /* fprintf(stderr, "DEBUG: Ireading(%d, %s)\n", rv, #MEMBER); */
#define READ_LL_DATA(MEMBER); temp_pps->MEMBER = atoll(data); rv++; /* fprintf(stderr, "DEBUG: LLeading(%d, %s)\n", rv, #MEMBER); */



/* ========================================================================= */
int parse_pid_stat(struct pps_listentry *thispid)
{
   struct pps_data *temp_pps;
   int rv;
   char *next;
   char *data;
   int i;

   /* Rotate the data */
   temp_pps = thispid->last_pps;
   thispid->last_pps = thispid->this_pps;
   thispid->this_pps = temp_pps;

   if (PullNewBuff(thispid->rbuf, thispid->filename))
      return(0);

   /* Terminate so we look like a standard string */
   thispid->rbuf->buf[thispid->rbuf->got] = 0;

#ifdef TOO_FRAGILE
   /*                                                               minflt          utime   cutime  pri          startt               starts      signal              nswap */
   /*                                                               V               V       V       V       V  0 V                    V           V                   X   X   V      < */
   rv = sscanf((char *)thispid->rbuf->buf, "%d (%s) %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %d 0 %llu %lu %ld %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %d %d %u %u %llu %lu %ld\n",
          &temp_pps->pid,
          temp_pps->comm,
          &state,
          &temp_pps->ppid,
          &temp_pps->pgrp,
          &temp_pps->session,
          &temp_pps->tty_nr,
          &temp_pps->tpgid,
          &temp_pps->flags,
          &temp_pps->minflt,
          &temp_pps->cminflt,
          &temp_pps->majflt,
          &temp_pps->cmajflt,
          &temp_pps->utime,
          &temp_pps->stime,
          &temp_pps->cutime,
          &temp_pps->cstime,
          &temp_pps->priority,
          &temp_pps->nice,
          &temp_pps->num_threads, /* Docs say %ld. Code says %d (int) */
          (unsigned long long *)&temp_pps->starttime,
          &temp_pps->vsize,
          &temp_pps->rss,
          &temp_pps->rsslim,
          &temp_pps->startcode,
          &temp_pps->endcode,
          &temp_pps->startstack,
          &temp_pps->kstkesp,
          &temp_pps->kstkeip,
          &temp_pps->signal,
          &temp_pps->blocked,
          &temp_pps->sigignore,
          &temp_pps->sigcatch,
          &temp_pps->wchan,
          &temp_pps->nswap,
          &temp_pps->cnswap,
          &temp_pps->exit_signal,
          &temp_pps->processor,
          &temp_pps->rt_priority,
          &temp_pps->policy,
          (unsigned long long *)&temp_pps->delayacct_blkio_ticks,
          &temp_pps->guest_time,
          &temp_pps->cguest_time);
#endif
   rv = 0;

   next = (char *)thispid->rbuf->buf;

   RATCHET_DATA();
   READ_I__DATA(pid); /* Pointless - this is not going to change */
   RATCHET_DATA();
   if ( *data == '(' ) /* move off the leading paren */
      data++;
   i = 0;
   while ( ( data[i] != 0 ) && ( data[i] != ')' ) )
   {
      temp_pps->comm[i] = data[i];
      i++;
   }
   temp_pps->comm[i] = 0;

#ifdef DRAIN_BRAMMAGE
   fprintf(stderr, "DEBUG: read() ===================================\n");        
   fprintf(stderr, "DEBUG: RD[%s]\n", temp_pps->comm);
   fprintf(stderr, "DEBUG: RB[%lX]\n", (unsigned long)temp_pps);
   fprintf(stderr, "DEBUG: RA[%lX]\n", (unsigned long)&temp_pps->comm);
#endif
   rv++;

   RATCHET_DATA();
   temp_pps->state[0] = *data;
   temp_pps->state[1] = 0;
   rv++;
   RATCHET_FAIL();
   READ_I__DATA(ppid);
   RATCHET_FAIL();
   READ_I__DATA(pgrp);
   RATCHET_FAIL();
   READ_I__DATA(session);
   RATCHET_FAIL();
   READ_I__DATA(tty_nr); /* Controlling terminal ***/
   RATCHET_FAIL();
   READ_I__DATA(tpgid); /* tpgid - PID of foreground process group of the controlling terminal... */
   RATCHET_FAIL();
   READ_I__DATA(flags);
   RATCHET_FAIL();
   READ_I__DATA(minflt);
   RATCHET_FAIL();
   READ_I__DATA(cminflt);
   RATCHET_FAIL();
   READ_I__DATA(majflt);
   RATCHET_FAIL();
   READ_I__DATA(cmajflt);
   RATCHET_FAIL();
   READ_L__DATA(utime);
   RATCHET_FAIL();
   READ_L__DATA(stime);
   RATCHET_FAIL();
   READ_L__DATA(cutime);
   RATCHET_FAIL();
   READ_L__DATA(cstime);
   RATCHET_FAIL();
   READ_L__DATA(priority);
   RATCHET_FAIL();
   READ_L__DATA(nice);
   RATCHET_FAIL();
   READ_L__DATA(num_threads);
   RATCHET_FAIL();
   READ_L__DATA(itrealvalue);
   RATCHET_FAIL();
   READ_L__DATA(starttime); /* LL */
   RATCHET_FAIL();
   READ_L__DATA(vsize); /* in bytes */
   RATCHET_FAIL();
   READ_L__DATA(rss); /* in pages */
   RATCHET_FAIL();
   READ_L__DATA(rsslim); /* in bytes */
   RATCHET_FAIL();
   READ_L__DATA(startcode);
   RATCHET_FAIL();
   READ_L__DATA(endcode);
   RATCHET_FAIL();
   READ_L__DATA(startstack);
   RATCHET_FAIL();
   READ_L__DATA(kstkesp);
   RATCHET_FAIL();
   READ_L__DATA(kstkeip);
   RATCHET_FAIL();
   READ_L__DATA(signal);    /* Use status instead */
   RATCHET_FAIL();
   READ_L__DATA(blocked);   /* Use status instead */
   RATCHET_FAIL();
   READ_L__DATA(sigignore); /* Use status instead */
   RATCHET_FAIL();
   READ_L__DATA(sigcatch);  /* Use status instead */
   RATCHET_FAIL();
   READ_L__DATA(wchan);
   RATCHET_FAIL();
   READ_L__DATA(nswap);     /* (not maintained) */
   RATCHET_FAIL();
   READ_L__DATA(cnswap);    /* (not maintained) */
   RATCHET_FAIL();
   READ_I__DATA(exit_signal);
   RATCHET_FAIL();
   READ_I__DATA(processor);
   RATCHET_FAIL();
   READ_L__DATA(rt_priority);
   RATCHET_FAIL();
   READ_L__DATA(policy);
   RATCHET_FAIL();
   READ_LL_DATA(delayacct_blkio_ticks);
   RATCHET_DATA();
   READ_L__DATA(guest_time);
   RATCHET_DATA();
   READ_L__DATA(cguest_time);

 data_done:
   return(rv);
}

/* ========================================================================= */
int pps_update(int interval)
{
   struct pps_listentry *thispid;
   struct pitem *update_pi;
   void *last_data;
   void *this_data;
   char **strptr;
   int rv;

   /* Update all the PIDs */
   thispid = pidlist;
   while(thispid)
   {
      /* At least 42 items should be read from this file */
      if ( 42 > parse_pid_stat(thispid) )
         return(1);

      thispid = thispid->next;
   }

   /* Update all the pitems */
   rv = 0;
   update_pi = myself->ui_list;
   while( update_pi )
   {
      thispid = (struct pps_listentry *)update_pi->dstruct;
      last_data = (void *)((unsigned long)thispid->last_pps + (unsigned long)update_pi->sioffset);
      this_data = (void *)((unsigned long)thispid->this_pps + (unsigned long)update_pi->sioffset);


      if ( update_pi->data_type != PDT_STRING )
         rv += CalcData(update_pi, last_data, this_data, interval);
      else
      {
         strptr = (char **)this_data;
         this_data = *strptr;

         update_pi->data_ptr = this_data;
     }

      update_pi = update_pi->next_ui;
   }

   return(rv);
}


