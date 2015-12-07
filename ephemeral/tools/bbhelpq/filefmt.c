#include <stdio.h>
#include <string.h>
#include "filefmt.h"

/* ========================================================================= */
int DumpFileTemplate(void)
{
   printf("%s \n", NAME_LINE_KEY);
   printf("%s \n", DESC_LINE_KEY);
   printf("%s \n", KEYS_LINE_KEY);
   printf("%s \n", LOCN_LINE_KEY);
   printf("%s \n", PFRM_LINE_KEY);
   printf("%s \n", IMPT_LINE_KEY);
   printf("%s \n", OWNR_LINE_KEY);

   return(0);
}


/* ========================================================================= */
int ParseImpact(char *impactstr)
{
   char IMPACTSTR[32];
   int i;

   if ( NULL == impactstr )
      return(IMPACT_FAIL); /* Input must account for this, even if it does
                              not have any impact */

   /* All whitespace is stripped. so the string is allowed to be empty */
   if ( impactstr[0] == 0 )
      return(IMPACT_NONE);

   i = 0;
   while ( impactstr[i] != 0 )
   {
      if (( impactstr[i] >= 'a' ) && ( impactstr[i] <= 'z' ))
         IMPACTSTR[i] = impactstr[i] - 32;
      else
         IMPACTSTR[i] = impactstr[i];

      i++;

      if ( i == 31 )
      {
         IMPACTSTR[i] = 0;
         break;
      }
   }

   IMPACTSTR[i] = 0;

   /* Now just do some brute force string compares */
   /* The input string is (now) in all upper case */
   if ( 0 == strcmp(IMPACTSTR, "MED") )
      return(IMPACT_MEDIUM);

   if ( 0 == strcmp(IMPACTSTR, "MEDIUM") )
      return(IMPACT_MEDIUM);

   if ( 0 == strcmp(IMPACTSTR, "LOW") )
      return(IMPACT_LOW);

   if ( 0 == strcmp(IMPACTSTR, "HIGH") )
      return(IMPACT_HIGH);

   if ( 0 == strcmp(IMPACTSTR, "ZERO") )
      return(IMPACT_NONE);

   if ( 0 == strcmp(IMPACTSTR, "NONE") )
      return(IMPACT_NONE);

   if ( 0 == strcmp(IMPACTSTR, "NO") )
      return(IMPACT_NONE);

   /* Nothing matched - report failure */
   return(IMPACT_FAIL);
}

