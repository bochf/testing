#ifndef CPUSTAT_H
#define CPUSTAT_H

/* ========================================================================= */
struct cpustat
{
   int instance;
   unsigned long long user;         /* Time spent in user mode.              */
   unsigned long long nice;         /* Time spent in user mode (niced).      */
   unsigned long long system;       /* Time spent in system mode.            */
   unsigned long long idle;         /* Time spent in system mode.            */
   unsigned long long iowait;       /* Time spent in system mode.            */
   unsigned long long irq;          /* Time servicing interrupts.            */
   unsigned long long softirq;      /* Time servicing softirqs.              */
   unsigned long long steal;        /* Stolen time (by other virt hosts)     */
   unsigned long long guest;        /* Time spent on guest OS instance       */
   unsigned long long guest_nice;   /* Time spent on guest OS instance niced */

   unsigned long long total;

   int itemsvalid;
};


/* ========================================================================= */
/* The tree structures to understand how a system is laid out */
/* Note: On a no-thread CPU (1 thread per CPU) you will see a single thread
         struct tied to each core. Counts (of cores per socket, sockets per
         system, or threads per core) could be derived, but there is 
         currently no API for this */
struct cpu_thread
{
   int id;
   struct cpustat *cs;      /* threads (logical CPUs) hold cpustat data */

   struct cpu_thread *next;
};

struct cpu_core
{
   int id;
   struct cpu_thread *tlist; /* Cores are parents of threads */

   struct cpu_core *next;
};

struct cpu_socket
{
   int id;
   struct cpu_core *clist;   /* Sockets are parents of cores */

   struct cpu_socket *next;
};



/* ========================================================================= */
/* Parent struct that ties all CPU information together */
struct cpustats
{
   /* Public members */
   int cpucnt;               /* The number of cpuXX lines in /proc/stat      */
   struct cpustat *total;    /* Stat struct for the total cpu line           */
   struct cpustat *cpulist;  /* Array of stat structs for individual CPUs    */
   struct cpustat **cpu_index; /* Guarenteed ordered/index of cpulist items  */
   int isvalid;              /* Flag to determine if the stats were read
                                corectly / without error.                    */

   struct cpu_socket *cpu_tree; /* Holds the tree info as determined by the 
                                   BuildCPUTree() function. This is NULL if
                                   BuildCPUTree() has not been called.       */

   /* Private members */
   void *buf;               /* Read buffer holding raw /proc/stat contents   */
   unsigned long bufsize;   /* Size of the above buffer (rounded to page sz) */
};

/* =========================================================================
 * Name: GetCPUStats
 * Description: Fill a cpustats struct with a snapshot of the CPU data
 * Parameters: NULL or a pre-allocated cpustats struct
 * Returns: NULL or a pointer to a cpustats struct
 * Side Effects: May allocate memory (if input is NULL)
 * Notes:
 *       - This is a (relatively) thin wrapper on the read of /proc/stat.
 *         For example: It does not keep track of diffs, timestamps, previous
 *         vs current values. It simply retrieves the values from /proc, and
 *         the caller then needs to handle the diffs, rotating current/previous
 *         values, etc... My recommendation is to have two cpustats structs
 *         that you simply rotate. You refresh the stats on the oldest set
 *         and the current set then becomes the oldest. (Rotate the pointers.)
 *       - Call with NULL on the first call. It will allocate all memory
 *         required for the results.
 *       - It is better (read: Easier) to just call again with NULL, for
 *         the second struct (to compare between iterations), than to
 *         do some sort of memcpy().
 *       - The resulting struct holds a buffer that holds the raw data
 *         from the read(). This is marked "private" but could be
 *         used if you *really* needed to.
 */
struct cpustats *GetCPUStats(struct cpustats *cpus);

/* =========================================================================
 * Name: CloneCPUStats
 * Description: Create and fill a cpustats tree with cloned data
 * Parameters: A cpustats struct returned from GetCPUStats()
 * Returns: NULL or a pointer to a cpustats struct
 * Side Effects: Allocates memory
 * Notes:
 *       - This function will REUSE the buffer memory. This means that the
 *         source and destination structs will have the same memory for
 *         buffering content. If you (for some unknown reason) intend to
 *         use a cloned struct in a threaded model - where it will be called
 *         more than once at the same time - then be sure to malloc() a new
 *         buffer (cpustats->buf). (HINT: There is no reason to do this.)
 */
struct cpustats *CloneCPUStats(struct cpustats *cpus);


/* =========================================================================
 * Name: BuildCPUTree
 * Description: Build a "tree" of CPUs given a cpustats struct from GetCPUStats
 * Parameters: pre-allocated cpustats struct
 * Returns: 0 on success, 1 on failure
 * Side Effects: Will allocate memory, read lots of files
 * Notes:
 *       - Your application should be able to cope with a failure of this
 *         function. If the function fails, the cpu_tree member of the
 *         cpustats struct remains NULL, and the return value is non-zero.
 */
int BuildCPUTree(struct cpustats *cpus);

#endif

