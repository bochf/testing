#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include <errno.h>

//#include "undocumented.h"
//#include "smtchk.h"
#include "gather.h"

#ifdef UNDEFINED
/* ========================================================================= */
struct phys_cpu_info *GetCPUInfo(struct phys_cpu_info *pci)
{
   struct phys_cpu_info *obuf;

   if(pci != NULL)
      obuf = pci;
   else
   {
      if(NULL == (obuf = (struct phys_cpu_info *)malloc(sizeof(struct phys_cpu_info))))
         return(NULL);
   }

   /* STUB: We are not actually getting info here - entirely stubbed */

   obuf->ncpus = 0;
   obuf->ncpus_cfg = 0;
   obuf->description[0] = 0;
   obuf->processorHZ = 0;

   obuf->valid_data = 0;

   if(0 == (obuf->smt = GetSMT()))
      obuf->valid_data = 0;

   return(obuf);
}
#endif

/* ========================================================================= */
struct cpu_list *InitCPUList(void)
{
   struct cpu_list *newlist;

   struct cpu_socket *thiss;
   struct cpu_core *thisc;
   struct cpu_thread *thist;

   int i;  /* General index - but specifically used for ORDERED lists */
   int t;  /* Thread index - always used for thread mappings */

   if(NULL == (newlist = (struct cpu_list *)malloc(sizeof(struct cpu_list))))
      return(NULL);

   if (NULL == (newlist->lastcpus = GetCPUStats(NULL)))
      return(NULL);

   /* This is really just about allocating memory */
   if (NULL == (newlist->thiscpus = GetCPUStats(NULL)))
      return(NULL);
   
   /* Wait! What?
      Well... The API only holds current stats. But the tree is held in the
      cpustats struct - and now I have two of them. I cannot simply copy
      the pointer because the tree references the individual stats! So I run
      it once, and then reference that mapping ONLY in this function. It will
      get swapped back and forth between "this" and "last" each iteration 
      going forward. I don't run it twice because it takes time to build that
      tree, and it takes memory to hold both data structures that are used
      to build the tree. (These implementation details are kind of hidden
      by the library - but I know they exist.)
   */
   BuildCPUTree(newlist->thiscpus);

   newlist->iteration_time = 0; /* Set it to a bad default. The responsibility 
               to set this lies elsewhere. If it is not set there, then a 
               divide by zero error will happen and be very apparent. I would
				   rather have 1 crash / bad result than a lifetime of bad
				   data because this was incorrect. */
   newlist->last_munge_time = 0; /* Set to an easily recognizible non-valid date */

   /* Transfer the count to the parent struct - this is about limiting code
      modifications as we move between source libraries. */
   newlist->count = newlist->thiscpus->cpucnt;

   /* Next three not used here, but allocated in the same place for 
      cleanliness */
#ifdef UNDEFINED
   if(NULL == (newlist->cpua_calc = (perfstat_cpu_t *)malloc(newlist->cpua_size)))
      return(NULL);
#endif

   if(NULL == (newlist->derived = (struct cpua_derived *)malloc(sizeof(struct cpua_derived) * newlist->count)))
      return(NULL);

   /* Initialize the thread mapping array to safe defaults */
   if ( NULL == (newlist->thread_mapping = (int *)malloc(newlist->count * sizeof(int))) )
      return(NULL);

   /* Fill out the thread_mapping table we just created */
   if ( NULL != newlist->thiscpus->cpu_tree )
   {
      /* Mapping found - use it to derive the mapping "table" */
      i = 0;
      thiss = newlist->thiscpus->cpu_tree;
      while(thiss)
      {
         thisc = thiss->clist;
         while(thisc)
         {
            thist = thisc->tlist;
            while(thist)
            {
               newlist->thread_mapping[i] = thist->cs->instance;

               i++;
               thist = thist->next;
            }
            thisc = thisc->next;
         }
         thiss = thiss->next;
      }
   }
   else
   {
      /* If no mapping, then use 1:1 */
      i = 0;
      while ( i < newlist->count )
      {
         newlist->thread_mapping[i] = i;
         i++;
      }
   }

   /* Now... that the mapping is known... set the instance number of the derived list */
   i = 0;
   while ( i <  newlist->count )
   {
      t = newlist->thread_mapping[i];
      newlist->derived[i].instance = t;
      i++;
   }

   if(NULL == (newlist->totals = (struct cpua_derived *)malloc(sizeof(struct cpua_derived))))
      return(NULL);

   if(NULL == (newlist->ANSI = (int *)malloc(sizeof(int) * newlist->count)))
      return(NULL);

   /* Initialize the ANSI array to all 0's */
   /* This could be done with a bzero() or something like that... */
   i = 0;
   while(i < newlist->count)
      newlist->ANSI[i++] = 0;


   return(newlist);
}


/* ========================================================================= */
int FreshenCPUList(struct cpu_list *c)
{
   struct cpustats *swapcpus;

   /* Check input */
   if (NULL == c)
      return(1);

   /* Swap pointers */
   swapcpus = c->thiscpus;
   c->thiscpus = c->lastcpus;
   c->lastcpus = swapcpus;

   /* Read in stats */
   if (NULL == (GetCPUStats(c->thiscpus)))
       return(1);

   /* Back at ya */
   return(0);
}

