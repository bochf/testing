#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
/* For the timeval and timespec data types */
#include <time.h>
#include <sys/time.h>

#include "../mainhelp.h"
#include "wmcommon.h"
#include "wmfile.h"

/* This writer was largely copied from the stdout writer. It uses essentially
   all the same options but also has a few addtional modifications (options)
   to deal with issues arrising out of writing to files. */


/* When printing to a simple file, the data can be distinguished by several
   different "dividers". These can be white-space, colons, tabs, or
   potentially other field separators. These defines register what the
   writer can/will do.
*/
#define CS_COLON 0
#define CS_TAB   1
#define CS_FIXED 2
#define CS_DEFAULT CS_COLON

/* ========================================================================= */
/* The file writer data handle
   This contains all formatting, state, and data references for the stdout
   writer. Everything required to operate the stdout writer is contained in
   this struct. This struct is passed between methods as a void pointer
*/
struct wmf_data
{
   int TYPE;            /* The MAGIC used to recognize/validate this struct  */
   int header_every;    /* Option setting for when to display headers        */
   int hlinecnt;        /* Counter used for header_every                     */
   int separator;       /* Enum (defined values) for how to CSV              */
   int fixed_size;      /* If separator == fixed, then this is how much      */
   int atomic_write;    /* Close file after each line write                  */
   FILE *fh;
   char *filename;

   struct proot *p;
   struct config *cfg;
};

/* ========================================================================= */
/* Prototypes for non-published (helper) functions                           */
int fh_dump_header(struct wmf_data *wmfd);
int fh_walk_csv_header(FILE *fh, struct proot *p, char sv);
int fh_walk_fixed_header(FILE *fh, struct proot *p, int fixed_size);
void fprintf_fixed_pitem(FILE *fh, struct pitem *pi, int fixedw);
void printf_pitem(struct pitem *pi);
int csv_file(FILE *fh, struct proot *p, char sc);
int fixed_file(FILE *fh, struct proot *p, int fixedw);



/* ========================================================================= */
void *InitFileWM(struct proot *p, struct config *cfg)
{
   void *rv;
   struct wmf_data *wmfd;
   struct cfg_wm_arg *ap;
   /* Various flags used in parsing */
   int parse_flag;   /* Per-item parse error detection */
   int error_stack;  /* Let errors stack up before bailing */
   int item_parsed;  /* Individual items parsed flag */
   char *dataloc;
   struct qparts qp;
   struct prov_name *this_pn;   /* Used to walk the prov_name (quad) list */

   if ( NULL == ( rv = malloc(sizeof(struct wmf_data)) ) )
   {
      error_msg("ERROR: Failed to allocate memory for writer data.");
      return(NULL);
   }

   /* Just one cast rather than many */
   wmfd = (struct wmf_data *)rv;

   /* Set our "magic" */
   wmfd->TYPE = WM_FILE_TYPE;

   /* Set defaults for all else */
   wmfd->header_every = 0;
   wmfd->hlinecnt = 0;
   wmfd->separator = CS_DEFAULT;
   wmfd->fixed_size = 12;  /* A semi-arbritray default */
   wmfd->fh = NULL;
   wmfd->atomic_write = 0;
   wmfd->filename = NULL;
   wmfd->p = p;  /* Save a reference to the proot (the data) */
   wmfd->cfg = cfg; /* Needed for the header info */
   
   /* Parse what we got as options */
   ap = cfg->walist;
   error_stack = 0; /* Assume no errors to start */
   while(ap)
   {
      parse_flag = 0; /* Start by "not" having parsed anything */

      if ( ap->option == StrStr(ap->option, "header_every") )
      {
         if ( is_numeric(ap->value) )
         {
            wmfd->header_every = atoi(ap->value);
            parse_flag = 1;
         }
         else
         {
            error_msg("ERROR: Unable to parse \"header_every\" value.");
            parse_flag = 1; /* We did not fully parse it, but error is accounted for */
            error_stack = 1;
         }
      }

      if ( ap->option == StrStr(ap->option, "atomic_write") )
      {
         item_parsed = 0; 

         if ( NULL == ap->value )
         {
            wmfd->atomic_write = 1;
            item_parsed = 1;
            parse_flag = 1;
         }

         if (( StrStr(ap->value, "TRUE") )   ||
             ( 0 == strcmp(ap->value, "1") ) ||
             ( 0 == strcmp(ap->value, "T") ) ||
             ( 0 == strcmp(ap->value, "t") ))
         {
            wmfd->atomic_write = 1;
            item_parsed = 1;
            parse_flag = 1;
         }

         if (( StrStr(ap->value, "FALSE") )   ||
             ( 0 == strcmp(ap->value, "0") ) ||
             ( 0 == strcmp(ap->value, "F") ) ||
             ( 0 == strcmp(ap->value, "f") ))
         {
            wmfd->atomic_write = 0;
            item_parsed = 1;
            parse_flag = 1;
         }


         if ( item_parsed == 0 )
         {
            error_msg("ERROR: Unable to parse \"atomic_write\" value.");
            error_stack = 1;
         }
      }

      if ( ap->option == StrStr(ap->option, "filename") )
      {
         if( NULL == ( wmfd->filename = mkstring(ap->value) ) )
         {
            error_msg("ERROR: Unable allocate memory for writer filename.");
            return(NULL);
         }
         parse_flag = 1;
      }

      if ( ap->option == StrStr(ap->option, "separator") )
      {
         item_parsed = 0;

         if ( StrStr(ap->value, "colon") )
         {
            wmfd->separator = CS_COLON;
            item_parsed = 1;
            parse_flag = 1;
         }

         if ( StrStr(ap->value, "tab") )
         {
            wmfd->separator = CS_TAB;
            item_parsed = 1;
            parse_flag = 1;
         }

         if ( StrStr(ap->value, "fixed") )
         {
            wmfd->separator = CS_FIXED;

            dataloc = StrStr(ap->value, "fixed");
            dataloc += 5;

            /* Step ahead until we get to a number. This can be:
               fixed at 10
               fixed = 10
               fixed10
               fixed 10
               Trailing whitespace and comments are previously clipped */
            while ((( *dataloc < '0' ) || ( *dataloc > '9' )) && ( *dataloc != 0 ))
               dataloc++;

            if ( is_numeric(dataloc) )
            {
               wmfd->fixed_size = atoi(dataloc);
            }
            else
            {
               error_msg("ERROR: Unable to parse \"separator\" \"fixed\"value.");
               parse_flag = 1; item_parsed = 1; /* Error - but accounted for */
               error_stack = 1;
            }


            /* Basic error checking  */
            if ( wmfd->fixed_size < 1 )
            {
               error_msg("ERROR: Invalid value for \"separator\" \"fixed\"value.");
               parse_flag = 1; item_parsed = 1; /* Error - but accounted for */
               error_stack = 1;
            }
            else
            {
               item_parsed = 1;
               parse_flag = 1;
            }
         } /* if( fixed ); */

         if ( item_parsed == 0 )
         {
            error_msg("ERROR: Unable to parse option for \"separator\" line.");
            parse_flag = 1; /* Error - but accounted for */
            error_stack = 1;
         }

      } /* if(option == separator) */

      if ( 0 == parse_flag )
      {
         error_msg("ERROR: Unable to parse: \"%s\" line.", ap->option);
         error_stack = 1;
      }

      ap = ap->next;
   }

   if ( NULL == wmfd->filename )
   {
      error_msg("ERROR: No \"filename\" was specified for file module.");
      return(NULL);
   }

   /* Errors have built up, now give up */
   if ( error_stack )
   {
      return(NULL);
   }

   /* Test that the file exists and we can write to it */
   /* Delete it, if it exists - This will insure that the new is the way
      we want it. */
   switch ( file_state(wmfd->filename) )
   {
   case FS_EXISTS:
   case FS_LOCKED: /* Even if !RW, we might be able to delete it */
      if ( unlink(wmfd->filename) )
      {
         error_msg("ERROR: Unable to clear existing file.");
         return(NULL);
      }
      break;
   case FS_SERROR:
      error_msg("ERROR: Unable to properly stat file.");
      return(NULL);
      break; /* Really about compiler happiness */
   case FS_NOFILE:
   default:
      /* Just fall through to the touching */
      break;
   }

   /* Touch it to insure that we are good */
   if ( touch_file(wmfd->filename) )
   {
      error_msg("ERROR: Unable to create the output file.");
      return(NULL);
   }

   /* If headers should be displayed, insure that the header names are defined */
   if ( wmfd->header_every )
   {
      this_pn = cfg->pnlist;
      while(this_pn)
      {
         /* Only create a header name if the user did not supply one */
         if ( NULL == this_pn->header )
         {
            StrToQuadPartsFill(&qp, this_pn->quad);
            
            if ( NULL == (this_pn->header = mkstring(qp.iname)) )
               return(NULL);
         }

         this_pn = this_pn->next;
      } 
   }

#ifdef DEBUGGERY
   fprintf(stderr, "struct wms_data\n{\n");
   fprintf(stderr, "   TYPE = %d;\n", wmsd->TYPE);
   fprintf(stderr, "   header_every = %d;\n", wmsd->header_every);
   fprintf(stderr, "   hlinecnt = %d;\n", wmsd->hlinecnt);
   fprintf(stderr, "   separator = %d;\n", wmsd->separator);
   fprintf(stderr, "   fixed_size = %d;\n", wmsd->fixed_size);
   fprintf(stderr, "};\n");
#endif

   return(rv);
}


/* ========================================================================= */
int WriteFileWM(void *data_handle)
{
   struct wmf_data *wmfd;

   wmfd = (struct wmf_data *)data_handle;

   /* Both of these are internal / developer issues */
   assert( NULL != wmfd );
   assert ( wmfd->TYPE == WM_FILE_TYPE );

   if ( NULL == wmfd->fh )
   {
      if ( NULL == ( wmfd->fh = fopen(wmfd->filename, "a") ) )
         return(1);
   }

   /* Do the header thing */
   if ( wmfd->header_every )
   {
      if ( wmfd->hlinecnt % wmfd->header_every == 0 )
      {
         fh_dump_header(wmfd);
         wmfd->hlinecnt = 0;
      }

      wmfd->hlinecnt++;
   }

   switch ( wmfd->separator )
   {
   case CS_COLON:
      csv_file(wmfd->fh, wmfd->p, ':');
      break;
   case CS_TAB:
      csv_file(wmfd->fh, wmfd->p, '\t');
      break;
   case CS_FIXED:
      fixed_file(wmfd->fh, wmfd->p, wmfd->fixed_size);
      break;
   }

   /* This is the ONLY place that atomic_write is used! It could easily
      be a local to this function - and we could just check to see if
      the file handle is NULL for all other instances. */
   if ( wmfd->atomic_write )
   {
      fclose(wmfd->fh);
      wmfd->fh = NULL;
   }

   return(0);
}

/* ========================================================================= */
int FinishFileWM(void *data_handle)
{
   struct wmf_data *wmfd;

   wmfd = (struct wmf_data *)data_handle;

   /* Both of these are internal / developer issues */
   assert( NULL != wmfd );
   assert ( wmfd->TYPE == WM_FILE_TYPE );

   if ( wmfd->fh )
      fclose(wmfd->fh);

   return(0);
}

/* ========================================================================= */
int fh_dump_header(struct wmf_data *wmfd)
{
   assert(NULL != wmfd);
   assert(NULL != wmfd->cfg);

   switch ( wmfd->separator )
   {
   case CS_COLON:
      fh_walk_csv_header(wmfd->fh, wmfd->p, ':');
      break;
   case CS_TAB:
      fh_walk_csv_header(wmfd->fh, wmfd->p, '\t');
      break;
   case CS_FIXED:
      fh_walk_fixed_header(wmfd->fh, wmfd->p, wmfd->fixed_size);
      break;
   }

   fprintf(wmfd->fh, "\n");

   return(0);
}

/* ========================================================================= */
int fh_walk_csv_header(FILE *fh, struct proot *p, char sv)
{
   struct pitem *this_pitem;
   int items = 0;

   assert(NULL != p);
   assert(NULL != p->pi_olist);

   this_pitem = p->pi_olist;
   while(this_pitem)
   {
      if ( items > 0 )
         fprintf(fh, "%c", sv);

      fprintf(fh, "%s", this_pitem->header);

      items++;
      this_pitem = this_pitem->next_opi;
   }

   return(items);
}

/* ========================================================================= */
int fh_walk_fixed_header(FILE *fh, struct proot *p, int fixed_size)
{
   struct pitem *this_pitem;
   int items = 0;
   char fmt_str[12];

   assert(NULL != p);
   assert(NULL != p->pi_olist);

   sprintf(fmt_str, "%%%ds ", fixed_size);

   this_pitem = p->pi_olist;
   while(this_pitem)
   {
      fprintf(fh, fmt_str, this_pitem->header);

      items++;
      this_pitem = this_pitem->next_opi;
   }

   return(items);
}

/* ========================================================================= */
void fprintf_pitem(FILE *fh, struct pitem *pi)
{
   /* Follow the lead used by the stdout writer implementation... that is
      specifically - promote all to a 64-bit value, and assign the sign.
      This limits us only in the most extreme of edge cases. For example:
      An adapter recieving 1 million packets a second would run for over
      200 thousand years before overflowing this value. It is just an
      edge case beyond reasonable concern.
   */
#ifdef _TIMESPEC
   struct timespec *ts;
#endif
   struct timeval *tv;

   char factstr[32];
   int data_type;
   void *data_ptr;
   int64_t value;

   /* Intercept factor arguments, turn them into strings */
   data_type = pi->data_type;
   data_ptr = pi->data_ptr;

   if ( pi->munge_flag & MUNGE_FACTOR )
   {
      if ( 0 == factor_data(factstr, pi) )
      {
         data_ptr = factstr;
         data_type = PDT_STRING;
      }
   }

   value = 0;

   /* Promote everything that can be promoted */
   /* and at least one type (uint64_t) that cant */
   switch(data_type)
   {
   case PDT_INT8:
      value = *((int8_t *)data_ptr);
      break;
   case PDT_UINT8:
      value = *((uint8_t *)data_ptr);
      break;
   case PDT_INT16:
      value = *((int16_t *)data_ptr);
      break;
   case PDT_UINT16:
      value = *((uint16_t *)data_ptr);
      break;
   case PDT_INT32:
      value = *((int32_t *)data_ptr);
      break;
   case PDT_UINT32:
      value = *((uint32_t *)data_ptr);
      break;
   case PDT_INT64:
      value = *((int64_t *)data_ptr);
      break;
   case PDT_UINT64:
      value = *((uint64_t *)data_ptr);
      break;
   }

   /* Apply the sign */
   if ( pi->sign_flag )
      value *= -1;

   /* Print it out */
   switch(data_type)
   {
   case PDT_STRING:
      fprintf(fh, "%s", (char *)data_ptr);
      break;
   case PDT_INT8:
   case PDT_UINT8:
   case PDT_INT16:
   case PDT_UINT16:
   case PDT_INT32:
   case PDT_UINT32:
   case PDT_INT64:
   case PDT_UINT64:
      fprintf(fh, "%ld", value);
      break;
   case PDT_TIMEVAL:
      tv = (struct timeval *)data_ptr;
      fprintf(fh, "%ld.%ld", tv->tv_sec, tv->tv_usec);
      break;
#ifdef _TIMESPEC
   case PDT_TIMESPEC:
      ts = (struct timespec *)data_ptr;
      fprintf(fh, "%ld.%ld", ts->tv_sec, ts->tv_nsec);
      break;
#endif
   }
}

/* ========================================================================= */
void fprintf_fixed_pitem(FILE *fh, struct pitem *pi, int fixedw)
{
   char fmt[16];
   char tmpstr[64]; /* An obnoxously long, yet still unsafe size */
#ifdef _TIMESPEC
   struct timespec *ts;
#endif
   struct timeval *tv;
   int64_t value;
   char factstr[32];
   int data_type;
   void *data_ptr;

   /* Intercept factor arguments, turn them into strings */
   data_type = pi->data_type;
   data_ptr = pi->data_ptr;

   if ( pi->munge_flag & MUNGE_FACTOR )
   {
      if ( 0 == factor_data(factstr, pi) )
      {
         data_ptr = factstr;
         data_type = PDT_STRING;
      }
   }

   value = 0; /* Compiler cries otherwise */

   /* Promote everything that can be promoted */
   /* and at least one type (uint64_t) that cant */
   switch(data_type)
   {
   case PDT_INT8:
      value = *((int8_t *)data_ptr);
      break;
   case PDT_UINT8:
      value = *((uint8_t *)data_ptr);
      break;
   case PDT_INT16:
      value = *((int16_t *)data_ptr);
      break;
   case PDT_UINT16:
      value = *((uint16_t *)data_ptr);
      break;
   case PDT_INT32:
      value = *((int32_t *)data_ptr);
      break;
   case PDT_UINT32:
      value = *((uint32_t *)data_ptr);
      break;
   case PDT_INT64:
      value = *((int64_t *)data_ptr);
      break;
   case PDT_UINT64:
      value = *((uint64_t *)data_ptr);
      break;
   }

   /* Apply the sign */
   if ( pi->sign_flag )
      value *= -1;

   switch(data_type)
   {
   case PDT_STRING:
      sprintf(fmt, "%%%ds ", fixedw);
      fprintf(fh, fmt, (char *)data_ptr);
      break;
   case PDT_INT8:
   case PDT_UINT8:
   case PDT_INT16:
   case PDT_UINT16:
   case PDT_INT32:
   case PDT_UINT32:
   case PDT_INT64:
   case PDT_UINT64:
      sprintf(fmt, "%%%dld ", fixedw);
      fprintf(fh, fmt, value);
      break;
      /* The next two items really don't work with fixed sizes, but they
         are officially supported just the same. */
   case PDT_TIMEVAL:
      tv = (struct timeval *)data_ptr;
      sprintf(tmpstr, "%ld.%ld", tv->tv_sec, tv->tv_usec);
      sprintf(fmt, "%%%ds ", fixedw);
      fprintf(fh, fmt, tmpstr);
      break;
#ifdef _TIMESPEC
   case PDT_TIMESPEC:
      ts = (struct timespec *)data_ptr;
      sprintf(tmpstr, "%ld.%ld", ts->tv_sec, ts->tv_nsec);
      sprintf(fmt, "%%%ds ", fixedw);
      fprintf(fh, fmt, tmpstr);
      break;
#endif
   }
}

/* ========================================================================= */
int csv_file(FILE *fh, struct proot *p, char sc)
{
   int items;

   struct pitem *this_pi;

   items = 0;
   this_pi = p->pi_olist;
   while( this_pi )
   {
      if ( 0 != items )
         fprintf(fh, "%c", sc);

      fprintf_pitem(fh, this_pi);

      this_pi = this_pi->next_opi;
      items++;
   }
   
   fprintf(fh, "\n");
   return(0);
}

/* ========================================================================= */
int fixed_file(FILE *fh, struct proot *p, int fixedw)
{
   int items;
   struct pitem *this_pi;

   items = 0;
   this_pi = p->pi_olist;
   while( this_pi )
   {
      fprintf_fixed_pitem(fh, this_pi, fixedw);
      
      this_pi = this_pi->next_opi;
      items++;
   }
   
   fprintf(fh, "\n");
   return(0);
}
