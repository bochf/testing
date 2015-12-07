#include <assert.h>
#include <string.h>
#include <procinfo.h>
#include <sys/types.h>

#include "process.h"
#include "../support.h"

struct procentry64 *proc = NULL;

/* ========================================================================= */
struct procitem *InitProcessEntry(struct procitem *p)
{

   if ( NULL == p )
   {
      if ( NULL == ( p = (struct procitem *)malloc(sizeof(struct procitem)) ) )
      {
         error_msg("ERROR: Failed to allocate memory for process list."); 
         return(NULL);
      }
   }

   if ( NULL == proc )
   {
      if ( NULL == ( proc = (struct procentry64 *)malloc(sizeof(struct procentry64)) ) )
      {
         error_msg("ERROR: Failed to allocate memory for process list."); 
         return(NULL);
      }
   }

   p->pid = 0; /* First request should be 0 */

   return(p);
}

/* ========================================================================= */
int StartProcessEntry(struct procitem *p)
{
   assert ( NULL != p );

   /* Used specifically by this implementation */
   p->index = 0;

   return(0);
}

/* ========================================================================= */
int GetProcessEntry(struct procitem *p)
{
   assert ( NULL != p );

   /* Why while()?
      Because getprocs64() will return pid=0 as a valid process. This is
      designed to work through that. */
   while ( 1 == getprocs64 (proc, sizeof(struct procentry64), NULL, 0, &p->index, 1) )
   {
      /* Go back for more if getprocs64() returned PID 0 */
      if ( 0 != proc->pi_pid )
      {
         /* Copy over (translate) all the values */
         p->pid = proc->pi_pid;
         strcpy(p->pname, proc->pi_comm);
         p->K_size = proc->pi_size * 4; /* Assuming 4K page */
         p->K_rss = proc->pi_drss * 4;  /* No mention of units (it is pages) */
         p->stime = proc->pi_start;
      
         return(1); /* We got a process, return that "count" */
      }
   }

   return(0);
}


