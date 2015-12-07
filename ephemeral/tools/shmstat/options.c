#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <assert.h>

#include "options.h"

/* ========================================================================= */
/* A function to create strings */
char *mkstring(const char *stub)
{
   char *rv;

   if (NULL == stub)
      return(NULL);

   if (NULL == (rv = (char *)malloc( 1 + strlen(stub) )))
      return(NULL);

   strcpy(rv, stub);

   return(rv);
}

/* ========================================================================= */
void set_opt_defaults(struct options *o)
{
   o->bAbout = 0;
   o->bHelp = 0;
   o->bError = 0;
   o->bPerUser = 0;
   o->bSizeGph = 0;
}


/* ========================================================================= */
struct options *ReadOptions(int argc, char *argv[])
{
   struct options *o;
   int c;

   if ( NULL == (o = (struct options *)malloc(sizeof(struct options))))
   {
      fprintf(stderr, "ERROR: Unable to allocate memory. Exiting.\n");
      return(NULL);
   }

   set_opt_defaults(o);

   while ( -1 != ( c = getopt(argc, argv, "ahsu" ) ) )
   {
      switch(c)
      {
      case 'a':
         o->bAbout = 1;
         break;
      case 'h':
         o->bHelp = 1;
         break;
      case 's':
         o->bSizeGph = 1;
         break;
      case 'u':
         o->bPerUser = 1;
         break;
      case '?':
         if (optopt == 't')
            fprintf (stderr, "ERROR: Option -%c requires an argument.\n", optopt);
         else if (isprint (optopt))
            fprintf (stderr, "ERROR: Unknown option `-%c'.\n", optopt);
         else
            fprintf (stderr,
                     "ERROR: Unknown option character `\\x%x'.\n",
                     optopt);
         /* Fall through */
      default:
         o->bError = 1;
         return(o);
      }
   }

   /* No additional processing required if any of these are set */
   if (( o->bAbout ) || ( o->bHelp ) || ( o->bError ))
      return(o);
   




   return(o);
}
