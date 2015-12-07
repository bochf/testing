#ifndef DATATREE_H
#define DATATREE_H

#define DI_TYPE_NODE   0
#define DI_TYPE_BRANCH 1

struct dataitem
{
   char *label;
   int type;
   struct dataitem *sublist;
   long (*datasrc)(char *, unsigned long);
   struct dataitem *next;
};

/* =========================================================================
 * Name: 
 * Description: 
 * Paramaters: 
 * Returns: 
 * Side Effects: 
 * Notes: 
 */
struct dataitem *NewDataItem(char *label, long (*fptr)(char *, unsigned long));

/* =========================================================================
 * Name: 
 * Description: 
 * Paramaters: 
 * Returns: 
 * Side Effects: 
 * Notes: 
 */
struct dataitem *NewDataBranch(char *label,
                               struct dataitem *sublist,
                               long (*fptr)(char *, unsigned long));


#endif
