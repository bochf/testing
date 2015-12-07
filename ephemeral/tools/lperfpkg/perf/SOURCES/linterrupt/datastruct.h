#ifndef DATASTRUCT_H
#define DATASTRUCT_H

#include "../../../common/procperf/procperf.h"

#define CPU_STR_SZ 8

/* Here is where you change the data type for the interrupt values. Note...
   changes here will break your printf() statements. I could get crafty and
   fix this, but I have not at this time. */
typedef long long Interrupt;
/* NOTE: I keep MAX_INTERRUPT_VALUE at a 2^30 value because it represents
         a sufficiently high value for interrupts in any reasonable given
         period. This is used to determine minimum values. (Set it high,
         then lower when a lower value is encountered. This is not a perfect
         algorithim, but this value seems sufficiently high to prevent issues).
 */
#define MAX_INTERRUPT_VALUE 1073741824

/* --------------------------------------------- */
struct interrupt
{
   char tag[3];           /* LOC, 122, 233, NMI, etc...                      */
   char *type;            /* PCI-MSI-X, IO-APIC-level, etc...                */
   char *label;           /* eth1-q0, rtc, serial, cciss0, etc...            */

   Interrupt *this_ic;    /* This Interrupt CPU list */
   Interrupt *last_ic;    /* Last Interrupt CPU list */
   Interrupt *diff_ic;    /* The diff that gets displayed */

   struct interrupt *next;
};

/* --------------------------------------------- */
struct cpuderived
{
   /* int instance;  -  Not really necessary in an indexed array */

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

   int color;
};

/* --------------------------------------------- */
struct interrupts
{
   char *buf;
   unsigned long bufsz;
   unsigned long lrs;     /* Last read size */
   int cpucnt;
   /* Things retrieved from command line options */
   int bTimestamp;        /* Show timestamp on each iteration                */
   int bHeader;           /* Show the header information                     */
   int bCollapse;         /* Remove interrupt lines that are all 0           */
   int bMonochrome;
   int bNames;
   int bShortNames;       /* Display a shortened interrupt name              */
   int bSkipLOC;          /* Skip LOC stats                                  */
   int bCPUStats;         /* Collect & show per-CPU stats */

   Interrupt interval;    /* Input type is int, but this type is Interrupt 
                             because we will math this in creating interrupts
                             per second.                                     */

   Interrupt *totl_ic;    /* Total interrupts handled per-cpu                */

   Interrupt cpu_rzone;   /* Per-CPU heat range for CPU label coloring       */
   Interrupt cpu_ozone;   /* Per-CPU heat range for CPU label coloring       */
   Interrupt cpu_yzone;   /* Per-CPU heat range for CPU label coloring       */
   Interrupt intr_rzone;  /* Per-Interrupt heat range for CPU label coloring */
   Interrupt intr_ozone;  /* Per-Interrupt heat range for CPU label coloring */
   Interrupt intr_yzone;  /* Per-Interrupt heat range for CPU label coloring */

   /* CPU perf stats from libprocperf */
   struct cpustats *last_cpus;
   struct cpustats *this_cpus;
   /* CPU stats derived from the above data */
   struct cpuderived *cpu_d;

   /* Non-CPU data from /proc/interrupts file */
   Interrupt ERR;
   Interrupt MIS;

   char *cpunames;       /* Multi-dimensional array using CPU_STR_SZ */

   struct interrupt *ilist;   /* Interrupt list */
};

/* ========================================================================= */
inline int is_not_loc_intr(struct interrupt *one_intr);

#endif
