#include <fcntl.h>
#include <sys/uio.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <sys/stat.h>

#include "fileops.h"

int OpenNewTrunc(char *filename)
{
  return(open(filename, O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH));
}

int OpenExistingForWrite(char *filename)
{
  return(open(filename, O_WRONLY));
}

int OpenExistingForMmap(char *filename)
{
  return(open(filename, O_RDWR));
}

int OpenExistAppend(char *filename)
{
  return(open(filename, O_RDWR | O_APPEND | O_CREAT));
}

int Unlink(char *filename)
{
  assert(NULL != filename);

  /* I cannot decide which one I want. The assert is good for development
     and the "good" return is good for production (if it were to ever
     happen. */
  if ( NULL == filename )
    return(0);

  if ( filename[0] == 0 )
    return(0);

  /* I was going to do an access(filename, F_OK) here... but I decided to 
     just to the unlink and return conditionally based on errno */

  if(unlink(filename))
  {
    if ( ENOENT == errno )
      return(0);

    return(1);
  }

  return(0);
}

/* ========================================================================= */
ssize_t Write(int fd, void *buf, size_t n_bytes)
{
  size_t wrote = 0;
  size_t put;
  ssize_t r;

  while ( wrote < n_bytes )
  {
    put = n_bytes - wrote;
    if ( -1 == ( r = write(fd, buf + wrote, put)) )
    {
      if ( EINTR != errno )
	return(-1);
    }
    else
    {
      wrote += r;
    }
  }

  return(wrote);
}

/* ========================================================================= */
ssize_t Read(int fd, void *buf, size_t n_bytes)
{
  size_t got = 0;
  size_t get;
  ssize_t r;

  while ( got < n_bytes )
  {
    get = n_bytes - got;
    if ( -1 == (r = read (fd, buf + got, get)) )
    {
      if ( EINTR != errno )
      {
	return(-1);
      }
    }
    else
    {
      if ( r == 0 )
      {
	return(got);
      }

      got += r;
    }
  }

  return(got);
}

/* ========================================================================= */
size_t GetFileSize(const char *filename)
{
  struct stat s;

  assert( NULL != filename );

  if ( 0 != stat(filename, &s) )
  {
    return(0);
  }

  return(s.st_size);
}

