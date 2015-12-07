/* The following line is for Solaris & gcc only */
#define _POSIX_C_SOURCE 200112L
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <assert.h>

#include "options.h"
#include "mainhelp.h"

/* These functions still print to stderr. This is ok. There is no change that
   the prgram will have fork()ed off at this point. */


/* ========================================================================= */
int append_conf_arg(struct options *o, char *arg)
{
   struct conf_args *thisca;
   struct conf_args *lastca;
   struct conf_args *new_ca;

   assert(o != NULL);

   /* This really should never happen - consider it an error */
   if ( NULL == arg )
      return(1);

   /* Same with this */
   if ( 0 == arg[0] )
      return(1);

   if(NULL == (new_ca = (struct conf_args *)malloc(sizeof(struct conf_args))))
      return(1);

   if(NULL == (new_ca->argv = mkstring(arg)))
      return(1);

   /* This will go to end, so terminate the list */
   new_ca->next = NULL;

   if ( o->calist == NULL )
   {
      /* Insert top of list */
      o->calist = new_ca;
      return(0);
   }

   lastca = NULL; /* Unnessary */
   thisca = o->calist;
   while( thisca )
   {
      lastca = thisca;
      thisca = thisca->next;
   }

   assert( lastca != NULL ); /* Something went horribly wrong */

   /* Append to end */
   lastca->next = new_ca;
   return(0);
}


/* ========================================================================= */
void set_opt_defaults(struct options *o)
{
   o->bAbout = 0;
   o->bHelp = 0;
   o->bError = 0;
   o->bVerbose = 0;
   o->bListProv = 0;
   o->bListAllD = 0;
   o->bListPD = 0;
   o->bDanger = 0;
   o->bListTypes = 0;

   o->provider = NULL;
   o->confile = NULL;

   o->calist = NULL;
}

/* ========================================================================= */
struct options *ReadOptions(int argc, char *argv[])
{
   struct options *o;
   int index;
   int c;

   if ( NULL == (o = (struct options *)malloc(sizeof(struct options))))
   {
      fprintf(stderr, "ERROR: Unable to allocate memory. Exiting.\n");
      return(NULL);
   }

   set_opt_defaults(o);

   while ( -1 != ( c = getopt(argc, argv, ":ad:Dhlptv" ) ) )
   {
      switch(c)
      {
      case 'a':
         o->bAbout = 1;
         break;
      case 'd':
         o->bListPD = 1;
         if ( NULL == (o->provider = mkstring(optarg)) )
            return(NULL);
         break;
      case 'D':
         o->bDanger = 1;
         break;
      case 'h':
         o->bHelp = 1;
         break;
      case 'l':
         o->bListAllD = 1;
         break;
      case 'p':
         o->bListProv = 1;
         break;
      case 't':
         o->bListTypes = 1;
         break;
      case 'v':
         o->bVerbose++;
         break;  

      case ':': /* User forgot required argument */
         /* NOTE: I kind of like the idea of skipping the argument means to
            list all. (Speaking specifically of the list data items for a 
            provider or all providers.) That could be done here, but it would
            be "non-standard" so I am not doing it. Sooo... this becomes an
            error, not an oppurtunity to enhance usability. */
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

   /* Read the non-flag options (the config file) */
   index = optind;
   while ( index < argc )
   {
      /* What is going on:
         - Save every post getopt() arg as an arg list.
         - Consider the first as the config file
         - Save the first (also) as $ARG0 in the conf_args (o->calist)
         - Save the remainding as $ARGx in the conf_args (o->calist)
      */

      if (append_conf_arg(o, argv[index]))
      {
         /* malloc() failure or some extreme event is how we got here */
         fprintf(stderr, "ERROR: Failed to save config file argument.\n");
         return(NULL);
      }

      if ( NULL == o->confile )
      {
         if (NULL == (o->confile = mkstring(argv[index])))
            return(NULL);
      }

      index++;
   }

   /* No additional processing required if any of these are set */
   if (( o->bAbout ) || ( o->bHelp ) || ( o->bError ))
      return(o);
   

   /* Any *obvious* errors can be checked here. But.... because the command-
      line options are so light - there really is not much to check. Noting
      the possiblity here - but also noting the absence of checks. */

   return(o);
}




/* ========================================================================= */
char *GetConfArgument(struct options *o, int argn)
{
   struct conf_args *thisca;
   int argc;

   assert(o != NULL);
   
   /* Out of bounds check */
   if ( argn < 0 )
      return(NULL);

   thisca = o->calist;

   /* Walk the list until our index matches */
   argc = 0;
   while ( thisca )
   {
      if ( argn == argc )
      {
         return(thisca->argv);
      }
      
      thisca = thisca->next;
      argc++;
   }

   /* Item not found */
   return(NULL);
}
