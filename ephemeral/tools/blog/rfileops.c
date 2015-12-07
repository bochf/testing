#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/uio.h>

#include "fileops.h"
#include "rfileops.h"
#include "rmetadata.h"

#define stdebug stderr

static int RUN_FOREVER = 1; /* This is about working around a xlc warning */


/* ========================================================================= */
void *manage_unlinks(void *arg);

/* ========================================================================= */
/* Must return 0. non-zero is an error. Check this value! */
int new_rotating_file(struct rfile *rf);


int append_rotating_file(struct rfile *rf);

/* ========================================================================= */
int handle_double_roll(char *prevfile, char *thisfile);

/* ========================================================================= */
ssize_t WriteToRotatingFile(struct rfile *rf, char *buf, size_t wsize)
{
  size_t writ;
  ssize_t wrot;
  ssize_t r;

  assert(NULL != rf);

  rf->bytes_recvd += wsize;

  /* No more writing - something went wrong */
  if (rf->bfErrors)
    return(0);

  if(-1 == rf->cur_fd)
  {
    /* Oh noes! File is not open */
     /* if ( -1 == (rf->cur_fd = new_rotating_file(rf)) ) */
     if ( new_rotating_file(rf) )
     {
        /* Error message will be in called function */
        return(-1);
     }
  }

  if ( rf->bytes_lastfile + wsize > rf->max_file_size )
  {
    /* File roll */
    writ = rf->max_file_size - rf->bytes_lastfile;
    if(-1 == (r = Write(rf->cur_fd, buf, writ)))
    {
      rf->bfErrors |= RLOG_ERR_NOWRIT;
      return(0);
    }

    wrot = r;
    writ = wsize - wrot;

    if(new_rotating_file(rf))
      return(0);

    if ( -1 == (r = Write(rf->cur_fd, buf + wrot, writ)) )
    {
      rf->bfErrors |= RLOG_ERR_NOWRIT;
      return(wrot);
    }

    wrot += r;
    rf->bytes_lastfile = r;

    return(wrot);
  }
  else
  {
    /* Please check return value */
    if ( -1 == (r = Write(rf->cur_fd, buf, wsize)) )
    {
      rf->bfErrors |= RLOG_ERR_NOWRIT;
      return(0);
    }

    rf->bytes_lastfile += r;

    return(r);
  }

  return(wsize);
}

/* ========================================================================= */
struct rfile *NewRFile(struct options *o)
{
  struct rfile *rf;
  struct rfilelist *head;
  struct rfilelist *here;
  struct rfilelist *last;
  int filename_len;
  int i;

  if ( NULL == (rf = (struct rfile *)malloc(sizeof(struct rfile))) )
  {
    fprintf(stderr, "ERROR: Failed to allocate memory.\n");
    return ( NULL );
  }

  /* Transfer over related info so we only need to pass around a single
     structure. */
  rf->maxfile = o->iRolling;
  rf->basename = GetBaseName(o->filename);
  rf->eRlogType = o->eRlogType;
  rf->max_file_size = o->bytesSize;
  rf->description = o->description;

  /* Initialize the rest */
  rf->nextfilenum = 0;
  rf->bDeleteLastFile = 0;
  rf->bfErrors = RLOG_ERR_NOERRS;
  rf->bytes_recvd = 0;
  rf->bytes_lastfile = 0;
  rf->cur_fd = -1;

  /* We will be needing this */
  /* Discussion: filename_len is *based* on the full string passed by user.
     It is used to allocate space for both the shortened filename and the
     full-pathed filename. This consumes a bit more memory - but nothing
     of significance. */
  filename_len = strlen(rf->basename) + 2; /* +1 for NULL, +1 for '.' */
  switch(rf->eRlogType)
  {
  case RLOG_TYPE_TIMESTAMP:
    filename_len += 16;  /* MMDDYY-HHMMSS + single char for two turns in a sec */
    break;
  case RLOG_TYPE_NUMERICAL:
  default:  /* No reason to ever be here (unless we are falling through) */
    filename_len += RLOG_NUM_MAXDIGITS;
    break;
  }

  rf->dir_part = GetDirPart(rf->basename);
  rf->file_part = GetFilePart(rf->basename);


  /* Loop through - creating rfilelist members (into a circular list) */
  i = rf->maxfile + 1;
  head = NULL;
  last = NULL;
  while(i)
  {
    if ( NULL == ( here = (struct rfilelist *)malloc(sizeof(struct rfilelist))) )
    {
      fprintf(stderr, "ERROR: Unable to allocate memory.\n");
      free(rf);
      return(NULL);
    }

    if ( NULL == head )
      head = here;

    if ( NULL == (here->filename = (char *)malloc(filename_len)) )
    {
      fprintf(stderr, "ERROR: Unable to allocate memory.\n");
      free(rf);
      return(NULL);
    }

    if ( NULL == (here->fullfn = (char *)malloc(filename_len)) )
    {
      fprintf(stderr, "ERROR: Unable to allocate memory.\n");
      free(rf);
      return(NULL);
    }
    
    here->filename[0] = 0;
    here->fullfn[0] = 0;
    here->bFileExists = 0;
    here->next = last;

    last = here;
    i--;
  }

  head->next = last;   /* Close the loop */
  rf->filelist = last; /* Attach the loop to the main structure */

  /* Initialize the mutex */
  if ( pthread_mutex_init(&rf->big_lock, NULL) )
  {
    fprintf(stderr, "ERROR: Unable to initialize a thread mutex.\n");
    /* Just leak the memory - we are not long for this world */
    return(NULL);
  }
  
  /* Lock the mutex */
  pthread_mutex_lock(&rf->big_lock);

  /* Now that the lock is set, we can create the thread */

  if ( pthread_create(&rf->unlinker_thread, NULL, manage_unlinks, (void *)rf) )
  {
    fprintf(stderr, "ERROR: Unable to spawn rotator thread.\n");
    /* Once again, acknowledging a leak here - it is ok */
    return(NULL);
  }

  if(new_rotating_file(rf))
     return(NULL);

  return(rf);

}

/* ========================================================================= */
struct rfile *AppendRFile(struct options *o)
{
  struct rfile *rf;

  if(NULL == (rf = LoadMetaDataFile(o->filename)))
    return(NewRFile(o));

  /* Initialize the mutex */
  if ( pthread_mutex_init(&rf->big_lock, NULL) )
  {
    fprintf(stderr, "ERROR: Unable to initialize a thread mutex.\n");
    /* Just leak the memory - we are not long for this world */
    return(NULL);
  }
  
  /* Lock the mutex */
  pthread_mutex_lock(&rf->big_lock);

  /* Now that the lock is set, we can create the thread */

  if ( pthread_create(&rf->unlinker_thread, NULL, manage_unlinks, (void *)rf) )
  {
    fprintf(stderr, "ERROR: Unable to spawn rotator thread.\n");
    /* Once again, acknowledging a leak here - it is ok */
    return(NULL);
  }

  append_rotating_file(rf);

  return(rf);
}


/* ========================================================================= */
void *manage_unlinks(void *arg)
{
  struct rfile *rf; /* This just makes the constant casting go away */
  struct rfilelist *rfl;

  assert(NULL != arg);
  rf = (struct rfile *)arg;

  while(RUN_FOREVER)
  {
    pthread_mutex_lock(&rf->big_lock);
    if(rf->bDeleteLastFile)
    {
      /* filelist is the protected member, next will never change (for each filelist) */
      rfl = rf->filelist->next;

      if(rfl->fullfn[0] != 0)
      {
         if ( -1 == unlink( rfl->fullfn ) )
         {
            /* The first step in killing the logging */
            rf->bfErrors |= RLOG_ERR_UNLINK;

            /* We could exit here - but choose not to in the odd event that
               more deletes are coming. There should not be any as all file
               I/O should stop at this point. - It has to, or we will fill fs.
            */
         }
         else
         {
            rfl->filename[0] = 0;
            rfl->fullfn[0] = 0;
            rfl->bFileExists = 0;
         }
      }
    }
    pthread_mutex_unlock(&rf->big_lock);
  }

  return((void *)0);
}

/* ========================================================================= */
int PrintMetaData(struct rfile *rf)
{
  size_t bytesondisk = 0;
  struct rfilelist *rfl;
  struct rfilelist *rflstop;

  assert(NULL != rf);

  /* This iterative structure copied from below - see commentary there */
  rfl = rf->filelist->next;
  rflstop = NULL;
  while(rfl != rflstop)
  {
    rflstop = rf->filelist->next;

    if ( rfl->fullfn[0] != 0 )
    {
      bytesondisk += GetFileSize(rfl->fullfn);
    }

    rfl = rfl->next;
  }

  printf("Base file name: %s\n", rf->basename);
  printf("File (only) name: %s\n", rf->file_part);
  printf("Dir (only) name: %s\n", rf->dir_part);
  printf("Max files to keep: %d\n", rf->maxfile);
  printf("Files created: %d\n", rf->nextfilenum);
  printf("Description: %s\n", rf->description ? rf->description : "NONE");
  printf("Max file size: %llu\n", (unsigned long long)rf->max_file_size);
  printf("Total bytes input: %llu\n", (unsigned long long)rf->bytes_recvd);
  printf("Total bytes on disk: %llu\n", (unsigned long long)bytesondisk);
  printf("Total bytes dropped: %llu\n", (unsigned long long)(rf->bytes_recvd - bytesondisk));
  printf("Current files:\n");

  /* This could best be a do-while - but that might be against my religion */
  rfl = rf->filelist->next;
  rflstop = NULL;
  while(rfl != rflstop)
  {
    /* See - this is how stubborn I am */
    rflstop = rf->filelist->next;

    if ( rfl->filename[0] != 0 )
      printf("  %s\n", rfl->filename);

    rfl = rfl->next;
  }

  printf("Errors encountered:\n");

  if (rf->bfErrors == RLOG_ERR_NOERRS)
    printf("  None\n");

  if (rf->bfErrors && RLOG_ERR_UNLINK)
    printf("  Failed to unlink a file - Writing stopped\n");

  if (rf->bfErrors && RLOG_ERR_CREATE)
    printf("  Failed to create a file - Data lost\n");

  if (rf->bfErrors && RLOG_ERR_NOWRIT)
    printf("  Failed to write to a file - Data lost\n");

  if (rf->bfErrors && RLOG_ERR_CLOSEF)
    printf("  Failed to close a file descriptor - Data lost\n");

  fflush(stdout);
  return(0);
}




/* ========================================================================= */
int new_rotating_file(struct rfile *rf)
{
  char format[16];
  time_t t;
  struct tm lt;
  struct rfilelist *prevfile;

  assert(NULL != rf);
  assert(NULL != rf->filelist);

  
  /* First close the open file - if it exists */
  if ( rf->cur_fd > -1 )
  {
     while ( -1 == close(rf->cur_fd) )
     {
        if ( errno != EINTR )
        {
           rf->bfErrors |= RLOG_ERR_CLOSEF;
           fprintf(stderr, "ERROR: Unable to close file descriptor.\n");
           return(1);
        }
     }	
     rf->cur_fd = -1;
  }

  /* Rotate the linked list */
  prevfile = rf->filelist;
  rf->filelist = rf->filelist->next;

  switch(rf->eRlogType)
  {
  case RLOG_TYPE_TIMESTAMP:
    time(&t);
    localtime_r(&t, &lt);
    sprintf(rf->filelist->filename,
	    "%s.%02d%02d%02d-%02d%02d%02d",
	    rf->file_part,
	    lt.tm_year > 100 ? lt.tm_year - 100 : lt.tm_year,
	    lt.tm_mon + 1,
	    lt.tm_mday,
	    lt.tm_hour,
	    lt.tm_min,
	    lt.tm_sec);
    sprintf(rf->filelist->fullfn,
	    "%s.%02d%02d%02d-%02d%02d%02d",
	    rf->basename,
	    lt.tm_year > 100 ? lt.tm_year - 100 : lt.tm_year,
	    lt.tm_mon + 1,
	    lt.tm_mday,
	    lt.tm_hour,
	    lt.tm_min,
	    lt.tm_sec);

    if (( handle_double_roll(prevfile->filename, rf->filelist->filename)) ||
        ( handle_double_roll(prevfile->fullfn, rf->filelist->fullfn)))
    {
       /* Bail (here) on this error */
       fprintf(stderr, "ERROR: Files rolling too fast. Giving up.\n");
       exit(1);
    }

    break;
  case RLOG_TYPE_NUMERICAL:
  default:
    sprintf(format, "%%s.%%0%dd", RLOG_NUM_MAXDIGITS);
    sprintf(rf->filelist->filename, format, rf->file_part, rf->nextfilenum);
    sprintf(rf->filelist->fullfn, format, rf->basename, rf->nextfilenum);
    break;
  }

  /* increment the nextnum */
  rf->nextfilenum++;
  /* Flag the next file can be deleted */
  rf->bDeleteLastFile = 1;

  /* Unlock, so the delete thread can do its work */
  pthread_mutex_unlock(&rf->big_lock);

  if(-1 == (rf->cur_fd = OpenNewTrunc(rf->filelist->fullfn)))
  {
    fprintf(stderr, "ERROR: Unable to open %s. (%d)\n", rf->filelist->filename, rf->cur_fd);
    return(1);
  }

  rf->filelist->bFileExists = 1;

  /* Gotta have the lock - if we do not have it already */
  pthread_mutex_lock(&rf->big_lock);

  return(0);
}


/* ========================================================================= */
int append_rotating_file(struct rfile *rf)
{
  assert(NULL != rf);
  assert(NULL != rf->filelist);

  /* First close the open file - if it exists - This should NEVER happen */
  if ( rf->cur_fd > -1 )
  {
    while ( -1 == close(rf->cur_fd) )
    {
      if ( errno != EINTR )
      {
	rf->bfErrors |= RLOG_ERR_CLOSEF;
	fprintf(stderr, "ERROR: Unable to close file descriptor.\n");
	return(1);
      }
    }	
    rf->cur_fd = -1;
  }

  /* This is the only place this is called. So, of note: If this file does not exist
     (for some crazy reason) it will be created. It will NOT error if the file does
     not exist. */
  if(-1 == (rf->cur_fd = OpenExistAppend(rf->filelist->fullfn)))
  {
    fprintf(stderr, "ERROR: Unable to open %s. (%d)\n", rf->filelist->filename, rf->cur_fd);
    return(1);
  }

  rf->filelist->bFileExists = 1;

  return(0);
}


/* ========================================================================= */
int handle_double_roll(char *prevfile, char *thisfile)
{
  int dot;
  int i;

  /* They should NEVER be null */
  assert(NULL != prevfile);
  assert(NULL != thisfile);

  /* But they can be empty - or at least one of them can */
  if ( prevfile[0] == 0 )
    return(0);

  /* Move to the extension - this is denoted by the "." character */
  /* There can be more than one dot, so make sure we are on the
     last dot. */
  dot = -1;
  i = 0;
  while ( prevfile[i] != 0 )
  {
    if ( prevfile[i] == '.' )
      dot = i;

    i++;
  }

  if ( dot == -1 )
  {
     fprintf(stderr, "ERROR: Unable to parse previous file name.\n");
     return(1);
  }

  /* Compare the extension after the last dot (starting at the dot) */
  i = 0;
  while ( i < 14 )
  {
    if ( prevfile[dot + i] != thisfile[dot + i] )
    {
      return(0);
    }
    i++;
  }

  /* If we are here - then we rolled twice in a second */
  if ( prevfile[dot + 14] == 0 )
  {
    thisfile[dot + 14] = 'a';
    thisfile[dot + 15] = 0;
  }
  else
  {
    /* During testing I actually walked off the end of the ASCII table (printable chars) */
    if ( prevfile[dot + 14] == 'Z' )
    {
       return(1);
    }

    /* This will let you roll 26 * 2 times in a second - before moving off into other chars */
    if ( prevfile[dot + 14] == 'z' )
       thisfile[dot + 14] = 'A';
    else
       thisfile[dot + 14] = prevfile[dot + 14] + 1;

    thisfile[dot + 15] = 0;
  }

  return(0);
}

/* ========================================================================= */
int simple_dump_file(char *filename)
{
  int fd;
  char buf[1024];
  ssize_t got;
  ssize_t get;

  if(NULL == filename)
    return(0);
  
  if(filename[0] == 0)
    return(0);

  if(-1 == (fd = open(filename, O_RDONLY)))
  {
    fprintf(stderr, "ERROR: Unable to open %s.\n", filename);
    return(1);
  }

  get = 1024;
  while(0 != (got = Read(fd, buf, get)))
  {
    if(got == -1)
    {
      fprintf(stderr, "ERROR: Unable to read from %s.\n", filename);
      return(1);
    }

    write(1, buf, got);
  }

  return(0);
}

/* ========================================================================= */
int DumpRotatingFile(struct options *o)
{
  struct rfile *rf;
  struct rfilelist *rfl, *rflstop;


  assert(NULL != o);
  assert(NULL != o->filename);

  if(NULL == (rf = LoadMetaDataFile(o->filename)))
    return(1);

  rfl = rf->filelist->next;
  rflstop = NULL;
  while(rfl != rflstop)
  {
    rflstop = rf->filelist->next;

    if ( rfl->fullfn[0] != 0 )
    {
      simple_dump_file(rfl->fullfn);
    }

    rfl = rfl->next;
  }

  return(0);
}
