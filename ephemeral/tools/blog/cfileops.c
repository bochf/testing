#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <sys/mman.h>

#include "fileops.h"
#include "cfileops.h"
#include "endian.h"


/* ========================================================================= */
struct cfile *NewCircularFile(struct options *o)
{
  struct cfile *c;

  assert( NULL != o );

  if ( NULL == (c = NewCFileStruct(o, 1)))
    return(NULL);

  if ( NewCFileHeader(c) )
    return(NULL);           /* Technically this is a memory leak, but exit comes soon */

  return(c);
}

struct cfile *NewCircularMmap(struct options *o)
{
  struct cfile *c;

  assert( NULL != o );

  if ( NULL == (c = NewCFileStruct(o, 1)))
    return(NULL);

  if ( NewMMCFileHeader(c) )
    return(NULL);           /* Technically this is a memory leak, but exit comes soon */

  return(c);
}

/* ========================================================================= */
struct cfile *OpenCircularFile(struct options *o)
{
   struct cfile *c;
   int endian;

   assert( NULL != o );

   if ( NULL == (c = NewCFileStruct(o, 1)))
      return(NULL);

   /* We have created the structure with user supplied and derived values.
      Most of which we will ignore, and use the values stored in the cfile.
      We DO need to check endian-ness of the file to insure that our pointers
      are understanable. */
   endian = c->fbits && FBITS_ENDIAN_MASK;

   if ( ReadCFileHeader(c) )
   {
      /* This means we could not read the header - so overwrite */
      if ( NewCFileHeader(c) )
         return(NULL);           /* memory leak */
   }
   else
   {
      if ( endian != c->fbits && FBITS_ENDIAN_MASK )
      {
         /* Wrong endian - do not append */
         if ( NewCFileHeader(c) )
            return(NULL);           /* memory leak */
      }
      
      if ( c->fbits && FBITS_ANYERR_MASK )
      {
         /* File contents are suspect - do not append, overwrite */
         if ( NewCFileHeader(c) )
            return(NULL);           /* memory leak */
      }
   }
   
   if ( -1 == c->fd ) /* We will append (but file is closed) */
   {
      /* Header was read, but file is not open at this time. */
      if (-1 == (c->fd = OpenExistingForWrite(c->filename)))
         return(NULL);
      
      /* Now position file pointer */
      c->here = c->head;
      if (c->head != lseek(c->fd, c->head, SEEK_SET))
         return(NULL);
   }
   
   return(c);
}

struct cfile *OpenCircularMmap(struct options *o)
{
  struct cfile *c;
  int endian;
  char zero = 0;

  assert( NULL != o );

  if ( NULL == (c = NewCFileStruct(o, 1)))
    return(NULL);

  /* We have created the structure with user supplied and derived values.
     Most of which we will ignore, and use the values stored in the cfile.
     We DO need to check endian-ness of the file to insure that our
     pointers are understanable. */
  endian = c->fbits && FBITS_ENDIAN_MASK;

  if ( ReadCFileHeader(c) )
  {
    /* This means we could not read the header - so overwrite */
    if ( NewMMCFileHeader(c) )
      return(NULL);           /* memory leak */
  }
  else
  {
    c->here = c->head;

    if ( endian != c->fbits && FBITS_ENDIAN_MASK )
    {
      /* Wrong endian - do not append */
      if ( NewMMCFileHeader(c) )
	return(NULL);           /* memory leak */
    }

    if ( c->fbits && FBITS_ANYERR_MASK )
    {
      /* File contents are suspect - do not append, overwrite */
      if ( NewMMCFileHeader(c) )
	return(NULL);           /* memory leak */
    }

  }


  if ( -1 == c->fd ) /* We can append, but file is closed - and not mmap()ed */
  {
    /* Header was read, but file is not open at this time. */
    if (-1 == (c->fd = OpenExistingForMmap(c->filename)))
      return(NULL);

    /* Gotta lseek() out to the end - in case this was not mmap()ed before and 
       is not full of data. */
    lseek(c->fd, c->size - 1, SEEK_SET);
    write(c->fd, &zero, 1);
   
    if (NULL == (c->mmcfile = mmap(NULL, c->size, PROT_READ | PROT_WRITE, MAP_SHARED, c->fd, 0)) )
    {
      fprintf(stderr, "ERROR: mmap() of %s failed.\n", c->filename);
      return(NULL);
    }
  }

  return(c);
}

/* ========================================================================= */
int CloseCircularFile(struct cfile *c)
{
  if( UpdateCFileHeader(c) )
  {
    return(1);
  }

  close(c->fd);
  c->fd = -1;

  exit(0);

  return(0);
}

int CloseCircularMmap(struct cfile *c)
{
  assert(c != NULL);
  
  if ( NULL != c->mmcfile )
  {
    if ( munmap(c->mmcfile, c->size) )
    {
      return(1);
    }
    c->mmcfile = NULL;
  }

  if ( c->fd != -1 )
    close(c->fd);

  c->fd = -1;

  return(0);
}

/* ========================================================================= */
ssize_t WriteToCircularFile(struct cfile *c, char *buf, size_t w_size)
{
   size_t writ;  /* To be written */
   size_t wrot;  /* How much wrote */
   size_t strt;  /* start of the data section of the file (the offset) */
   size_t r;     /* Return value of write ops */
   off_t  rollback;
   time_t tempt;

   assert(NULL != c);
   assert(NULL != buf);
 
   if (w_size == 0)
      return(0);

   /* time_t is implementation specific. They will be placed into a 64
      bit value in the struct. */
   if ( c->first_write == 0 )
   {
      time(&tempt);
      c->first_write = tempt;
   }

   time(&tempt);
   c->last_write = tempt;

   /* Find our location in the file (so we can get back if we have a problem). */
   rollback = lseek(c->fd, 0, SEEK_CUR);

   if ( w_size + c->here > c->size )
   {
      /* The file will wrap on this write */
      writ = c->size - c->here;

      /* Write in two parts */
      if ( -1 == (r = Write(c->fd, buf, writ)) )
      {
         /* Return to the start point */
         if ( rollback > 0 )
         {
            if (rollback != lseek(c->fd, rollback, SEEK_SET))
               fprintf(stderr, "ERROR: Unable to rollback the last failed write.\n");

            /* Commentary:
               Basically the write operation is boned if we find ourselves here.
               We take percaussions to save the integrity of the file, but
               failures here mean that something has gone horriby wrong and it
               does not really matter as corruption is likely anyways. This
               commentary applies to each -1 return value in this funciton. */
         }

         return(-1);
      }

      wrot = r;

      strt = c->data_start;
      if ( strt != lseek(c->fd, strt, SEEK_SET))
      {
         fprintf(stderr, "ERROR: Unable to position in source file.\n");

         /* Return to the start point */
         if ( rollback > 0 )
         {
            if (rollback != lseek(c->fd, rollback, SEEK_SET))
               fprintf(stderr, "ERROR: Unable to rollback the last failed write.\n");
         }

         return(-1);
      }

      writ = w_size - wrot; 
      if ( -1 == (r = Write(c->fd, buf + wrot, writ)) )
      {
         /* Return to the start point */
         if ( rollback > 0 )
         {
            if (rollback != lseek(c->fd, rollback, SEEK_SET))
               fprintf(stderr, "ERROR: Unable to rollback the last failed write.\n");
         }

         return(-1);
      }

      wrot += r;
      c->here = strt + r;
      c->head = strt + r;
      c->tail = strt + r;
      c->recv += w_size;

      if (UpdateCFileHeader(c))
      {
         /* Don't rollback. It is *possible* that the write will be fixed on
            the next iteration. In short... This is such an odd exception
            because all other operations have worked to this point - then we
            fail on one of the pwrites. */
         fprintf(stderr, "ERROR: Circular file header write failed. File is now suspect.\n");
      }

      return(wrot);
   }
   else /* Simple write */
   {
      writ = w_size;

      /* Write in a single operation */
      if ( -1 == (r = Write(c->fd, buf, writ)) )
      {
         /* Return to the start point */
         if ( rollback > 0 )
         {
            if (rollback != lseek(c->fd, rollback, SEEK_SET))
               fprintf(stderr, "ERROR: Unable to rollback the last failed write.\n");
         }

         return(-1);
      }

      wrot = r;

      if ( (c->head == c->tail) && (c->recv > 0) )
      {
         /* File has wrapped - at one time in the past */
         c->tail += wrot;
      }

      c->here += wrot;
      c->head += wrot;
      c->recv += w_size;

      if (UpdateCFileHeader(c))
      {
         /* Don't rollback. It is *possible* that the write will be fixed on
            the next iteration. In short... This is such an odd exception
            because all other operations have worked to this point - then we
            fail on one of the pwrites. */
         fprintf(stderr, "ERROR: Circular file header write failed. File is now suspect.\n");
      }

      return(wrot);
   }
 
   /* Unreachable */
   return(0);
}

/* ========================================================================= */
ssize_t WriteToCircularMmap(struct cfile *c, char *buf, size_t w_size)
{
   size_t writ;  /* To be written */
   size_t wrot;  /* Written - not a return value, but a place holder */
   time_t tempt;

   assert(NULL != c);
   assert(NULL != buf);
   assert(NULL != c->mmcfile);
   
   if (w_size == 0)
      return(0);

   /* time_t is implementation specific. They will be placed into a 64
      bit value in the struct. */
   if ( c->first_write == 0 )
   {
      time(&tempt);
      c->first_write = tempt;
   }

   time(&tempt);
   c->last_write = tempt;

   if ( w_size + c->here > c->size )
   {
      /* The file will wrap on this write */
      writ = c->size - c->here;

      memcpy(c->mmcfile + c->here, buf, writ);

      wrot = writ;
      writ = w_size - wrot; 
      c->here = c->data_start;
      
      memcpy(c->mmcfile + c->here, buf + wrot, writ);
      
      wrot += writ;

      c->here = c->data_start + writ;
      c->head = c->data_start + writ;
      c->tail = c->data_start + writ;
      c->recv += w_size;
      memcpy(c->mmcfile + OFFSET_HEADPTR, &c->head, POINTER_SIZE);
      memcpy(c->mmcfile + OFFSET_TAILPTR, &c->tail, POINTER_SIZE);
      memcpy(c->mmcfile + OFFSET_RECVPTR, &c->recv, POINTER_SIZE);
      memcpy(c->mmcfile + OFFSET_FILSIZE, &c->size, POINTER_SIZE);
      
      memcpy(c->mmcfile + OFFSET_FRSTWRT, &c->first_write, TIME_SIZE);
      memcpy(c->mmcfile + OFFSET_LSTWRIT, &c->last_write, TIME_SIZE);

    return(wrot);
  }
  else /* Simple write */
  {
    writ = w_size;

    memcpy(c->mmcfile + c->here, buf, writ);

    wrot = writ;

    if ( (c->head == c->tail) && (c->recv > 0) )
    {
      /* File has wrapped - at one time in the past */
      c->tail += wrot;
    }

    c->here += wrot;
    c->head += wrot;
    c->recv += w_size;
    memcpy(c->mmcfile + OFFSET_HEADPTR, &c->head, POINTER_SIZE);
    memcpy(c->mmcfile + OFFSET_TAILPTR, &c->tail, POINTER_SIZE);
    memcpy(c->mmcfile + OFFSET_RECVPTR, &c->recv, POINTER_SIZE);
    memcpy(c->mmcfile + OFFSET_FILSIZE, &c->size, POINTER_SIZE);

    memcpy(c->mmcfile + OFFSET_FRSTWRT, &c->first_write, TIME_SIZE);
    memcpy(c->mmcfile + OFFSET_LSTWRIT, &c->last_write, TIME_SIZE);

    return(wrot);
  }
 
  /* Unreachable */
  return(0);
}

/* ========================================================================= */
#define IO_BUFFER_SIZE 4096

int DumpCircularFile(struct options *o)
{
  struct cfile *c;
  char buf[IO_BUFFER_SIZE];
  size_t here;        /* Current location in the file */
  size_t get;
  ssize_t got;
  size_t size;

  assert(NULL != o);

  /* Allocate and minimally fill a new cfile struct */
  if ( NULL == (c = NewCFileStruct(o, 0)))
    return(1);

  /* Fill the cfile with data from the header */
  if ( ReadCFileHeader(c) )
    return(1);

  /* Sanity check before attempting to use these values */
  if ( c->head < c->tail )
  {
    /* This cannot be - tail cannot be greater than the head - ever */
    fprintf(stderr, "ERROR: Possible header corruption in circular file. (tail > head).\n");
    return(1);
  }

  if ( c->fd == -1 ) /* The file should be closed. ReadCFileHeader() closes on exit */
  {
    if ( -1 == (c->fd = open(c->filename, O_RDONLY)))
    {
      fprintf(stderr, "ERROR: Unable to open %s.\n", c->filename);
      return(1);
    }
  }

  /* Determine if we are wrapped or not */
  if ( c->head > c->tail )
  {
    here = c->tail;
    /* This is the simple case. Less data written than (potential) file size */
    if ( here != lseek(c->fd, here, SEEK_SET))
    {
      fprintf(stderr, "ERROR: Unable to position in source file.\n");
      return(1);
    }

    while ( here < c->head )
    {
      get = c->head - here;

      if ( get > IO_BUFFER_SIZE )
	get = IO_BUFFER_SIZE;

      if ( -1 == (got = Read(c->fd, buf, get)) )
      {
	fprintf(stderr, "ERROR: Unable to read from file.\n");
	return(1);
      }
      else
      {
	here += got;

	Write(1, buf, got);
      }
    } /* while( here < c->head) */
  }
  else
  {
    size = GetFileSize(c->filename);

    here = c->tail;

    /* File is wrapped - two operations */
    if ( here != lseek(c->fd, here, SEEK_SET))
    {
      fprintf(stderr, "ERROR: Unable to position in source file.\n");
      return(1);
    }

    while ( here < size )
    {
      get = size - here;

      if ( get > IO_BUFFER_SIZE )
	get = IO_BUFFER_SIZE;

      if ( -1 == (got = Read(c->fd, buf, get)) )
      {
	fprintf(stderr, "ERROR: Unable to read from file.\n");
	return(1);
      }
      else
      {
	if ( 0 == got )
	{
	  /* We should never be here */
	  fprintf(stderr, "ERROR: Premature end of file.\n");
	  return(1);
	}

	here += got;

	Write(1, buf, got);
      }
    } /* while ( reading end half ) */

    here = c->data_start;

    if ( here != lseek(c->fd, here, SEEK_SET))
    {
      fprintf(stderr, "ERROR: Unable to position in source file.\n");
      return(1);
    }
    
    while ( here < c->head )
    {
      get = c->head - here;

      if ( get > IO_BUFFER_SIZE )
	get = IO_BUFFER_SIZE;

      if ( -1 == (got = Read(c->fd, buf, get)) )
      {
	fprintf(stderr, "ERROR: Unable to read from file.\n");
	return(1);
      }
      else
      {
	if ( 0 == got )
	{
	  /* We should never be here */
	  fprintf(stderr, "ERROR: Premature end of file.\n");
	  return(1);
	}
	  
	here += got;

	Write(1, buf, got);
      }
    } /* while ( reading to head ) */
  } /* if ( simple ) else (wrapped) --- end */

  close(c->fd);
  return(0);
}

/* ========================================================================= */
int DumpHeader(struct options *o)
{
  struct cfile *c;
  int endian;
  time_t tempt;

  assert( NULL != o );

  if ( NULL == (c = NewCFileStruct(o, EMPTY_CFILE_STRUCT)))
    return(1);

  if ( ReadCFileHeader(c) )
    return(1);

  endian = c->fbits && FBITS_ENDIAN_MASK;

  printf("Magic = \"%c%c%c%c\"\n",
	 c->magic[0],
	 c->magic[1],
	 c->magic[2],
	 c->magic[3]);

  printf("File version = %d\n", c->version);
  printf("Data offset = %d\n", c->data_start);
  printf("Endian = %s\n", endian ? "Big" : "Little");
  printf("Description size = %d\n", c->description_size);
  printf("Head pointer   = %llu\n", (unsigned long long)c->head);
  printf("Tail pointer   = %llu\n", (unsigned long long)c->tail);
  printf("Bytes received = %llu\n", (unsigned long long)c->recv);
  printf("Max file size  = %llu\n", (unsigned long long)c->size);
  tempt = c->first_write;
  printf("First write time = %s", ctime(&tempt));
  tempt = c->last_write;
  printf("Last write time  = %s", ctime(&tempt));
  if (c->description)
    printf("Description = %s\n", c->description);
  else
    printf("Description = NONE\n");
  /* printf("DEBUG: fbits = %d (0x%02x)\n", c->fbits, c->fbits); */
  printf("Errors:\n");
  if ( c->fbits & FBITS_ANYERR_MASK )
  {
    if( c->fbits & CLOG_LSEEK_FAIL )
      printf(" lseek() error.\n");

  }
  else
    printf(" None.\n");



  return(0);
}

