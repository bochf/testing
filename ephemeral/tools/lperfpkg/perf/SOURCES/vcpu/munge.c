#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "munge.h"

/* ========================================================================= */
/* Add the desired extended data into the cpu_list structure */
int InitMungeData(struct options *o, struct cpu_list *c)
{
   struct boldll *bl;
   int i;
   int b;
   int b_on;

   /* The memory allocations for the arrays used in the munge functions, but
      always allocated with the source data in gather.c. */
   c->iteration_time = o->iteration_time;
   
   /* Here we must override the colums count if it exceeds the actual CPU 
      count. */
   /* This section is now depricated as this condition is explicitly checked for
      in the calling function (main())*/
   if(o->columns > c->count)
      o->columns = c->count;

   /* Set our bold option */
   if(NULL != (bl = o->bl))
   {
      if(bl->type == BLT_BAND)
      {
         i = 0;
         b = 0;
         b_on = 1;
         while(i < c->count)
         {
            if(b_on)
               c->ANSI[i] = 1;

            b++;
            i++;
            if(b == bl->cpu)
            {
               b_on = (! b_on);
               b = 0;
            }
         }
      }
      else
      {
         while(bl)
         {
            c->ANSI[bl->cpu] = 1;
            bl = bl->next;
         }
      }
   }

  return(0);
}

/* ========================================================================= */
int CalcLatest(struct options *o, struct cpu_list *c)
{
  int i;
  int t;
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
  time_t this_munge_time;
  int difftime;

  int virtualized;  /* Boolean - virtualized or not */

  float dbgidle;



  if(NULL == c)
    return(0);

  /* Find the difference in time - do some basic calcs */
  time(&this_munge_time);

  if(0 == c->last_munge_time)
  {
    difftime = 1;
  }
  else
  {
    difftime = this_munge_time - c->last_munge_time;
  }

  if(0 == difftime)
    difftime = 1;

  c->last_munge_time = this_munge_time;

  /* Calculate totals */
  idle       = c->thiscpus->total->idle        - c->lastcpus->total->idle;
  user       = c->thiscpus->total->user        - c->lastcpus->total->user;
  nice       = c->thiscpus->total->nice        - c->lastcpus->total->nice;
  sys        = c->thiscpus->total->system      - c->lastcpus->total->system;
  wait       = c->thiscpus->total->iowait      - c->lastcpus->total->iowait;
  irq        = c->thiscpus->total->irq         - c->lastcpus->total->irq;
  softirq    = c->thiscpus->total->softirq     - c->lastcpus->total->softirq;
  steal      = c->thiscpus->total->steal       - c->lastcpus->total->steal;
  guest      = c->thiscpus->total->guest       - c->lastcpus->total->guest;
  guest_nice = c->thiscpus->total->guest_nice  - c->lastcpus->total->guest_nice;

  /* Roll together - as necessary */
  if ( o->column_output == O_COLS_BASIC )
  {
     idle = idle + steal;
     steal = 0;

     sys = sys + irq + softirq;
     irq = 0;
     softirq = 0;

     user = user + guest + guest_nice + nice;
     guest = 0;
     guest_nice = 0;
     nice = 0;
  }

  if ( o->column_output == O_COLS_EXTENDED )
  {
     user = user + guest;
     nice = nice + guest_nice;
     guest = 0;
     guest_nice = 0;
  }

  total = idle + user + sys + wait + nice + irq + softirq + steal + guest + guest_nice;

  virtualized = 0;
  if ( total < 90 )
  {
     /* This means we are likely virtualized - assume 100 ticks, and work
        backwards to get idle time. */
     total = 100;
     idle = total - ( user + sys + wait);

     virtualized = 1;
  }

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
  
  c->totals->user = user * H;
  c->totals->nice = nice * H;
  c->totals->sys  = sys  * H;
  c->totals->idle = idle * H;
  c->totals->wait = wait * H;
  c->totals->irq = irq * H;
  c->totals->softirq = softirq * H;
  c->totals->steal = steal * H;
  c->totals->guest = guest * H;
  c->totals->guest_nice = guest_nice * H;


  i = 0;
  while(i < c->count)
  {

     /* Go ahead and set total (all) values even if we will not use them. We will set unconditionally,
        but calculate later conditionally. */
     user       = c->thiscpus->cpulist[i].user       - c->lastcpus->cpulist[i].user;
     nice       = c->thiscpus->cpulist[i].nice       - c->lastcpus->cpulist[i].nice;
     sys        = c->thiscpus->cpulist[i].system     - c->lastcpus->cpulist[i].system;
     idle       = c->thiscpus->cpulist[i].idle       - c->lastcpus->cpulist[i].idle;
     wait       = c->thiscpus->cpulist[i].iowait     - c->lastcpus->cpulist[i].iowait;
     irq        = c->thiscpus->cpulist[i].irq        - c->lastcpus->cpulist[i].irq;
     softirq    = c->thiscpus->cpulist[i].softirq    - c->lastcpus->cpulist[i].softirq;
     steal      = c->thiscpus->cpulist[i].steal      - c->lastcpus->cpulist[i].steal;
     guest      = c->thiscpus->cpulist[i].guest      - c->lastcpus->cpulist[i].guest;
     guest_nice = c->thiscpus->cpulist[i].guest_nice - c->lastcpus->cpulist[i].guest_nice;

     dbgidle = idle;


     /* Roll together - as necessary */
     if ( o->column_output == O_COLS_BASIC )
     {
        idle = idle + steal;
        steal = 0;

        sys = sys + irq + softirq;
        irq = 0;
        softirq = 0;
        
        user = user + guest + guest_nice + nice;
        guest = 0;
        guest_nice = 0;
        nice = 0;
     }

     if ( o->column_output == O_COLS_EXTENDED )
     {
        user = user + guest;
        nice = nice + guest_nice;
        guest = 0;
        guest_nice = 0;
     }

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

     /* This gets the logical CPU to thread mapping */
     t = c->thread_mapping[i];

     c->derived[t].user = user * H;
     c->derived[t].nice = nice * H;
     c->derived[t].sys  = sys  * H;
     c->derived[t].idle = idle * H;
     c->derived[t].wait = wait * H;
     c->derived[t].irq = irq * H;
     c->derived[t].softirq = softirq * H;
     c->derived[t].steal = steal * H;
     c->derived[t].guest = guest * H;
     c->derived[t].guest_nice = guest_nice * H;

     i++;
  }


  return(0);
}









