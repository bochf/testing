#include <stdio.h>

/* #define STUB_DEBUGGERY */

#ifdef STUB_DEBUGGERY
#include <string.h>
#endif

#include <unistd.h>

#include "options.h"
#include "ringbuf.h"
#include "version.h"
#include "threads.h"

/* ========================================================================= */
int help(void);
int about(void);
int dump_stats(struct ringbuf *rb);

/* ========================================================================= */
int main(int argc, char *argv[])
{
   struct options *o;
   struct ringbuf *rb;
   int go;

   /* Read in command line options */
   if ( NULL == (o = ReadOptions(argc, argv)) )
      return(1);

   /* Handle key operational methods */
   if ( o->bDebug )
      DbgDumpConfig(o);

   if ( o->bAbout )
      return(about());

   if ( o->bHelp )
      return(help());

   /* Initialize the buffer structure */
   if ( NULL == ( rb = InitRingBuffer(o) ) )
      return(1);

   /* Launch reader and writer threads */
   if ( LaunchThreads(o, rb) )
      return(1);

   go = 1;
   while(go)
   {
      switch(GetNextEvent(rb->tevt))
      {
      case EVENT_SLOW_QFLUSH:
         fprintf(stderr, "WARNING: Reading has completed and writing is stalled. Waiting on the writer.\n");
         break;
      case EVENT_FAIL_QFLUSH:
         fprintf(stderr, "ERROR: Reader timed out waiting on the writer. Copy failure.\n");
         unlink(rb->dfn);
         go = 0;
         break;
      case EVENT_WRITE_DONE:
         go = 0;
         break;
      case EVENT_WRITE_ERROR:
         fprintf(stderr, "ERROR: Problems writing to destination file.\n");
         unlink(rb->dfn);
         go = 0;
         break;
      case EVENT_READ_ERROR:
         fprintf(stderr, "ERROR: Problems reading from source file.\n");
         unlink(rb->dfn);
         go = 0;
         break;
      }
   }

   if ( o->bStats )
      dump_stats(rb);

   return(0);
}

/* ========================================================================= */
int help(void)
{
   printf("ob - A threaded copy tool\n");
   printf("  Usage: ob [options] <src> <dst>\n");
   printf("  Options:\n");
   printf("     -a       Show the about info.\n");
   printf("     -d X     Set direct I/O. X = r | w | rw\n");
   printf("     -b X     Set the block/buffer size to X bytes. Default = %d\n", DEFAULT_BUFFER_SIZE);
   printf("     -o X     Set the number of buffer objects. Default = %d\n", DEFAULT_BUFFER_OBJECTS);

   fflush(stdout);
   return(0);
}

/* ========================================================================= */
int about(void)
{
   printf("ob - A threaded copy tool\n");
   printf("   Version: %s\n", VERSION_STRING);
   printf("   Xxxxxxxx Xxxxxxx <xxxxxxxx@xxxx.xxx>\n");


   fflush(stdout);
   return(0);
}

/* ========================================================================= */
int dump_stats(struct ringbuf *rb)
{
   printf("Times Q was full     : %5llu\n", rb->qfull);
   printf("Times Q was empty    : %5llu\n", rb->qempty);
   printf("Calls to head get    : %5llu\n", rb->hgets);
   printf("Calls to tail get    : %5llu\n", rb->tgets);
   printf("Successful head gets : %5llu\n", rb->hgot);
   printf("Successful tail gets : %5llu\n", rb->tgot);
   fflush(stdout);

   return(0);
}
