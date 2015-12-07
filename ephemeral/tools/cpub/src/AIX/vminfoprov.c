#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/vminfo.h>

#include "../mainhelp.h"
#include "vminfoprov.h"

/* ========================================================================= */
/* Notes on this provider:
   - This provider uses an "update list" method to update data. So there is no
     list of hard linked items that can be directly referenced. So, to do
     calculations, the latest struct is refreshed, and then we walk through
     the ui list and update only those items on the list.
   - The complete list of supported items is kept in the diname array. (It has
     a related ditype array. They are indexed the same.) These lists are used
     to generate a list of known items.
*/


/* This info is all in <sys/vminfo.h> */

static struct vminfo64 *lastvm;
static struct vminfo64 *thisvm;

/* The items we support */
static const char *diname[] = {"numwseguse",  "numpseguse",  "numclseguse", \
                               "numwsegpin",  "numpsegpin",  "numclsegpin", \
                               "pgexct",      "pgrclm",      "lockexct",    \
                               "backtrks",    "pageins",     "pageouts",    \
                               "pgspgins",    "pgspgouts",   "numsios",     \
                               "numiodone",   "zerofills",   "exfills",     \
                               "scans",       "cycles",      "pgsteals",    \
                               "freewts",     "extendwts",   "pendiowts",   \
                               "numpout",     "numremote",   "lgpg_cnt",    \
                               "lgpg_numfrb", "lgpg_inuse",  "lgpg_hi",     \
                               "numralloc",                                 \
                               NULL};

static const int ditype[]  =  {PDT_INT64,     PDT_INT64,     PDT_INT64,     \
                               PDT_INT64,     PDT_INT64,     PDT_INT64,     \
                               PDT_UINT64,    PDT_UINT64,    PDT_UINT64,    \
                               PDT_UINT64,    PDT_UINT64,    PDT_UINT64,    \
                               PDT_UINT64,    PDT_UINT64,    PDT_UINT64,    \
                               PDT_UINT64,    PDT_UINT64,    PDT_UINT64,    \
                               PDT_UINT64,    PDT_UINT64,    PDT_UINT64,    \
                               PDT_UINT64,    PDT_UINT64,    PDT_UINT64,    \
                               PDT_INT64,     PDT_INT64,     PDT_INT64,     \
                               PDT_INT64,     PDT_INT64,     PDT_INT64,     \
                               PDT_INT64,                                   \
                               };


static struct provider *myself;

/* ========================================================================= */
int init_vdata(void)
{
   if ( NULL == (lastvm = (struct vminfo64 *)malloc(sizeof(struct vminfo64))) )
      return(1);

   if ( NULL == (thisvm = (struct vminfo64 *)malloc(sizeof(struct vminfo64))) )
      return(1);
   
   if(0 != vmgetinfo(thisvm, VMINFO64, sizeof(struct vminfo64)))
   {
      error_msg("ERROR: Unable to collect memory stats.");
      return(1);
   }

   return(0);
}

/* ========================================================================= */
int vminfo_update(int interval)
{
   struct pitem *update_pi;
   struct vminfo64 *tempvm;

   int64_t *lllast;
   int64_t *llthis;
   int64_t *lltemp;
   uint64_t *ulllast;
   uint64_t *ullthis;
   uint64_t *ulltemp;

   /* rotate this and last */
   tempvm = thisvm;
   thisvm = lastvm;
   lastvm = tempvm;


   if(0 != vmgetinfo(thisvm, VMINFO64, sizeof(struct vminfo64)))
   {
      /* Sorry - No stderr messages here!
      fprintf(stderr, "ERROR: Unable to collect memory stats.\n");
      */
      return(1);
   }

   update_pi = myself->ui_list;
   while( update_pi )
   {
      if ( update_pi->data_type == PDT_INT64 )
      {
         /* Find our data */
         lllast = (int64_t *)((unsigned long)lastvm + (unsigned long)update_pi->sioffset);
         llthis = (int64_t *)((unsigned long)thisvm + (unsigned long)update_pi->sioffset);

         if ( update_pi->munge_flag & MUNGE_DIFF )
         {
            lltemp = update_pi->data_ptr;
            *lltemp = *llthis - *lllast;
         }
         else
         {
            lltemp = update_pi->data_ptr;
            *lltemp = *llthis;
         }

         if ( update_pi->munge_flag & MUNGE_PSAVG )
         {
            *lltemp = *lltemp / (int64_t)interval;
         }
      }

      if ( update_pi->data_type == PDT_UINT64 )
      {
         /* Find our data */
         ulllast = (uint64_t *)((unsigned long)lastvm + (unsigned long)update_pi->sioffset);
         ullthis = (uint64_t *)((unsigned long)thisvm + (unsigned long)update_pi->sioffset);

         if ( update_pi->munge_flag & MUNGE_DIFF )
         {
            ulltemp = update_pi->data_ptr;
            *ulltemp = *ullthis - *ulllast;
         }
         else
         {
            ulltemp = update_pi->data_ptr;
            *ulltemp = *ullthis;
         }

         if ( update_pi->munge_flag & MUNGE_PSAVG )
         {
            *ulltemp = *ulltemp / (uint64_t)interval;
         }
      }

      update_pi = update_pi->next_ui;
   }


   return(0);
}


/* ========================================================================= */
int vminfo_list(int dflag)
{
   struct pitem *this_pitem;
   int icount = 0;

   while ( diname[icount] )
   {
      DumpQuadData(dflag, ditype[icount]);
      printf("vminfo::%s\n", diname[icount]);

      icount++;
   }

   return(icount);
}

/* ========================================================================= */
struct pitem *vminfo_itemenable(int data_size, int data_type, unsigned long sioffset, struct qparts *qp)
{
   struct pitem *this_pi;
   void *data_ptr;

   /* Validate input */
   if ( NULL == qp )
      return(NULL);

   if ( data_type == PDT_UNKNOWN )
      return(NULL);

   /* Make sure we were called with the right quad (provider) name */
   if ( 0 != strcmp(qp->pname, "vminfo") )
      return(NULL);

   /* Allocate the memory for our item */
   if (NULL == (data_ptr = malloc(data_size)))
      return(NULL);

   /* Allocate pitem */
   if ( NULL == (this_pi = NewPItem(qp, data_type, data_ptr)) )
      return(NULL);

   /* Set pitem */
   this_pi->sioffset = sioffset;

   /* Parse iargs */
   if ( ShouldDiff(qp) )
      this_pi->munge_flag |= MUNGE_DIFF;

   if ( ShouldPSAvg(qp) )
      this_pi->munge_flag |= MUNGE_DIFF;

   /* Insert into ui list */
   this_pi->next_ui = myself->ui_list;
   myself->ui_list = this_pi;

   return(this_pi);
}


/* ========================================================================= */


#define COMP_AND_ENABLE_INT64(INAME);  if ( 0 == strcmp(qp->iname, #INAME ) )                                                    \
                                       {                                                                                         \
                                          return(vminfo_itemenable(sizeof(int64_t),                                              \
                                                                   PDT_INT64,                                                    \
                                                                   (unsigned long)&thisvm->INAME - (unsigned long)thisvm, qp));  \
                                       }

#define COMP_AND_ENABLE_UINT64(INAME); if ( 0 == strcmp(qp->iname, #INAME ) )                                                    \
                                       {                                                                                         \
                                          return(vminfo_itemenable(sizeof(uint64_t),                                             \
                                                                   PDT_UINT64,                                                   \
                                                                   (unsigned long)&thisvm->INAME - (unsigned long)thisvm, qp));  \
                                       }

#define COMP_AND_ENABLE_UINT32(INAME); if ( 0 == strcmp(qp->iname, #INAME ) )                                                    \
                                       {                                                                                         \
                                          return(vminfo_itemenable(sizeof(uint32),                                               \
                                                                   PDT_UINT32,                                                   \
                                                                   (unsigned long)&thisvm->INAME - (unsigned long)thisvm, qp));  \
                                       }



struct pitem *vminfo_enablepitem(struct qparts *qp)
{
   struct pitem *this_pitem;

   /* Make sure we were called with the right quad (provider) name */
   if ( 0 != strcmp(qp->pname, "vminfo") )
      return(NULL);

   /* Flag update for the provider */
   myself->update_required = 1;

   /* Initialize the memory that will be required for this provider to
      go active. */
   if ( NULL == thisvm )
   {
      if(init_vdata())
      {
         fprintf(stderr, "ERROR: Failed to allocate memory for vminfo provider.\n");
         return(NULL);
      }
   }

	COMP_AND_ENABLE_UINT64(pgexct); 	/* count of page faults     		*/
	COMP_AND_ENABLE_UINT64(pgrclm); 	/* count of page reclaims 		*/
	COMP_AND_ENABLE_UINT64(lockexct);	/* count of lockmisses	    		*/
	COMP_AND_ENABLE_UINT64(backtrks);    	/* count of backtracks	    		*/
	COMP_AND_ENABLE_UINT64(pageins);	/* count of pages paged in  		*/
	COMP_AND_ENABLE_UINT64(pageouts);	/* count of pages paged out 		*/
	COMP_AND_ENABLE_UINT64(pgspgins);	/* count of page ins from paging space	*/
	COMP_AND_ENABLE_UINT64(pgspgouts);	/* count of page outs from paging space */
	COMP_AND_ENABLE_UINT64(numsios);	/* count of start I/Os	    		*/
	COMP_AND_ENABLE_UINT64(numiodone);	/* count of iodones	    		*/
	COMP_AND_ENABLE_UINT64(zerofills);	/* count of zero filled pages 		*/
	COMP_AND_ENABLE_UINT64(exfills);	/* count of exec filled pages		*/
	COMP_AND_ENABLE_UINT64(scans);		/* count of page scans by clock 	*/
	COMP_AND_ENABLE_UINT64(cycles);	/* count of clock hand cycles		*/
	COMP_AND_ENABLE_UINT64(pgsteals);	/* count of page steals	   		*/
	COMP_AND_ENABLE_UINT64(freewts);	/* count of free frame waits		*/
	COMP_AND_ENABLE_UINT64(extendwts);	/* count of extend XPT waits		*/
	COMP_AND_ENABLE_UINT64(pendiowts);	/* count of pending I/O waits  		*/

	COMP_AND_ENABLE_INT64(numpout);	/* number of fblru page-outs        */
	COMP_AND_ENABLE_INT64(numremote);	/* number of fblru remote page-outs */

	COMP_AND_ENABLE_INT64(numwseguse);	/* count of pages in use for working seg */
	COMP_AND_ENABLE_INT64(numpseguse);	/* count of pages in use for persistent seg */
	COMP_AND_ENABLE_INT64(numclseguse);	/* count of pages in use for client seg */
	COMP_AND_ENABLE_INT64(numwsegpin);	/* count of pages pinned for working seg */
	COMP_AND_ENABLE_INT64(numpsegpin);	/* count of pages pinned for persistent seg */
	COMP_AND_ENABLE_INT64(numclsegpin);	/* count of pages pinned for client seg */

	COMP_AND_ENABLE_INT64(lgpg_cnt);	/* Total number of large pages on the system */
	COMP_AND_ENABLE_INT64(lgpg_numfrb);	/* Number of large pages on the freelist(s) */
	COMP_AND_ENABLE_INT64(lgpg_inuse);	/* Number of large pages in use */
	COMP_AND_ENABLE_INT64(lgpg_hi);	/* Large page high water count */
	COMP_AND_ENABLE_INT64(numralloc);	/* number of remote allocations */

   /* The item was not found */
   return(NULL);
}

/* ========================================================================= */
int VMInfoRegister(struct proot *p)
{
   struct provider *this_provider;

   /* Assure that at least the bitness of our assumptions are correct */
   assert(sizeof(uint) == sizeof(uint32_t)); /* Not used in the 64bit variant of the call - that I can see */
   assert(sizeof(rpn64_t) == sizeof(int64_t));  /* vm_types.h - A signed, compile dependent, type */
   assert(sizeof(size64_t) == sizeof(uint64_t));

   /* This is an item that will be allocated by init_vdata(). It is set
      to NULL here so that it can be tested later. The point is that no 
      malloc() will be done until an item is registered. On registration
      this value will be checked and a conditional init will happen then. */
   thisvm = NULL;

   if ( NULL == (myself = RegisterProvider(p, "vminfo",
                                           vminfo_update,
                                           vminfo_list,
                                           vminfo_enablepitem)) )
   {
      /* The above section is indented differently. This bracket is only
         to assist in making this error condition block more clear */
      return(1);
   }

   return(0);

}
