#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include "../lib/optparser.h"
#include "../lib/verbosity.h"

#define NS_PER_SECOND 1000000000LL
#define TIMESPEC_TO_NS( aTime ) ( ( NS_PER_SECOND * ( ( long long int ) aTime.tv_sec ) ) + aTime.tv_nsec )


#define CLOCK CLOCK_REALTIME 
/* #define CLOCK CLOCK_REALTIME_COARSE */
/* #define CLOCK CLOCK_MONOTONIC */
/* #define CLOCK CLOCK_MONOTONIC_COARSE */
/* #define CLOCK CLOCK_PROCESS_CPUTIME_ID */
/* #define CLOCK CLOCK_THREAD_CPUTIME_ID */


struct timespec lLastTime;
struct timespec lCurrentTime;
long long int  lDifference, ltotal=0;


long long int gettime (void)
{

   struct timespec t;
   int rcode;
   if ((rcode= clock_gettime( CLOCK, &t )) <0) {
       printf("error getting time! (%i)\n", r);
       exit(1)
   }
  
  return TIMESPEC_TO_NS( t );

}


long long int sarttime(void)
{

  starttime= gettime();

}


long long int elapsedtime(long long int starttime, long long int &total) 
{

  long long int endtime=gettime();
  
  long long ldiff=endtime - starttime;
  total= total + ldiff;
  return ldiff;
}





