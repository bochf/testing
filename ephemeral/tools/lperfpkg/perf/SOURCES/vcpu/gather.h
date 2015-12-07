#ifndef GATHER_H
#define GATHER_H

#include <time.h>
#include "../../../common/procperf/procperf.h"
#include "options.h"

/* ========================================================================= */
/*
  Some important discussion on the thread mapping concept:
   - WHY?
     Because Linux gives up the CPUs in no particularly good order. This
     means that the resulting ordering hides the archiceture of the box
     that is immediately visible in the AIX (and LoP) variant of this tool.
   - How?
     The libprocperf API now has a concept that allows you to build a
     device tree that consists of socket->core->thread mappings. From 
     this tree, the local (non-library) code then creates a mapping 
     "table" that it uses to properly display the information.
   - So how does this mapping work?
     The cpustats structs hold the CPUs in order given by the OS. This
     is the order that they show up in /proc/stat and /proc/cpuinfo, as
     well as the order that they are "numbered" (cpuX, cpuY, cpuZ). The
     cpua_derived struct (specifically "derived", not "totals") is ordered
     based upon this mapping. So.... the munge code (CalcLatest()) takes
     what is unordered (or ordered by an inperfect means) in the cpustats
     structs, and then orders the totals (derved results) into the
     cpua_derived *derived list. It uses the thread_mapping array to get
     this right.
*/


struct cpua_derived
{
   int instance;   /* This is used because the derived set of CPUs does 
                      not match the actual order as returned from the
                      system. We will be re-ordering them to properly
                      lay out the thread to core relationships. */

   float user;
   float nice;
   float sys;
   float idle;
   float wait;
   float irq;
   float softirq;
   float steal;
   float guest;
   float guest_nice;
};

struct cpu_list
{
   struct cpustats *thiscpus;
   struct cpustats *lastcpus;
   
   int *thread_mapping;   /* This is where we map threads to CPUs. It works
                             like this:
                               t = thread_mapping[i]
                             where i is the index of CPUs returned by the
                             call to GetCPUStats(), and t is the order in
                             which we wish to display them. So, the i-th
                             CPU as seen by the system might be displayed
                             as the t-th CPU. For example: the 0th and the
                             12th CPUs are "thread pairs" and need to be
                             displayed on the same line. So they will be
                             the 0th and 1st, as displayed by the tool. */
   int count;                  /* Members in the struct array                */
   int iteration_time;         /* Time between iterations in seconds         */   
   int *ANSI;                  /* (Additional) ANSI code for this CPU (an 
                                  array that corresponds to cpua_* arrays)   */
   struct cpua_derived *derived;
   struct cpua_derived *totals;
   size_t cpua_size;           /*                                            */

   
   time_t last_munge_time;     /* This is used to diff the actual time for
                                  time/iteration calculations.               */
};

/* ========================================================================= */
struct cpu_list *InitCPUList(void);

/* ========================================================================= */
int FreshenCPUList(struct cpu_list *cpul);

#endif
