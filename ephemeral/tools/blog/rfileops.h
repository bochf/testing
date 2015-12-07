#ifndef RFILEOPS_H
#define RFILEOPS_H

#include <pthread.h>

#include "options.h"
#include "filefinder.h"

/* This is the number of spaces in the filename - 4 is 10,000 files */
#define RLOG_NUM_MAXDIGITS 4

/* Error conditions */
/* NOTE: This currently maps to a single byte in the metadata file. So if
         we have more than 8 types of errors, we need to modify the metadata
         file. */
#define RLOG_ERR_NOERRS 0
#define RLOG_ERR_UNLINK 1
#define RLOG_ERR_CREATE 2
#define RLOG_ERR_NOWRIT 4
#define RLOG_ERR_CLOSEF 8


/* A note on the various filenames in these structs:
   rfilelist->filename
      This is JUST the filename (with no relative or absolute path). It
      includes the extension of the file. This is what is written to the
      metadata file.
   rfilelist->fullfn
      This is the full file name (path + '/' + file_part + extension) as 
      required by open() and other syscalls. 
   rfile->basename
      This is the basename as passed by the user. It includes a path (either
      relative or absolute) but it does not include whatever extension is
      used.
   rfile->dir_part
      This is the directory (relative or absolute) that the files are in.
      it does NOT contain a trailing '/' character.
   rfile->file_part
      This is the filename with no path. It does not have an extension.
*/




/* ========================================================================= */
struct rfilelist
{
   char *filename;
   char *fullfn;
   int bFileExists;
   int fd;
   struct rfilelist *next;
};

/* ========================================================================= */
struct rfile
{
   int maxfile;                 /* Max numbers of files to hold */
   char *dir_part;
   char *file_part;
   char *basename;              /* Base name for all files      */
   
   unsigned int nextfilenum;    /* Next file number             */
   
   pthread_t unlinker_thread;

   size_t bytes_recvd;          /* Total bytes in               */
   size_t bytes_lastfile;       /* Bytes in last file           */
   size_t max_file_size;
   int eRlogType;               /* Timestamp or simple numeric rolling */

   char *description;

   int cur_fd;

   /* The following items are accessed by all threads and are
      controlled by a single lock */
   pthread_mutex_t big_lock;       /* The lock */

   int bDeleteLastFile;         /* This is a flag for the thread to look for */
   unsigned int bfErrors;

   struct rfilelist *filelist;  /* Circular list of files       */
};

/* ========================================================================= */
/* Name:
 * Description:
 * Input:
 * Returns:
 * Side effects:
 * Notes:
 */
struct rfile *NewRFile(struct options *o);
struct rfile *AppendRFile(struct options *o);


/* ========================================================================= */
/* Name:
 * Description:
 * Input:
 * Returns:
 * Side effects:
 * Notes:
 */
ssize_t WriteToRotatingFile(struct rfile *rf, char *buf, size_t wsize);

/* ========================================================================= */
/* Name:
 * Description:
 * Input:
 * Returns:
 * Side effects:
 * Notes:
 */
int DumpMetaDataFile(struct rfile *rf);  /* Dump data to file */
int PrintMetaData(struct rfile *rf);     /* Print data to stdout */


/* ========================================================================= */
/* Name:
 * Description:
 * Input:
 * Returns:
 * Side effects:
 * Notes:
 */
int DumpRotatingFile(struct options *o);

#endif
