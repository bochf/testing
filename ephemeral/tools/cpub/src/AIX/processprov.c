#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <procinfo.h>
#include <sys/types.h>

#include "processprov.h"
#include "../mainhelp.h"


struct proc_data
{
   pid_t pid;
   struct procentry64 *lastpe;
   struct procentry64 *thispe;

   struct proc_data *next;
};

#ifdef RAW_CUT_OF_SUPPORTED_ITEMS

	uint 		pi_thcount;	/* thread count */
	uint		pi_maxofile;	/* maximum u_ofile index in use */
	u_longlong_t 	pi_adspace;	/* process address space */
	longlong_t 	pi_majflt;	/* i/o page faults */
	longlong_t 	pi_minflt;	/* non i/o page faults */
	longlong_t 	pi_repage;	/* repaging count */
	longlong_t 	pi_size;	/* size of image (pages) */
	longlong_t	pi_ioch;	/* I/O character count  */
	longlong_t 	pi_irss;	/* accumulator for memory integral */
	u_longlong_t 	pi_drss;	/* data resident set size */
	u_longlong_t 	pi_trss;	/* text resident set size */
	u_longlong_t 	pi_dvm;		/* data virtual memory size */
	u_longlong_t 	pi_prm;		/* percent real memory usage */
	u_longlong_t 	pi_tsize;	/* size of text */
	u_longlong_t 	pi_dsize;	/* current break value */
	u_longlong_t 	pi_sdsize;	/* data size from shared library*/

	suseconds_t	pi_chk_utime;	/* user time at checkpoint  */
	suseconds_t	pi_chk_ctime;	/* child time at checkpoint  */

	uint            pi_policy;           /* process policy */
	uint            pi_ppri;             /* process priority */


	u_longlong_t    pi_adspace_ldr;


#endif

static const char *piname[] = { "pi_thcount",      \
                                "pi_maxofile",     \
                                "pi_adspace",      \
                                "pi_majflt",       \
                                "pi_minflt",       \
                                "pi_repage",       \
                                "pi_size",         \
                                "pi_ioch",         \
                                "pi_irss",         \
                                "pi_drss",         \
                                "pi_trss",         \
                                "pi_dvm",          \
                                "pi_prm",          \
                                "pi_tsize",        \
                                "pi_dsize",        \
                                "pi_sdsize",       \
                                "pi_chk_utime",    \
                                "pi_chk_ctime",    \
                                "pi_policy",       \
                                "pi_ppri",         \
                                "pi_adspace_ldr",  \
                                "pid",             \
                                NULL};

static const int pitype[]  =  {PDT_UINT32,      \
                               PDT_UINT32,      \
                               PDT_UINT64,      \
                               PDT_INT64,       \
                               PDT_INT64,       \ 	
                               PDT_INT64,       \
                               PDT_INT64,       \
                               PDT_INT64,       \
                               PDT_INT64,       \
                               PDT_UINT64,      \
                               PDT_UINT64,      \
                               PDT_UINT64,      \
                               PDT_UINT64,      \
                               PDT_UINT64,      \
                               PDT_UINT64,      \
                               PDT_UINT64,      \
                               PDT_UINT32,      \
                               PDT_UINT32,      \
                               PDT_UINT32,      \   
                               PDT_UINT32,      \
                               PDT_UINT64,      \
                               PDT_UINT32};

static struct provider *myself;
static struct proc_data *pdlist;    /* List of PROCESS (not pitems) that
                                       must be updated. */

/* ========================================================================= */
int ProcessRegister(struct proot *p);
int process_list(int dflag);
struct pitem *process_enablepitem(struct qparts *qp);
pid_t str_to_pid(char *pidstr);
struct proc_data *insert_pid_support(char *pargs);
int process_update(int interval);

/* ========================================================================= */
int ProcessRegister(struct proot *p)
{
   /* Make sure that our data type assumptions hold */
   assert(sizeof(suseconds_t) == sizeof(uint32_t));
   assert(sizeof(longlong_t) == sizeof(int64_t));
   assert(sizeof(u_longlong_t) == sizeof(uint64_t));
   assert(sizeof(uint) == sizeof(uint32_t));
   assert(sizeof(pid_t) == sizeof(uint32_t));

   /* NULL out the pdlist - no members yet */
   pdlist = NULL;


   if ( NULL == (myself = RegisterProvider(p, "process",
                                           process_update,
                                           process_list,
                                           process_enablepitem)) )
   {
      /* The above section is indented differently. This bracket is only
         to assist in making this error condition block more clear */
      return(1);
   }

   return(0);

}

/* ========================================================================= */
int process_list(int dflag)
{
   struct pitem *this_pitem;
   int icount = 0;

   while ( piname[icount] )
   {
      DumpQuadData(dflag, pitype[icount]);
      printf("process:*:%s\n", piname[icount]);

      icount++;
   }

   return(icount);
}

/* ========================================================================= */
#define COMP_AND_ENABLE_U64(INAME); if ( 0 == strcmp(qp->iname, #INAME) ) \
                                    {                                                                               \
                                       this_pitem->data_ptr = malloc(sizeof(uint64_t));                             \
                                       this_pitem->dstruct = (void *)this_procd;                                    \
                                       this_pitem->sioffset = (unsigned long)&this_procd->thispe->INAME - (unsigned long)this_procd->thispe; \
                                       this_pitem->data_type = PDT_UINT64;                                          \
                                       goto itemfound;                                                              \
                                    }

#define COMP_AND_ENABLE_64(INAME);  if ( 0 == strcmp(qp->iname, #INAME) ) \
                                    {                                                                               \
                                       this_pitem->data_ptr = malloc(sizeof(int64_t));                              \
                                       this_pitem->dstruct = (void *)this_procd;                                    \
                                       this_pitem->sioffset = (unsigned long)&this_procd->thispe->INAME - (unsigned long)this_procd->thispe; \
                                       this_pitem->data_type = PDT_INT64;                                           \
                                       goto itemfound;                                                              \
                                    }

#define COMP_AND_ENABLE_U32(INAME); if ( 0 == strcmp(qp->iname, #INAME) ) \
                                    {                                                                               \
                                       this_pitem->data_ptr = malloc(sizeof(uint32_t));                             \
                                       this_pitem->dstruct = (void *)this_procd;                                    \
                                       this_pitem->sioffset = (unsigned long)&this_procd->thispe->INAME - (unsigned long)this_procd->thispe; \
                                       this_pitem->data_type = PDT_UINT32;                                          \
                                       goto itemfound;                                                              \
                                    }

/* ========================================================================= */
struct pitem *process_enablepitem(struct qparts *qp)
{
   struct pitem *this_pitem;
   struct proc_data *this_procd;
   pid_t pid;

   /* Make sure we were called with the right quad (provider) name */
   if ( 0 != strcmp(qp->pname, "process") )
      return(NULL);

   /* Flag update for the provider */
   myself->update_required = 1;


   /* Insert or find and return pointer to proc_data struct for this PID */
   if ( NULL == (this_procd = insert_pid_support(qp->pargs)) )
      return(NULL);

   /* Simply allocate here - the next block of COMP_AND_ENABLE_*() macros
      will assign the public and required private values. */
   /* THIS IS A CHEAT! The data pointer (the third arg to NewPItem() cannot
      be NULL. Here I set it to an existing pointer THAT IS FOR THE WRONG
      ITEM... knowing full well I will reset it later. I am overriding the
      safety! */
   if ( NULL == (this_pitem = NewPItem(qp, PDT_UNKNOWN, qp)) )
   {
      return(NULL);
   }

   /* A special case just for pid */
   if ( 0 == strcmp(qp->iname, "pid") )
   {
      /* Don't allocate space, just point to existing PID value */
      this_pitem->data_ptr = &this_procd->pid;
      this_pitem->dstruct = NULL;
      this_pitem->sioffset = 0;
      this_pitem->data_type = PDT_UINT32;
      /* Go ahead and return now because:
         - No need to (test) pull PID data
         - No need to put into the update list
         - No need to test the malloc() (that never happened)
      */
      return(this_pitem);
   }
   
   COMP_AND_ENABLE_U64(pi_thcount);
   COMP_AND_ENABLE_64(pi_maxofile);
   COMP_AND_ENABLE_64(pi_adspace);
   COMP_AND_ENABLE_64(pi_majflt);
   COMP_AND_ENABLE_64(pi_minflt);
   COMP_AND_ENABLE_64(pi_repage);
   COMP_AND_ENABLE_64(pi_size);
   COMP_AND_ENABLE_U64(pi_ioch);
   COMP_AND_ENABLE_U64(pi_irss);
   COMP_AND_ENABLE_U64(pi_drss);
   COMP_AND_ENABLE_U64(pi_trss);
   COMP_AND_ENABLE_U64(pi_dvm);
   COMP_AND_ENABLE_U64(pi_prm);
   COMP_AND_ENABLE_U64(pi_tsize);
   COMP_AND_ENABLE_U64(pi_dsize);
   COMP_AND_ENABLE_U64(pi_sdsize);
   COMP_AND_ENABLE_U32(pi_chk_utime);
   COMP_AND_ENABLE_U32(pi_chk_ctime);
   COMP_AND_ENABLE_U32(pi_policy);
   COMP_AND_ENABLE_U32(pi_ppri);
   COMP_AND_ENABLE_U64(pi_adspace_ldr);

   /* Item was not found - We fell through to the basement!
      Returning without clearing up the registration of the proc_data item
      would be a problem - but this non-0 return is going to kill us all
      so - who cares. */
   return(NULL);

 itemfound:
   /* The item *was* found */
   
   /* Make sure our malloc was good - CHECK FOR MALLOC() IS DONE HERE */
   if ( NULL == this_pitem->data_ptr )
      return(NULL);

   /* Insert into ui list */
   InsertUpdateItem(myself, this_pitem);

   /* Pull the data as a test */
   pid = this_procd->pid;
   if ( 1 != getprocs64(this_procd->thispe, sizeof(struct procentry64), NULL, 0, &pid, 1) )
      return(NULL);

   if ( this_procd->pid != this_procd->thispe->pi_pid)
      return(NULL);

   /* Just to be sure... This prevents disaster with the HACK above
      that prevents the failure on the NewPItem() call */
   if ( this_pitem->data_ptr == qp )
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
struct proc_data *insert_pid_support(char *pargs)
{
   struct proc_data *this_procd;
   pid_t pid;

   if ( 0 == (pid = str_to_pid(pargs)) )
   {
      fprintf(stderr, "ERROR: Unable to parse 2nd quad \"%s\" into a PID.\n", pargs);
      return(NULL);
   }

   this_procd = pdlist;
   while(this_procd)
   {
      if ( this_procd->pid == pid )
         return(this_procd);

      this_procd = this_procd->next;
   }

   /* No match was found - create a new one */
   if ( NULL == (this_procd = (struct proc_data *)malloc(sizeof(struct proc_data))) )
   {
      error_msg("ERROR: Unable to allocate memory for proc_data structure.\n");
      return(NULL);
   }

   if ( NULL == (this_procd->lastpe = (struct procentry64 *)malloc(sizeof(struct procentry64))) )
   {
      error_msg("ERROR: Unable to allocate memory for procentry64 structure.\n");
      return(NULL);
   }

   if ( NULL == (this_procd->thispe = (struct procentry64 *)malloc(sizeof(struct procentry64))) )
   {
      error_msg("ERROR: Unable to allocate memory for procentry64 structure.\n");
      return(NULL);
   }

   this_procd->pid = pid;

   /* Insert into the list */
   this_procd->next = pdlist;
   pdlist = this_procd;

   /* Return a reference to what was created */
   return(this_procd);
}

/* ========================================================================= */
/* This is a TWO STEP update. The per-process data needs to be updated
   FOR EVERY PROCESS. Then the per-item calculations need to be done for
   every pitem.
*/
#define PUDBG(MESSAGE); fprintf(stderr, "DEBUG: process_update(%s);\n", MESSAGE);

int process_update(int interval)
{
   /* per-process data */
   struct proc_data *this_procd;
   struct procentry64 *temppe;
   pid_t pid;
   /* per-pitem data */
   struct pitem *update_pi;
   void *ptrlast;
   void *ptrthis;


   struct procentry64 *lastpe;
   struct procentry64 *thispe;

   /* Update the per-process information */
   this_procd = pdlist;
   while(this_procd)
   {
      /* Save the PID (getprocs64 will modify it) */
      pid = this_procd->pid;

      /* Rotate data prior to update */
      temppe = this_procd->lastpe;
      this_procd->lastpe = this_procd->thispe;
      this_procd->thispe = temppe;
      
      /* temppe holds the pointer that will be updated (so just use it) */

      /* Pull the data */
      if ( 1 != getprocs64(temppe, sizeof(struct procentry64), NULL, 0, &pid, 1) )
         return(1);

      /* Insure we got data for PID we expected */
      if ( this_procd->pid != temppe->pi_pid )
         return(1);

      /* Move to the next process item */
      this_procd = this_procd->next;
   }

   update_pi = myself->ui_list;
   while( update_pi )
   {
      /* Simplify the pointer stuff a bit - multiple steps */
      this_procd = (struct proc_data *)update_pi->dstruct; /* from void * to struct proc_data * */
      lastpe = this_procd->lastpe; /* Reference to the "last" procentry64 struct */
      thispe = this_procd->thispe; /* Reference to the "latest" procentry64 struct */
      /* data_ptr is a link to the datape item */

      ptrlast = (void *)((unsigned long)lastpe + (unsigned long)update_pi->sioffset);
      ptrthis = (void *)((unsigned long)thispe + (unsigned long)update_pi->sioffset);

      CalcData(update_pi, ptrlast, ptrthis, interval);

      update_pi = update_pi->next_ui;
   }

   return(0);
}


