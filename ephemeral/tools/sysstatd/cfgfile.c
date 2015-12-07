#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "cfgfile.h"
#include "support.h"
#include "options.h"
#include "defaults.h"

struct cfoptions *new_cfoptions(void);
int add_arg_line(struct cfoptions *cfo, char *line);
int insert_value(struct cfoptions *cfo, char *lhs, char *rhs);

/* Whatever you put on a config file line, it has to fit in this length. */
#define MAX_CLINE_LEN 1024

/* ========================================================================= */
struct cfoptions *ReadConfigFile(char *filename)
{
   FILE *f;
   struct cfoptions *cfo;
   char whole_line[MAX_CLINE_LEN + 1];
   char *line;
   int lineno;
   char errstr[80];

   if ( NULL == (cfo = new_cfoptions()) )
      return(NULL);

   if ( NULL == filename )
   {
      if ( NULL == (filename = mkstring(DEFAULT_CONFIG_FILE)) )
      {
         error_msg("ERROR: Unable to create a filename string.");
         return(NULL);
      }
   }

   if ( NULL == ( f = fopen(filename, "r") ) )
   {
      dotdotdot_string(errstr, filename, 40);
      error_msg("ERROR: Unable to open config file \"%s\".", errstr); 
      return(NULL);
   }

   lineno = 0;
   while(fgets(whole_line, MAX_CLINE_LEN, f))
   {
      lineno++;

      /* Remove leading white space */
      line = unlead(whole_line);

      /* chomp() - Remove EOL */
      chomp(line);

      /* clamp() - Remove trailing comments */
      clamp(line);

      /* wstrim() - Remove trailing spaces */
      wstrim(line);

      if ( line[0] == 0 )
         continue;

      if ( add_arg_line(cfo, line) )
      {
         /* Don't free memory - that is a waste of effort. */
         /* We are exiting soon - so who cares */
         /* Close the file - that is easy */
         fclose(f);
         return(NULL);
      }
   }

   fclose(f);

   return(cfo);
}

/* ========================================================================= */
int SaveCFileToOptions(struct options *o, struct cfoptions *cfo)
{
   int i;

   /* Handle LISTEN_ADDRESS */
   if ( NULL != cfo->listen_addr )
   {
      if ( (0 == strcmp(cfo->listen_addr, "0.0.0.0")) ||
           (0 == strcmp(cfo->listen_addr, "*"))       ||
           (0 == strcmp(cfo->listen_addr, "ALL")))
      {
         free(cfo->listen_addr);
         cfo->listen_addr = NULL;
         o->cf_host = NULL;
      }
      else
         o->cf_host = cfo->listen_addr;
   }
   else
      o->cf_host = cfo->listen_addr;

   /* Handle LISTEN_PORT */
   if ( NULL == cfo->listen_port )
   {
      if ( NULL == (cfo->listen_port = mkstring( DEFAULT_HTTPD_LISTEN_PORT)) )
      {
         error_msg("ERROR: Unable to allocate memory for default port.");
         return(1);
      }
   }
   
   o->cf_port = cfo->listen_port;

   /* Handle HTTPD_SERVER_THREADS */
   if ( NULL == cfo->httpd_server_threads )
   {
      o->cf_hst = DEFAULT_HTTPD_THREADS;
   }
   else
   {
      o->cf_hst = 0;
      i = 0;
      while ( cfo->httpd_server_threads[i] != 0 )
      {
         if ((cfo->httpd_server_threads[i] < '0')||(cfo->httpd_server_threads[i] < '0'))
         {
            error_msg("ERROR: Non-numeric data in HTTPD_SERVER_THREADS config file value.");
            return(1);
         }
         o->cf_hst *= 10;
         o->cf_hst += ( cfo->httpd_server_threads[i] - '0' );
         i++;
      }

      if ( o->cf_hst > 12 )
      {
         error_msg("ERROR: Value for HTTPD_SERVER_THREADS exceeds reasonable limit.");
         return(1);
      }
         
   }

   /* Handle OUTPUT_BUFFER_SIZE */
   if ( NULL == cfo->output_buffer_size )
   {
      o->cf_obufsz = DEFAULT_OUTPUT_BUFFER_SIZE;
   }
   else
   {
      o->cf_obufsz = 0;
      i = 0;
      while ( cfo->output_buffer_size[i] != 0 )
      {
         if ((cfo->output_buffer_size[i] < '0')||(cfo->output_buffer_size[i] < '0'))
         {
            error_msg("ERROR: Non-numeric data in OUTPUT_BUFFER_SIZE config file value.");
            return(1);
         }
         o->cf_obufsz *= 10;
         o->cf_obufsz += ( cfo->output_buffer_size[i] - '0' );
         i++;
      }
   }

   /* Handle INPUT_BUFFER_SIZE */
   if ( NULL == cfo->input_buffer_size )
   {
      o->cf_ibufsz = DEFAULT_INPUT_BUFFER_SIZE;
   }
   else
   {
      o->cf_ibufsz = 0;
      i = 0;
      while ( cfo->input_buffer_size[i] != 0 )
      {
         if ((cfo->input_buffer_size[i] < '0')||(cfo->input_buffer_size[i] < '0'))
         {
            error_msg("ERROR: Non-numeric data in INPUT_BUFFER_SIZE config file value.");
            return(1);
         }
         o->cf_ibufsz *= 10;
         o->cf_ibufsz += ( cfo->input_buffer_size[i] - '0' );
         i++;
      }
   }

   /* Handle QC_FILE_DIR */
   if ( NULL == cfo->qc_file_dir )
   {
      o->cf_qcfiledir = mkstring(DEFAULT_QC_FILE_DIR);
   }
   else
   {
      o->cf_qcfiledir = cfo->qc_file_dir;
   }

    /* Handle MESSAGE_FILE and ACCESS_FILE */
   /* Just copy it over - NULL or otherwise */
   o->cf_msgfile = cfo->message_file;
   o->cf_accessfile = cfo->access_file;








   /* STUB: There are others!!!! */








   return(0);
}


/* ========================================================================= */
struct cfoptions *new_cfoptions(void)
{
   struct cfoptions *cfo;

   if ( NULL == (cfo = (struct cfoptions *)malloc(sizeof(struct cfoptions))) )
   {
      error_msg("ERROR: Unable to allocate memory for config file options data.");
      return(NULL);
   }

   cfo->listen_addr = NULL;
   cfo->listen_port = NULL;
   cfo->httpd_server_threads = NULL;
   cfo->output_buffer_size = NULL;
   cfo->input_buffer_size = NULL;
   cfo->message_file = NULL;
   cfo->access_file = NULL;
   cfo->qc_file_dir = NULL;

   cfo->max_request_min = NULL;
   cfo->max_request_timeo = NULL;

   return(cfo);
}

/* ========================================================================= */
int insert_value(struct cfoptions *cfo, char *lhs, char *rhs)
{
   if ( 0 == strcmp("LISTEN_ADDRESS", lhs) )
   {
      if ( cfo->listen_addr )
      {
         error_msg("WARNING: Duplicate conf file entry for LISTEN_ADDRESS. Prior value ignored.");
         free(cfo->listen_addr);
      }

      cfo->listen_addr = mkstring(rhs);
   }

   if ( 0 == strcmp("LISTEN_PORT", lhs) )
   {
      if ( cfo->listen_port )
      {
         error_msg("WARNING: Duplicate conf file entry for LISTEN_PORT. Prior value ignored.");
         free(cfo->listen_port);
      }

      cfo->listen_port = mkstring(rhs);
   }

   if ( 0 == strcmp("HTTPD_SERVER_THREADS", lhs) )
   {
      if ( cfo->httpd_server_threads )
      {
         error_msg("WARNING: Duplicate conf entry for HTTPD_SERVER_THREADS. Prior value ignored.");
         free(cfo->httpd_server_threads);
      }

      cfo->httpd_server_threads = mkstring(rhs);
   }

   if ( 0 == strcmp("OUTPUT_BUFFER_SIZE", lhs) )
   {
      if ( cfo->output_buffer_size )
      {
         error_msg("WARNING: Duplicate conf entry for OUTPUT_BUFFER_SIZE. Prior value ignored.");
         free(cfo->output_buffer_size);
      }

      cfo->output_buffer_size = mkstring(rhs);
   }

   if ( 0 == strcmp("INPUT_BUFFER_SIZE", lhs) )
   {
      if ( cfo->input_buffer_size )
      {
         error_msg("WARNING: Duplicate conf entry for INPUT_BUFFER_SIZE. Prior value ignored.");
         free(cfo->input_buffer_size);
      }

      cfo->input_buffer_size = mkstring(rhs);
   }

   /* STUB - Items below are NOT properly handled at this time */

   if ( 0 == strcmp("MAX_REQUEST_MIN", lhs) )
   {
      if ( cfo->max_request_min )
      {
         error_msg("WARNING: Duplicate conf file entry for MAX_REQUEST_MIN. Prior value ignored.");
         free(cfo->max_request_min);
      }

      cfo->max_request_min = mkstring(rhs);
   }

   if ( 0 == strcmp("MAX_REQUEST_TIMEO", lhs) )
   {
      if ( cfo->max_request_timeo )
      {
         error_msg("WARNING: Duplicate conf file entry for MAX_REQUEST_TIMEO. Prior value ignored.");
         free(cfo->max_request_timeo);
      }

      cfo->max_request_timeo = mkstring(rhs);
   }

   if ( 0 == strcmp("MESSAGE_FILE", lhs) )
   {
      if ( cfo->message_file )
      {
         error_msg("WARNING: Duplicate conf file entry for MESSAGE_FILE. Prior value ignored.");
         free(cfo->message_file);
      }

      cfo->message_file = mkstring(rhs);
   }

   if ( 0 == strcmp("ACCESS_FILE", lhs) )
   {
      if ( cfo->access_file )
      {
         error_msg("WARNING: Duplicate conf file entry for ACCESS_FILE. Prior value ignored.");
         free(cfo->access_file);
      }

      cfo->access_file = mkstring(rhs);
   }

   if ( 0 == strcmp("QC_FILE_DIR", lhs) )
   {
      if ( cfo->qc_file_dir )
      {
         error_msg("WARNING: Duplicate conf file entry for QC_FILE_DIR. Prior value ignored.");
         free(cfo->qc_file_dir);
      }

      cfo->qc_file_dir = mkstring(rhs);
   }

   return(0);
}

/* ========================================================================= */
int add_arg_line(struct cfoptions *cfo, char *line)
{
   int i;       /* Used to walk through char arrays */
   char *ls;    /* Used to take away leading space */
   char *rhs;   /* Rhight hand side of the equation */
   char *lhs;   /* Left hand side of the equation */
   char qfirst, qlast;

   /* Let's check the input */
   assert( line != NULL );
   assert( cfo != NULL );
   if ( line[0] == 0 )
      return(0); /* Blank lines are not an issue */

   /* Start by assuming we have neither rhs or lhs */
   rhs = NULL;
   lhs = NULL;

   /* Skip over leading space */
   ls = unlead(line);

   /* Strip off EOL marker */
   chomp(line);

   /* Strip off trailing comments */
   clamp(line);

   /* Strip off trailing white space */
   wstrim(line);

   /* ls now sits at the beginning of data (or a CR) */
   if ( ls[0] == 0 )
      return(0);     /* Not an error, just no data (possibly was a comment) */

   /* Determine RHS and LHS */
   i = 0;
   while ( (ls[i] != 0 ) && ( ls[i] != '=' ) )
      i++;

   if ( ls[i] == '=' )
   {
      ls[i] = 0;

      /* There is a LHS and (potentially) a RHS */
      lhs = ls;
      
      if ( ls[i + 1] != 0 )
      {
         rhs = &ls[i + 1];
      }
   }

   if ( ls[i] == 0 )
   {
      lhs = ls;
   }

   /* Clean up the data a bit */
   wstrim(lhs);  /* Space between the option and the = */

   if ( rhs ) /* Don't try to clean it if it is null */
   {
      rhs = unlead(rhs); /* Space between the value and the = */

      /* Un-quote it */
      qfirst = rhs[0];
      qlast = rhs[strlen(rhs) - 1];

      /* " */
      if ((qfirst == 34) && (qlast == 34))
      {
         rhs++;
         rhs[strlen(rhs) - 1] = 0;
      }

      /* ' */
      if ((qfirst == 39) && (qlast == 39))
      {      
         rhs++;
         rhs[strlen(rhs) - 1] = 0;
      }
   }

   /* Convert LHS to all caps */
   i = 0;
   while ( lhs[i] != 0 )
   {
      if ((lhs[i] >= 'a') && (lhs[i] <= 'z'))
         lhs[i] = lhs[i] - 32;

      i++;
   }
 
   insert_value(cfo, lhs, rhs);

   return(0);
}



















