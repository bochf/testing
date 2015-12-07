#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

/* Version history:
    0.1.0   3/6/15 - Original creation
    0.2.0   3/9/15 - Changed interval to be much quicker
    0.3.0  3/10/15 - Additional reporting, minor fix
*/
#define VERSION_STRING "0.3.0"
/* ToDo:

   Done:
    [X] Add TZ and input reporting at end of output
    [X] Add max, min, and avg iteration per second reporting
    [X] Shorten interval for more accuracy
    [X] The seconds in test and iteration count variables are not
        properly initialized.
    [X] Write it
    [X] Remove the call to usleep(). This will not work as it is based
        on the same infrastructure as what you are monitoring.
*/

#define ONE_DAY (60 * 60 * 24)


float CalculateBell(short in);
float Combinations(float n, float r);
float Factorial(float in);
void show_help(void);
void fixed_pause(short in);

/* ========================================================================= */
int main(int argc, char *argv[])
{
   int watch_seconds;
   short bell_no;
   time_t now;
   time_t last;
   time_t stop_time;
   struct timeval tv;
   unsigned long sec_of_test;
   unsigned long iter_in_sec;
   unsigned long iter_in_last_sec;
   char *ctime_str;

   unsigned long max_i_per_s = 0;
   unsigned long min_i_per_s = 0xFFFFFFFFFFFFFFFF;
   unsigned long tot_i_per_s = 0;
   unsigned long cnt_i_per_s = 0;
   double avg_i_per_s;



   if ( argc != 3 )
   {
      show_help();
      return(1);
   }

   bell_no = atoi(argv[1]);
   watch_seconds = atoi(argv[2]) + 2;
   if ( ( watch_seconds <= 0 ) || ( watch_seconds > ONE_DAY ) )
   {
      fprintf(stderr, "ERROR: Invalid input for seconds to watch.\n");
      return(1);
   }

   printf("skewatch started. Watching for %d seconds.\n", watch_seconds);

   time(&now);
   gettimeofday(&tv, NULL);
   stop_time = now + watch_seconds;
   last = now;
   iter_in_sec = 0;
   iter_in_last_sec = 0;
   sec_of_test = 0;
   
   while ( now < stop_time )
   {
      ctime_str = ctime(&now);
      ctime_str[strlen(ctime_str) - 1] = 0; /* Chomp() */

      if ( sec_of_test >= 2 )
      {
         printf("%12ld %12ld : %s %-5lu %-5lu", (long)tv.tv_sec, (long)tv.tv_usec, ctime_str, sec_of_test, iter_in_sec);

         if ( iter_in_sec == 0 )
         {
            printf(" last_sec_iter = %lu\n", iter_in_last_sec);
          
            /* Save some sampling stats */
            if ( iter_in_last_sec > max_i_per_s )
               max_i_per_s = iter_in_last_sec;

            if ( iter_in_last_sec < min_i_per_s )
               min_i_per_s = iter_in_last_sec;

            tot_i_per_s += iter_in_last_sec;

            cnt_i_per_s++;
         }
         else
            printf("\n");
      }


      /* This won't work, it uses system time */
      /* usleep(100000); */

      /* This is np-hard and scales in a non-linear way */
      /* CalculateBell(bell_no); */

      /* This scales lineraly */
      fixed_pause(bell_no);


      last = now;
      time(&now);
      gettimeofday(&tv, NULL);
      iter_in_sec++;

      if ( last != now )
      {
         iter_in_last_sec = iter_in_sec;
         iter_in_sec = 0;
         sec_of_test++;
      }

   }

   avg_i_per_s = (double)tot_i_per_s / cnt_i_per_s;
   
   printf("=======================================================\n");
   printf("Iteration jitter statistics\n");
   printf("  Maximum number of iterations per second : %lu\n", max_i_per_s);
   printf("  Minimum number of iterations per second : %lu\n", min_i_per_s);
   printf("  Average number of iterations per second : %f\n",  avg_i_per_s);
   printf("  Total number of seconds iterated        : %lu\n", cnt_i_per_s);
   printf("Time zone and run information\n");
   printf("  TZ=%s\n", ( getenv("TZ") != NULL ) ? getenv("TZ") : "NULL" );
   printf("  argv[] = %s %s %s\n", argv[0], argv[1], argv[2]);
   fflush(stdout);

   return(0);
}

/* ========================================================================= */
float CalculateBell(short in)
{
  short k;
  float temp = 0,
  result = 0;

  in--;
  if(in <= 0)
    return(1);

  for(k=0;k <= in; k++)
  {
    temp = Combinations(in, k);
    temp = temp * CalculateBell(in - k);
    result = result + temp;
  }

  return(result);
}

/* ========================================================================= */
float Combinations(float n, float r)
{
  return(Factorial(n) / (Factorial(r) * Factorial(n - r)));
}

/* ========================================================================= */
float Factorial(float in)
{
  if(in <= 0)
    return(1);
  else
    return(in * Factorial(in - 1));
}

/* ========================================================================= */
void show_help(void)
{
   printf("skewatch - A tool to watch time resolution over a set period.\n");
   printf("   Version %s\n", VERSION_STRING);
   printf("   Shulmanu Asharedu <Shalmaneser@numrud.gov>\n");
   printf("   William Favorite <wfavorite2@bloomberg.net>\n");
   printf(" Usage: skewatch <pause_time> <secs_to_watch>\n");
}


/* What is going on here:
   - CalculateBell() is np-hard and scales appropriately. This makes it hard
     to manage correctly on different systems.
   - fixed_pause() scales linerally, but is optimized with vectorization on
     Linux. So it requires no-optimization and a smaller loop size. So there
     is a different scaling number in fixed_pause() for Linux.
   - The inner_loop in fixed_pause() is hard set at one.
   - The BIG_ARRAY_SIZE value is a knob to tune
   - The value for ltval is the key tunable per-platform.
*/


/* ========================================================================= */
#define BIG_ARRAY_SZ 1000
void loop_two(unsigned long in)
{
   int i;
   unsigned long one_bigarray[BIG_ARRAY_SZ];

   unsigned long val;

   while(in > 0)
   {
      i = 0;
      while ( i < BIG_ARRAY_SZ )
      {
         val = in; /* Copied in case we want to do some math to slow things down */

         one_bigarray[i] = val + 1;

         i++;
      }
      in--;
   }
}

/* ========================================================================= */
void fixed_pause(short in)
{
   unsigned long ltval;

   ltval = 0x000000000004FFFF;

#ifdef PORT_Linux
   ltval = 0x00000000000000FF;
#endif

   int inner_loop;

   while ( in > 0 )
   {
      inner_loop = 1; /* Currently locked at one iteration */
      while ( inner_loop > 0 )
      {
         loop_two(ltval);
         inner_loop--;
      }

      in--;
   }
}
