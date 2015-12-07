/* this is an enanchment for determin if futex has degradation this is 
a comparative test before and after the upgrade

to compile with library

 gcc -O2 -o futexlat  futexbm.c ../lib/libmb64.a -lrt

*/

/* This is required for clock_gettime() warning/declaration on Solaris */
#ifdef PORT_SunOS
#define __EXTENSIONS__
#endif



#define _GNU_SOURCE

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <sys/time.h>
#include "../lib/optparser.h"
#include "../lib/verbosity.h"

//this TRY maybe should be in a lib
#define TRY(_expr, _err_val) \
	if((_expr) == _err_val) { \
		const int	err = errno; \
		fprintf(stderr, "Line %d, %s: error %d (%s).\n", __LINE__, (#_expr), err, strerror(err)); \
		exit(-1); \
	}


/* MAX_VALUE_SIZE is defined in optparser.h. It is the largest value allowed.*/
#define MAXL ( MAX_VALUE_SIZE + 1 )


#define CLOCK CLOCK_REALTIME
/* #define CLOCK CLOCK_REALTIME_COARSE */
/* #define CLOCK CLOCK_MONOTONIC */
/* #define CLOCK CLOCK_MONOTONIC_COARSE */
/* #define CLOCK CLOCK_PROCESS_CPUTIME_ID */
/* #define CLOCK CLOCK_THREAD_CPUTIME_ID */



#ifdef USE_NANOSEC


    /*this part of the code is for simple acquire the time stamp, 
    we should add the define as done for mmapexe for using different clock
    */
    typedef struct timespec	time_type;

    static inline
    void getTime(time_type * _ts)
    {
    	clock_gettime(CLOCK, _ts);
    }

     static inline long int timeDiff(const time_type * _start, const time_type * _finish)
    {
	    return _finish->tv_nsec - _start->tv_nsec + (_finish->tv_sec - _start->tv_sec) * 1000000000l;
    }

#else

    typedef struct timeval	time_type;

    static inline void getTime(time_type * _ts)
    {
	    gettimeofday(_ts, NULL);
    }

    static inline long int timeDiff(const time_type * _start, const time_type * _finish)
    {
    	return (_finish->tv_usec - _start->tv_usec + (_finish->tv_sec - _start->tv_sec) * 1000000l) * 1000l;
    }

#endif /* USE_NANOSEC */






/* ========================================================================= */
static int semGet(sem_t* _sem)
{
	int			l_val = -1;
	TRY(sem_getvalue(_sem, &l_val), -1);
	return l_val;
}


/* ========================================================================= */
static void semInit(sem_t** _sem, const char* _name)
{
	VerboseMessage("Initializing %s...\n", _name);
	TRY(*_sem = sem_open(_name, O_CREAT, S_IRUSR | S_IWUSR, 0), SEM_FAILED);
	VerboseMessage("Value of %s = %d\n", _name, semGet(*_sem));
}


/* ========================================================================= */
inline static void semWait(sem_t* _sem)
{
	int l_err;
	do {
		l_err = sem_trywait(_sem);
		if(l_err == -1)
			l_err = errno;
	} while(l_err == EAGAIN);
}

/* ========================================================================= */

static sem_t*	g_semPing;
static sem_t*	g_semPong;


/* ========================================================================= */
inline static void ping(int _N)
{
	while(_N--) {
		sem_post(g_semPing);
		sem_wait(g_semPong);
	}
}

/* ========================================================================= */
inline static void ping_busy(int _N)
{
	while(_N--) {
		sem_post(g_semPing);
		semWait(g_semPong);
	}
}


/* ========================================================================= */
inline static void pong(int _N)
{
	while(_N--) {
		sem_wait(g_semPing);
		sem_post(g_semPong);
	}
}

/* ========================================================================= */
inline static void pong_busy(int _N)
{
	while(_N--) {
		semWait(g_semPing);
		sem_post(g_semPong);
	}
}


static const char* const	g_pingName = "ping";
static const char* const	g_pongName = "pong";

/* ========================================================================= */
/*using new interface err message may not be hit*/
static void usage(const char * _progName)
{
	ErrorMessage("\nUsage:\n%s ping_cpu pong_cpu num_iterations [pingbusy] [pongbusy]\n\n", _progName);
}


/* ========================================================================= */
/* ================================= The maim starts here ================== */
/* ========================================================================= */


int main(int argc, char *argv[])
{
	/* int		err;  <------ Unused. Commenting out */
	int		N;                 //number of cycles
	int		ping_cpu=0;          //cpu number to pin the ping process
	int		pong_cpu=1;          //cpu number to pin the pong process 
	int		pingbusy = 0;
	int		pongbusy = 0;
   int     ncpus=0;

   struct optbase *ob;

 
   if(NULL == (ob = ReadOptions(argc, argv)))
      return(1);


   ncpus=sysconf( _SC_NPROCESSORS_ONLN );


   if ( ncpus > 2) {

      VerboseMessage ("number of cpu on the box %d\n",ncpus);
   } 
   else{

      ErrorMessage ("Not enough cpus to run the test\n");

      return -1;
   }
   

   /* 
      The cpu_set_t data structure represents a set of CPUs.  CPU sets are
      
      used by sched_setaffinity(2) and similar interfaces.
      cpu_set_t data type is implemented as a bitset. 
      
      mask should have only the total number of max cpu
   */

	cpu_set_t	mask[128];

   /* 
      inizialize mask with all 0
   */
	memset(mask, 0, sizeof(mask));

   /* 
      work the test options
      it must be update
   */


      int min_cpu=0;
      int max_cpu=ncpus-1;
      int busy=1;
      int nobusy=0;

      GetOptionValue(ob, "PING_CPU", GOV_INT16, &ping_cpu, &min_cpu, &max_cpu, NULL);
      GetOptionValue(ob, "PONG_CPU", GOV_INT16, &pong_cpu, &min_cpu, &max_cpu, NULL);
      GetOptionValue(ob, "N", GOV_INT16, &N, NULL, NULL, NULL);
      GetOptionValue(ob, "PING_BUSY", GOV_INT16, &pingbusy,&nobusy,&busy,NULL);
      GetOptionValue(ob, "PONG_BUSY", GOV_INT16, &pongbusy,&nobusy,&busy,NULL); 
      EvalOptions(ob);


if (1<0){
	if(argc < 4) {
		usage(argv[0]);
		return -1;
	}

	ping_cpu = atoi(argv[1]);
	pong_cpu = atoi(argv[2]);
	N = atoi(argv[3]);

	if(argc > 4) {
		if(strcmp(argv[4], "pongbusy") == 0)
			pongbusy = 1;
		else
		if(strcmp(argv[4], "pingbusy") == 0)
			pingbusy = 1;
	}

	if(argc > 5) {
		if(strcmp(argv[5], "pongbusy") == 0)
			pongbusy = 1;
		else
		if(strcmp(argv[5], "pingbusy") == 0)
			pingbusy = 1;
	}
}





/*
Let's create sem_open the two semaphore
oflag is O_CREATE, mode is 
mode is 0600 S_IRUSR | S_IWUSR from umask
*/



	semInit(&g_semPing, g_pingName);
	semInit(&g_semPong, g_pongName);



	if(fork() == 0) {
          /* child
           CPU_SET Add CPU cpu to set. 
         */
        VerboseMessage("run child\n");
		CPU_SET(pong_cpu, mask);
		sched_setaffinity(0, 128, mask); // If pid is zero, then the calling process is used.
	
        
    	if(pongbusy)
			pong_busy(N);
		else
			pong(N);
		TRY(sem_close(g_semPong), -1);
		TRY(sem_unlink(g_pongName), -1);
      VerboseMessage ("wrap child\n");
	  } else {
		/* parent */
		time_type start, finish;
		/* time_type start_one, finish_one; <---- Unused. Commenting out */
      VerboseMessage ("run parent\n");
		CPU_SET(ping_cpu, mask);
		sched_setaffinity(0, 128, mask);

		getTime(&start);
		if(pingbusy)
			ping_busy(N);
		else
			ping(N);
        VerboseMessage("wrapping parent\n");
		getTime(&finish);

		const long int elapsed = timeDiff(&start, &finish);

		printf("avg RTT = %.1f nsec.\n", (elapsed * 1.0) / N);
		TRY(sem_close(g_semPing), -1);
		TRY(sem_unlink(g_pingName), -1);

	}

	return 0;
}

