#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
/* For the timeval and timespec data types */
#include <sys/time.h>

/* NOTE: This builds on all platforms - on systems that I used. I am not
         clear that this is the ideal place to find this header/library. I am
         clear that this is the ONLY place to find it on (at least some) AIX
         systems. */
#include </opt/swt/include/zlib.h>

#include "../mainhelp.h"
#include "wmcommon.h"
#include "wmbcd.h"


/* =========================================================================
  Notes on this implementation
  * There is no clear representation of compound data types (timeval and
    timespec). They are currently not supported in this format version. The
    expectation is that they may be supported in a later version (v2?)
  * Because of the diff option on unsigned types, it is possible for an
    unsigned type to go negative. cpub handles this with the sign flag that
    is associated with the pitem data. This is much harder to represent in
    binary output. Here are the options for dealing with this:
    1. Save the sign bit externally in an array (just like the skip
       compression flags).
    2. Do not allow diffs on the data when using this writer.
    3. Determine on the first write if data can go negative and then just
       promote that type when writing to the next largest signed type.
    4. Save the raw values to the output along with the diff directive and
       do the diff in post processing.
    Analysis of the options:
    + Item one is the most likely way forward - but in a v2 versioned file.
    + Item two is a reasonable v1 solution.
    + Item three is a less optimal solution to option one.
    + Item four is not an option as the provider is responsible for the
      diff (and holds the data required for the diff).
    Summary:
    Implement option two in v1, and option one in v2.

*/


/*
  ToDos:
   [ ] Test.
  Done:
   [X] Yank rollnum and the get_current_filename()
   [X] Implement ROLL_AT (seconds past midnight)
   [X] Add calls into main.c - add linkage (etc) into Makefile
   [X] Remove all stubs from the code
   [X] Fix write_cdata()
   [X] Make sure that keep_files input is >= 2
   [X] Add support for rolling files (after write)
   [X] Set define for filename[LEN], check for LEN on input.
   [X] Check length of filename input.
   [X] Port on Linux & Solaris
   [X] Document options for the writer
   [X] Remove the original code that remains
   [X] InitBCDWM(): Scan the list of output data for unsupported types.
   [X] Implement the remaining two PublicFunctions()
   [X] get_compressablitiy needs to set the skippable flag
   [X] get_compressibility() should not set the skip bit if skip 
       compression is turned on.
   [X] Document items in the last_value struct

*/




/* This defines when to roll to a new file (how to roll)                     */
#define ROLL_ON_NEVER 0  /* Do not roll the file(s)                          */
#define ROLL_ON_SIZE  1  /* Roll based on size                               */
#define ROLL_ON_TIME  2  /* Roll based on time                               */
#define ROLL_AT_TIME  3  /* Roll at x minutes past midnight                  */

/* Compression flags that can be or'd for total compression method           */
#define COMPRESS_NONE  0
#define COMPRESS_GZIP  1
#define COMPRESS_SKIP  2

#define MAX_FN_LEN 256   /* The (max) length of the filename input */
/* ========================================================================= */
/* The file writer data handle
   This contains all formatting, state, and data references for the stdout
   writer. Everything required to operate the stdout writer is contained in
   this struct. This struct is passed between methods as a void pointer
*/
struct bcd_data
{
   int TYPE;            /* The MAGIC used to recognize/validate this struct  */
   int compress_flags;  /* Or'd flags for compression (Uses COMPRESS_*)      */
   int roll_on_type;    /* Mutually exclusive type of roll (Uses ROLL_ON_*)  */
   char *filename;      /* Filename. Base if roll. Entire name if no roll.   */
   /* Only one of the two following items will be used at one time           */
   gzFile *gd;          /* Gzip file descriptor (handle)                     */
   int fd;              /* Regular file descriptor                           */
   int first_insert;    /* Used to denote data must not be skip compressed   */
   /* Only two of the next four items will be used at one time               */
   size_t sz_roll_at;   /* The value (size in bytes) to roll at              */
   size_t sz_next_roll; /* The bytes sent to the file (used for roll calc)   */
   time_t tm_roll_at;   /* Seconds between rolls                             */
   time_t tm_next_roll; /* Time when we will roll next                       */
   unsigned short keep_files;
   int fmt_version;     /* Version of the file format to use (Only 1)        */
   int endian_flag;     /* Value from get_endianness() 0 = big, 1 = little   */
   unsigned char *cbits;/* The array of compression bits                     */
   int cbit_count;      /* The number of compression bytes == ceil(#pitems/8)*/
   int cbyte_count;     /* The number of compression bits == number of pitems*/

   struct proot *p;
   struct config *cfg;
};

/* ========================================================================= */
/* This struct is attached to the pitem->wdata. It stores the last value that
   was printed. This is used for compares between iterations.
   Why use data_ptr when the provider understands this~last? Because: the
   this and last values in the provider is about diff-ing data. It is NOT
   about what is written.
*/
struct last_value
{
   void *data_ptr;     /* A pointer to the last value                        */
   int sign_flag;      /* A pointer to the last value sign                   */
   int skippable;      /* This data is skippable using skip compression      */
   int data_sz;        /* The size of the data (if it is a string)           */
};

/* ========================================================================= */
/* Prototypes for non-published (helper) functions                           */
int parse_roll_at(struct bcd_data *bcdd, char *value);
int bcd_write(struct bcd_data *bcdd, void *buf, size_t len);
int new_v1_bcd_file(struct bcd_data *bcdd);
int write_v1_bcd_line(struct bcd_data *bcdd);
int close_v1_bcd_file(struct bcd_data *bcdd);
int mk_wdata(struct pitem *thispi, int size);
int init_compressability(struct pitem *thispi, int skip_on);
unsigned char get_compressability(struct pitem *thispi);
int write_cdata(struct bcd_data *bcdd, struct pitem *thispi);
int init_file_output(struct bcd_data *bcdd);
int should_roll_file_output(struct bcd_data *bcdd);
int roll_file_output(struct bcd_data *bcdd);
int get_best_sz(void *vstr);
time_t get_next_alarm(time_t offset);

/* ========================================================================= */
void *InitBCDWM(struct proot *p, struct config *cfg)
{
   void *rv;
   struct bcd_data *bcdd;
   struct cfg_wm_arg *ap;
   int pitem_count;
   struct pitem *pi;
   int cbitlen;
   int required_version;  /* The version required to accomplish request */
   /* Various flags used in parsing */
   int parse_flag;      /* Per-item parse error detection                    */
   int error_stack;     /* Let errors stack up before bailing                */
   int item_parsed;     /* Individual items parsed flag                      */
   int diff_wanted;
   int fact_wanted;

   /* Set some defaults */
   required_version = 1; /* Set for the minimal version format */

   /* Allocate memory for the core memory struct */
   if ( NULL == ( rv = malloc(sizeof(struct bcd_data)) ) )
   {
      error_msg("ERROR: Failed to allocate memory for BCD writer data.");
      return(NULL);
   }

   /* Just one cast rather than many */
   bcdd = (struct bcd_data *)rv;

   /* Set our "magic" */
   bcdd->TYPE = WM_BCD_TYPE;

   /* Set defaults for all else */
   bcdd->compress_flags = COMPRESS_NONE;
   bcdd->roll_on_type = ROLL_ON_NEVER;
   bcdd->filename = NULL;
   bcdd->gd = NULL;
   bcdd->fd = -1;
   bcdd->first_insert = 1;
   bcdd->sz_roll_at = 0;
   bcdd->sz_next_roll = 0;
   bcdd->tm_roll_at = 0;
   bcdd->tm_next_roll = 0;
   bcdd->keep_files = 0; /* All of them */
   bcdd->fmt_version = 1;
   bcdd->endian_flag = GetEndianness();

   bcdd->p = p;  /* Save a reference to the proot (the data) */
   bcdd->cfg = cfg; /* Needed for the header info */
   
   /* Count the number of pitems */
   pitem_count = 0;
   pi = bcdd->p->pi_olist; /* Position at top of output list */
   while ( pi )
   {
      pitem_count++;
      pi = pi->next_opi;
   }

   /* Figure how many places of cbits will be used */
   cbitlen = pitem_count / 8;
   if ( pitem_count % 8 )
      cbitlen++;

   if (NULL == (bcdd->cbits = (unsigned char *)malloc(cbitlen)))
   {
      error_msg("ERROR: Failed to allocate memory for BCD writer data.");
      return(NULL);
   }
   
   bcdd->cbit_count = pitem_count;
   bcdd->cbyte_count = cbitlen;


   /* Parse what we got as options */
   ap = cfg->walist;
   error_stack = 0; /* Assume no errors to start */
   while(ap)
   {
      parse_flag = 0; /* Start by "not" having parsed anything */

      /* ------------------------------ */
      if ( ap->option == StrStr(ap->option, "keep_files") )
      {
         if ( is_numeric(ap->value) )
         {
            bcdd->keep_files = atoi(ap->value);
            parse_flag = 1;
         }
         else
         {
            error_msg("ERROR: Unable to parse \"keep_files\" value.");
            parse_flag = 1; /* We did not fully parse it, but error is accounted for */
            error_stack = 1;
         }
      }

      /* ------------------------------ */
      if ( ap->option == StrStr(ap->option, "format_version") )
      {
         if ( is_numeric(ap->value) )
         {
            bcdd->fmt_version = atoi(ap->value);
            parse_flag = 1;
         }
         else
         {
            error_msg("ERROR: Unable to parse \"format_version\" value.");
            parse_flag = 1; /* We did not fully parse it, but error is accounted for */
            error_stack = 1;
         }
      }

      /* ------------------------------ */
      if ( ap->option == StrStr(ap->option, "roll_at") )
      {
         item_parsed = 0; 

         if ( parse_roll_at(bcdd, ap->value) )
         {
            error_msg("ERROR: Unable to parse \"roll_at\" value.");
            error_stack = 1;
         }
         else
         {
            item_parsed = 1;
            parse_flag = 1;
         }
      }

      /* ------------------------------ */
      if ( ap->option == StrStr(ap->option, "skip_compress") )
      {
         item_parsed = 0; 

         if ( NULL == ap->value )
         {
            bcdd->compress_flags |= COMPRESS_SKIP;
            item_parsed = 1;
            parse_flag = 1;
         }

         if (( StrStr(ap->value, "TRUE") )   ||
             ( StrStr(ap->value, "ON") )     ||
             ( 0 == strcmp(ap->value, "1") ) ||
             ( 0 == strcmp(ap->value, "T") ) ||
             ( 0 == strcmp(ap->value, "t") ))
         {
            bcdd->compress_flags |= COMPRESS_SKIP;
            item_parsed = 1;
            parse_flag = 1;
         }

         if (( StrStr(ap->value, "FALSE") )   ||
             ( StrStr(ap->value, "OFF") )     ||
             ( 0 == strcmp(ap->value, "0") ) ||
             ( 0 == strcmp(ap->value, "F") ) ||
             ( 0 == strcmp(ap->value, "f") ))
         {
            item_parsed = 1;
            parse_flag = 1;
         }

         if ( item_parsed == 0 )
         {
            error_msg("ERROR: Unable to parse \"skip_compress\" value.");
            error_stack = 1;
         }
      }

      /* ------------------------------ */
      if ( ap->option == StrStr(ap->option, "gzip_compress") )
      {
         item_parsed = 0; 

         if ( NULL == ap->value )
         {
            bcdd->compress_flags |= COMPRESS_GZIP;
            item_parsed = 1;
            parse_flag = 1;
         }

         if (( StrStr(ap->value, "TRUE") )   ||
             ( StrStr(ap->value, "ON") )     ||
             ( 0 == strcmp(ap->value, "1") ) ||
             ( 0 == strcmp(ap->value, "T") ) ||
             ( 0 == strcmp(ap->value, "t") ))
         {
            item_parsed = 1;
            parse_flag = 1;
            bcdd->compress_flags |= COMPRESS_GZIP;
         }

         if (( StrStr(ap->value, "FALSE") )   ||
             ( StrStr(ap->value, "OFF") )     ||
             ( 0 == strcmp(ap->value, "0") ) ||
             ( 0 == strcmp(ap->value, "F") ) ||
             ( 0 == strcmp(ap->value, "f") ))
         {
            item_parsed = 1;
            parse_flag = 1;
         }

         if ( item_parsed == 0 )
         {
            error_msg("ERROR: Unable to parse \"gzip_compress\" value.");
            error_stack = 1;
         }
      }

      /* ------------------------------ */
      if ( ap->option == StrStr(ap->option, "filename") )
      {
         /* Basically require that the local filename (static array) hold
            the input filename plus 8 characters and a period. That is 
            ten-million -1 files that can roll. Quite safe. */
         if ( strlen(ap->value) >= (MAX_FN_LEN - 10) )
         {
            error_msg("ERROR: The value used for \"filename\" is too long.");
            error_stack = 1;
         }

         if( NULL == ( bcdd->filename = mkstring(ap->value) ) )
         {
            error_msg("ERROR: Unable allocate memory for writer filename.");
            return(NULL);
         }
         parse_flag = 1;
      }

      if ( 0 == parse_flag )
      {
         error_msg("ERROR: Unable to parse: \"%s\" line.", ap->option);
         error_stack = 1;
      }

      ap = ap->next;
   } /* while(ap) */


   /* === Validate input === */
   /* Don't try to clean up we will be dying soon anyways. */
   if ( NULL == bcdd->filename )
   {
      error_msg("ERROR: No \"filename\" was specified for bcd writer module.");
      error_stack = 1;
   }

   if ( bcdd->fmt_version > 1 )
   {
      error_msg("ERROR: The requested BCD format is not currently implemented.");
      error_stack = 1;
   }

   if ( 2 > bcdd->keep_files )
   {
      error_msg("ERROR: The number of kept files must be greater than 1.");
      error_stack = 1;
   }



   /* Walk the list of output items looking for unsupported data types */
   pi = bcdd->p->pi_olist;
   diff_wanted = 0;
   fact_wanted = 0;
   while ( pi )
   {
      switch ( pi->data_type )
      {
      case PDT_INT8:
      case PDT_UINT8:
      case PDT_INT16:
      case PDT_UINT16:
      case PDT_INT32:
      case PDT_UINT32:
      case PDT_INT64:
      case PDT_UINT64:
      case PDT_STRING:
         if( init_compressability(pi, bcdd->compress_flags & COMPRESS_SKIP) )
            return(NULL);
         break;
      case PDT_UNKNOWN:
      case PDT_TIMEVAL:
      case PDT_TIMESPEC:
         error_msg("ERROR: Compound types not supported with BCD writer.");
         required_version = 2;
         break;
      }

      /* This will be handled in v2 - so not necessarily an *error*. */
      if ( pi->munge_flag & MUNGE_DIFF )
         diff_wanted = 1;

      if ( pi->munge_flag & MUNGE_FACTOR )
         fact_wanted = 1;

      pi = pi->next_opi;
   }

   if ( diff_wanted )
   {
      required_version = 2;

      if ( bcdd->fmt_version < 2 )
      {
         /* GOAT: This is a temporary message - Remove with V2 implementation */
         error_msg("ERROR: Diffs of data is not supported with BCD writer.");
         error_stack = 1;
      }
   }

   if ( fact_wanted )
   {      
      error_msg("ERROR: Factoring is not supported with the BCD writer.");
      error_stack = 1;
   }

   /* Errors have built up, now give up */
   if ( error_stack )
   {
      return(NULL);
   }

   if ( init_file_output(bcdd) )
      return(NULL);

#ifdef DEBUGGERY
   fprintf(stderr, "struct bcd_data\n{\n");
   fprintf(stderr, "   TYPE = %d;\n", bcdd->TYPE);
   fprintf(stderr, "   compress_flags = %d;\n", bcdd->compress_flags);
   fprintf(stderr, "   roll_on_type = %d;\n", bcdd->roll_on_type);
   fprintf(stderr, "   filename = %s;\n", bcdd->filename);
   fprintf(stderr, "   first_insert = %d;\n", bcdd->first_insert);
   fprintf(stderr, "   sz_roll_at = %d;\n", bcdd->sz_roll_at);
   fprintf(stderr, "   sz_next_roll = %d;\n", bcdd->sz_next_roll);
   fprintf(stderr, "   tm_roll_at = %d;\n", bcdd->tm_roll_at);
   fprintf(stderr, "   tm_next_roll = %d;\n", bcdd->tm_next_roll);
   fprintf(stderr, "   keep_files = %d;\n", bcdd->keep_files);
   fprintf(stderr, "   fmt_version = %d;\n", bcdd->fmt_version);
   fprintf(stderr, "   endian_flag = %d;\n", bcdd->endian_flag);
   fprintf(stderr, "};\n");
#endif

   return(rv);
}

/* ========================================================================= */
/* This is just a strategy function that directs us to the correct version
   handler. */
int WriteBCDWM(void *data_handle)
{
   struct bcd_data *bcdd;
   int rv = 1;

   if ( NULL == (bcdd = (struct bcd_data *)data_handle) )
      return(1);

   if ( 1 == bcdd->fmt_version )
      rv = write_v1_bcd_line(bcdd);

   if ( should_roll_file_output(bcdd) )
      roll_file_output(bcdd);

   return(rv);
}

/* ========================================================================= */
int FinishBCDWM(void *data_handle)
{
   struct bcd_data *bcdd;

   if ( NULL == (bcdd = (struct bcd_data *)data_handle) )
      return(1);

   if ( 1 == bcdd->fmt_version )
      return(close_v1_bcd_file(bcdd));

   return(1);
}

/* ========================================================================= */
time_t get_next_alarm(time_t offset)
{
   time_t now;
   time_t alarm;

   time(&now);
   alarm = LastMidnight();
   alarm += offset;

   if ( now >= alarm )
      alarm += 24 * 60 * 60;

   return(alarm);
}

/* ========================================================================= */
int parse_roll_at(struct bcd_data *bcdd, char *value)
{
   unsigned long long numeric;
   long  hour, minute, second;


   /* Test for the never string first */
   if ( StrStr(value, "never") )
   {
      bcdd->roll_on_type = ROLL_ON_NEVER;
      return(0);
   }

   /* Chew up leading whitespace (if any) */
   while(*value == ' ')
      value++;

   /* Parse the numeric part */
   numeric = 0;
   while((*value >= '0') && (*value <= '9'))
   {
      numeric *= 10;
      numeric += (*value - '0');

      value++;
   }

   /* See if we got a leading number */
   if ( 0 == numeric )
   {
      /* If not, it needs to have a leading "at " string" */
      if ( StrStr(value, "at ") )
      {
         value += 3;

         hour = 0;
         minute = 0;
         second = 0;

         while((*value >= '0') && (*value <= '9'))
         {
            hour *= 10;
            hour += (*value - '0');
            
            value++;
         }

         if ( *value == ':' )
         {
            value++;
            
            while((*value >= '0') && (*value <= '9'))
            {
               minute *= 10;
               minute += (*value - '0');
               
               value++;
            }

            if ( *value == ':' )
            {
               value++;
            
               while((*value >= '0') && (*value <= '9'))
               {
                  second *= 10;
                  second += (*value - '0');
               
                  value++;
               }
            }
            else
            {
               if (( *value != ' ' ) && ( *value != 0 ))
                  return(1);
            }
         }
         else
         {
            if (( *value != ' ' ) && ( *value != 0 ))
               return(1);
         }

         /* This is the offset */
         bcdd->tm_roll_at = (((hour * 60) + minute) * 60) + second;
         /* This is the actual alarm */
         bcdd->tm_next_roll = get_next_alarm(bcdd->tm_roll_at);
         bcdd->roll_on_type = ROLL_AT_TIME;

         return(0);

      } /* if ( "at " ) */
      else
      {
         /* Unable to parse this:
            - Not a leading numeric
            - Not an "at" directive */
         return(1);
      }
   }

   /* Chew up leading whitespace (if any) */
   while(*value == ' ')
      value++;

   /* A quasi-switch() for quantifiers */
   if (( StrStr(value, "meg" ) ) || ( value == StrStr(value, "m" ) ))
   {
      bcdd->roll_on_type = ROLL_ON_SIZE;
      bcdd->sz_roll_at = numeric * (1024 * 1024);
      bcdd->sz_next_roll = 0;
      return(0);
   }

   if (( StrStr(value, "gig" ) ) || ( value == StrStr(value, "g" ) ))
   {
      bcdd->roll_on_type = ROLL_ON_SIZE;
      bcdd->sz_roll_at = numeric * (1024 * 1024 * 1024);
      bcdd->sz_next_roll = 0;
      return(0);
   }

   if (( StrStr(value, "hour" ) ) || ( value == StrStr(value, "h" ) ))
   {
      bcdd->roll_on_type = ROLL_ON_TIME;
      bcdd->tm_roll_at = numeric * (60 * 60);
      bcdd->tm_next_roll = 0;
      return(0);
   }

   if (( StrStr(value, "day" ) ) || ( value == StrStr(value, "d" ) ))
   {
      bcdd->roll_on_type = ROLL_ON_TIME;
      bcdd->tm_roll_at = numeric * (60 * 60 * 24);
      bcdd->tm_next_roll = 0;
      return(0);
   }

   return(1);
}

/* ========================================================================= */
int new_v1_bcd_file(struct bcd_data *bcdd)
{
   struct pitem *pi;
   char filename[MAX_FN_LEN];
   unsigned char version;
   unsigned char endian_flag;
   int pitem_count;
   unsigned char hi_cnt;
   unsigned char lo_cnt;
   unsigned char data_type;
   unsigned char hdr_len;
   unsigned char data_size;

   assert(NULL != bcdd);
   assert((NULL == bcdd->gd) && (-1 == bcdd->fd));
   assert(NULL != bcdd->filename);

   /* The filename we will be working on is *always* <filename>.0 */
   sprintf(filename, "%s.0", bcdd->filename);

   /* Just yank anything in existence. This prevents simultaneous opens. */
   /* This behaviour might want to be changed later. */
   unlink(filename);


   if ( bcdd->compress_flags & COMPRESS_GZIP )
   {
      if (NULL == (bcdd->gd = gzopen(filename, "wb")))
      {
         error_msg("Failed to open %s as a GZ file.\n", filename);
         return(1);
      }
   }
   else
   {
      if (-1 == (bcdd->fd = open(filename, O_WRONLY | O_TRUNC | O_CREAT, 0666)))
      {
         error_msg("Failed to open %s file.\n", filename);
         return(1);
      }
   }

   version = bcdd->fmt_version;
   endian_flag = bcdd->endian_flag;

   if (bcd_write(bcdd, "CBCD", 4)) return(1);
   if (bcd_write(bcdd, &version, 1)) return(1);
   if (bcd_write(bcdd, &endian_flag, 1)) return(1);

   /* Count the number of pitems */
   pitem_count = 0;
   pi = bcdd->p->pi_olist; /* Position at top of output list */
   while ( pi )
   {
      pitem_count++;
      pi = pi->next_opi;
   }

   hi_cnt = pitem_count / 256;
   lo_cnt = pitem_count % 256;

   /* Write a byte-order independent count. The count is
      explicitly given here, and can be independently derived from
      the list that follows. This forms sort of a "checksum" on he 
      header. */
   if (bcd_write(bcdd, &hi_cnt, 1)) return(1);
   if (bcd_write(bcdd, &lo_cnt, 1)) return(1);
   
   pi = bcdd->p->pi_olist; /* Position at top of output list */
   while ( pi )
   {
      data_type = pi->data_type;
      hdr_len = (unsigned char)strlen(pi->header);

      switch(data_type)
      {
      case PDT_INT8:
      case PDT_UINT8:
         data_size = sizeof(int8_t);
         break;
      case PDT_INT16:
      case PDT_UINT16:
         data_size = sizeof(int16_t);
         break;
      case PDT_INT32:
      case PDT_UINT32:
         data_size = sizeof(int32_t);
         break;
      case PDT_INT64:
      case PDT_UINT64:
         data_size = sizeof(int64_t);
         break;
      case PDT_STRING:
         data_size = 1; /* This is the guarenteed size, nothing more */
         break;
      case PDT_TIMEVAL:
      case PDT_TIMESPEC:
      default:
         fprintf(stderr, "ERROR: Unhandled data type (%d).\n", data_type);
         exit(1);
      }

      if (bcd_write(bcdd, &data_type, 1)) return(1);
      if (bcd_write(bcdd, &data_size, 1)) return(1);
      if (bcd_write(bcdd, &hdr_len, 1)) return(1);
      if (bcd_write(bcdd, pi->header, hdr_len)) return(1);

      pi = pi->next_opi;
   }

   /* Terminate the list of data items - This is a "triple null" */
   data_type = 0;
   hdr_len = 0;
   data_size = 0; /* An impossible size */

   if (bcd_write(bcdd, &data_type, 1)) return(1);
   if (bcd_write(bcdd, &data_size, 1)) return(1);
   if (bcd_write(bcdd, &hdr_len, 1)) return(1);

   return(0);
}

/* ========================================================================= */
int bcd_write(struct bcd_data *bcdd, void *buf, size_t len)
{
   ssize_t wrote;

   if ( bcdd->compress_flags & COMPRESS_GZIP )
   {
      wrote = gzwrite(bcdd->gd, buf, len);
   }
   else
   {
      wrote = write(bcdd->fd, buf, len);
   }

   if ( wrote > 0 )
   {
      bcdd->sz_next_roll += wrote;
      return(0);
   }

   return(1);
}

/* ========================================================================= */
int write_v1_bcd_line(struct bcd_data *bcdd)
{
   struct pitem *thispi;
   int i;
   int byte_loc;
   int bit_loc;
   int this_bit;

   /* NULL out the cbits */
   i = 0;
   while ( i < bcdd->cbyte_count )
      bcdd->cbits[i++] = 0;

   /* Handle the first insert */
   if ( bcdd->first_insert )
   {
      /* If the data skip compression method is enabled, then start
         skipping data on the next iteration. If not, then treat every
         row write as a "first" (no skip) write. */
      if ( bcdd->compress_flags & COMPRESS_SKIP )
         bcdd->first_insert = 0;
      /* fprintf(stderr, "DEBUG: first insert\n"); */

      /* cbits are all empty at this point */
      bcd_write(bcdd, &bcdd->cbits, bcdd->cbyte_count);

      thispi = bcdd->p->pi_olist; /* Position at top of output list */
      while(thispi)
      {
         write_cdata(bcdd, thispi);
         thispi = thispi->next_opi;
      }

      return(0);
   }

   /* Default/fall-through - Handle a compressed insert */
   /* fprintf(stderr, "DEBUG: subsiquent insert\n"); */
   bit_loc = 0;

   thispi = bcdd->p->pi_olist; /* Position at top of output list */
   while(thispi)
   {
      this_bit = get_compressability(thispi);
      /*
      if ( this_bit )
         fprintf(stderr, "DEBUG: Compressible data\n");
      */
      /* insert it into the cbits array */
      byte_loc = bit_loc / 8;      /* Determine what byte we are modifying */
      this_bit  = this_bit << (7 - (bit_loc % 8));   /* Shift the bit into the right location */
      bcdd->cbits[byte_loc] |= this_bit; /* Set the appropriate bit */

      bit_loc++;
      thispi = thispi->next_opi;
   }

   /* DEBUG: bcd_write(bcd, "[", 1); */
   bcd_write(bcdd, bcdd->cbits, bcdd->cbyte_count);
   /* * /
   fprintf(stderr, "DEBUG: compress is %02x\n", bcdd->cbits[0]);
   / * */

   thispi = bcdd->p->pi_olist; /* Position at top of output list */
   while(thispi)
   {
      write_cdata(bcdd, thispi);
      thispi = thispi->next_opi;
   }

   /* bcd_write(bcd, "]", 1); */

   return(0);
}

/* ========================================================================= */
int mk_wdata(struct pitem *thispi, int size)
{
   struct last_value *wdata;

   /* Insure that we are called on non-initialized members */
   if ( NULL != thispi->wdata )
      return(1);

   /* Create the struct */
   if ( NULL == (wdata = (struct last_value *)malloc(sizeof(struct last_value))) )
      return(1);
   
   /* Create the data location */
   if ( NULL == (wdata->data_ptr = malloc(size)) )
   {
      free(wdata);
      return(1);
   }

   /* Set the value(s) */
   /* NOTE: memcpy will copy beyond the end of the string. This is a bit
            dangerous, but it will copy *something* into the new location and
            it will just not be used. */
   memcpy(wdata->data_ptr, thispi->data_ptr, size);
   wdata->sign_flag = thispi->sign_flag;
   wdata->skippable = 0;
   /* data_sz is really about the size of the string - but just set if for
      everything. */
   wdata->data_sz = size;
   
   /* Attach the new struct to the pitem */
   thispi->wdata = (void *)wdata;

   return(0);
}

/* ========================================================================= */
/* The point: To find an optimal size for a string based upon expected growth
   over the life of the string. Always size-up and size to a "round" number. */
int get_best_sz(void *vstr)
{
   int size;
   char *str = (char *)vstr;
   
   size = strlen(str);

   if ( size < 32 )
      return(64);

   /* This code is likely not reachable. It really should be an assertion */
   if ( size < 64 )
      return(128);

   if ( size < 128 )
      return(256);

   if ( size < 256 )
      return(512);

   return(size * 2);
}
   
/* ========================================================================= */
int init_compressability(struct pitem *thispi, int skip_on)
{
   if ( skip_on )
   {
      switch(thispi->data_type)
      {
      case PDT_INT8:
      case PDT_UINT8:
         return(mk_wdata(thispi, 1));
         break;
      case PDT_INT16:
      case PDT_UINT16:
         return(mk_wdata(thispi, 2));
         break;
      case PDT_INT32:
      case PDT_UINT32:
         return(mk_wdata(thispi, 4));
         break;
      case PDT_INT64:
      case PDT_UINT64:
         return(mk_wdata(thispi, 8));
         break;
      case PDT_STRING:
         return(mk_wdata(thispi, get_best_sz(thispi->data_ptr)));
         break;
      }
   }
   else
   {
      thispi->wdata = NULL;
      return(0);
   }
   
   /* Default - fall through to unsupported data type */
   return(1);
}

/* ========================================================================= */
unsigned char get_compressability(struct pitem *thispi)
{
   void *last_data_ptr;
   void *this_data_ptr;
   struct last_value *wdata;

   /* Notes on the skippable value
      I set it to 0 as soon as possible because all the elimination options
      at the top of the function are non-skippable situations. I set it to
      1 at the top of the switch because all exit conditions in the switch
      are *only* skippable. I set it to 0 after the switch because the fall
      through behaviour is to not-skip.
   */

   /* no historical data, therefore not compressable */
   if ( NULL == ( wdata = (struct last_value *)thispi->wdata ) )
      return(0);

   wdata->skippable = 0;      

   last_data_ptr = wdata->data_ptr;
   this_data_ptr = thispi->data_ptr;

   /* Check the sign bit first - that can be done universally */
   /* STUB: Document that the sign flag must always be set, even in cases
      STUB:   where the data type does not use a sign (like a string) because
      STUB:   of this comparison. */
   if ( wdata->sign_flag != thispi->sign_flag )
      return(0);
      
   /* The alternate here is to compare memory locations, but we still need a 
      switch statement to get the size of the location. So just do compares.
   */
   wdata->skippable = 1;
   switch(thispi->data_type)
   {
   case PDT_INT8:
      if ( *((int8_t *)last_data_ptr) == *((int8_t *)this_data_ptr) )
         return(1);
      break;
   case PDT_UINT8:
      if ( *((uint8_t *)last_data_ptr) == *((uint8_t *)this_data_ptr) )
         return(1);
      break;
   case PDT_INT16:
      if ( *((int16_t *)last_data_ptr) == *((int16_t *)this_data_ptr) )
         return(1);
      break;
   case PDT_UINT16:
      if ( *((uint16_t *)last_data_ptr) == *((uint16_t *)this_data_ptr) )
         return(1);
      break;
   case PDT_INT32:
      if ( *((int32_t *)last_data_ptr) == *((int32_t *)this_data_ptr) )
         return(1);
      break;
   case PDT_UINT32:
      if ( *((uint32_t *)last_data_ptr) == *((uint32_t *)this_data_ptr) )
         return(1);
      break;
   case PDT_INT64:
      if ( *((int64_t *)last_data_ptr) == *((int64_t *)this_data_ptr) )
         return(1);
      break;
   case PDT_UINT64:
      if ( *((uint64_t *)last_data_ptr) == *((uint64_t *)this_data_ptr) )
         return(1);
      break;
   case PDT_STRING:
      if ( 0 == strcmp(last_data_ptr, this_data_ptr) )
         return(1);
      break;
   }
   
   wdata->skippable = 0;
   return(0);
}

/* ========================================================================= */
int close_v1_bcd_file(struct bcd_data *bcdd)
{
   if ( NULL == bcdd )
      return(0);

   if ( bcdd->compress_flags & COMPRESS_GZIP )
   {
      gzclose(bcdd->gd);
      bcdd->gd = NULL;
   }
   else
   {
      close(bcdd->fd);
      bcdd->fd = -1;
   }

   return(0);
}

/* ========================================================================= */
int write_cdata(struct bcd_data *bcdd, struct pitem *thispi)
{
   uint8_t strsize;
   /* wdata exists as a separate variable primarily because it is a void 
      pointer in the pitem struct so casting it becomes an annoyance. This
      is partially a means to make the explicit pointer casting to go away
      but also a means to deal with the fact that wdata might be NULL. It
      is NULL when there is no skip compression. (And that is a bad idea
      as it might not be null in the v2 format and that would wreck the 
      already crappy design. */
   struct last_value *wdata;
   /* dowrite is set up because we cannot directly (always) test for 
      wdata->skippable. And... the fact that skippable is a really stupid
      means of describing this operation. Do you write or not - that is 
      the question. */
   int dowrite = 1;

   /* * /
#define DEBUG_BIN_FILE
   / * */
#ifdef DEBUG_BIN_FILE
   bcd_write(bcdd, "[", 1);
#endif

   /* Move (cast) the pitem->wdata pointer into the local wdata pointer. */
   if ( NULL == (wdata = (struct last_value *)thispi->wdata) )
   {
      /* if it is NULL, then skip compression is off and we should write */
      dowrite = 1;
   }
   else
   {
      /* If the data is skippable, then we should not write */
      if ( wdata->skippable )
         dowrite = 0;
      else
         dowrite = 1; /* Otherwise we should write. */
   }

   if ( dowrite )
   {
      switch(thispi->data_type)
      {
      case PDT_INT8:
         bcd_write(bcdd, thispi->data_ptr, sizeof(int8_t));
         break;
      case PDT_UINT8:
         bcd_write(bcdd, thispi->data_ptr, sizeof(int8_t));
         break;
      case PDT_INT16:
         bcd_write(bcdd, thispi->data_ptr, sizeof(int16_t));
         break;
      case PDT_UINT16:
         bcd_write(bcdd, thispi->data_ptr, sizeof(int16_t));
         break;
      case PDT_INT32:
         bcd_write(bcdd, thispi->data_ptr, sizeof(int32_t));
         break;
      case PDT_UINT32:
         bcd_write(bcdd, thispi->data_ptr, sizeof(int32_t));
         break;
      case PDT_INT64:
         bcd_write(bcdd, thispi->data_ptr, sizeof(int64_t));
         break;
      case PDT_UINT64:
         bcd_write(bcdd, thispi->data_ptr, sizeof(int64_t));
         break;
      case PDT_STRING:
         strsize = strlen(thispi->data_ptr);
         bcd_write(bcdd, &strsize, 1);
         bcd_write(bcdd, thispi->data_ptr, strsize);
         break;
      } /* switch(data_type) */
   }

#ifdef DEBUG_BIN_FILE
   bcd_write(bcdd, "]", 1);
#endif

   if ( wdata )
   {
      wdata->skippable = 0;

      /* NOTE: This entire switch statement could be replaced with the memcpy()
               that is used for the string. */
      switch(thispi->data_type)
      {
      case PDT_INT8:
         *((int8_t *)wdata->data_ptr) = *((int8_t *)thispi->data_ptr);
         break;
      case PDT_UINT8:
         *((uint8_t *)wdata->data_ptr) = *((uint8_t *)thispi->data_ptr);
         break;
      case PDT_INT16:
         *((int16_t *)wdata->data_ptr) = *((int16_t *)thispi->data_ptr);
         break;
      case PDT_UINT16:
         *((uint16_t *)wdata->data_ptr) = *((uint16_t *)thispi->data_ptr);
         break;
      case PDT_INT32:
         *((int32_t *)wdata->data_ptr) = *((int32_t *)thispi->data_ptr);
         break;
      case PDT_UINT32:
         *((uint32_t *)wdata->data_ptr) = *((uint32_t *)thispi->data_ptr);
         break;
      case PDT_INT64:
         *((int64_t *)wdata->data_ptr) = *((int64_t *)thispi->data_ptr);
         break;
      case PDT_UINT64:
         *((uint64_t *)wdata->data_ptr) = *((uint64_t *)thispi->data_ptr);
         break;
      case PDT_STRING:
         /* Brute force is easiest */
         memcpy(wdata->data_ptr, thispi->data_ptr, wdata->data_sz);
      } /* switch(data_type) */
   }

   return(0);
}

/* ========================================================================= */
int init_file_output(struct bcd_data *bcdd)
{
   char filename[MAX_FN_LEN];
   unsigned short roll;
   unsigned short stoproll;

   assert(bcdd != NULL);
   assert(bcdd->filename != NULL);

   /* Wack everything that meets the pattern (including the basename) */
   unlink(bcdd->filename);
   roll = 0;
   stoproll = bcdd->keep_files + 3;
   while ( roll < stoproll )
   {
      sprintf(filename, "%s.%u", bcdd->filename, roll);

      if ( unlink(filename) )
      {
         if ( errno != ENOENT )
         {
            error_msg("ERROR: Failed to remove %s.", filename);
            return(1);
         }
      }
      else
         stoproll++; /* Keep deleting if they exist */

      roll++;
   }

   /* Starting at 0 (this is all about init) */
   sprintf(filename, "%s.0", bcdd->filename); 

   switch ( bcdd->fmt_version )
   {
   case 1:
      return(new_v1_bcd_file(bcdd));
      break;
   }
   /* default will fall-through */

   /* This *should* be unreachable as invalid formats should have been
      tested for previously */
   return(1);
}

/* ========================================================================= */
int should_roll_file_output(struct bcd_data *bcdd)
{
   time_t now;

   //if ( bcdd->roll_on_type == ROLL_ON_NEVER )

   switch ( bcdd->roll_on_type )
   {
   case ROLL_ON_NEVER:
      return(0);
      break;
   case ROLL_ON_SIZE:
      if ( bcdd->sz_next_roll >= bcdd->sz_roll_at )
         return(1);
      else
         return(0);
      break;
   case ROLL_ON_TIME:
      time(&now);
      if ( bcdd->tm_next_roll >= now )
         return(1);
      else
         return(0);
      break;
   case ROLL_AT_TIME:
      time(&now);
      if ( now >= bcdd->tm_next_roll )
         return(1);
      else
         return(0);
      break;
   }

   return(-1);
}

/* ========================================================================= */
int roll_file_output(struct bcd_data *bcdd)
{
   char fn_to[MAX_FN_LEN];
   char fn_from[MAX_FN_LEN];
   int rollnum;

   assert(NULL != bcdd);
   assert(NULL != bcdd->filename);
   assert(1 <= bcdd->keep_files);

   /* Close the existing file */
   switch ( bcdd->fmt_version )
   {
   case 1:
      close_v1_bcd_file(bcdd);
      break;
      /* This should be an error condition. But this is checked elsewhere */
   }

   /* Set up for the next file */
   switch ( bcdd->roll_on_type )
   {
   case ROLL_ON_SIZE:
      bcdd->sz_next_roll = 0;
      break;
   case ROLL_ON_TIME:
      bcdd->tm_next_roll += bcdd->tm_roll_at;
      break;
   case ROLL_AT_TIME:
      bcdd->tm_next_roll = get_next_alarm(bcdd->tm_roll_at);
      break;

   }

   /* Roll the files */
   rollnum = bcdd->keep_files - 1;
   while ( rollnum > 0 )
   {
      sprintf(fn_from, "%s.%d", bcdd->filename, rollnum - 1);
      sprintf(fn_to,   "%s.%d", bcdd->filename, rollnum);
      /* Ignore the rename() errors. Not the best situation as it is *possible*
         that someone could have chown() or chmod() one of the files and then
         that rename will fail and basically cause a blackhole that the data
         will fall into. */
      rename(fn_from, fn_to);
      rollnum--;
   }
      
   switch ( bcdd->fmt_version )
   {
   case 1:
      return(new_v1_bcd_file(bcdd));
      break;
   }
   /* default will fall-through */

   /* This *should* be unreachable as invalid formats should have been
      tested for previously */
   return(1);
}
   

