#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "gather.h"
#include "basicdiff.h"
#include "options.h"
#include "munge.h"
#include "version.h"

int main(int argc, char *argv[])
{
   struct interrupts *intr;
   struct options *o;
   struct cpustats *temp_cpus;

   if ( NULL == (o = ReadOptions(argc, argv)) )
      return(1);

   if ( o->bAbout )
   {
      printf("%s - A Linux tool to watch interrrupts\n", BIN_NAME);
      printf("  Version: %s\n", VERSION_STRING);
      printf("    Philip Metacomet <dismembered@mieryswamp.ma.us>\n");
      printf("    William Favorite <wfavorite2@bloomberg.net>\n");
      fflush(stdout);
      return(0);
   }

   if ( o->bHelp )
   {
      printf("%s - A Linux tool to watch interrrupts\n", BIN_NAME);
      printf("  Version: %s\n", VERSION_STRING);
      printf("  Usage: %s <options>\n", BIN_NAME);
      printf("  Options:\n");
      printf("     -a   Show information about this tool\n");
      printf("     -c   Show CPU utilization stats\n");
      printf("     -h   Show this help screen\n");
      printf("     -L   Skip LOC (local timer) stats\n");
      printf("     -m   Display results in monochrome\n");
      printf("     -n   Drop interrupt names from output\n");
      printf("     -t   Show timestamps\n");
      printf("     -u   Collapse unused (interrupt) lines\n");
   /* printf("     -p   Pivot the chart [NOT IMPLEMENTED]\n"); */
      printf("     -s   Display shortened device names\n");
      fflush(stdout);
      return(0);
   }

   if ( NULL == ( intr = InitIntrData(o) ) )
      return(1);

   if ( intr->bCPUStats )
   {
      if ( NULL == (intr->last_cpus = GetCPUStats(NULL)) )
      {
         fprintf(stderr, "ERROR: Unable to retrieve CPU statistics.\n");
         return(1);
      }
      
      if ( NULL == (intr->this_cpus = CloneCPUStats(intr->last_cpus)) )
      {
         fprintf(stderr, "ERROR: Unable to clone CPU statistics.\n");
         return(1);
      }
   }

   /* Debuggery */
   if ( o->bDebug )
   {
      printf("Starting version %s\n", VERSION_STRING);
      printf("Buffer size = %lu\n", intr->bufsz);
      printf("CPU count = %d\n", intr->cpucnt);
   }

   while ( 1 )
   {
      MungeData(intr);

      /* NOTE: I use the intr version of this value (it is also in the options
         struct) because it is much more likely to be local/in-cache. */
      if ( 0 == intr->bMonochrome )
         SecondMunge(intr);

      if ( intr->bCPUStats )
         MungeCPU(intr);

      DisplayBasicDiff(intr);

      sleep(o->interval);

      if ( intr->bCPUStats )
      {
         /* Rotate CPU data */
         temp_cpus = intr->this_cpus;
         intr->this_cpus = intr->last_cpus;
         intr->last_cpus = temp_cpus;

         GetCPUStats(intr->this_cpus);
      }

      if(SampleStats(intr))
         return(1);
   }


   return(0);
}
