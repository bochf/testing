#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
/* For the timeval and timespec data types */
#include <time.h>
#include <sys/time.h>

#include "../mainhelp.h"
#include "wmcommon.h"
#include "wmstdout.h"

/* When printing to stdout, the data can be distinguished by several different
   "dividers". These can be white-space, colons, tabs, or potentially other
   field separators. These defines register what the writer can/will do.
*/
#define CS_COLON 0
#define CS_TAB   1
#define CS_FIXED 2
#define CS_DEFAULT CS_COLON

/* ========================================================================= */
/* The stdout data handle
   This contains all formatting, state, and data references for the stdout
   writer. Everything required to operate the stdout writer is contained in
   this struct. This struct is passed between methods as a void pointer
*/
struct wms_data
{
   int TYPE;            /* The MAGIC used to recognize/validate this struct  */
   int header_every;    /* Option setting for when to display headers        */
   int hlinecnt;        /* Counter used for header_every                     */
   int separator;       /* Enum (defined values) for how to CSV              */
   int fixed_size;      /* If separator == fixed, then this is how much      */

   struct proot *p;
   struct config *cfg;
};

/* ========================================================================= */
/* Prototypes for non-published (helper) functions                           */
int dump_header(struct wms_data *wmsd);
int walk_csv_header(struct pitem *pi_olist, char sv);
int walk_fixed_header(struct pitem *pi_olist, int fixed_size);
void printf_fixed_pitem(struct pitem *pi, int fixedw);
void printf_pitem(struct pitem *pi);
int csv_stdout(struct proot *p, char sc);
int fixed_stdout(struct proot *p, int fixedw);

/* ========================================================================= */
void *InitStdoutWM(struct proot *p, struct config *cfg)
{
   void *rv;
   struct wms_data *wmsd;
   struct cfg_wm_arg *ap;

   /* The following are used for the option parsing */
   int parsed_item;
   int parsed_line;
   int error_parsing;

   char *dataloc;
   struct qparts qp;
   struct prov_name *this_pn;   /* Used to walk the prov_name (quad) list */

   if ( NULL == ( rv = malloc(sizeof(struct wms_data)) ) )
   {
      fprintf(stderr, "ERROR: Failed to allocate memory for writer data.\n");
      return(NULL);
   }

   /* Just one cast rather than many */
   wmsd = (struct wms_data *)rv;

   /* Set our "magic" */
   wmsd->TYPE = WM_STDOUT_TYPE;

   /* Set defaults for all else */
   wmsd->header_every = 0;
   wmsd->hlinecnt = 0;
   wmsd->separator = CS_DEFAULT;
   wmsd->fixed_size = 12;  /* A semi-arbritray default */
   wmsd->p = p;  /* Save a reference to the proot (the data) */
   wmsd->cfg = cfg; /* Needed for the header info */

   /* Parse what we got as options */
   ap = cfg->walist;
 
   error_parsing = 0;
   while(ap)
   {
      parsed_line = 0;

      if ( ap->option == StrStr(ap->option, "header_every") )
      {
         parsed_line = 1;

         if ( is_numeric(ap->value) )
            wmsd->header_every = atoi(ap->value);
         else
         {
            fprintf(stderr, "ERROR: Unable to parse \"header_every\" value.\n");
            error_parsing = 1;
         }
      }


      if ( ap->option == StrStr(ap->option, "separator") )
      {
         parsed_item = 0;
         parsed_line = 1;

         if ( StrStr(ap->value, "colon") )
         {
            wmsd->separator = CS_COLON;
            parsed_item = 1;
         }

         if ( StrStr(ap->value, "tab") )
         {
            wmsd->separator = CS_TAB;
            parsed_item = 1;
         }

         if ( StrStr(ap->value, "fixed") )
         {
            wmsd->separator = CS_FIXED;
            parsed_item = 1;

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
               wmsd->fixed_size = atoi(dataloc);
            }
            else
            {
               fprintf(stderr, "ERROR: Unable to parse \"separator\" \"fixed\"value.\n");
               error_parsing = 1;
            }


            /* Basic error checking  */
            if ( wmsd->fixed_size < 1 )
            {
               fprintf(stderr, "ERROR: invalid value for \"separator\" \"fixed\"value.\n");
               error_parsing = 1;
            }

         } /* if( fixed ); */

         if ( parsed_item == 0 )
         {
            fprintf(stderr, "ERROR: Unable to parse option for \"separator\" line.\n");
            error_parsing = 1;
         }

      } /* if(option == separator) */

      if ( 0 == parsed_line )
      {
         fprintf(stderr, "ERROR: Unable to parse line \"%s\".\n", ap->option);
         error_parsing = 1;
      }


      ap = ap->next;
   }

   /* Errors were allowed to compound - NOW exit */
   if ( error_parsing )
      return(NULL);

   /* If headers should be displayed, insure that the header names are defined */
   if ( wmsd->header_every )
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

   return(rv);
}

/* ========================================================================= */
int StdoutWMWrite(void *data_handle)
{
   struct wms_data *wmsd;

   wmsd = (struct wms_data *)data_handle;

   /* Both of these are internal / developer issues */
   assert( NULL != wmsd );
   assert ( wmsd->TYPE == WM_STDOUT_TYPE );

   /* Do the header thing */
   if ( wmsd->header_every )
   {
      if ( wmsd->hlinecnt % wmsd->header_every == 0 )
      {
         /* STUB - Header MUST come from the pitems! */
         dump_header(wmsd);
         wmsd->hlinecnt = 0;
      }

      wmsd->hlinecnt++;
   }

   switch ( wmsd->separator )
   {
   case CS_COLON:
      csv_stdout(wmsd->p, ':');
      break;
   case CS_TAB:
      csv_stdout(wmsd->p, '\t');
      break;
   case CS_FIXED:
      fixed_stdout(wmsd->p, wmsd->fixed_size);
      break;
   }

   /* Flush in-case we are dumping to a file */
   fflush(stdout);

   return(0);
}

/* ========================================================================= */
int StdoutWMFinish(void *data_handle)
{
   /* This will largely be a no-op. There really just is not much to clean
      up for a stdout writer. */

   /* Flush data - one last time */
   fflush(stdout);

   return(0);
}

/* ========================================================================= */
int dump_header(struct wms_data *wmsd)
{
   assert(NULL != wmsd);
   assert(NULL != wmsd->p);

   switch ( wmsd->separator )
   {
   case CS_COLON:
      walk_csv_header(wmsd->p->pi_olist, ':');
      break;
   case CS_TAB:
      walk_csv_header(wmsd->p->pi_olist, '\t');
      break;
   case CS_FIXED:
      walk_fixed_header(wmsd->p->pi_olist, wmsd->fixed_size);
      break;
   }

   printf("\n");

   return(0);
}

/* ========================================================================= */
int walk_csv_header(struct pitem *pi_olist, char sv)
{
   struct pitem *this_pitem;
   int items = 0;

   assert(NULL != pi_olist);

   this_pitem = pi_olist;
   while(this_pitem)
   {
      if ( items > 0 )
         printf("%c", sv);

      printf("%s", this_pitem->header);

      items++;
      this_pitem = this_pitem->next_opi;
   }

   return(items);
}

/* ========================================================================= */
int walk_fixed_header(struct pitem *pi_olist, int fixed_size)
{
   struct pitem *this_pitem;
   int items = 0;
   char fmt_str[12];

   assert(NULL != pi_olist);

   sprintf(fmt_str, "%%%ds ", fixed_size);

   this_pitem = pi_olist;
   while(this_pitem)
   {
      printf(fmt_str, this_pitem->header);

      items++;
      this_pitem = this_pitem->next_opi;
   }

   return(items);
}

/* ========================================================================= */
void printf_pitem(struct pitem *pi)
{
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

   /* Phuck it, just promote everything that can be promoted */
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
      printf("%s", (char *)data_ptr);
      break;
   case PDT_INT8:
   case PDT_UINT8:
   case PDT_INT16:
   case PDT_UINT16:
   case PDT_INT32:
   case PDT_UINT32:
   case PDT_INT64:
   case PDT_UINT64:
      printf("%ld", value);
      break;
   case PDT_TIMEVAL:
      tv = (struct timeval *)data_ptr;
      printf("%ld.%ld", tv->tv_sec, tv->tv_usec);
      break;
#ifdef _TIMESPEC
   case PDT_TIMESPEC:
      ts = (struct timespec *)data_ptr;
      printf("%ld.%ld", ts->tv_sec, ts->tv_nsec);
      break;
#endif
   }
}

/* ========================================================================= */
void printf_fixed_pitem(struct pitem *pi, int fixedw)
{
   char fmt[16];
   char tmpstr[64]; /* An obnoxously long, yet still unsafe size */

#ifdef UNDEFINED
   /* These pointers are to beat compiler issues */
   /* The compiler views casting a void pointer to a specific data type as
      an issue (when used as a param to a function (printf()). I cast and 
      assign to a specific data type, then use that. */
   int8_t *sc;
   uint8_t *uc;
   int16_t *ss;
   uint16_t *us;
   int32_t *si;
   uint32_t *ui;
   int64_t *sl;
   uint64_t *ul;
#endif

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

   value = 0; /* Compiler happiness issue */

   /* Phuck it, just promote everything that can be promoted */
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
      printf(fmt, (char *)data_ptr);
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
      printf(fmt, value);
      break;
      /* The next two items really don't work with fixed sizes, but they
         are officially supported just the same. */
   case PDT_TIMEVAL:
      tv = (struct timeval *)data_ptr;
      sprintf(tmpstr, "%ld.%ld", tv->tv_sec, tv->tv_usec);
      sprintf(fmt, "%%%ds ", fixedw);
      printf(fmt, tmpstr);
      break;
#ifdef _TIMESPEC
   case PDT_TIMESPEC:
      ts = (struct timespec *)data_ptr;
      sprintf(tmpstr, "%ld.%ld", ts->tv_sec, ts->tv_nsec);
      sprintf(fmt, "%%%ds ", fixedw);
      printf(fmt, tmpstr);
      break;
#endif
   }
}

/* ========================================================================= */
int csv_stdout(struct proot *p, char sc)
{
   int items;

   struct pitem *this_pi;

   items = 0;
   this_pi = p->pi_olist;
   while( this_pi )
   {
      if ( 0 != items )
         printf("%c", sc);

      printf_pitem(this_pi);

      this_pi = this_pi->next_opi;
      items++;
   }
   
   printf("\n");
   fflush(stdout);
   return(0);
}

/* ========================================================================= */
int fixed_stdout(struct proot *p, int fixedw)
{
   int items;
   struct pitem *this_pi;

   items = 0;
   this_pi = p->pi_olist;
   while( this_pi )
   {
      printf_fixed_pitem(this_pi, fixedw);
      
      this_pi = this_pi->next_opi;
      items++;
   }
   
   printf("\n");
   fflush(stdout);
   return(0);
}
