#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <syscall.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include "../lib/optparser.h"
#include "../lib/verbosity.h"

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
     long size;
     char unit;
     int mult;
     int16_t domemset;

};


void dummy_thread(int *j );

int malloc_thread(struct test_t *args1);



int main (int argc, char *argv[]){

    
    


    time_type start, finish;
    struct test_t *args= malloc(sizeof *args);
    
    int i;
    int main_pid=syscall(SYS_gettid);
    struct mallinfo main_mi;
    struct optbase *ob;
    args->mult=1;
    
    int acc=0;
    int min=INT_MAX;
    int max=0;


 
    if(NULL == (ob = ReadOptions(argc, argv)))
      return(1);
      

   /*parameter to acquire*/
    int16_t N;
    int MS;
    char UNIT;
    int32_t NMALL;
    int32_t NARENA;
    char *lchar=malloc(4*sizeof(char));
    mb_bool MEMSET;

   
    /*defining default values*/
    int16_t def_N=sysconf(_SC_NPROCESSORS_ONLN);;
    int def_MS= sysconf(_SC_PAGESIZE);
    char def_UNIT=' ';
    int32_t def_NMALL=10000;
    int32_t def_NARENA=0;
    mb_bool def_MEMSET=1;

    

    
    
    GetOptionValue(ob, "UNIT", GOV_STRING, lchar,NULL,NULL, &def_UNIT);
    GetOptionValue(ob, "NMALL", GOV_INT32, &NMALL,NULL, NULL, &def_NARENA);
    GetOptionValue(ob, "NTHREAD", GOV_INT16, &N,NULL, NULL, &def_N);
    GetOptionValue(ob, "MS", GOV_INT32, &MS,NULL, NULL, &def_MS);
    GetOptionValue(ob, "NARENA", GOV_INT32, &NARENA,NULL,NULL, &def_NARENA);
    GetOptionValue(ob, "DOMEMSET", GOV_BOOLEAN, &MEMSET,NULL,NULL, &def_MEMSET);   
    EvalOptions(ob);

    UNIT=*lchar; 
  
   printf ("this is domemset %d\n",MEMSET); 


    pthread_t  *thread = malloc(sizeof(pthread_t)*N);
    
    
   mallopt (M_ARENA_MAX, NARENA);



     


    switch (UNIT){
        case 'K':
        case 'k':
              args->mult=1024;
              args->unit='K';
              break;
        case 'M':
        case 'm':
              args->mult=1024*1024;
              args->unit='M';
              break;
        case 'G':
        case 'g':
              args->mult=1024*1024*1024;
              args->unit='G';
              break;
        default:
           printf ("Unit %s not understood blank,K,M,G accepted or not present assuming SIZE in bytes\n",UNIT);
           args->unit=' ';
          
           break;}
     
   


    args->n=NMALL;
    args->size=MS*args->mult;
    args->domemset=MEMSET;
 


   


    VerboseMessage("This system has  numcpu %d CPUS\n",def_N);
    VerboseMessage("This is the father pid is %d \n", main_pid);

    if(MEMSET){
    printf("Spinning  %d threads doing malloc>memset>free of %d%c bytes %d time \n", N,args->size/args->mult,args->unit,args->n);
   }else{
    printf("Spinning  %d threads doing malloc>>free of %d%c bytes %d time with no memset\n", N,args->size/args->mult,args->unit,args->n);
   }

    getTime(&start);


    for ( i = 0; i < N; i++)
     { 
          if (pthread_create ( &thread[i], NULL, (void *) malloc_thread, args ) != 0)
         {
            
             ErrorMessage ("Error to create thread. \n"); 
             exit(EXIT_FAILURE); 
         }   
     }; 

 
  if (SetVerbosityLevel(-1) >1){
     malloc_info(0,stdout);
    };

int ret[N];
int abc;
   for(i=0;i<N;i++){

        if(pthread_join(thread[i],(void **)&abc)!=0){
        ErrorMessage("Unable to join the &d thread\n",i);
        }

        else{
         ret[i]=(intptr_t)abc;
         //printf("this is ret %d\n",(intptr_t)abc);
       }
   }; 

  getTime(&finish);

  const long int total = timeDiff(&start, &finish);

  printf("Total Elapsed  Time = %d usec.\n", (total)/1000/1000);


  for(i=0;i<N;i++){
    // printf("this is ret %d\n",ret[i]);
     acc=acc+ret[i];
    if(ret[i]<min)
        min=ret[i];
    if(ret[i]>max)
       max=ret[i];
       }

 printf("Avg thread time per thread = %.1f msec with min=%ld msec and max=%ld msec\n", (acc * 1.0) / N,min,max);
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


int malloc_thread(struct test_t *args1) {

   int sid = syscall(SYS_gettid);
   int *test = NULL; 
   int j; 
   int sizeb=args1->size;
   time_type start1, finish1;

   getTime(&start1);


   for(j=0; j<args1->n; j++)
    {
        /*
        test = calloc(j, sizeof(int));
        free(test);
        */
 
        
        test = malloc(sizeb);
     if( test != NULL){

        if (args1->domemset !=0){
           memset(test,0,sizeb);}
        free(test);}
     else{
      ErrorMessage ("Malloc failed returning NULL pointer exiting thread %d \n",sid);
      exit(-1);
     };
   
       
 
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
     VerboseMessage ("->This is the thread number %d, completed its %d mallocs of %d%c bytes in %lld msec\n",sid,args1->n,args1->size/args1->mult,args1->unit,elapsed/1000/1000);
     
     //pthread_exit(0);
    int elapsedms= elapsed/1000/1000; 
    return elapsedms;
 
}
