#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <sys/mman.h>
#include <strings.h>

#include "fileops.h"
#include "cfileops.h"
#include "endian.h"

/* ========================================================================= */
struct cfile *NewCFileStruct(struct options *o, int initialize)
{
  struct cfile *c;

  assert( NULL != o );

  if ( NULL == (c = (struct cfile *)malloc(sizeof(struct cfile))) )
  {
    fprintf(stderr, "ERROR: Unable to allocate memory.\n");
    return(NULL);
  }

  c->magic[0] = 'C';
  c->magic[1] = 'L';
  c->magic[2] = 'O';
  c->magic[3] = 'G';

  c->version = HEADER_VERSION;
  c->data_start = 0;             /* Unset */
  c->fbits = 0;                  /* Unset */
  c->description_size = 0;       /* Unset */
  c->description = NULL;         /* Unset */
  c->filename = o->filename;
  
  c->first_write = 0;
  c->last_write = 0;

  c->head = 0;
  c->tail = 0;
  c->here = 0;
  c->size = 0;
  c->recv = 0;

  c->mmcfile = NULL;

  c->fd = -1;   /* -1 means, not open */

  if ( initialize )
  {
    c->data_start = OFFSET_MINDATA;
    c->here = c->data_start;
    c->tail = c->data_start;
    c->head = c->data_start;

    if ( GetEndianValue() )
    {
      /* BIG */
      c->fbits |= 0x01;
    }
    else
    {
      /* little */
      c->fbits &= 0xFE;
    }

    c->size = o->bytesSize;

    if (o->description)
    {
      c->description_size = strlen(o->description);   /* description size */
      c->data_start = strlen(o->description) + OFFSET_MINDATA;
      c->tail = c->data_start;
      c->here = c->data_start;
      c->head = c->data_start;
      c->description = o->description;
    }
  }

  return(c);
}

/* ========================================================================= */
int NewCFileHeader(struct cfile *c)
{
  char basics[4];
  char zeros[POINTER_SIZE * 4];

  assert ( NULL != c );

  if ( c->fd == -1 )
  {
    if ( -1 == ( c->fd = OpenNewTrunc(c->filename) ) )
    {
      fprintf(stderr, "ERROR: Unable to open %s.\n", c->filename);
      return(1);
    }
  }
  else
  {
    /* There is just no reason to be here */
    close(c->fd);

    /* The previous code just did a lseek() to the beginning of the
       file. This was problematic in that the file may have been opened
       read-only. (Although this is basically imposible as this is only
       called by a single function - upon creating a new circular file */
    if ( -1 == ( c->fd = OpenNewTrunc(c->filename) ) )
    {
      fprintf(stderr, "ERROR: Unable to open %s.\n", c->filename);
      return(1);
    }
  }

  /* MAGIC */
  if ( 4 != write(c->fd, c->magic, 4) )
  {
    fprintf(stderr, "ERROR: Unable to write to %s.\n", c->filename);
    close(c->fd);
    c->fd = -1;
    return(1);
  }

  /* Byte header data */
  basics[0] = c->version;
  basics[1] = c->data_start;
  basics[2] = c->fbits;
  basics[3] = c->description_size;
  
  if ( 4 != write(c->fd, basics, 4) )
  {
    fprintf(stderr, "ERROR: Unable to write to %s.\n", c->filename);
    close(c->fd);
    c->fd = -1;
    return(1);
  }

  /* Null out head, tail, recv, size (all "pointers") */
  bzero(zeros, POINTER_SIZE * 4);

  if ( POINTER_SIZE * 4 != write(c->fd, zeros, POINTER_SIZE * 4) )
  {
    fprintf(stderr, "ERROR: Unable to write header to %s.\n", c->filename);
    close(c->fd);
    c->fd = -1;
    return(1);
  }

  /* Two time_t values (8 bytes each) */
  if ( TIME_SIZE * 2 != write(c->fd, zeros, TIME_SIZE * 2) )
  {
    fprintf(stderr, "ERROR: Unable to write header to %s.\n", c->filename);
    close(c->fd);
    c->fd = -1;
    return(1);
  }

  if ( c->description )
  {
    if ( c->description_size != write(c->fd, c->description, c->description_size) )
    {
      fprintf(stderr, "ERROR: Unable to write header to %s.\n", c->filename);
      close(c->fd);
      c->fd = -1;
      return(1);
    }
  }

  return(0);
}

/* ========================================================================= */
int NewMMCFileHeader(struct cfile *c)
{
  size_t i;
  char *cp;
  char zero=0;

  assert ( NULL != c );

  if ( c->fd == -1 )
  {
    if ( -1 == (c->fd = OpenNewTrunc(c->filename)) )
    {
      fprintf(stderr, "ERROR: Unable to open %s.\n", c->filename);
      return(1);
    }
  }

  lseek(c->fd, c->size - 1, SEEK_SET);
  write(c->fd, &zero, 1);

  /* We don't care where the file pointer is now - because we will be mmap()ing 
     and will not use it any more. We did need to seek out this far to have a
     properly sized backing file */

  if (NULL == (c->mmcfile = mmap(NULL, c->size, PROT_READ | PROT_WRITE, MAP_SHARED, c->fd, 0)) )
  {
    fprintf(stderr, "ERROR: mmap() of %s failed.\n", c->filename);
    return(1);
  }

  cp = (char *)c->mmcfile;
  
  /* Magic */
  cp[0] = c->magic[0];
  cp[1] = c->magic[1];
  cp[2] = c->magic[2];
  cp[3] = c->magic[3];

  /* Byte field */
  cp[4] = c->version;
  cp[5] = c->data_start;
  cp[6] = c->fbits;
  cp[7] = c->description_size;

  /* Pointers */
  bzero(c->mmcfile + OFFSET_HEADPTR, POINTER_SIZE * 3);

  memcpy(c->mmcfile + OFFSET_FILSIZE, &c->size, POINTER_SIZE);

  bzero(c->mmcfile + OFFSET_FRSTWRT, TIME_SIZE * 2);


  if ( c->description )
  {
    cp = (char *)(c->mmcfile + OFFSET_MINDATA);

    i = 0;
    while ( i < c->description_size )
    {
      cp[i] = c->description[i];
      i++;
    }
  }

  return(0);
}

/* ========================================================================= */
int UpdateCFileHeader(struct cfile *c)
{
   if ( c->fd == -1 )
   {
      if ( -1 == (c->fd = open(c->filename, O_WRONLY)))
      {
         fprintf(stderr, "ERROR: Unable to open %s.\n", c->filename);
         return(1);
      }
   }

   if ( POINTER_SIZE != pwrite(c->fd, &c->head, POINTER_SIZE, OFFSET_HEADPTR) )
   {
      fprintf(stderr, "ERROR: Unable to write header to %s.\n", c->filename);
      return(1);
   }
   
   if ( POINTER_SIZE != pwrite(c->fd, &c->tail, POINTER_SIZE, OFFSET_TAILPTR) )
   {
      fprintf(stderr, "ERROR: Unable to write header to %s.\n", c->filename);
      return(1);
   }

   if ( POINTER_SIZE != pwrite(c->fd, &c->recv, POINTER_SIZE, OFFSET_RECVPTR) )
   {
      fprintf(stderr, "ERROR: Unable to write header to %s.\n", c->filename);
      return(1);
   }

   if ( POINTER_SIZE != pwrite(c->fd, &c->size, POINTER_SIZE, OFFSET_FILSIZE) )
   {
      fprintf(stderr, "ERROR: Unable to write header to %s.\n", c->filename);
      return(1);
   }

   if ( TIME_SIZE != pwrite(c->fd, &c->first_write, TIME_SIZE, OFFSET_FRSTWRT) )
   {
      fprintf(stderr, "ERROR: Unable to write header to %s.\n", c->filename);
      return(1);
   }

   if ( TIME_SIZE != pwrite(c->fd, &c->last_write, TIME_SIZE, OFFSET_LSTWRIT) )
   {
      fprintf(stderr, "ERROR: Unable to write header to %s.\n", c->filename);
      return(1);
   }

   return(0);
}

/* ========================================================================= */
int ReadCFileHeader(struct cfile *c)
{
  char basics[4];
  size_t dsize;
  int our_endian;
  int file_endian;

  assert(NULL != c);

  if ( c->fd == -1 )
  {
    if ( -1 == (c->fd = open(c->filename, O_RDONLY)))
    {
      fprintf(stderr, "ERROR: Unable to open %s.\n", c->filename);
      return(1);
    }
  }
  else
  {
    if ( 0 != lseek(c->fd, 0, SEEK_SET) )
    {
      fprintf(stderr, "ERROR: Unable to position in file.\n");
      close(c->fd);
      c->fd = -1;
      return(1);
    }
  }

  if ( 4 != read(c->fd, c->magic, 4) )
  {
    fprintf(stderr, "ERROR: Unable to read from %s.\n", c->filename);
    close(c->fd);
    c->fd = -1;
    return(1);
  }

  if ( (c->magic[0] != 'C' ) ||
       (c->magic[1] != 'L' ) ||
       (c->magic[2] != 'O' ) ||
       (c->magic[3] != 'G' ) )
  {
    fprintf(stderr, "ERROR: %s is not a circular blog file.\n", c->filename);
    close(c->fd);
    c->fd = -1;
    return(1);
  }    

  if ( 4 != read(c->fd, basics, 4) )
  {
    fprintf(stderr, "ERROR: Unable to read from %s.\n", c->filename);
    close(c->fd);
    c->fd = -1;
    return(1);
  }

  c->version = basics[0];
  c->data_start = basics[1];
  c->fbits = basics[2];
  c->description_size = basics[3];

  if ( POINTER_SIZE != read(c->fd, &c->head, POINTER_SIZE) )
  {
    fprintf(stderr, "ERROR: Unable to read from %s.\n", c->filename);
    close(c->fd);
    c->fd = -1;
    return(1);
  }

  if ( POINTER_SIZE != read(c->fd, &c->tail, POINTER_SIZE) )
  {
    fprintf(stderr, "ERROR: Unable to read from %s.\n", c->filename);
    close(c->fd);
    c->fd = -1;
    return(1);
  }

  if ( POINTER_SIZE != read(c->fd, &c->recv, POINTER_SIZE) )
  {
    fprintf(stderr, "ERROR: Unable to read from %s.\n", c->filename);
    close(c->fd);
    c->fd = -1;
    return(1);
  }

  if ( POINTER_SIZE != read(c->fd, &c->size, POINTER_SIZE) )
  {
    fprintf(stderr, "ERROR: Unable to read from %s.\n", c->filename);
    close(c->fd);
    c->fd = -1;
    return(1);
  }

  if ( TIME_SIZE != read(c->fd, &c->first_write, TIME_SIZE) )
  {
    fprintf(stderr, "ERROR: Unable to read from %s.\n", c->filename);
    close(c->fd);
    c->fd = -1;
    return(1);
  }

  if ( TIME_SIZE != read(c->fd, &c->last_write, TIME_SIZE) )
  {
    fprintf(stderr, "ERROR: Unable to read from %s.\n", c->filename);
    close(c->fd);
    c->fd = -1;
    return(1);
  }

  dsize = c->description_size;
  if (dsize > 0)
  {
    if ( NULL == (c->description = (char *)malloc(dsize + 1)) )
    {
      fprintf(stderr, "ERROR: Unable to allocate memory.\n");
      close(c->fd);
      c->fd = -1;
      return(1);
    }

    if ( dsize != read(c->fd, c->description, dsize) )
    {
      fprintf(stderr, "ERROR: Unable to read from %s.\n", c->filename);
      close(c->fd);
      c->fd = -1;
      return(1);
    }
    
    c->description[dsize] = 0;
  }
  else
    c->description = NULL;


  /* Do the endian conversion here */
  file_endian = c->fbits && FBITS_ENDIAN_MASK;
  our_endian = GetEndianValue();

  if ( file_endian != our_endian )
  {
     
     c->head = endian_swap_u64(c->head);
     c->tail = endian_swap_u64(c->tail);
     c->recv = endian_swap_u64(c->recv);
     c->size = endian_swap_u64(c->size);

     c->first_write = endian_swap_u64(c->first_write);
     c->last_write = endian_swap_u64(c->last_write);
  }


  close(c->fd);
  c->fd = -1;
  return(0);
}




