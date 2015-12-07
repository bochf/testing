#include <stdlib.h>
#include <string.h>

#include "ssearch.h"

/* ========================================================================= */
char *MakeString(char *cpin)
{
   char *cptr;

   if ( NULL == cpin )
      return(NULL);

   if ( NULL == ( cptr = (char *)malloc(strlen(cpin) + 1) ) )
      return(NULL);

   strcpy(cptr, cpin);

   return(cptr);
}

/* ========================================================================= */
/* Used for case insensitive compares - all compares are in upper case */
char *MakeUCString(char *cpin)
{
   char *cptr;
   int i;

   if ( NULL == cpin )
      return(NULL);

   if ( NULL == ( cptr = (char *)malloc(strlen(cpin) + 1) ) )
      return(NULL);

   i = 0;
   while ( cpin[i] != 0 )
   {
      if ( ( cpin[i] >= 'a' ) && ( cpin[i] <= 'z' ) )
         cptr[i] = cpin[i] - 32;
      else
         cptr[i] = cpin[i];

      i++;
   }

   cptr[i] = 0;

   return(cptr);
}

/* ========================================================================= */
char *ConvertToLC(char *cpin)
{
   int i;

   if (NULL == cpin)
      return(NULL);

   i = 0;
   while(cpin[i] != 0)
   {
      if (( cpin[i] >= 'A' ) && ( cpin[i] <= 'Z' ))
         cpin[i] += 32;

      i++;
   }

   return(cpin);
}

/* ========================================================================= */
char *ConvertToUC(char *cpin)
{
   int i;

   if (NULL == cpin)
      return(NULL);

   i = 0;
   while(cpin[i] != 0)
   {
      if (( cpin[i] >= 'a' ) && ( cpin[i] <= 'z' ))
         cpin[i] -= 32;

      i++;
   }

   return(cpin);
}

