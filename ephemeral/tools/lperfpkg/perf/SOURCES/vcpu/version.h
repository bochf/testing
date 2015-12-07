#define APP_NAME "vcpu"
/*
  Version History:
   0.1.0     5/4/13 - Original versioned copy (for Linux - from AIX version)
   0.2.0     5/6/13 - Work on calculations
   0.3.0    5/15/13 - Modifications of library calls
   0.4.0    5/16/13 - Added GetSMTValue() call.
   0.5.0     6/6/13 - Added calculations for virtualized CPUs.
   0.6.0    6/13/13 - Fixed issue with big /proc/stat files (in library)
   0.7.0   11/26/13 - Added the ordered list capability to this code.
   0.8.0     8/6/14 - Working in Jose's color changes
 */
#define VERSION_STRING "0.8.0"
/* ========================================================================= */
/*
  ToDo:
   [ ] The change to 32 cores seems to instigate a new crash on start condition.
   [ ] Need to handle virtualized CPUs. The library should collect tick info
       and if the total ticks are not == (or very close), then it should ADD
       those ticks to the idle thread. This way, all ticks add up to the total
       number of ticks in the iteration. This is a called library issue as
       well as a local handing of code issue.
  Done:
   [X] Most (idle) stats are showing up as "nan". This means that the total
       of the stats is 0. Which means we have a parse issue - or something.
   [X] Move over portions of code that can work in Linux
   [X] Options code needs to be simplified to basic stats and all stats



 */

