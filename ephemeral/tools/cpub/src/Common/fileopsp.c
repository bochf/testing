#ifdef PORT_Linux
/* See: http://yarchive.net/comp/linux/o_direct.html */
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>

#include "../Stevens.h"
#include "../mainhelp.h"
#include "fileopsp.h"


/* ========================================================================= */
/*
  Notes on this provider:
  - A FULL path to the file name is required. The initial test will be
    run as a interactive program with a CWD different than the running
    version that may be a daemon with a CWD of /.
  - The opsize will be one of the different types. For example, when
    listing the pitems there should be:
      file.ops:*:4Bopen    <----- Super small
      file.ops:*:4Kopen    <----- One page
      file.ops:*:4Mopen    <----- 1024x larger...
  - The listing has very specific op-sizes, but the tool supports more
    general op-sizes (such as 512B, 513b, 1024K, 1m...).
  - psavg, diff, and factor are all invalid input and will be considered
    errors.
  - sioffset is NOT used as an offset. It is used as a flag that determines
    what piece of data is requested. The TS_* defines are what is assigned
    to this item.
  - Time is expressed in a 64bit value as the resolution of gettimeofday() /
    struct timeval (microseconds (1-millionth of a sec)). The time is always
    diffed - so it makes little sense to use a timeval as the output data
    type.
*/

/* ========================================================================= */
/* These are IDs that are put in sioffset */
#define TS_START   0
#define TS_AOPEN   1
#define TS_AREAD   2
#define TS_ASEEK   3
#define TS_AWRIT   4
#define TS_ACLOS   5
#define TS_TOTAL   6
#define TS_FINSH   TS_TOTAL

/* ========================================================================= */
struct file_ops
{
   char *fname;      /* Full path file name to run against */
   
   void *buffer;     /* Buffer to use for the operation    */
   size_t opsize;    /* How big the buffer is              */

   int last_op_rv;   /* Results of the last operation      */

   struct timeval  start_time;
   struct timeval  open_time;
   struct timeval  read_time;
   struct timeval  seek_time;
   struct timeval  write_time;
   struct timeval  close_time;

   struct file_ops *next;
};

/* ========================================================================= */
/* The items we support */
static const char *diname[] = {"open",       "read",       "lseek",     \
                               "write",      "close",      "total",     \
                               NULL};
static const int ditype[]  =  {PDT_UINT64,   PDT_UINT64,   PDT_UINT64,  \
                               PDT_UINT64,   PDT_UINT64,   PDT_UINT64,  \
                               };

static struct provider *myself;
static struct file_ops *folist;


/* ========================================================================= */
int init_file(char *filename, size_t opsize);
struct file_ops *new_file_ops(char *filename, size_t opsize);
struct file_ops *get_file_ops(char *filename, size_t opsize);
int parse_iname(unsigned long *sioffset, unsigned long *readsz, const char *iname);
int run_file_ops(struct file_ops *fo);
int snap_time(struct file_ops *fo, int tN);
uint64_t diff_time(struct timeval *ts, struct timeval *te);
int pitem_update(struct pitem *this_pitem);
/* --- The "three" functions --- */
int fops_list(int dflag);
int fops_update(int interval);
struct pitem *fops_enablepitem(struct qparts *qp);

/* ========================================================================= */
int init_file(char *filename, size_t opsize)
{
   int fd;
   int oflag;
   char buf[64];
   size_t left_to_write;
   size_t write_this_time;
   size_t written;

   /* Validate input */
   assert( filename != NULL );
   assert( opsize > 0 );

   /* Fill buffer (pad with spaces for optimal "block" size) */
   strcpy(buf, "CPUB file.perfops file. Do not delete if CPUB running.         ");
   /* buf[62] = ' '; */
   buf[63] = '\n';
   /* The "string" is NOT terminated. We will write a line without termination. */

   oflag = O_WRONLY | O_CREAT | O_EXCL;
   if( -1 == ( fd = open(filename, oflag, 0666) ) )
      return(1);

   left_to_write = opsize;
   while ( left_to_write > 0 )
   {
      if ( left_to_write > 64 )
         write_this_time = 64;
      else
         write_this_time = left_to_write;

      written = Write(fd, buf, write_this_time);

      if ( written != write_this_time )
      {
         close(fd);
         return(1);
      }

      left_to_write -= written;
   }

   fsync(fd);
   close(fd);

   return(0);
}

/* ========================================================================= */
struct file_ops *new_file_ops(char *filename, size_t opsize)
{
   struct file_ops *fo;
   char shortfn[48];

   assert ( filename != NULL );
   assert ( filename[0] != 0 );
   assert ( opsize > 0 );

   /* Not allowed to overwrite a file! */
   if ( file_exists(filename) )
   {
      dotdotdot_string(shortfn, filename, 32);
      error_msg("ERROR: \"%s\" exists. Overwrite not allowed.", shortfn);
      return(NULL);
   }

   /* Allocate the memory structures for the file */
   if ( NULL == ( fo = (struct file_ops *)malloc(sizeof(struct file_ops)) ) )
   {
      error_msg("ERROR: Unable to allocate memory in file.perfops setup.");
      return(NULL);
   }

   if ( NULL == ( fo->fname = (char *)malloc(strlen(filename) + 1)) )
   {
      error_msg("ERROR: Unable to allocate memory in file.perfops string.");
      return(NULL);
   }

   if ( NULL == ( fo->buffer = malloc(opsize) ) )
   {
      error_msg("ERROR: Unable to allocate memory in file.perfops buffer.");
      return(NULL);
   }

   /* Copy in the data */
   strcpy(fo->fname, filename);
   fo->opsize = opsize;
   fo->last_op_rv = -1;
   fo->next = NULL;

   if ( init_file(filename, opsize) )
   {
      dotdotdot_string(shortfn, fo->fname, 48);
      error_msg("ERROR: Unable to initialize \"%s\".", shortfn);
      return(NULL);
   }

   /* Try to do a run of the file ops on the file */
   if ( run_file_ops(fo) )
   {
      dotdotdot_string(shortfn, fo->fname, 38);
      error_msg("ERROR: Unable to run performance ops on \"%s\".", shortfn);
      return(NULL);
   }

   return(fo);
}

/* ========================================================================= */
struct file_ops *get_file_ops(char *filename, size_t opsize)
{
   struct file_ops *fothis;

   /* No list, so make a new item */
   if ( NULL == folist )
   {
      folist = new_file_ops(filename, opsize);
      return(folist);
   }

   /* List exists, so look for a match in the list */
   fothis = folist;
   while ( fothis )
   {
      /* Discussion: You cannot have two op sizees on the same file.
         So we can test for it here, or we can let the init happen,
         and the second (with a different size) will fail on init.
         If we tested here we would look for the same filename, and
         then check to insure that the opsizes were the same. Which
         still is not a bad idea. */
      if (( 0 == strcmp(fothis->fname, filename) ) && ( fothis->opsize == opsize ))
         return(fothis);

      fothis = fothis->next;
   }

   /* No match, so make one. */
   if ( NULL != (fothis = new_file_ops(filename, opsize)) )
   {
      /* Into the linked list you go */
      fothis->next = folist;
      folist = fothis;
   }

   /* Return whatever this is, good or bad */
   return(fothis);  
}

/* ========================================================================= */
struct pitem *fops_enablepitem(struct qparts *qp)
{
   struct pitem *this_pitem;
   struct file_ops *fothis;
   void *timevalptr;
   unsigned long sioffset;   /* Used to hold the operation of the pitem */
   unsigned long opsize;

   /* Make sure we were called with the right quad (provider) name */
   if ( 0 != strcmp(qp->pname, "file.perfops") )
      return(NULL);

   if ( qp->pargs[0] == 0 )
   {
      error_msg("ERROR: No filename specified in file.perfops quad.");
      return(NULL);
   }

   /* No relative pathing */
   if ( qp->pargs[0] != '/' )
   {
      error_msg("ERROR: file.perfops filenames must be fully pathed.");
      return(NULL);
   }

   /* Parse iname */
   if ( parse_iname(&sioffset, &opsize, qp->iname) )
   {
      error_msg("ERROR: file.perfops 3rd quad (\"%s\") unparsable.", qp->iname);
      return(NULL);
   }

   /* Parse iargs */
   if ( ShouldDiff(qp) )
   {
      error_msg("ERROR: DIFF is not supported on file.perfops items.");
      return(NULL);
   }

   if ( ShouldPSAvg(qp) )
   {
      error_msg("ERROR: PSAVG is not supported on file.perfops items.");
      return(NULL);
   }

   if ( NULL == (fothis = get_file_ops(qp->pargs, opsize)) )
   {
      /* Obscure message for an obscure error */
      error_msg("ERROR: Failure to create a file.perfops file object.");
      return(NULL);
   }

   /* Allocate a timeval struct - NO, WAIT - do uint64 */
   if ( NULL == (timevalptr = malloc(8)) )
   {
      error_msg("ERROR: Failure to allocate memory for a file.perfops structure.");
      return(NULL);
   }
   
   if ( NULL == (this_pitem = NewPItem(qp, PDT_INT64, timevalptr)) )
      return(NULL);

   /* Add the additional data items */
   this_pitem->dstruct = fothis;     /* File ops struct for the file (data source) */
   this_pitem->sioffset = sioffset;  /* The specific data item requested.
                                        NOTE: sioffset is NOT used as an offset, but
                                        as a flag using one of the TS_* defines above */

   /* Shove this into the ui_list */
   this_pitem->next_ui = myself->ui_list;
   myself->ui_list = this_pitem;

   /* Flag update for the provider */
   myself->update_required = 1;

   return(this_pitem);
}

/* ========================================================================= */
int parse_iname(unsigned long *sioffset, unsigned long *readsz, const char *iname)
{
   char INAME[MAX_QPART_LEN];  /* iname when converted to upper case - for compares */
   unsigned long size;
   unsigned long oper;
   int i;

   /* Three (four) things need to be parsed:
      - Numeric of the size of the op
      - Multiplier (B, K, M)
      - "." dividing specifier (not a data item)
      - The name of the timestamp wanted
   */

   i = 0;
   while ( i < MAX_QPART_LEN )
   {
      if (( iname[i] >= 'a' ) && ( iname[i] <= 'z' ))
         INAME[i] = iname[i] - 32;
      else
         INAME[i] = iname[i];

      i++;
   }

   size = 0;
   i = 0;
   while (( INAME[i] >= '0' ) && ( INAME[i] <= '9' ))
   {
      size *= 10;
      size += INAME[i] - '0';
      i++;
   }

   switch ( INAME[i] )
   {
   case 'B':
      i++;
      break;
   case 'K':
      i++;
      size *= 1024;
      break;
   case 'M':
      i++;
      size *= 1024;
      size *= 1024;
      break;
   default:
      return(1);
      break;
   }

   if ( INAME[i] == '.' )
   {
      /* Get off the period */
      i++;
   }
   else
   {
      /* Input is wrong */
      return(1);
   }

   oper = TS_START;

   if ( 0 == strcmp(&INAME[i], "OPEN") )
      oper = TS_AOPEN;

   if ( 0 == strcmp(&INAME[i], "READ") )
      oper = TS_AREAD;

   if ( 0 == strcmp(&INAME[i], "LSEEK") )
      oper = TS_ASEEK;

   if ( 0 == strcmp(&INAME[i], "WRITE") )
      oper = TS_AWRIT;

   if ( 0 == strcmp(&INAME[i], "CLOSE") )
      oper = TS_ACLOS;

   if ( 0 == strcmp(&INAME[i], "TOTAL") )
      oper = TS_TOTAL;

   if ( oper == TS_START )
      return(1);

   /* Everything looks good, set values, return 0 */
   *sioffset = oper;
   *readsz = size;

   return(0);
}

/* ========================================================================= */
int fops_update(int interval)
{
   struct file_ops *fothis;

   struct pitem *this_pitem;
   int trv;

   /* Walk through the fileops tests - and do them all */
   trv = 0;
   fothis = folist;
   while(fothis)
   {
      trv += run_file_ops(fothis);

      fothis = fothis->next;
   }

   /* Now update all the data items (converting timestamps to diffs) */
   this_pitem = myself->ui_list;
   while ( this_pitem )
   {
      trv += pitem_update(this_pitem);

      this_pitem = this_pitem->next_ui;
   }

   return(trv);
}

/* ========================================================================= */
int FileOpsRegister(struct proot *p)
{
   /* No data types are checked - we only export *specific* data types. */

   if ( NULL == (myself = RegisterProvider(p, "file.perfops",
                                           fops_update,
                                           fops_list,
                                           fops_enablepitem)) )
   {
      /* All about readability */
      return(1);
   }

   return(0);

}

/* ========================================================================= */
int fops_list(int dflag)
{
   int icount = 0;

   /* The list() function only displays options for Sm, Md, and Lg. But in-
      reality, the provider will support just about any data size and modifier.

      In short, when enabling the data item, the string is parsed - not simply
      pattern matched.
   */

   icount = 0;
   while ( diname[icount] )
   {
      DumpQuadData(dflag, ditype[icount]);
      printf("file.perfops:*:4B.%s\n", diname[icount]);

      icount++;
   }

   icount = 0;
   while ( diname[icount] )
   {
      DumpQuadData(dflag, ditype[icount]);
      printf("file.perfops:*:4K.%s\n", diname[icount]);

      icount++;
   }

   icount = 0;
   while ( diname[icount] )
   {
      DumpQuadData(dflag, ditype[icount]);
      printf("file.perfops:*:4M.%s\n", diname[icount]);

      icount++;
   }

   return(icount * 3);
}

/* ========================================================================= */
int run_file_ops(struct file_ops *fo)
{
   int mfd;     /* The monitor file descriptior */
   int oflag;
   size_t rwr;
   char shortfn[48];

   assert( fo != NULL );
   assert( fo->fname != NULL );

#ifdef PORT_SunOS
   /* Because Solaris does not support O_DIRECT */
   oflag = O_RDWR;
#else
   oflag = O_DIRECT | O_RDWR;
#endif

   if ( 0 == file_exists(fo->fname) )
   {
      /* File was deleted, re-create */
      if ( init_file(fo->fname, fo->opsize) )
      {
         dotdotdot_string(shortfn, fo->fname, 48);

         error_msg("ERROR: Unable to initialize \"%s\".", shortfn);
         return(1);
      }
   }

   snap_time(fo, TS_START);

   if( -1 == ( mfd = open(fo->fname, oflag) ) )
   {
      fo->last_op_rv = 1;
      return(1);
   }

#ifdef PORT_SunOS
   /* Because Solaris does not support O_DIRECT */
   if(directio(mfd, DIRECTIO_ON))
      return(1);
#endif


   snap_time(fo, TS_AOPEN);

   if ( fo->opsize != ( rwr = Read(mfd, fo->buffer, fo->opsize) ) )
   {
      fo->last_op_rv = 1;
      return(1);
   }

   snap_time(fo, TS_AREAD);
      
   lseek(mfd, SEEK_SET, 0);

   snap_time(fo, TS_ASEEK);

   if ( fo->opsize != ( rwr = Write(mfd, fo->buffer, fo->opsize) ) )
   {
      fo->last_op_rv = 1;
      return(1);
   }

   snap_time(fo, TS_AWRIT);

   close(mfd);

   snap_time(fo, TS_ACLOS);

   fo->last_op_rv = 0;
   return(0);
}

/* ========================================================================= */
#define DBGPRINT(TIMEV); fprintf(stderr, "DEBUG: stamp=%s sec=%ld usec=%ld\n", #TIMEV, TIMEV.tv_sec, TIMEV.tv_usec);

int snap_time(struct file_ops *fo, int tN)
{
   switch(tN)
   {
   case TS_START:
      gettimeofday(&fo->start_time, NULL);
      break;
   case TS_AOPEN:
      gettimeofday(&fo->open_time, NULL);
      break;
   case TS_AREAD:
      gettimeofday(&fo->read_time, NULL);
      break;
   case TS_ASEEK:
      gettimeofday(&fo->seek_time, NULL);
      break;
   case TS_AWRIT:
      gettimeofday(&fo->write_time, NULL);
      break;
   case TS_ACLOS:
      gettimeofday(&fo->close_time, NULL);
      break;
   }

   return(0);
}

/* ========================================================================= */
uint64_t diff_time(struct timeval *ts, struct timeval *te)
{
   uint64_t dt;
   uint64_t ONE_MILLION;
   uint64_t sval;
   uint64_t eval;

   ONE_MILLION = 1000000;

   sval = (ts->tv_sec * ONE_MILLION) + ts->tv_usec;
   eval = (te->tv_sec * ONE_MILLION) + te->tv_usec;

   dt = eval - sval;

   return(dt);
}

/* ========================================================================= */
int pitem_update(struct pitem *this_pitem)
{
   struct file_ops *fo;
   
   fo = (struct file_ops *)this_pitem->dstruct;

   switch(this_pitem->sioffset)
   {
   case TS_START:
      return(1);
      break;
   case TS_AOPEN:
      *((uint64_t *)this_pitem->data_ptr) = diff_time(&fo->start_time, &fo->open_time);
      break;
   case TS_AREAD:
      *((uint64_t *)this_pitem->data_ptr) = diff_time(&fo->open_time, &fo->read_time);
      break;
   case TS_ASEEK:
      *((uint64_t *)this_pitem->data_ptr) = diff_time(&fo->read_time, &fo->seek_time);
      break;
   case TS_AWRIT:
      *((uint64_t *)this_pitem->data_ptr) = diff_time(&fo->seek_time, &fo->write_time);
      break;
   case TS_ACLOS:
      *((uint64_t *)this_pitem->data_ptr) = diff_time(&fo->write_time, &fo->close_time);
      break;
   case TS_TOTAL:
      *((uint64_t *)this_pitem->data_ptr) = diff_time(&fo->start_time, &fo->close_time);
      break;
   default:
      return(1);
      break;
   }

   return(0);
}





