#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "munge.h"

/* ========================================================================= */
int MungeData(struct interrupts *intr)
{
   struct interrupt *thisi;
   int cpu;
   Interrupt tempi;
   Interrupt toti;   /* Total interrupts of this type */
   
   /* Zero out CPU totals */
   cpu = 0;
   while ( cpu < intr->cpucnt )
   {
      intr->totl_ic[cpu] = 0;

      cpu++;
   }

   /* Calculate all interrupt/cpu totals */
   thisi = intr->ilist;
   while ( thisi )
   {
      /* Skip the munging of LOC stats if they will not be used */
      if (( intr->bSkipLOC ) && ( 0 == is_not_loc_intr(thisi) ))
      {
         thisi = thisi->next;
         continue;
      }

      cpu = 0;
      toti = 0;
      while ( cpu < intr->cpucnt )
      {
         tempi = thisi->this_ic[cpu] - thisi->last_ic[cpu];

         /* Make this a per-second value */
         if ( intr->interval > 1 )
         {
            tempi = tempi / intr->interval;
         } 

         thisi->diff_ic[cpu] = tempi;
         
         toti += tempi;
         
         intr->totl_ic[cpu] += tempi; /* Total interrput per CPU */

         cpu++;
      }

      /* Save the total interrupts of this type */
      thisi->diff_ic[intr->cpucnt] = toti;

      thisi = thisi->next;
   }

   /* Now walk through all CPU totals and sum that */
   cpu = 0;
   intr->totl_ic[intr->cpucnt] = 0;
   while ( cpu < intr->cpucnt )
      intr->totl_ic[intr->cpucnt] += intr->totl_ic[cpu++];

   return(0);
}

/* ========================================================================= */
/* Primarily about heat map and other derived values */
int SecondMunge(struct interrupts *intr)
{
   struct interrupt *thisi;
   int cpu;
   Interrupt maxi;
   Interrupt mini;
   Interrupt difi;
   float ftemp;
   
   /* This is a potential bug, but I think it is safe */
   mini = MAX_INTERRUPT_VALUE;
   maxi = 0;

   assert(NULL != intr);
   assert(NULL != intr->ilist); /* There is no reason for this to be NULL. */

   /* Walk interrupt totals (total per interrupt, not per CPU) and find
      the max and min. */
   mini = MAX_INTERRUPT_VALUE;
   maxi = 0;
   thisi = intr->ilist;
   while ( thisi )
   {
      if ( is_not_loc_intr(thisi) ) /* Don't include LOC interrupts */
      {
         if ( thisi->diff_ic[intr->cpucnt] > maxi )
            maxi = thisi->diff_ic[intr->cpucnt];

         if ( thisi->diff_ic[intr->cpucnt] < mini )
            mini = thisi->diff_ic[intr->cpucnt];
      }

      thisi = thisi->next;
   }

   difi = maxi - mini;
   ftemp = difi;
   ftemp = ftemp * (float).33;
   intr->intr_yzone = (Interrupt)ftemp + mini;
   ftemp = difi;
   ftemp = ftemp * (float).66;
   intr->intr_ozone = (Interrupt)ftemp + mini;
   ftemp = difi;
   ftemp = ftemp * (float).9;
   intr->intr_rzone = (Interrupt)ftemp + mini;

   /* Now derive the ranges for CPU coloring */
   mini = MAX_INTERRUPT_VALUE;
   maxi = 0;

   cpu = 0;
   while ( cpu < intr->cpucnt )
   {
      if ( intr->totl_ic[cpu] > maxi )
         maxi = intr->totl_ic[cpu];

      if ( intr->totl_ic[cpu] < mini )
         mini = intr->totl_ic[cpu];

      cpu++;
   }

   difi = maxi - mini;
   ftemp = difi;
   ftemp = ftemp * (float).33;
   intr->cpu_yzone = (Interrupt)ftemp + mini;
   ftemp = difi;
   ftemp = ftemp * (float).66;
   intr->cpu_ozone = (Interrupt)ftemp + mini;
   ftemp = difi;
   ftemp = ftemp * (float).9;
   intr->cpu_rzone = (Interrupt)ftemp + mini;

   return(0);
}

/* ========================================================================= */
int MungeCPU(struct interrupts *intr)
{
  int i;

  float H = 100;
  float idle;
  float user;
  float sys;
  float wait;

  float irq;
  float nice;
  float softirq;
  float steal;
  float guest;
  float guest_nice;

  float total;

  i = 0;
  while(i < intr->cpucnt)
  {
     /* Go ahead and set total (all) values even if we will not use them. We will set unconditionally,
        but calculate later conditionally. */
     user       = intr->this_cpus->cpulist[i].user       - intr->last_cpus->cpulist[i].user;
     nice       = intr->this_cpus->cpulist[i].nice       - intr->last_cpus->cpulist[i].nice;
     sys        = intr->this_cpus->cpulist[i].system     - intr->last_cpus->cpulist[i].system;
     idle       = intr->this_cpus->cpulist[i].idle       - intr->last_cpus->cpulist[i].idle;
     wait       = intr->this_cpus->cpulist[i].iowait     - intr->last_cpus->cpulist[i].iowait;
     irq        = intr->this_cpus->cpulist[i].irq        - intr->last_cpus->cpulist[i].irq;
     softirq    = intr->this_cpus->cpulist[i].softirq    - intr->last_cpus->cpulist[i].softirq;
     steal      = intr->this_cpus->cpulist[i].steal      - intr->last_cpus->cpulist[i].steal;
     guest      = intr->this_cpus->cpulist[i].guest      - intr->last_cpus->cpulist[i].guest;
     guest_nice = intr->this_cpus->cpulist[i].guest_nice - intr->last_cpus->cpulist[i].guest_nice;

     total = idle + user + sys + wait + nice + softirq + irq + steal + guest + guest_nice;

     if ( total < 100 )
     {
        total = 100;
        idle = total - ( user + sys + wait + nice + softirq + irq + steal + guest + guest_nice);
     }

     if ( 0 == total )
     {
        user = 0;
        nice = 0;
        sys = 0;
        idle = 1;
        wait = 0;
        irq = 0;
        softirq = 0;
        steal = 0;
        guest = 0;
        guest_nice = 0;
     }
     else
     {
        user = user / total;
        nice = nice / total;
        sys = sys / total;
        idle = idle / total;
        wait = wait / total;
        irq = irq / total;
        softirq = softirq / total;
        steal = steal / total;
        guest = guest / total;
        guest_nice = guest_nice / total;
     }
     
     intr->cpu_d[i].user = user * H;
     intr->cpu_d[i].nice = nice * H;
     intr->cpu_d[i].sys  = sys  * H;
     intr->cpu_d[i].idle = idle * H;
     intr->cpu_d[i].wait = wait * H;
     intr->cpu_d[i].irq = irq * H;
     intr->cpu_d[i].softirq = softirq * H;
     intr->cpu_d[i].steal = steal * H;
     intr->cpu_d[i].guest = guest * H;
     intr->cpu_d[i].guest_nice = guest_nice * H;

     i++;
  }

  return(0);
}

