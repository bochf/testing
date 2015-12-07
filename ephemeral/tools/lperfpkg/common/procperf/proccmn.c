#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>

#include "proccmn.h"

unsigned long ReadProcFile(void *buf, unsigned long bufsz, char *filename)
{
   int fd;
   void *junkbuf = NULL;
   size_t got;
   size_t totalgot;
   size_t rv;

   if ( -1 == (fd = open(filename, O_RDONLY)) )
   {
      fprintf(stderr, "ERROR: Unable to open %s.\n", filename);
      return(0);
   }

   if ( NULL == buf )
   {
      /* The user wants to know the data size only */

      bufsz = 4096; /* Set to a page */

      if ( NULL == (junkbuf = malloc(bufsz)) )
      {
         fprintf(stderr, "ERROR: Unable to allocate a temp buffer.\n");
         return(0);
      }

      buf = junkbuf;
   }


   totalgot = 0;
   while( 0 != (got = read(fd, buf + totalgot, bufsz)) )
   {
      if ( got == -1 )
      {
         /* Could be valid. */
         fprintf(stderr, "ERROR: read() on %s failed.\n", filename);
         return(0);
      }

      totalgot += got;

   }

   close(fd);

   if ( junkbuf )
   {
      if ( 0 == (rv = 4096 * ( totalgot / 4096 )) )
         rv = 4096;

      free(junkbuf);
   }
   else
      rv = totalgot;

   return(rv);
}


