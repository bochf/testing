#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#include <stdio.h>

#include "../Stevens.h"
#include "../mainhelp.h"

#include "linproviders.h"
#include "meminfoprov.h"
#include "procpidstat.h"
#include "procstat.h"

/* Public ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */


/* ========================================================================= */
int AllOSRegister(struct proot *p)
{
   int x;

   x = 0;

   x += MemInfoRegister(p);
   x += ProcPidStatRegister(p);
   x += ProcStatRegister(p);

   return(x);
}


/* Private +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* Private here refers to in-library usage only. */

/* ========================================================================= */
struct buffer *InitReadBuff(char *filename, int pct_growth)
{
   struct buffer *b;
   unsigned long bsize;

   /* Find the size of the buffer */
   if ( 0 == ( bsize = GetOptimalBufferSize(filename, pct_growth) ) )
      return(NULL);

   /* Allocate the struct */
   if ( NULL == (b = (struct buffer *)malloc(sizeof(struct buffer))) )
      return(NULL);
   
   /* Allocate the actual buffer */
   if ( NULL == (b->buf = (char *)malloc(bsize)) )
      return(NULL);

   /* Allocate (and fill) the filename */
   b->filename = mkstring(filename);

   /* Set the remaining values */
   b->bsize = bsize;
   b->index = 0;
   b->got = 0;

   return(b);
}

/* ========================================================================= */
int FillReadBuff(struct buffer *b)
{
   int f;

   assert ( b != NULL );
   if ( b->filename == NULL )
      return(1);

   /* Read in the data - one giant read */
   if ( -1 == (f = open(b->filename, O_RDONLY)) )
      return(1);

   b->got = 0;
   b->index = 0;

   if ( 0 >= (b->got = Read(f, b->buf, b->bsize)) )
      return(1);

   close(f);

   return(0);
}

/* ========================================================================= */
int KillReadBuff(struct buffer *b)
{
   if ( NULL == b )
      return(0);

   if ( b->filename != NULL )
      free(b->filename);

   free(b->buf);

   free(b);

   return(0);
}


/* ========================================================================= */
struct buffer *InitProcBuff(unsigned long size)
{
   struct buffer *b;

   /* This should go to a common support function */
   if ( NULL == (b = (struct buffer *)malloc(sizeof(struct buffer))) )
      return(NULL);
   
   if ( NULL == (b->buf = (char *)malloc(size)) )
      return(NULL);

   b->filename = NULL; /* NULL this out, we are not using it! */
   b->bsize = size;

   /* For no good reason - just accounting for everything */
   b->index = 0;
   b->got = 0;

   return(b);
}

/* ========================================================================= */
int PullNewBuff(struct buffer *b, char *filename)
{
   int f;

   /* Read in the data - one giant read */
   if ( -1 == (f = open(filename, O_RDONLY)) )
      return(1);

   b->got = 0;
   b->index = 0;

   if ( 0 >= (b->got = Read(f, b->buf, b->bsize)) )
      return(1);

   close(f);

   return(0);
}

/* ========================================================================= */
/* The "buffer size" here should really be a page size. */
#define SIMPLE_BUFFER_SIZE 4096
unsigned long GetOptimalBufferSize(char *filename, int pct_growth)
{
   char buffer[SIMPLE_BUFFER_SIZE];
   ssize_t thisgot;
   ssize_t totalgot;
   int buffm;   /* Buffer multiple */
   int f;

   assert( NULL != filename );
   assert( pct_growth >= 0 );

   /* Open the file */
   if ( -1 == (f = open(filename, O_RDONLY)) )
      return(0);

   totalgot = 0;

   while ( 0 < (thisgot = Read(f, buffer, SIMPLE_BUFFER_SIZE) ) )
   {
      totalgot += thisgot;
   }

   close(f);

   /* Add on our percentage of growth number */
   if ( pct_growth > 0 )
   {
      totalgot *= ( 100 + pct_growth );
      totalgot /= 100;
   }

   /* If it is less than a buffer - then round up to a buffer */
   if ( totalgot < SIMPLE_BUFFER_SIZE )
      return(SIMPLE_BUFFER_SIZE);

   /* If greater than a buffer, then push back one more buffer
      multiple than the size */
   buffm = 0;
   while ( totalgot > SIMPLE_BUFFER_SIZE )
   {
      totalgot -= SIMPLE_BUFFER_SIZE;
      buffm++;
   }

   if ( totalgot > 0 )
      buffm++;

   return( buffm * SIMPLE_BUFFER_SIZE );
}

/* ========================================================================= */
int GetNextLine(char *line, struct buffer *bufs, int line_len)
{
   int i;

   if ( NULL == line )
      return(0);
   
   if ( NULL == bufs )
      return(0);

   if ( bufs->index >= bufs->got )
      return(0);

   i = 0;
   while((bufs->buf[bufs->index] != 0 ) && (bufs->buf[bufs->index] != '\n' ))
   {
      if ( i < ( line_len - 1 ) )
         line[i] = bufs->buf[bufs->index];

      bufs->index ++;
      i++;
   }

   if ( bufs->buf[bufs->index] == '\n' )
      bufs->index ++;

   if ( i >= (line_len - 1) )
   {
      line[line_len - 1] = 0;
   }
   else
   {
      line[i] = 0;
   }

   return(1);
}

/* ========================================================================= */
char *PtrNextLine(struct buffer *bufs)
{
   if ( NULL == bufs )
      return(NULL);

   if ( bufs->index >= bufs->got )
      return(0);

   while((bufs->buf[bufs->index] != 0 ) && (bufs->buf[bufs->index] != '\n' ))
   {
      bufs->index++;
   }

   if ( bufs->buf[bufs->index] == '\n' )
      bufs->index++;


   if ( bufs->buf[bufs->index] == 0 )
      return(NULL);

   return(bufs->buf + bufs->index);
}
