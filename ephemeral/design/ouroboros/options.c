#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <assert.h>

#include "options.h"

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
void set_opt_defaults(struct options *o)
{
   o->bAbout = 0;
   o->bHelp = 0;
   o->bError = 0;

   o->source = NULL;
   o->dest = NULL;

   o->bDebug = 0;

   o->buf_rsize = DEFAULT_BUFFER_OBJECTS;
   o->buf_bsize = DEFAULT_BUFFER_SIZE;
   o->direct = DIRECT_NONE;
   o->bStats = 0;
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

   while ( -1 != ( c = getopt(argc, argv, ":+ab:d:ho:s" ) ) )
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
      case 'd':
         if ((( optarg[0] == 'r' ) || ( optarg[0] == 'R' )) && (optarg[1] == 0))
            o->direct = DIRECT_READ;
         
         if ((( optarg[0] == 'w' ) || ( optarg[0] == 'W' )) && (optarg[1] == 0))
            o->direct = DIRECT_WRITE;

         if ((( optarg[0] == 'r' ) || ( optarg[0] == 'R' )) && 
             (( optarg[1] == 'w' ) || ( optarg[1] == 'W' )) && 
             (optarg[2] == 0))
            o->direct = DIRECT_ALL;

         if ( DIRECT_NONE == o->direct )
         {
            fprintf(stderr, "ERROR: Failed to parse -d option \"%s\".\n", optarg);
            o->bError = 1;
         }
         break;
      case 'o':
         o->buf_rsize = atoi(optarg);
         break;
      case 'b':
         o->buf_bsize = atol(optarg);
         break;
      case 's':
         o->bStats = 1;
         break;

     case ':': /* User forgot required argument */
         o->bError = 1;
         fprintf(stderr, "ERROR: Missing argument to the \"-%c\" option.\n", optopt);
         break;

      case '?': /* User entered some unknown/unsupported argument */
         o->bError = 1;
         if (isprint (optopt))
            fprintf(stderr, "ERROR: Unknown option \"-%c\".\n", optopt);
         else
            fprintf(stderr, "ERROR: Unknown option character `\\x%x'.\n",
                      optopt);
         break;
      default: /* Really an unreachable place */
         o->bError = 1;
         return(o);
      }
   }

   /* Read the non-flag options (If there are any... ) */
   index = optind;
   while ( index < argc )
   {
      if ( NULL == o->source )
      {
         if ( NULL == (o->source = mkstring(argv[index])) )
         {
            fprintf(stderr, "ERROR: Failed to create source filename string.\n");
            return(NULL);
         }
      }
      else
      {
         if ( NULL == o->dest )
         {
            if ( NULL == (o->dest = mkstring(argv[index])) )
            {
               fprintf(stderr, "ERROR: Failed to create destination filename string.\n");
               return(NULL);
            }
         }
         else
         {
            fprintf(stderr, "ERROR: Additional argument not understood.\n");
            return(NULL);
         }
      }

      index++;
   }

   /* Start validating user input */
   if ( o->bError )
   {
      /* Error has already been printed */
      return(NULL);
   }

   if (( o->bAbout ) || ( o->bHelp ))
   {
      if (( o->source ) || ( o->dest ))
      {
         fprintf(stderr, "NOTE: The -a and -h options are not compatible with other options.\n");
         return(NULL);
      }

      /* No additional processing required if any of these are set */
      return(o);
   }
   else
   {
      if (( NULL == o->source ) || ( NULL == o->dest ))
      {
         fprintf(stderr, "NOTE: Source and destination paths must be specified.\n");
         return(NULL);
      }
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

   printf("  Source file name   : %s\n", o->source);
   printf("  Dest file name     : %s\n", o->dest);

   printf("  Direct read        : %s\n", (o->direct & DIRECT_READ)  ? "true" : "false");
   printf("  Direct write       : %s\n", (o->direct & DIRECT_WRITE)  ? "true" : "false");
   printf("  Buffer objects     : %u\n", o->buf_rsize);
   printf("  Buffer size        : %ld\n", o->buf_bsize);
   printf("  Report buffer stats: %s\n", o->bStats ? "true" : "false");


#ifdef STUB_YOINK
   printf("  Dump file (-o)     : %s\n", o->dump_file ? o->dump_file : "N/A");
   printf("  Listen addr (CF)   : %s\n", o->cf_host ? o->cf_host : "N/A");
   printf("  Listen port (CF)   : %s\n", o->cf_port ? o->cf_port : "N/A");
   printf("  HTTPd threads (CF) : %d\n", o->cf_hst);
   printf("  Input buf sz (CF)  : %lu\n", o->cf_ibufsz);
   printf("  Message file (CF)  : %s\n", o->cf_msgfile);
   printf("  Access file (CF)   : %s\n", o->cf_accessfile);
   printf("  QC file dir (CF)   : %s\n", o->cf_qcfiledir);
#endif
   printf("=====================================================\n");
   fflush(stdout);
   return(0);
}
