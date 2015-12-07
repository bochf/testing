
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "iobuf.h"
#include "support.h"

/* ========================================================================= */
struct iobuf *NewIOBuf(size_t size)
{
   struct iobuf *iob;

   if ( NULL == (iob = (struct iobuf *)malloc(sizeof(struct iobuf))) )
   {
      error_msg("ERROR: Failed to allocate output buffer.");
      return(NULL);
   }

   if ( 0 >= size )
      size = DEFAULT_BUFFER_SIZE;

   if ( size < MINIMUM_BUFFER_SIZE )
      size = MINIMUM_BUFFER_SIZE;

   if ( NULL == (iob->buf = (char *)malloc(size)) )
   {
      /* Messages are slightly different. But failed malloc()s are not a 
         problem for reporting. */
      error_msg("ERROR: Failed to allocate memory for output buffer.");
      return(NULL);
   }

   iob->size = size;
   iob->eod = 0;
   iob->error = IOBUF_ERR_NONE;

   return(iob);
}

/* ========================================================================= */
int IOBufWrite(struct iobuf *iob, char *fmt, ...)
{
   va_list ap;
   char *writeloc;
   int rv, absrv;
   size_t n;

   if ( iob->size <= iob->eod )
   {
      iob->error |= IOBUF_ERR_OVRFLOW;
      return(-1);
   }

   n = iob->size - iob->eod;

   writeloc = &iob->buf[iob->eod];

   va_start(ap, fmt);
   rv = vsnprintf(writeloc, n, fmt, ap);
   va_end(ap);

   absrv = rv;
   if ( absrv < 0 )
      absrv *= -1;

   if ( absrv > n )
   {
      iob->error |= IOBUF_ERR_OVRFLOW;
      return(rv);
   }

   if ( rv > 0 )
      iob->eod += rv;

   return(rv);
}

/* ========================================================================= */
int ResetIOBuf(struct iobuf *iob)
{
   /* I could memcpy() over the whole thing. If we are coded properly, 
      then this is entierly unnessary. */
   iob->buf[0] = 0;

   iob->eod = 0;
   iob->error = IOBUF_ERR_NONE;

   return(0);
}

/* ========================================================================= */
int IOBufError(struct iobuf *iob)
{
   if ( iob->error == IOBUF_ERR_NONE )
      return(0);

   return(iob->error);
}
