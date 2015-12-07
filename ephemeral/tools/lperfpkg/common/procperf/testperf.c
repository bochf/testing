#include <stdio.h>
#include <stdlib.h>

#include "procperf.h"

int cpu_perf_harness(void);
int cpu_info_harness(void);

/* ========================================================================= */
int main(int argc, char *argv[])
{
   printf("procperf test harness starting.\n");

   cpu_perf_harness();
   //cpu_info_harness();

   return(0);
}


/* ========================================================================= */
int cpu_perf_harness(void)
{
   struct cpustats *cpus;

   printf("Returned SMT value is %d.\n", GetSMTValue());

   if ( NULL == (cpus = GetCPUStats(NULL)) )
   {
      fprintf(stderr, "ERROR: issues parsing CPU stats.\n");
      return(1);
   }

   sleep(1);

   
   if ( NULL == GetCPUStats(cpus) )
   {
      fprintf(stderr, "ERROR: issues parsing CPU stats.\n");
      return(1);
   }

   printf("Counted %d CPUs.\n", cpus->cpucnt);

   printf("Sample CPU stat for CPU0:\n");
   printf("   instance  %d\n",   cpus->cpulist[0].instance);
   printf("   user      %llu\n", cpus->cpulist[0].user);
   printf("   nice      %llu\n", cpus->cpulist[0].nice);
   printf("   system    %llu\n", cpus->cpulist[0].system);
   printf("   idle      %llu\n", cpus->cpulist[0].idle);
   printf("   iowait    %llu\n", cpus->cpulist[0].iowait);
   printf("   irq       %llu\n", cpus->cpulist[0].irq);
   printf("   softirq   %llu\n", cpus->cpulist[0].softirq);

   fflush(stdout);

   return(0);
}

/* ========================================================================= */
int cpu_info_harness(void)
{
   unsigned long readsz;
   unsigned long bufsiz;
   /* struct cpuinfo *cpui; */
   void *filebuf;

   bufsiz = GetCPUInfo(NULL, 0);
 
   printf("CPU info buffer size is: %lu\n", bufsiz);

   if ( NULL == (filebuf = malloc(bufsiz) ) )
   {
      fprintf(stderr, "ERROR: Failed to allocate memory for buffer.\n");
      return(1);
   }

   readsz = GetCPUInfo(filebuf, bufsiz);

   printf("First read got %lu bytes.\n", readsz);

   /*
   if (NULL == (cpus = ParseCPUInfo(NULL, filebuf)))
   {
      fprintf(stderr, "ERROR: issues parsing CPU info.\n");
      return(1);
   }
   */
   fflush(stdout);

   return(0);
}



