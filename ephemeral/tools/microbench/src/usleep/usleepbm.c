
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sched.h>
#include <string.h>
#include "../lib/optparser.h"
#include "../lib/verbosity.h"

#ifdef PORT_SunOS
#define __EXTENSIONS__
#endif
#define _GNU_SOURCE
//#define CLOCK CLOCK_REALTIME
/* #define CLOCK CLOCK_REALTIME_COARSE */
#define CLOCK CLOCK_MONOTONIC 
/* #define CLOCK CLOCK_MONOTONIC_COARSE */
/* #define CLOCK CLOCK_PROCESS_CPUTIME_ID */
/* #define CLOCK CLOCK_THREAD_CPUTIME_ID */

#define NS_PER_SECOND 1000000000LL
#define TIMESPEC_TO_NS( aTime ) ( ( NS_PER_SECOND * ( ( long long int ) aTime.tv_sec ) ) + aTime.tv_nsec )


static long long int sleeploop(int nr, long long int nsec) {


        struct timespec sleept;
        struct timespec lLastTime;
        struct timespec lCurrentTime;
        clock_gettime( CLOCK, &lLastTime );

        



        clock_gettime( CLOCK, &lLastTime );
 
        sleept.tv_sec = 0;
        sleept.tv_nsec = nsec;
   
   int i;
 
   clock_gettime( CLOCK, &lLastTime );


  
   for (i = 0; i < nr; i++)
    {

      
        //clock_nanosleep(CLOCK, TIMER_ABSTIME,&sleept,NULL);
          clock_nanosleep(CLOCK,0,&sleept,NULL);
    }


        clock_gettime( CLOCK, &lCurrentTime );
        long long int lDifference = ( TIMESPEC_TO_NS( lCurrentTime ) - TIMESPEC_TO_NS( lLastTime ) ) ;
          //lDifference=lDifference -lfortime;
        printf( "->Waited (average nsecs): %lld when asked to sleep %lld nsec\n", lDifference  / nr,nsec );

     return lDifference;




     
     


}
        


int main(int argc, char *argv[])

{

 
   int N =5000;
   int n_cycle=N;
   int ns=1;
   int min_cycl=100;
   int max_cycl=100000000;
   long long int min_ns=1;
   long long int max_ns=500000;
   long long int total=0;
   long long int n_res;
   struct optbase *ob;
   struct timespec res;

    
   
   struct sched_param sp;
   int ret;
   sp.sched_priority = 99;
   ret = sched_setscheduler(0, SCHED_FIFO, &sp);
   if (ret == -1) {
   printf("unable to set scheduler to FIFO");
  
   }   


   if(NULL == (ob = ReadOptions(argc, argv)))
      return(1);

   
    GetOptionValue(ob,"NSEC", GOV_INT64, &ns, &min_ns, &max_ns, NULL);
    GetOptionValue(ob,"CYCLE", GOV_INT32, &n_cycle, &min_cycl, &max_cycl, NULL);
    EvalOptions(ob);
  
    clock_getres(CLOCK, &res);
    n_res=TIMESPEC_TO_NS(res);
    VerboseMessage("\n CLOCK resolution is %lld\n",n_res); 
  

    VerboseMessage("\nI will run the test for %d time sleeping %lld nsecs\n",n_cycle,ns);
    
    VerboseMessage("\nNote: First we run a test for 1 nsec and 0 nsec to show overhead of nanosleep syscall\n\n");    
    total=total + sleeploop(n_cycle,min_ns);
    total=total + sleeploop(n_cycle,0);
    VerboseMessage("\n\nNow I run your test cycling %d times doing nanosleeps of %lld nsecs\n\n", n_cycle,ns); 
    
    total=total + sleeploop(n_cycle,ns);
   
    VerboseMessage ("\nTotal time accumlated (nsecs): %lld\n\n",total/n_cycle);

   

    
return 0;


}



