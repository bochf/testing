#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>



#include "options.h"

/* ========================================================================= */
char *make_string(char *cpin)
{
   char *cp;

   if ( NULL == cpin )
      return(NULL);

   if (NULL == (cp = (char *)malloc(strlen(cpin) + 1)))
      return(NULL);

   strcpy(cp, cpin);

   return(cp);
}

/* ========================================================================= */
struct options *init_options(void)
{
   struct options *o;

   if (NULL == (o = (struct options *)malloc(sizeof(struct options))))
      return(NULL);

   o->opt_error = 0;
   o->opt_help = 0;
   o->opt_ufilter = 0;
   o->ufilter_name = NULL;
   o->opt_action = ACTION_DFLT;
   o->opt_rake = 0;
   o->opt_verbose = 0;
   o->kill_sig = 0;

   o->opt_debug = 0;

   return(o);
}

/* ========================================================================= */
int validate_options(struct options *o)
{
   /* As a rule here... Do not try to "fix" errors. Any error in the input
      is a HARD FAIL ERROR. */

   /* This should probably just be an assert() - This is checked elsewhere,
      so no message. */
   if ( NULL == o )
      return(1);

   /* Error message handled at point of error - this just gets us out of here */
   if ( o->opt_error )
      return(1);

   if ( NULL != o->ufilter_name )
   {
      if ( 0 == strcmp(o->ufilter_name, "root") )
      {
         fprintf(stderr, "ERROR: Unable to filter on user \"root\". Giving up on safety concerns.\n");
         return(1);
      }   

      if ( 0 == strcmp(o->ufilter_name, "op") )
      {
         fprintf(stderr, "ERROR: Unable to filter on user \"op\". Giving up on safety concerns.\n");
         return(1);
      }   
   }

   /* STUB: Test for contradictory -l and -k (gotta add something) 
            Try this --> have it unset initially, then set default if it
            is not explicitly set. Then you can check for non-zero before
            setting it. */

   /* STUB: Test for -h and anything else */

   /* STUB: Test for -a(bout) and anything else */

   /* STUB: Test for opt_error */

   /* STUB: Test for invalid signals */

   return(0);
}

/* ========================================================================= */
struct options *ParseOptions(int argc, char *argv[])
{
   struct options *o;
   int c;

   if (NULL == (o = init_options()))
      return(o);


   while ((c = getopt(argc, argv, "+hk:lru:v")) != EOF)
   {
      switch (c)
      {
      case '+':
         o->opt_debug = 1;
         o->opt_verbose = 1;
         break;
      case 'h':
         o->opt_help = 1;
         break;
      case 'l':
         o->opt_action = ACTION_LIST;
         break;
      case 'k':
         if ((optarg[0] > '0') && (optarg[0] <= '9'))
         {
            o->opt_action = ACTION_KILL;
            o->kill_sig = atoi(optarg);
         }
         else
         {
            fprintf(stderr, "ERROR: Unable to parse -k option of \"%s\".\n", optarg);
            o->opt_error = 1;
         }
         break;
      case 'r':
         o->opt_rake = 1;
         break;
      case 'u':
         if ( optarg[0] == '-' )
         {
            o->opt_error = 1;
            fprintf(stderr, "ERROR: Unable to parse -u user name \"%s\".\n", optarg);
         }
         else
         {
            if (NULL == (o->ufilter_name = make_string(optarg)))
               o->opt_error = 1;
            else
               o->opt_ufilter = 1;
         }
         break;
      case 'v':
         o->opt_verbose = 1;
         break;
      case '?':
         o->opt_error = 1;
      }
   }

   /* No memory clean up (we will exit soon anyways). The called function will
      handle the error message (if appropriate). */
   if (validate_options(o))
      return(NULL);

   return(o);
}
