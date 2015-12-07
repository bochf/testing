#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <assert.h>

#include "options.h"
#include "support.h"
#include "defaults.h"

/* ========================================================================= */
void set_opt_defaults(struct options *o)
{
   o->bAbout = 0;
   o->bHelp = 0;
   o->bForeground = 0;
   o->bError = 0;
   o->config_file = NULL;
   o->url = NULL;
   o->dump_file = NULL;

   o->bDebug = 0;

   o->cf_host = NULL;
   o->cf_port = NULL;
   o->cf_hst = DEFAULT_HTTPD_THREADS;
   o->cf_obufsz = DEFAULT_OUTPUT_BUFFER_SIZE;
   o->cf_ibufsz = DEFAULT_INPUT_BUFFER_SIZE;
   o->cf_msgfile = NULL;
   o->cf_accessfile = NULL;
   o->cf_qcfiledir = NULL;
}

/* ========================================================================= */
struct options *ReadOptions(int argc, char *argv[])
{
   struct options *o;
   int index;
   int c;

   if ( NULL == (o = (struct options *)malloc(sizeof(struct options))))
   {
      error_msg("ERROR: Unable to allocate memory. Exiting.");
      return(NULL);
   }

   set_opt_defaults(o);

   while ( -1 != ( c = getopt(argc, argv, ":+aFf:ho:u:" ) ) )
   {
      switch(c)
      {
      case '+':
         o->bDebug = 1;
         break;
      case 'a':
         o->bAbout = 1;
         break;
      case 'h':
         o->bHelp = 1;
         break;
      case 'F':
         o->bForeground = 1;
         break;

      case 'f':
         if ( NULL != o->config_file )
         {
            error_msg("ERROR: The -f option was specified more than once.");
            return(NULL);
         }
         if ( NULL == (o->config_file = mkstring(optarg)) )
         {
            error_msg("ERROR: Unable to allocate memory for simple string.");
            return(NULL);
         }
         break;

      case 'u':
         if ( NULL != o->url )
         {
            error_msg("ERROR: The -u option was specified more than once.");
            return(NULL);
         }
         if ( NULL == (o->url = mkstring(optarg)) )
         {
            error_msg("ERROR: Unable to allocate memory for simple string.");
            return(NULL);
         }
         break;

      case 'o':
         if ( NULL != o->dump_file )
         {
            error_msg("ERROR: The -o option was specified more than once.");
            return(NULL);
         }
         if ( NULL == (o->dump_file = mkstring(optarg)) )
         {
            error_msg("ERROR: Unable to allocate memory for simple string.");
            return(NULL);
         }
         break;

     case ':': /* User forgot required argument */
         o->bError = 1;
         error_msg("ERROR: Missing argument to the \"-%c\" option.", optopt);
         break;

      case '?': /* User entered some unknown/unsupported argument */
         o->bError = 1;
         if (isprint (optopt))
            error_msg("ERROR: Unknown option \"-%c\".", optopt);
         else
            error_msg("ERROR: Unknown option character `\\x%x'.",
                      optopt);
         break;
      default: /* Really an unreachable place */
         o->bError = 1;
         return(o);
      }
   }

   /* Read the non-flag options (If there are any... ) */
   index = optind;
   if ( index < argc )
   {
      error_msg("ERROR: Encountered this (extra) unhandled option: %s", argv[index]);
      return(NULL);
   }

   /* Start validating user input */
   if ( o->bError )
   {
      error_msg("ERROR: Problems parsing command line input.");
      return(NULL);
   }

   if (( o->bAbout ) || ( o->bHelp ))
   {
      if ( o->config_file ) 
      {
         error_msg("NOTE: The -a and -h options are not compatible with other options.");
      }

      /* No additional processing required if any of these are set */
      return(o);
   }

   return(o);
}

/* ========================================================================= */
int DbgDumpConfig(struct options *o)
{
   printf("==Config Options=====================================\n");
   printf("  Show about (-a)    : %s\n", o->bAbout ? "true" : "false");
   printf("  Show help  (-h)    : %s\n", o->bHelp  ? "true" : "false");
   printf("  Show help  (-F)    : %s\n", o->bForeground ? "true" : "false");
   printf("  Error flag         : %s\n", o->bError ? "true" : "false");
   printf("  Config file (-f)   : %s\n", o->config_file ? o->config_file : "N/A");
   printf("  Query \"URL\" (-u)   : %s\n", o->url ? o->url : "N/A");
   printf("  Dump file (-o)     : %s\n", o->dump_file ? o->dump_file : "N/A");
   printf("  Listen addr (CF)   : %s\n", o->cf_host ? o->cf_host : "N/A");
   printf("  Listen port (CF)   : %s\n", o->cf_port ? o->cf_port : "N/A");
   printf("  HTTPd threads (CF) : %d\n", o->cf_hst);
   printf("  Output buf sz (CF) : %lu\n", o->cf_obufsz);
   printf("  Input buf sz (CF)  : %lu\n", o->cf_ibufsz);
   printf("  Message file (CF)  : %s\n", o->cf_msgfile);
   printf("  Access file (CF)   : %s\n", o->cf_accessfile);
   printf("  QC file dir (CF)   : %s\n", o->cf_qcfiledir);
   printf("=====================================================\n");
   fflush(stdout);
   return(0);
}
