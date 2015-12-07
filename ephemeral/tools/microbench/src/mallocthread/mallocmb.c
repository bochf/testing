#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <syscall.h>
#include <malloc.h>
#include <string.h>
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
    typedef struct timespec     time_type;

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

    typedef struct timeval      time_type;

    static inline void getTime(time_type * _ts)
    {
            gettimeofday(_ts, NULL);
    }

    static inline long int timeDiff(const time_type * _start, const time_type * _finish)
    {
        return (_finish->tv_usec - _start->tv_usec + (_finish->tv_sec - _start->tv_sec) * 1000000l) * 1000l;
    }

#endif /* USE_NANOSEC */



struct test_t {
     int n;
     int size;

};


void dummy_thread(int *j );

void malloc_thread(struct test_t *args1);


int main (){
int N=10;
time_type start, finish;




struct test_t *args= malloc(sizeof *args);

args->n=1000;
args->size=32;

pthread_t  *thread = malloc(sizeof(pthread_t)*N);

int i;

int main_pid=syscall(SYS_gettid);
printf("This is the father pid is %d \n", main_pid);



 getTime(&start);


for ( i = 0; i < N; i++)
    { 
        //if (pthread_create ( &thread[i], NULL, (void *) dummy_thread, &i ) != 0)
         if (pthread_create ( &thread[i], NULL, (void *) malloc_thread, args ) != 0)
        {
           
            printf("Error to create thread. \n"); 
            exit(EXIT_FAILURE); 
        }    
    } 


    struct mallinfo main_mi;
    main_mi=mallinfo();

    printf("This is the father mallinfo is %d \n", main_mi.arena);


for(i=0;i<N;i++)
        pthread_join(thread[i],NULL); 

            getTime(&finish);

                const long int total = timeDiff(&start, &finish);

              printf("avg malloc time = %.1f usec.\n", (total * 1.0) / (N*args->n)/1000);
return 0;

};



void dummy_thread(int *j){
           
     int k=*j;
     
     int sid = syscall(SYS_gettid);
     struct mallinfo mi;
     int r = rand();
     int *a=(int*)calloc(r, sizeof(int));
     mi = mallinfo(); 
     
     printf("->This is the thread number %d, LWP id is %d, arena bytes %d \n", k,sid,mi.arena);
     sleep(100);  
    
     free (a);
     
    
   
    return ; 

}


void malloc_thread(struct test_t *args1) {

   int sid = syscall(SYS_gettid);
   int *test = NULL; 
   int j; 
   int sizeb=args1->size*1024*1024;
   time_type start1, finish1;

   getTime(&start1);


   for(j=0; j<args1->n; j++)
    {
        /*
        test = calloc(j, sizeof(int));
        free(test);
        */
 
        
        test = malloc(sizeb);
        memset(test,0,sizeb);
        free(test);
       
 
        /*
        test = realloc(NULL, j * sizeof(int)); //same as malloc
        free(test);
        */
 
        /*
        test = malloc(j * sizeof(int));
        free(test);
        */
 
        /*
        test = realloc(test, j * sizeof(int));
        */
    }

     getTime(&finish1);
    const long int elapsed = timeDiff(&start1, &finish1);
    printf("->This is the thread number %d, completed its %d mallocs of %dMB/free in %lld msec\n",sid,args1->n,args1->size,elapsed/1000/1000);

    return;

}
