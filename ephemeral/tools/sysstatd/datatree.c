#include <stdio.h>
#include <stdlib.h>

#include "support.h"
#include "datatree.h"

/* ========================================================================= */
struct dataitem *NewDataItem(char *label, long (*fptr)(char *, unsigned long))
{
   struct dataitem *ndi;

   if ( NULL == (ndi = (struct dataitem *)malloc(sizeof(struct dataitem))) )
   {
      error_msg("ERROR: Unable to allocate memory for data item.");
      return(NULL);
   }

   ndi->type = DI_TYPE_NODE;
   ndi->label = mkstring(label);
   ndi->datasrc = fptr;
   ndi->sublist = NULL;

   ndi->next = NULL;

   return(ndi);
}


/* ========================================================================= */
struct dataitem *NewDataBranch(char *label,
                               struct dataitem *sublist,
                               long (*fptr)(char *, unsigned long))
{
   struct dataitem *ndi;

   if ( NULL == (ndi = (struct dataitem *)malloc(sizeof(struct dataitem))) )
   {
      error_msg("ERROR: Unable to allocate memory for data item.");
      return(NULL);
   }

   ndi->type = DI_TYPE_BRANCH;
   ndi->label = mkstring(label);
   ndi->sublist = sublist;
   ndi->datasrc = fptr;
   ndi->next = NULL;

   return(ndi);
}
