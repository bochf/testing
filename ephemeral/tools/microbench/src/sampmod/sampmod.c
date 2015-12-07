#include <stdio.h>
#include <stdlib.h>

#include "../lib/optparser.h"
#include "../lib/verbosity.h"

int main(int argc, char *argv[])
{
   struct optbase *ob;
   struct optpair *op;
   mb_bool fork_for_threads;
   uint8_t run_cycles;
   uint8_t rc_min;
   uint8_t rc_max;
   uint8_t rc_default;
   time_t time_to_run;
   time_t ttr_default;

   char run_pattern[MAX_VALUE_SIZE];

   if(NULL == (ob = ReadOptions(argc, argv)))
      return(1);

   GetOptionValue(ob, "FORK_FOR_THREADS", GOV_BOOLEAN, &fork_for_threads, NULL, NULL, NULL);

   rc_min = 7;
   rc_max = 150;
   rc_default = 88;
   GetOptionValue(ob, "RUN_CYCLES", GOV_UINT8, (void *)&run_cycles, &rc_min, &rc_max, &rc_default);

   ttr_default = 60;
   GetOptionValue(ob, "TIME_TO_RUN", GOV_TIME, (void *)&time_to_run, NULL, NULL, &ttr_default);

   GetOptionValue(ob, "RUN_PATTERN", GOV_STRING, run_pattern, NULL, NULL, "AbCdEfGhIjKlMnOpQrStUvWxYz");

   EvalOptions(ob);


   ReportStart("sampmod", NULL, "A sample module to demonstrate the MicroBench library API");


   printf("The parsed list of option pairs:\n");
   printf("  FORK_FOR_THREADS = %d\n", fork_for_threads);
   printf("  RUN_CYCLES = %d\n", run_cycles);
   printf("  RUN_PATTERN = \"%s\"\n", run_pattern);
   printf("  TIME_TO_RUN = %ld sec\n", (long)time_to_run);




   /* This is atypical (in my experience). The idea is to avoid an entire
      code path based on verbosity. We could make the internal printf()
      a VerboseMessage() call - but we don't want to step through the
      list unless we need to.
   */
   if ( VL_VERBOSE <= SetVerbosityLevel(VL_JSTASKN) )
   {
      /* Note again, we are using printf() only because this portion of the
         code is "protected" by the verbosity query in SetVerbosityLevel()
         call. */
      printf("The complete list of option pairs found (before \"go\"):\n");

      /* Dump the parsed list */
      op = ob->opstart;
      while ( op )
      {
         printf("  %s = %s\n", op->name, op->cpvalue);
         op = op->next;
      }
   }

   return(0);
}
