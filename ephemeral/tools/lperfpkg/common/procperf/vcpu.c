#include <stdio.h>
#include <stdlib.h>

#include "procperf.h"

int run_cpu_stats(void);

int main(int argc, char *argv[])
{
   return(run_cpu_stats());
}


int dump_cpu_line(int instance, struct cpustat *lastcs, struct cpustat *thiscs)
{
   float user;
   float sys;
   float idle;
   float wait;
   float total;
   float HUNDRED = 100;

   total = thiscs->total - lastcs->total;
   if ( 0 == total )
      total = 1;

   user = thiscs->user - lastcs->user;
   user = (user / total) * HUNDRED;

   sys = thiscs->system - lastcs->system;
   sys = (sys / total) * HUNDRED;

   idle = thiscs->idle - lastcs->idle;
   idle = (idle / total) * HUNDRED;

   wait = thiscs->iowait - lastcs->iowait;
   wait = (wait / total) * HUNDRED;


   if ( instance >= 0 )
   {
      printf("cpu%-4d %5.1f %5.1f %5.1f %5.1f\n", instance, user, sys, idle, wait);
   }

   return(0);
}




int run_cpu_stats(void)
{
   unsigned long readsz;
   unsigned long bufsiz;
   struct cpustats *thiscpus;
   struct cpustats *lastcpus;
   struct cpustats *swapcpus;
   void *filebuf;

   bufsiz = GetCPUData(NULL, 0);
 
   if ( NULL == (filebuf = malloc(bufsiz) ) )
   {
      fprintf(stderr, "ERROR: Failed to allocate memory for buffer.\n");
      return(1);
   }

   readsz = GetCPUData(filebuf, bufsiz);

   if (NULL == (lastcpus = ParseCPUStats(NULL, filebuf)))
   {
      fprintf(stderr, "ERROR: issues parsing CPU stats.\n");
      return(1);
   }

   /* This is really just about allocating memory */
   if (NULL == (thiscpus = ParseCPUStats(NULL, filebuf)))
   {
      fprintf(stderr, "ERROR: issues parsing CPU stats.\n");
      return(1);
   }


   while ( 1 )
   {
      readsz = GetCPUData(filebuf, bufsiz);

      if (NULL == ParseCPUStats(thiscpus, filebuf))
      {
         fprintf(stderr, "ERROR: issues parsing CPU stats.\n");
         return(1);
      }

      printf("        %5s %5s %5s %5s\n", "user", "sys", "idle", "wait");
      dump_cpu_line(0, &lastcpus->cpulist[0], &thiscpus->cpulist[0]);
      dump_cpu_line(1, &lastcpus->cpulist[1], &thiscpus->cpulist[1]);

      printf("\n");

      fflush(stdout);

      swapcpus = lastcpus;
      lastcpus = thiscpus;
      thiscpus = swapcpus;
      sleep(1);
   }

   /*
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
   */


   return(0);
}



