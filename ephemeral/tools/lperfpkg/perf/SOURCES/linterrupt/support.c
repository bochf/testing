#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "support.h"


/* ========================================================================= */
char *mkstring(char *cpin)
{
   char *cpout;

   if (NULL == cpin)
      return(NULL);

   if ( 0 == cpin[0] )
      return(NULL);

   if ( NULL == ( cpout = (char *)malloc(strlen(cpin) + 1) ) )
   {
      fprintf(stderr, "ERROR: Failed to allocate memory for a simple string.\n");
      return(NULL);
   }

   strcpy(cpout, cpin);

   return(cpout);
}

/* ========================================================================= */
int is_a_number(char *str)
{
   if ( NULL == str )
      return(0);

   if ( 0 == str[0] )
      return(0);

   while( *str != 0 )
   {
      if (( *str < '0' ) || ( *str > '9' ))
         return(0);

      str++;
   }

   return(1);
}


