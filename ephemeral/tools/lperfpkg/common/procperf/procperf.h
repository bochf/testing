#ifndef PROCPERF_H
#define PROCPERF_H

/* Read the following header files for API definitions */
#include "cpustat.h"   /* CPU stats from /proc/stat and system CPU layout
                          as derived from /sys/devices/system/cpu/           */
#include "cpuinfo.h"   /* Information about the CPUs in the system.
                          Note: This is currently PowerPC specific.          */

/* ========================================================================= */
/* Data below this point is about versioning information and library maint.  */

/*
  Version History:
   0.1.0    4/28/13 - Original creation
   0.2.0    4/28/13 - Added API for /proc/cpuinfo
   0.3.0     5/9/13 - Depricated the two-stage CPU stat gathering API
                    - Added a single GetCPUStats() API.
   0.4.0    5/16/13 - Added GetSMTValue() API.
   0.4.1    6/13/13 - Fixed issue with large /proc/stat files.
   0.5.0    6/14/13 - Now can parse up to the latest kernel /proc/stat
   0.6.0   11/25/13 - New BuildCPUTree() API
                    - Holee crap... the test harness is massively out of date
                      ...fixing that.
   0.7.0   11/26/13 - Minor revisions, major documentation efforts
   0.8.0    8/28/15 - CloneCPUStats() and some fixes for memory issues.

*/
#define PROCPERF_VERSION "0.8.0"
/*
  ToDo:
   [ ] testperf fails for cpu_info_harness() - a double free coredump. (The
       offending function is currently commented out to prevent coredumps.)
   [ ] Add code to parse the CPU tree
   [ ] Read the additional two columns out of /proc/stat. See the STUB items
       in cpustat.h.
   [ ] Make common API for basic tasks 
   [ ] Document the API (in the header files)
   [ ] Document the data structures

  Done:
   [X] Code requires documentation
   [X] Write the code to calculate SMT (Steal Gene's code)

*/


#endif
