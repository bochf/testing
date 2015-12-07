/*
Table 6: Theoretical maximum versus measured memory throughput for 2P ProLiant servers
Theoretical maximum
memory bandwidth
Measured maximum memory
throughput
Intel-based 2P ProLiant G5           25.6 GB/s (RDIMMs)   12 GB/s
                                     38.4 GB/s (FBDIMMs)  12 GB/s
Intel-based 2P ProLiant  G6/G7       64 GB/s              40 GB/s
Intel-based 2P ProLiant Gen8      102.4 GB/s              88.6 GB/

*/
















#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <unistd.h>
#include <numa.h>
#include <limits.h>
#include <stdint.h>
#include <wchar.h>
#include <sys/resource.h>
#include <time.h>
#include <sys/time.h>
#include <ctype.h>
#include "../lib/optparser.h"
#include "../lib/verbosity.h"
#include <omp.h>
#define  N 20
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
typedef struct timespec time_type;

static inline void
getTime (time_type * _ts)
{
  clock_gettime (CLOCK, _ts);
}

static inline long int
timeDiff (const time_type * _start, const time_type * _finish)
{
  return _finish->tv_nsec - _start->tv_nsec + (_finish->tv_sec -
					       _start->tv_sec) * 1000000000l;
}

#else

typedef struct timeval time_type;

static inline void getTime (time_type * _ts)
{
  gettimeofday (_ts, NULL);
}

static inline long int
timeDiff (const time_type * _start, const time_type * _finish)
{
  return (_finish->tv_usec - _start->tv_usec +
	  (_finish->tv_sec - _start->tv_sec) * 1000000l) * 1000l;
}

#endif /* USE_NANOSEC */




/* Fill up array with random number either every interleave with page or serial*/
int Rvec_gen (float V[], long long int S,int inter)
{

  float maxf = ((float) RAND_MAX);
  time_t t;
  srand ((unsigned) time (&t));
  long long int i;

  

  if (!inter){ 
    for (i = 0; i < S; i++)
        {
        V[i] = ((float) rand () / (maxf + 1));

        }
  }else{
  int hop=sysconf (_SC_PAGESIZE);

  int n_hop=S/hop;
  long long int x,y,z=0;
 
  for (x=0;x<hop;x++){
    for (y=0;y<n_hop;y++)
      V[x+y*hop] = ((float) rand () / (maxf + 1));
    }
  }
  return 0;

}

/* to print usage of memory as needed */
void print_rusage(void *abc){

   struct rusage *memory = malloc (sizeof (struct rusage));
   getrusage (RUSAGE_SELF, memory);
  VerboseMessage("\nInfo: Getting current Memory Usage\n");
  //VerboseMessage ("#%25s : %ld kB\n", "ru_ixrss", memory->ru_ixrss);
  //VerboseMessage ("#%25s : %ld kB\n", "ru_isrss", memory->ru_isrss);
  //VerboseMessage ("#%25s : %ld kB\n", "ru_idrss", memory->ru_idrss);
  VerboseMessage ("    #%25s : %ld kB\n", "Current Max RSS", memory->ru_maxrss);
   VerboseMessage ("\n");
  return;
}



void scaleV(float V[], long long int S){

long long int i;

float a=2;
#pragma omp parallel for
for (i=0; i<S;i++)
   V[i]=a*V[i];

return;
}

void sumV(float V1[],float V2[],float V3[],long long int S,float a1){

long long int i;

if (a1 !=1){
#pragma opm parallel for
for (i=0; i<S;i++)
   V3[i]=V1[i] +a1*V2[i];

}else{

#pragma opm parallel for
for (i=0; i<S;i++)
   V3[i]=V1[i] +V2[i];
}

return;
}


void copyV(float V1[],float V2[], long long int S){


long long int i;
#pragma opm parallel for
for (i=0; i<S;i++)
   V2[i]=V1[i];

return;
}

/* here start the main program */

int main (int argc, char *argv[])
{

  //numa_run_on_node (0);
  printf("here something about the test would be good\n");
  VerboseMessage ("Thread auto binded to numa node 0\n");
  time_type start, finish;


  float *mem_array[N];
  float *mem_array_inter;
  long long int n_page;
  long long int mem_size;
  int u_size;
  int n_node;
  int p_size;
  long long int n_size[N];
  int j;
  long long int mult;
  long long int M;
  long int def_mem_alloc,mem_alloc;
  struct optbase *ob;
  char def_UNIT=' ';
  char UNIT;
  long long int min_mem=0,max_mem;
  char *lchar=malloc(4*sizeof(char));
  int interleave;
  int  def_inter=0;
  long int total;
  int n_cpus;
  double BW; 


//start code
 
  n_page = sysconf (_SC_PHYS_PAGES);
  u_size = sizeof (float);
  p_size = sysconf (_SC_PAGESIZE) / 1024;
  mem_size = n_page * p_size;
  n_cpus=sysconf(_SC_NPROCESSORS_ONLN);
 

    if(NULL == (ob = ReadOptions(argc, argv)))
      return(1);

  if (numa_available () == 0)
    {

      n_node = numa_max_node () + 1;

      for (j = 0; j < n_node; j++)
	{
	  n_size[j] = numa_node_size64 (j, NULL);
	}

    }
  else
    {
      printf ("Numa not available\n");

      n_node = -1;
      return -1;
    }
  VerboseMessage("\nSYS INFO DERIVED INFO\n");
  VerboseMessage ("#%25s : %u\n","Number of memory pages", n_page);
  VerboseMessage ("#%25s : %lld kB\n","Memory size", mem_size);
  VerboseMessage ("#%25s : %d\n","Array Unit size", u_size);
  VerboseMessage ("#%25s : %d kB\n", "Page size", p_size);
  VerboseMessage ("#%25s : %d\n","Number of numa nodes", n_node);
  VerboseMessage ("#%25s : %d\n","Number of cpus online", n_cpus);  


  for (j = 0; j < n_node; j++)
    {

      VerboseMessage ("#%23s %d : %lld kB\n","Size of Numa node", j ,n_size[j]/1024);
    }


  
  long long int min_size;

  min_size = n_size[0];
  for (j = 1; j < n_node; j++)
    {
      if (n_size[j] < min_size)
	{
	  min_size = n_size[j];
	}
    }

  VerboseMessage ("#%25s : %lld kB\n","Min numa node size", min_size/1024);

  
  
  max_mem=min_size;
 
  def_mem_alloc = (long int) min_size/2  ;
  
  VerboseMessage("#%25s : %lld kB\n","Default size alloc Mem", def_mem_alloc/1024);

  VerboseMessage("\nReading your options.....\n\n");  
  
  
  GetOptionValue(ob, "UNIT", GOV_STRING, lchar,NULL,NULL, &def_UNIT);
  GetOptionValue(ob, "SIZE", GOV_INT64, &mem_alloc,&min_mem, &max_mem, &def_mem_alloc);
  GetOptionValue(ob, "INTER", GOV_INT32,&interleave, NULL,NULL,&def_inter);
  EvalOptions(ob);

 
  if (interleave){
     printf("I see you want fill memory page interleave inside numa node, i index jump pages\n");
  }else{
      printf("Page interleave inside the node is off, array filled sequtentially\n");
 }

  if ((mem_alloc==def_mem_alloc)&&(*lchar !=def_UNIT)) {
    mult=1;
    printf ("\nUnit specified and Size not specified or value equal to default %lld, reset size\n", mem_alloc);
  }else{

   
   UNIT=*lchar;
   

  
   mult=1;

   switch (UNIT){
        case 'K':
        case 'k':
              mult=1024;
              
              break;
        case 'M':
        case 'm':
              mult=1024*1024;
              break;
        case 'G':
        case 'g':
              mult=1024*1024*1024;
              break;
        default:
           printf ("Unit %c not understood blank,K,M,G accepted or not present or size not specified, assuming SIZE in bytes\n",UNIT);
           mult=1; 

       break;}
   }
   mem_alloc=mem_alloc*mult;
   M = mem_alloc / sizeof (float);

  //printf ("Number of Array elements: %lld,  allocating %lldk\n", M, M * sizeof (float)/1024);
  VerboseMessage("\n\nThese are the option provided for the test \n\n");
  


  printf ("*%25s : %lld kB\n", "Allocating memory",M * sizeof (float)/1024);

  printf ("*%25s : %lld\n","Nr. of Array elements",M);

  
  for (j = 0; j < n_node; j++)
    {
      mem_array[j] = numa_alloc_onnode (mem_alloc, j);
    }

  
  print_rusage(NULL);
  
  for (j = 0; j < n_node; j++)
    {
   getTime (&start);

  printf ("Allocating memory on node: %d\n",j);
  Rvec_gen (mem_array[j], M,interleave);

  getTime (&finish);

   total = timeDiff (&start, &finish);

  printf ("->Total Elapsed  Time allocating node %d = %ld usec.\n", j, (total) / 1000);
  }
  

  
   for (j = 0; j < n_node; j++)
    {
      getTime (&start);

       printf ("Scaling value   on node: %d\n",j);
       scaleV(mem_array[j], M);

       getTime (&finish);

         total = timeDiff (&start, &finish);

       printf ("->Total Elapsed  Time to scale array node %d = %ld usec.\n", j, (total) / 1000);
       printf ("->Estimate Bandwidth = %.1f GB/s \n",  (double)mem_alloc/(double)total);

  }
 
 if (n_node>1){
  print_rusage(NULL);


      getTime (&start);

       printf ("Summing value on two nodes: %d\n",j);
       sumV(mem_array[0],mem_array[1],mem_array[0], M,1);

       getTime (&finish);

         total = timeDiff (&start, &finish);
         //BW=(double)mem_alloc/(double)total;
       printf ("->Total Elapsed Time  to sum arrays on 2 nodes = %ld usec.\n",  (total) / 1000);
       
       printf ("->Estimate Bandwidth = %.1f GB/s \n",  (double)mem_alloc/(double)total);



       getTime (&start);
       float a=0.3;
       printf ("Summing value on two nodes and scale:\n");
       sumV(mem_array[0],mem_array[1],mem_array[0], M,a);

       getTime (&finish);

         total = timeDiff (&start, &finish);

       printf ("->Total Elapsed Time  to sum arrays on 2 nodes and scale = %ld usec.\n",  (total) / 1000);
       printf ("->Estimate Bandwidth = %.1f GB/s \n",  (double)mem_alloc/(double)total); 
       getTime (&start);
       
       printf ("Copy arrays from one node to another:\n");
       copyV(mem_array[0],mem_array[1], M);

       getTime (&finish);

         total = timeDiff (&start, &finish);

       printf ("->Total Elapsed Time  to copy one array from one node to another = %ld usec.\n",  (total) / 1000);
       printf ("->Estimate Bandwidth = %.1f GB/s \n",  (double)mem_alloc/(double)total);

    }


      getTime (&start);

   mem_array_inter=numa_alloc_interleaved(mem_alloc);

   printf ("Allocating memory on interleaved %d among nodes\n",n_node);
   Rvec_gen (mem_array_inter, M,interleave);

   getTime (&finish);

    total = timeDiff (&start, &finish);

   printf ("->Total Elapsed  Time  allocating interleave on %d nodes = %ld usec.\n", n_node, (total) / 1000);
   printf ("->Estimate Bandwidth = %.1f GB/s \n",  (double)mem_alloc/(double)total);
   print_rusage(NULL);
  







  for (j = 0; j < n_node; j++)
    {
  if (mem_array[j] != NULL)
    {
      VerboseMessage ("Info: Free Memory  on node %d\n",j);
      numa_free (mem_array[j], mem_alloc);

    }
  else
    {

      printf ("null pointer\n");

    }
}


 if (mem_array_inter != NULL){

   VerboseMessage("Info: Free Memory interleave again\n");
   numa_free(mem_array_inter,mem_alloc);
   }


printf("Starting with static array\n");
  
float A[M];
float B[M];
float C[M];



    getTime (&start);

    printf ("Filling Array A\n");
    Rvec_gen(A,M,0);
    getTime (&finish);

    total = timeDiff (&start, &finish);

    printf ("->Total Elapsed  Time  allocating array A = %ld usec.\n",  (total) / 1000);
    //printf ("->Estimate Bandwidth = %.1f GB/s \n",  (double)mem_alloc/(double)total);




    getTime (&start);

    printf ("Filling Array B\n");

    Rvec_gen(B,M,0);
    
    getTime (&finish);

    total = timeDiff (&start, &finish);

    printf ("->Total Elapsed  Time  allocating array B = %ld usec.\n",  (total) / 1000);
    //printf ("->Estimate Bandwidth = %.1f GB/s \n",  (double)mem_alloc/(double)total);


    getTime (&start);

    printf ("Filling Array B\n");

    scaleV(A,M);

    getTime (&finish);

    total = timeDiff (&start, &finish);

    printf ("->Total Elapsed  Time  Scale Array A = %ld usec.\n", (total) / 1000);
    printf ("->Estimate Bandwidth = %.1f GB/s \n",  (double)mem_alloc/(double)total);

    getTime (&start);

    printf ("Filling Array B\n");

    scaleV(B,M);

    getTime (&finish);

    total = timeDiff (&start, &finish);

    printf ("->Total Elapsed  Time  Scale Array B = %ld usec.\n",  (total) / 1000);
    printf ("->Estimate Bandwidth = %.1f GB/s \n",  (double)mem_alloc/(double)total);

    


    getTime (&start);

    printf ("Filling Array B\n");

    sumV(A,B,C,M,1);

    getTime (&finish);

    total = timeDiff (&start, &finish);

    printf ("->Total Elapsed  Time  sum Array A and B  = %ld usec.\n",  (total) / 1000);
    printf ("->Estimate Bandwidth = %.1f GB/s \n",  (double)mem_alloc/(double)total);

    getTime (&start);

    printf ("Filling Array B\n");

    sumV(A,B,C,M,0.3);

    getTime (&finish);

    total = timeDiff (&start, &finish);

    printf ("->Total Elapsed  Time  sum Array A and B and scale = %ld usec.\n",  (total) / 1000);
    printf ("->Estimate Bandwidth = %.1f GB/s \n",  (double)mem_alloc/(double)total);

  

    getTime (&start);

    printf ("Filling Array B\n");

    copyV(A,C,M);

    getTime (&finish);

    total = timeDiff (&start, &finish);

    printf ("->Total Elapsed  Time  copy Array A to C and scale = %ld usec.\n",  (total) / 1000);
    printf ("->Estimate Bandwidth = %.1f GB/s \n",  (double)mem_alloc/(double)total);

   return 0;
}
