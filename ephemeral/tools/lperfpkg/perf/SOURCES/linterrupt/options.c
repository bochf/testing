#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <assert.h>

#include "options.h"
#include "support.h"

/* ========================================================================= */
void set_opt_defaults(struct options *o)
{
   o->bAbout = 0;
   o->bHelp = 0;
   o->bError = 0;
   o->bTimestamp = 0;
   o->bHeader = 1;
   o->bCollapse = 0;
   o->bMonochrome = 0;
   o->bPivot = 0;
   o->bNames = 1;
   o->bShortNames = 0;
   o->bSkipLOC = 0;
   o->bCPUStats = 0;

   o->bDebug = 0;

   o->interval = DEFAULT_INTERVAL;
}

/* ========================================================================= */
struct options *ReadOptions(int argc, char *argv[])
{
   struct options *o;
   int index;
   int c;
   int postopts = 0;

   if ( NULL == (o = (struct options *)malloc(sizeof(struct options))))
   {
      fprintf(stderr, "ERROR: Unable to allocate memory. Exiting.\n");
      return(NULL);
   }

   set_opt_defaults(o);

   while ( -1 != ( c = getopt(argc, argv, ":+achHLmnstuv" ) ) )
   {
      switch(c)
      {
      case '+':
         o->bDebug = 1;
         break;
      case 'a':
         o->bAbout = 1;
         break;
      case 'c':
         o->bCPUStats = 1;
         break;
      case 'h':
         o->bHelp = 1;
         break;
      case 'H':
         o->bHeader = 0;
         break;
      case 'L':
         o->bSkipLOC = 1;
         break;
      case 'm':
         o->bMonochrome = 1;
         break;
      case 'n':
         o->bNames = 0;
         break;
      case 's':
         o->bShortNames = 1;
         break;
      case 't':
         o->bTimestamp = 1;
         break;
      case 'u':
         o->bCollapse = 1;
         break;
      case ':': /* User forgot required argument */
         o->bError = 1;
         fprintf (stderr, "ERROR: Missing the argument to the \"-%c\" option.\n", optopt);
         break;

      case '?': /* User entered some unknown/unsupported argument */
         o->bError = 1;
         if (isprint (optopt))
            fprintf (stderr, "ERROR: Unknown option \"-%c\".\n", optopt);
         else
            fprintf (stderr,
                     "ERROR: Unknown option character `\\x%x'.\n",
                     optopt);
         break;
      default: /* Really an unreachable place */
         o->bError = 1;
         return(o);
      }
   }

   /* Read the non-flag options (the interval period) */
   index = optind;
   while ( index < argc )
   {
      if ( postopts )
      {
         printf("Encountered this (extra) unhandled option: %s\n", argv[index]);
         return(NULL);
      }

      if ( is_a_number(argv[index]) )
      {
         o->interval = atoi(argv[index]);
         postopts++;
      }
      else
      {
         printf("Option \"%s\" is not understood.\n", argv[index]);
         return(NULL);
      }

      index++;
   }

   /* Start validating user input */
   if ( o->bError )
   {
      fprintf(stderr, "ERROR: Problems parsing command line input.\n");
      return(NULL);
   }

   if (( o->bAbout ) || ( o->bHelp ))
   {
      if (( o->bTimestamp ) || ( o->bCollapse ) || ( o->bMonochrome ) ||
          ( 0 == o->bNames) || ( o->bSkipLOC )  || ( o->bShortNames ) ||
          ( o->bCPUStats )  || ( o->bPivot )    || ( 0 == o->bHeader ))
      {
         fprintf(stderr, "NOTE: The -a and -h options are not compatible with other options.\n");
      }

      /* No additional processing required if any of these are set */
      return(o);
   }

   if ( o->bPivot )
   {
      fprintf(stderr, "WARNING: The -p option is not implemented at this time.\n");
   }
   
   if ( o->interval < 1 )
   {
      fprintf(stderr, "ERROR: Invalid value for the sampling interval.\n");
      return(NULL);
   }

   return(o);
}


