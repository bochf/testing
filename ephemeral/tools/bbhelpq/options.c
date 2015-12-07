#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "filefmt.h"
#include "ssearch.h"
#include "options.h"

#include "version.h"

/* ========================================================================= */
struct options *init_options(void)
{
   struct options *o;
   char *bbhelpdir;

   if ( NULL == ( o = (struct options *)malloc(sizeof(struct options))) )
   {
      fprintf(stderr, "ERROR: Unable to allocate memory for options info.\n");
      return(NULL);
   }

   o->bDumpTemplate = 0;
   o->bVersion = 0;
   o->bCompile = 0;
   o->bDecompile = 0;
   o->iHelp = 0;
   o->bDebug = 0;
   o->bError = 0;
   o->iImpact = IMPACT_UNSET;
   o->bForceSrc = 0;
   o->qList = LIST_NONE;
   o->tooldump = NULL;
   o->keycons = NULL;

   if ( NULL == ( bbhelpdir = getenv("BBHELPDIR") ) )
      o->bbhelpdir = MakeString(DEFAULT_BBHELPDIR);
   else
      o->bbhelpdir = MakeString(bbhelpdir);

   o->systype = MakeUCString(DEFAULT_SYSTYPE);

   return(o);
}

/* ========================================================================= */
int parse_list(char *arg)
{
   char ucarg[16];
   int i;

   i = 0;
   while ( i < 16 )
   {
      if ( arg[i] == 0 )
      {
         ucarg[i] = 0;
         break;
      }
         
      if (( arg[i] >= 'a' ) && ( arg[i] <= 'z' ))
         ucarg[i] = arg[i] - 32;
      else
         ucarg[i] = arg[i];

      i++;
   }
   ucarg[15] = 0; /* NOthing should be out here - so just terminate */

   if ( 0 == strcmp(ucarg, "NAME"))
        return(LIST_NAME);

   if ( 0 == strcmp(ucarg, "KEYS"))
        return(LIST_KEYS);

   if ( 0 == strcmp(ucarg, "PLATFORM"))
        return(LIST_PLFM);

   if ( 0 == strcmp(ucarg, "DESC"))
        return(LIST_DESC);

   if ( 0 == strcmp(ucarg, "DESCRIPTION"))
        return(LIST_DESC);

   return(LIST_PERR);
}

/* ========================================================================= */
struct options *ReadOptions(int argc, char *argv[])
{
   struct options *o;
   int opt;

   if ( NULL == ( o = init_options() ) )
      return(NULL);

#ifdef ALLOW_DECOMPILE
   while (( opt = getopt(argc, argv, "+cd:hi:l:k:nop:rt:v")) != -1 )
#else
   while (( opt = getopt(argc, argv, "+cd:hi:l:k:np:rt:v")) != -1 )
#endif
   {
      switch(opt)
      {
      case 'c':
         o->bCompile = 1;
         break;
      case 'd':
         if (o->bbhelpdir) free(o->bbhelpdir);
         o->bbhelpdir = MakeString(optarg);
         break;
      case '+':
         fprintf(stderr, "DEBUG: Enabling the debug flag.\n");
         o->bDebug = 1;
         break;
      case 'h':
         o->iHelp++;
         break;
      case 'i':
         if (IMPACT_FAIL == (o->iImpact = ParseImpact(optarg)))
            o->bError = 1;
         break;
      case 'k':
         if (o->keycons) free(o->keycons); /* Error condition - Only one allowed */
         o->keycons = MakeUCString(optarg);
         break;
      case 'l':
         o->qList = parse_list(optarg);
         break;
      case 'n':
         o->bDumpTemplate = 1;
         break;
#ifdef ALLOW_DECOMPILE
      case 'o':
         o->bDecompile = 1;
         break;
#endif
      case 'p':
         if (o->systype) free(o->systype);
         o->systype = MakeUCString(optarg);
         break;
      case 'r':
         o->bForceSrc = 1;
         break;
      case 't':
         if (o->tooldump) free(o->tooldump); /* Error condition - Only one allowed */
         o->tooldump = MakeString(optarg);
         break;
      case 'v':
         o->bVersion = 1;
         break;
         
      default:
         o->bError = 1;
         break;
      }
   }

   /* Make sure that bbhelpdir does not end in /. This is about consistency.
      The / will be added back later. */
   if ( o->bbhelpdir[strlen(o->bbhelpdir) - 1] == '/' )
      o->bbhelpdir[strlen(o->bbhelpdir) - 1] = 0;

   /*** Check options ***/
   if (( o->bVersion ) || ( o->iHelp ))
   {
      /* Just exit now, no additional checking */
      return(o);
   }

   if ( o->bCompile && o->bForceSrc )
   {
      /* Compile is verbose anyways - so just use stdout */
      printf("WARNING: -c and -r are incompatible. Ignoring -r option.\n");
      o->bForceSrc = 0;
   }

   if ( o->bDecompile )
   {
      if (( o->bCompile ) ||
          ( o->bForceSrc ) ||
          ( o->qList ) ||
          ( o->tooldump ) ||
          ( o->keycons ) ||
          ( o->bDumpTemplate ))
      {
         fprintf(stderr, "ERROR: -o and other options are mutually exclusive.\n");
         return(NULL); /* Stop checking, force exit */
      }
   }

   if ( o->bDumpTemplate )
   {
      if (( o->bCompile ) ||
          ( o->bForceSrc ) ||
          ( o->qList ) ||
          ( o->tooldump ) ||
          ( o->keycons ) )
      {
         fprintf(stderr, "ERROR: -n and other options are mutually exclusive.\n");
         return(NULL); /* Stop checking, force exit */
      }
   }

   if ( o->tooldump )
   {
      if (( o->qList ) || ( o->keycons ))
      {
         fprintf(stderr, "ERROR: The -t option is incompatible with the -k and -l options.\n");
         return(NULL); /* Stop checking, force exit */
      }
   }

   if ( o->keycons )
   {
      /* I was expecting different messages here - but they read the same */
      if ( o->bCompile )
      {
         fprintf(stderr, "ERROR: The -k option is incompatible with the -c option.\n");
         return(NULL); /* Stop checking, force exit */
      }

      if ( 0 == o->qList )
      {
         fprintf(stderr, "ERROR: The -k option can only be used with the -l option.\n");
         return(NULL); /* Stop checking, force exit */
      }
   }

   if ( LIST_PERR == o->qList )
   {
      fprintf(stderr, "ERROR: The sub-param to the -l option was not recognized.\n");
      return(NULL); /* Stop checking, force exit */
   }

   if ( o->bDebug )
   {
      fprintf(stderr, "DEBUG: Option c(ompile) %s\n", o->bCompile ? "On" : "Off");
      fprintf(stderr, "DEBUG: Option h(elp) %d\n", o->iHelp);
      fprintf(stderr, "DEBUG: Option v(ersion) %s\n", o->bVersion ? "On" : "Off");
      fprintf(stderr, "DEBUG: Option n(ew file) %s\n", o->bDumpTemplate ? "On" : "Off");
      fprintf(stderr, "DEBUG: Option r(ead forced) %s\n", o->bForceSrc ? "On" : "Off");
      fprintf(stderr, "DEBUG: helpdir = %s\n", o->bbhelpdir);
   }

   return(o);
}
