#include "Stevens.h"

#include <errno.h>

/* ========================================================================= */
ssize_t Read(int fd, void *buf, size_t n_bytes)
{
   size_t got = 0;
   size_t get;
   ssize_t r;

   while ( got < n_bytes )
   {
      get = n_bytes - got;
      if ( -1 == (r = read (fd, buf, get)) )
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

         buf = (char *)buf + r;
         got += r;
      }
   }

  return(got);
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
      if ( -1 == ( r = write(fd, buf, put)) )
      {
         if ( EINTR != errno )
            return(-1);
      }
      else
      {
         buf = (char *)buf + r;
         wrote += r;
      }
   }

   return(wrote);
}

