#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>

#include "cpustat.h"


/* ========================================================================= */
/* Name: read_proc_stat
 * Description: This reads /proc/stat into a buffer (if one is provided)
 * Parameters: - buffer (Can be NULL!)
 *             - the size of the buffer
 * Returns: the number of bytes (see notes below)
 * Side effects: Will malloc() a tempbuf, but free it if buf input is null.
 * Notes: The option to pass NULL for the buffer is to determine how much
 *        buffer you should allocate. You cannot simply stat() the file to 
 *        determine the size, so you call this with a NULL buffer to find 
 *        the size, then call again after you have allocated a buffer to 
 *        actually get the data.
 */
unsigned long read_proc_stat(void *buf, unsigned long bufsz)
{
   int fd;
   void *tempbuf = NULL;
   void *bufloc;
   size_t got;
   size_t totalgot;
   size_t rv;
   int getdata;

   /* DEBUG
   fprintf(stderr, "DEBUG: read_proc_stat(0X%08lX, %lu", (unsigned long)buf, bufsz);
   */
   getdata = 1; /* Assume we really care about the data */

   if ( NULL == buf )
   {
      /* The user wants to know the data size only */

      bufsz = 4096; /* Set to a page */

      if ( NULL == (tempbuf = malloc(bufsz)) )
         return(0);

      buf = tempbuf;
      getdata = 0; /* We don't care about the data */
   }

   if ( -1 == (fd = open("/proc/stat", O_RDONLY)) )
      return(0);

   totalgot = 0;
   bufloc = buf;
   while( 0 != (got = read(fd, bufloc, bufsz)) )
   {
      if ( got == -1 )
      {
         /* STUB: Could be valid. */

         if ( tempbuf )
         {
            free(tempbuf);
            tempbuf = NULL;
            buf = NULL;
         }
         close(fd);
         return(0);
      }

      totalgot += got;
      if ( getdata )
         bufloc = buf + totalgot;
   }

   close(fd);

   if ( tempbuf )
   {
      if ( 0 == (rv = 4096 * ( totalgot / 4096 )) )
         rv = 4096;

      /* If we are within 2K, then add another page to the buffer */
      if ( rv - totalgot >= 2048 )
         rv += 4096;

      free(tempbuf);
      tempbuf = NULL;
      buf = NULL;
   }
   else
      rv = totalgot;

   /* DEBUG
   fprintf(stderr, ") = %lu\n", rv);
   */
   return(rv);
}

/* ========================================================================= */
/* Name: bufhandle
 * Description: Internal library struct to act as a file pointer for buffer
 * Notes: This is not a read() on a file, it is a memory operation... so
 *        this struct holds the memory location where the "file pointer" is
 *        sitting at as it is moved through multiple functions.
 */
struct bufhandle
{
   void *buf;
   void *readptr;
};

/* ========================================================================= */
/* Name: get_cpu_line
 * Description: Retrieve a "cpu*" line from the /proc/stat *buffer*
 * Parameters: char* - buffer to read into
 *             int   - size of the buffer
 *             bufhandle - struct that keeps track of the buffer
 * Returns: NULL - no cpu line, pointer to input char* if pattern match
 * Side effects: MAKES LOTS OF ASSUMPTIONS! BUGS GALORE! See notes.
 * Notes: Why Johnny can't read:
 *         - There is no *real* protection against running off the end of 
 *           the buffer. The protection (that is holding the line) is the
 *           fact that the cpu lines show up early in the file and we just
 *           don't call again after the first failed result. The code checks
 *           for a null termination, but (to the best of my knowledge) this
 *           buffer is not NULL terminated.
 *         - If the line is not pattern matched the function returns NULL,
 *           and does NOT advance the read pointer! So... counter to the
 *           previous point... you can never read past the last cpu line
 *           using this function.
 */
char *get_cpu_line(char *s, int n, struct bufhandle *bh)
{
   int offset;
   char *readptr;

   offset = 0;
   readptr = (char *)bh->readptr;
   while((readptr[offset] != 0) && (readptr[offset] != '\n'))
   {
      s[offset] = readptr[offset];

      offset++;

      if ( offset == n )
         return(NULL);
   }

   s[offset] = 0;
   if (readptr[offset] == '\n')
      offset++;

   if ((s[0] != 'c') || (s[1] != 'p') || (s[2] != 'u'))
      return(NULL);

   bh->readptr += offset;
   return(s);
}

/* ========================================================================= */
/* Name: alloc_cpustats
 * Description: Allocates the cpustats structure(s)
 * Parameters: The number of cpu* lines in the /proc/stat file
 * Returns: A pointer to the resulting structure, NULL on failure
 * Side effects: Allocates a bit of memory
 * Notes: This is just a slightly cleaner method of allocating this data
 *        structure (than having it mixed in with more complex code)
 */
struct cpustats *alloc_cpustats(int cpulncnt)
{
   struct cpustats *cpus;

   if ( 2 > cpulncnt )
      return(NULL);

   /* Parent struct */
   if ( NULL == (cpus = (struct cpustats *)malloc(sizeof(struct cpustats))) )
      return(NULL);

   /* NOTE: This is a conversion of LINE COUNT to CPU COUNT. See the
            NOTE in CloneCPUStats(). */
   cpus->cpucnt = cpulncnt - 1;

   /* Total CPU stats */
   if ( NULL == (cpus->total = (struct cpustat *)malloc(sizeof(struct cpustat))) )
   {
      free(cpus);
      return(NULL);
   }

   /* Per-CPU stats */
   if ( NULL == (cpus->cpulist = (struct cpustat *)malloc(sizeof(struct cpustat) * cpus->cpucnt)) )
   {
      free(cpus->total);
      free(cpus);
      return(NULL);
   }

   /* Allocate memory used for the index */
   if ( NULL == (cpus->cpu_index = (struct cpustat **)malloc(sizeof(struct cpustat *) * cpus->cpucnt)) )
   {
      free(cpus->cpulist);
      free(cpus->total);
      free(cpus);
      return(NULL);
   }

   /* The CPU device tree */
   cpus->cpu_tree = NULL; /* Not established at this time */

   return(cpus);
}

/* ========================================================================= */
/* Name: parse_cpu_line
 * Description: Convert a line of text into a cpustat struct
 * Parameters: cpustatS struct (to fill)
 *             char* - line of the file to read from
 * Returns: Number of lines parsed (0 or 1)
 * Side effects: 
 * Notes: This function must be called FOR EACH LINE in the file. It will
 *        find the appropariate CPU number (or total) from the line, and
 *        then apply the findings to that to the correct cpustat member.
 */
int parse_cpu_line(struct cpustats *cpus, char *line)
{
   int instance = -1;
   unsigned long long user = 0;
   unsigned long long nice = 0;
   unsigned long long system = 0;
   unsigned long long idle = 0;
   unsigned long long iowait = 0;
   unsigned long long irq = 0;
   unsigned long long softirq = 0;
   unsigned long long steal = 0;
   unsigned long long guest = 0;
   unsigned long long guest_nice = 0;

   int itemsread = 0;

   assert(cpus != NULL);
      
   if (NULL == line)
      return(0);
   if (line[0] == 0)
      return(0);

   if ((line[0] != 'c') || (line[1] != 'p') || (line[2] != 'u'))
      return(0);
   
   if ( line[3] == ' ' )
   {
      /* This is the total line */
      itemsread = sscanf(line, "cpu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                         &user,
                         &nice,
                         &system,
                         &idle,
                         &iowait,
                         &irq,
                         &softirq,
                         &steal,
                         &guest,
                         &guest_nice);
      
      cpus->total->user = user;
      cpus->total->nice = nice;
      cpus->total->system = system;
      cpus->total->idle = idle;
      cpus->total->iowait = iowait;
      cpus->total->irq = irq;
      cpus->total->softirq = softirq;
      cpus->total->steal = steal;
      cpus->total->guest = guest;
      cpus->total->guest_nice = guest_nice;

      cpus->total->itemsvalid = itemsread;
      cpus->total->total = user + nice + system + idle + iowait + irq + softirq + steal + guest + guest_nice;
   }
   else
   {
      /* This is an individual CPU line */
      itemsread = sscanf(line, "cpu%d %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                         &instance,
                         &user,
                         &nice,
                         &system,
                         &idle,
                         &iowait,
                         &irq,
                         &softirq,
                         &steal,
                         &guest,
                         &guest_nice);
      
      /* Why set instance each time?
         1. Because it is not expensive.
         2. So we can insure that the /proc file was properly parsed. (The
            instance value should always match the array instance. This could
            be zero'd out (or set to -1) elsewhere to insure that it was
            properly refreshed.)
      */
      cpus->cpulist[instance].instance = instance;
      cpus->cpulist[instance].user = user;
      cpus->cpulist[instance].nice = nice;
      cpus->cpulist[instance].system = system;
      cpus->cpulist[instance].idle = idle;
      cpus->cpulist[instance].iowait = iowait;
      cpus->cpulist[instance].irq = irq;
      cpus->cpulist[instance].softirq = softirq;
      cpus->cpulist[instance].steal = steal;
      cpus->cpulist[instance].guest = guest;
      cpus->cpulist[instance].guest_nice = guest_nice;

      cpus->cpulist[instance].itemsvalid = itemsread;
      cpus->cpulist[instance].total = user + nice + system + idle + iowait + irq + softirq + steal + guest + guest_nice;
   }

   return(1);
}


/* ========================================================================= */
/* Name: build_cpu_index
 * Description: Builds an index of the CPUs for faster instance retrieval
 * Parameters: struct cpustats (that has not been indexed)
 * Returns: 0 on success, non-0 on failure
 * Side effects: Fills out cpu_index
 * Notes:
 */
int build_cpu_index(struct cpustats *cpus)
{
   int c;

   c = 0;
   while ( c < cpus->cpucnt )
   {
      cpus->cpu_index[cpus->cpulist[c].instance] = &cpus->cpulist[c];
      c++;
   }

   return(0);
}



/* ========================================================================= */
/* Name: GetCPUStats (Externally exposed function - see header file)
 * Description: -
 * Parameters: -
 * Returns: -
 * Side effects: -
 * Notes: This code has a STUB in it.
 */
struct cpustats *GetCPUStats(struct cpustats *cpus)
{
   char line[256];
   struct bufhandle bh;
   void *readbuf;
   unsigned long bufsize;
   int cpulncnt = 0;
   int index_cpus = 0;     /* Should we index the CPUs (or not) */

   //STUB fprintf(stderr, "GetCPUStats(");

   //STUB fprintf(stderr, "1");

   /* Check for NULL input - and handle the buffer part ONLY */
   if ( NULL == cpus )
   {
      /* Determine how big the buffer needs to be */
      if ( 0 == (bufsize = read_proc_stat(NULL, 0)) )
         return(NULL);

      /* Allocate some memory for it based on what we just found */
      if ( NULL == (readbuf = malloc(bufsize)) )
         return(NULL);
   }
   else
   {
      readbuf = cpus->buf;       /* Get the already allocated buffer */
      bufsize = cpus->bufsize;   /* Get the size of the buffer       */
      cpus->isvalid = 0;         /* Consider invalid until done      */
   }

   //STUBfprintf(stderr, "2");

   /* So now readbuf is good, and the bufsize is good, read in actual data  */
   if ( 0 == read_proc_stat(readbuf, bufsize) )
      return(cpus);

   /* If cpus struct is null - then allocate it */
   if ( cpus == NULL )
   {
      /* Set up our buffer pointers */
      bh.buf = readbuf;
      bh.readptr = readbuf;

      /* Make a note to index the CPU list */
      index_cpus = 1;

      /* Read data out of the buffer - just to count CPU lines */
      while(get_cpu_line(line, 256, &bh))
      {
         cpulncnt++;
      }

      /* Allocate the cpustats structure */
      if ( NULL == (cpus = alloc_cpustats(cpulncnt)) )
      {
         free(readbuf); /* Do this because we know it does not belong to a cpustats struct */
         return(NULL);
      }

      /* Add-in our previously allocated read buffer */
      cpus->buf = readbuf;
      cpus->bufsize = bufsize;
      
      /* Mark this read as invalid - as we are not done at this time */
      cpus->isvalid = 0;
   }

   /* Now start reading data into the cpustats struct */
   bh.buf = readbuf;
   bh.readptr = readbuf;

   while(get_cpu_line(line, 256, &bh))
   {
      parse_cpu_line(cpus, line);
   }

   /* STUB: Probably should be validating against the results of parse_cpu_line() */
   cpus->isvalid = 0;

#ifdef STUB_DEBUG
   if ( index_cpus )
   {
      if (build_cpu_index(cpus))
      {
         /* This will be our first time through. All memory has been 
            successfully allocated, so it must all be freed. */
         /* STUB: Call free_all_cpus() - after you write it */
         return(NULL);
      }
   }
#endif

   //STUB fprintf(stderr, ");\n");


   return(cpus);
}


/* ========================================================================= */
/* Name: CloneCPUStats (Externally exposed function - see header file)
 * Description: -
 * Parameters: -
 * Returns: -
 * Side effects: -
 * Notes: 
 */
struct cpustats *CloneCPUStats(struct cpustats *cpus)
{
   struct cpustats *ncpus;

   /* Check for NULL input - and handle the buffer part ONLY */
   /* NOTE: This really should be an assert(), but because we are in
            library code, I try to handle it as gracefully as possible */
   if ( NULL == cpus )
      return(GetCPUStats(NULL));

   /* Do all the allocations */
   /* NOTE: alloc_cpustats() has a single paramater of the *line count*
            NOT the number of CPUs. The line count has one more than
            the number of CPUs. So, if you are going to pass the CPU
            count to this function, you must add one to match what the
            line count is (/ would be). */
   if ( NULL == (ncpus = alloc_cpustats(cpus->cpucnt + 1)) )
      return(NULL);

   /* Copy cpu totals */
   memcpy(ncpus->total, cpus->total, sizeof(struct cpustat));
   /* Copy per-CPU totals */
   memcpy(ncpus->cpulist, cpus->cpulist, sizeof(struct cpustat) * cpus->cpucnt);
   /* No tree structure (GetCPUStats does not do this) */
   /* STUB: The appropriate response would be to check for !NULL here
            and fill the structure accordingly. */
   ncpus->cpu_tree = NULL;
   /* Copy over the validitiy boolean */
   ncpus->isvalid = cpus->isvalid;
   /* Save memory - reuse the existing buffer */
   /* fprintf(stderr, "DEBUG: CloneCPUStats(0X%08lX, %lu);\n", (unsigned long)cpus->buf, cpus->bufsize); */
   ncpus->buf = cpus->buf;
   ncpus->bufsize = cpus->bufsize;

   if (build_cpu_index(ncpus))
   {
      /* STUB: See notes in this section from GetCPUStats() */
      return(NULL);
   }

   /* ...aaaannnd we are out of here */
   return(ncpus);
}

/* ========================================================================= */
/* Name: rawsyscpu struct
 * Description: Holds the raw data as retrieved from SYS_CPU_DIR
 * Notes: The data is read into a linked list of this struct, it is then
          used to build a tree. This data structure is not exposed to the
          outside world. It is an intermediate structure used to simplfy
          the reading of the data.

          The thread_siblings_list is a hard-sized array that is based on
          the MAX_THREADS_PER_CORE value. As it turns out, it is not required
          to build the data tree... and it is not availiable on all versions
          of the kernel. So it is basically disabled via the:
          REQUIRE_SIBLING_LIST #define. (That is not defined.)

          The MAX_THREADS_PER_CORE is a "limation" on reading one file. It
          is not required (as mentioneed earlier) but more importantly IS
          NOT a limitation on the actual number of cores/threads/sockets on
          a system. These items are held in linked lists, and are unbounded.
 */
#define MAX_THREADS_PER_CORE 8  /* The max # of threads we may ever expect */
struct rawsyscpu
{
   int logical_id;
   int core_id;
   int physical_package_id;
#ifdef REQUIRE_SIBLING_LIST
   int thread_siblings_list[MAX_THREADS_PER_CORE];
#endif
   struct rawsyscpu *next;
};

#define SYS_CPU_DIR "/sys/devices/system/cpu/"

/* ========================================================================= */
/* Name: is_a_cpu_dir_name
 * Description: Boolean test for pattern match on a dir name.
 * Parameters: dirent from readdir_r()
 * Returns: 0 if NOT a cpu dir, 1 if a cpu[0-9]* dir
 * Side effects:
 * Notes: This is a simple test function to hide the details from
          the calling function.
 */
int is_a_cpu_dir_name(struct dirent *dir)
{
   if ( dir->d_name[0] != 'c' )
      return(0);

   if ( dir->d_name[1] != 'p' )
      return(0);

   if ( dir->d_name[2] != 'u' )
      return(0);

   if (( dir->d_name[3] < '0' ) || ( dir->d_name[3] > '9' ))
      return(0);

   /* This is sufficient. We only need to match cpu and then a
      number. This will exclude all non-cpu directories including the
      cpu<string> directories in the later kernels. */
   return(1);
}

/* ========================================================================= */
/* Name: read_physp_id
 * Description: Read the physical_package_id value and return it
 * Parameters: The cpuXX directory we are working in
 * Returns: The value from the file, or -1 on failure
 * Side effects: Opens, reads, closes the file in /sys (SYS_CPU_DIR)
 * Notes: read() is (Stevens) unwrapped, the exposure is insignificant.
 *        This is true for all the like functions that follow.
 */
int read_physp_id(char *d_name)
{
   char physp_id_fname[64];
   char buff[32];
   int physp_id;
   int fd;
   int got;
   int i;
   
   sprintf(physp_id_fname, "%s%s/topology/physical_package_id", SYS_CPU_DIR, d_name);

   if ( -1 == (fd = open(physp_id_fname, O_RDONLY)))
      return(-1);

   if ( 0 == (got = read(fd, buff, 32)) )
   {
      close(fd);
      return(-1);
   }

   physp_id = 0;
   i = 0;
   while(i < got)
   {
      if (( buff[i] >= '0' ) && ( buff[i] <= '9' ))
      {
         physp_id *= 10;
         physp_id += buff[i] - '0';
      }
      i++;
   }
      
   close(fd);

   return(physp_id);
}

/* ========================================================================= */
/* Name: read_core_id
 * Description: Read the core_id value and return it
 * Parameters: The cpuXX directory we are working on
 * Returns: Value of core_id, or -1 on error
 * Side effects: Opens, reads, closes the file in /sys (SYS_CPU_DIR)
 * Notes:
 */
int read_core_id(char *d_name)
{
   char core_id_fname[64];
   char buff[32];
   int core_id;
   int fd;
   int got;
   int i;
   
   sprintf(core_id_fname, "%s%s/topology/core_id", SYS_CPU_DIR, d_name);

   if ( -1 == (fd = open(core_id_fname, O_RDONLY)))
      return(-1);

   if ( 0 == (got = read(fd, buff, 32)) )
   {
      close(fd);
      return(-1);
   }

   core_id = 0;
   i = 0;
   while(i < got)
   {
      if (( buff[i] >= '0' ) && ( buff[i] <= '9' ))
      {
         core_id *= 10;
         core_id += buff[i] - '0';
      }
      i++;
   }
      
   close(fd);

   return(core_id);
}

/* ========================================================================= */
/* Name: read_logical_cpu_id
 * Description: Parses the (thread/logical) CPU # from the directory name
 * Parameters: The name of the directory as pulled from SYS_CPU_DIR
 * Returns: The cpu number, errors are not detected (will return a valid 0)
 * Side effects:
 * Notes: Note, this number is the number as seen in /proc/stat and
 *        /proc/cpuinfo and other places. It is the common/visible number.
 */
int read_logical_cpu_id(char *d_name)
{
   int logical_id;
   int i;

   /* We *know* that the first three characters are cpu and that is followed
      by at least one number (we tested for that earlier) */
   i = 3;
   logical_id = 0;
   while (( d_name[i] >= '0' ) && ( d_name[i] <= '9' ))
   {
      logical_id *= 10;
      logical_id += d_name[i] - '0';
      i++;
   }

   return(logical_id);
}

#ifdef REQUIRE_SIBLING_LIST
/* ========================================================================= */
/* Name: read_tsibling_list         ---> DEPRICATED <---
 * Description: Read the sibling list directly into the struct
 * Parameters:
 * Returns:
 * Side effects: Opens, reads, closes the file in /sys (SYS_CPU_DIR)
 * Notes: Simply not required, and is currently dead code
 */
int read_tsibling_list(struct rawsyscpu *rsc, char *d_name)
{
   char tsl_fname[64];
   char buff[64];
   int fd;
   int got;
   int i;   /* Character index - walks us through the char array */
   int tsi; /* thread sibling index - walks us through the int array */ 

   /* make the list "empty* */
   tsi = 0;
   while ( tsi < MAX_THREADS_PER_CORE )
      rsc->thread_siblings_list[tsi++] = -1;

   /* Now read in the data */
   sprintf(tsl_fname, "%s%s/topology/thread_siblings_list", SYS_CPU_DIR, d_name);

   if ( -1 == (fd = open(tsl_fname, O_RDONLY)))
      return(1);

   if ( 0 == (got = read(fd, buff, 64)) )
   {
      close(fd);
      return(1);
   }

   close(fd);

   i = 0;
   tsi = 0;
   while ( i < got )
   {
      if (( buff[i] >= '0' ) && ( buff[i] <= '9' ))
      {
         rsc->thread_siblings_list[tsi] = 0;
         while (( buff[i] >= '0' ) && ( buff[i] <= '9' ))
         {
            rsc->thread_siblings_list[tsi] *= 10;
            rsc->thread_siblings_list[tsi] += buff[i] - '0';
            i++;
         }
         tsi++;
      }

      /* Move off the comma */
      if ( buff[i] >= ',' )
         i++;
      
      i++;
   }

   return(0);
}
#endif

/* ========================================================================= */
/* Name: insert_new_cpu
 * Description: Inserts a new CPU (and data) into the rawsyscpu list
 * Parameters: Head of the rawsyscpu list
 *             The directory for the CPU that is to be read (NOTE: This
 *             is NOT the full path, just the relative dir name.)
 * Returns: The head of the list, NULL on failure!
 * Side effects: Will wreck your linked list and leak memory on error (if
 *             you blindly accept the return value).
 *             Also does a fair amount of file I/O.
 * Notes: Debuggery is left in place - on the outside chance that Linux
 *             /sys structures change, and you want to debug from here.
 */
struct rawsyscpu *insert_new_cpu(struct rawsyscpu *rscl, struct dirent *dir)
{
   struct rawsyscpu *rsci; /* The new list item */

#ifdef DEBUGGERY
   fprintf(stderr, "insert_new_cpu()\n");
#endif
   
   if ( NULL == (rsci = (struct rawsyscpu *)malloc(sizeof(struct rawsyscpu))) )
      return(NULL);

   if ( -1 == (rsci->logical_id = read_logical_cpu_id(dir->d_name)))
   {
      free(rsci);
      return(NULL);
   }

   if ( -1 == (rsci->core_id = read_core_id(dir->d_name)))
   {
      free(rsci);
      return(NULL);
   }

   if ( -1 == (rsci->physical_package_id = read_physp_id(dir->d_name)))
   {
      free(rsci);
      return(NULL);
   }

#ifdef REQUIRE_SIBLING_LIST
   if ( read_tsibling_list(rsci, dir->d_name) )
   {
      free(rsci);
      return(NULL);
   }
#endif

#ifdef DEBUGGERY
   fprintf(stderr, "  logical_id = %d\n", rsci->logical_id);
   fprintf(stderr, "  core_id = %d\n", rsci->core_id);
   fprintf(stderr, "  physical_package_id = %d\n", rsci->physical_package_id);
   fprintf(stderr, "  thread_siblings_list = [%d] [%d] [%d] [%d]\n",
           rsci->thread_siblings_list[0],
           rsci->thread_siblings_list[1],
           rsci->thread_siblings_list[2],
           rsci->thread_siblings_list[3]);
#endif

   rsci->next = rscl;
   return(rsci);
}



/* ========================================================================= */
/* Name: read_sys_cpus
 * Description: Open SYS_CPU_DIR and create a rawsyscpu linked list of CPUs
 * Parameters: None
 * Returns: A linked list of CPU data. NULL on error (failure to open dir)
 * Side effects: Allocates memory, reads files.
 * Notes: I used the reentrant readdir - I think that was simply overkill.
 */
struct rawsyscpu *read_sys_cpus(void)
{
   DIR *sdsc;
   struct dirent cpuXX;
   struct dirent *result;
   struct rawsyscpu *rscl; /*raw sys cpu list */

   rscl = NULL;   /* The list is empty */

   /* Open the directory - Is this properly reentrant? */
   if ( NULL == (sdsc = opendir(SYS_CPU_DIR)) )
      return(NULL);


   while(0 == readdir_r(sdsc, &cpuXX, &result))
   {
      if ( result == NULL )
      {
         /* The end of the dir has been reached */
         break;
      }
      else
      {
         if ( is_a_cpu_dir_name(&cpuXX) )
            rscl = insert_new_cpu(rscl, &cpuXX);
      }
   }
   
   closedir(sdsc);

   return(rscl);
}


/* ========================================================================= */
/* Name: new_[socket|core|thread]
 * Description: Create (and initialize) a CPU socket/core/thread structure
 * Parameters: The ID of the socket/core/thread to set the id field
 * Returns: A new cpu_XXXXXX struct, NULL on the unlikely event of a failure
 * Side effects: 
 * Notes: Simply a malloc & initialize type of operation
 */


/* ========================================================================= */
/* Name: insert_[socket|core|thread]
 * Description: Insert a CPU socket/core/thread structure into the tree
 * Parameters: The cpu_STRUCT tree base
 *             A rawsyscpu struct that contains the item to be inserted
 * Returns: The new base of the tree (which will be different socket # times)
 * Side effects: A tree grows in the middle of your application
 * Notes: How this works:
 *        1. Read in all cpuXX dirs using insert_new_cpu. This will create a
 *           simple (one-dimensional) linked list of CPU information.
 *        2. Take that list, and call insert_socket() for every member. This
 *           builds the socket (trunk) of the tree.
 *        3. Take that list (again) and call insert_core() for every member.
 *           This builds the branches of the tree.
 *        4. Take that list (finally) and call insert_thread() for every
 *           member. This populates the tree with leaves.
 *        5. Call insert_cpuastat() to link each leaf of this tree back to
 *           the appropriate CPU stats structure (defined elsewhere in this
 *           file/library).
 *        6. free() the linked list of rawsyscpu structs created in #1.
 *           NOTE: This code does not do #6. It (technically) leaks memory.
 *           This is not considered a (major) problem as you do not call
 *           this repeatedly. And the memory "leak" is a one time event.
 *
 *        The exception handling here is rather weak. Basically, we are
 *        replicating a tree that the kernel has by reading the files the
 *        kernel gives us. So (theoritically) the exposure is rather slim.
 */

/* ========================================================================= */
/* Name: new_socket
 * Notes: See the new_[socket|core|thread] description above
 */
struct cpu_socket *new_socket(int socket_id)
{
   struct cpu_socket *thiss;
   
   if (NULL == (thiss = (struct cpu_socket *)malloc(sizeof(struct cpu_socket))))
      return(NULL);

   thiss->id = socket_id;
   thiss->clist = NULL;
   thiss->next = NULL;

   return(thiss);
}


/* ========================================================================= */
/* Name: insert_socket
 * Notes: See the insert_[socket|core|thread] description above
 */
struct cpu_socket *insert_socket(struct cpu_socket *s, struct rawsyscpu *r)
{
   struct cpu_socket *slist;  /* for walking through list */
   struct cpu_socket *slast;  /* Ordered list insert (last item) */
   struct cpu_socket *thiss;  /* The item we are working with */

   /* If the linked list is empty, then make it the new list */
   if ( NULL == s )
   {
      if ( NULL == (thiss = new_socket(r->physical_package_id)) )
         return(NULL);

      return(thiss);
   }

   /* See if the value is in the list now. This is:
      1. Likely to be a hit (it is likely in the list)
      2. A very in-expensive lookup
   */
   slist = s;
   while ( slist )
   {
      if ( slist->id == r->physical_package_id )
      {
         return(s); /* got it, return list unmodified */
      }
      slist = slist->next;
   }

   if ( NULL == (thiss = new_socket(r->physical_package_id)) )
      return(NULL);

   /*** ordered insert ***/

   /* Insert at top */
   if ( s->id > thiss->id )
   {
      thiss->next = s;
      return(thiss);
   }

   /* insert in middle */
   slist = s->next;
   slast = s;
   while (slist)
   {
      if ( slist->id > thiss->id )
      {
         thiss->next = slast->next;
         slast->next = thiss;
         return(s);
      }
      slast = slist;
      slist = slist->next;
   }

   /* if we got here append at end */
   slast->next = thiss;
   return(s);
}
   
/* ========================================================================= */
/* Name: new_core
 * Notes: See the new_[socket|core|thread] description above
 */

struct cpu_core *new_core(int id)
{
   struct cpu_core *c;

   if ( NULL == ( c = (struct cpu_core *)malloc(sizeof(struct cpu_core)) ) )
      return(NULL);

   c->id = id;
   c->tlist = NULL;
   c->next = NULL;

   return(c);
}

/* ========================================================================= */
/* Name: insert_core
 * Notes: See the insert_[socket|core|thread] description above
 */
int insert_core(struct cpu_socket *s, struct rawsyscpu *r)
{
   struct cpu_socket *slist;  /* for walking through list */
   struct cpu_socket *thiss;  /* The socket we belong on */
   struct cpu_core *thisc;    /* The item we are working with */
   struct cpu_core *clist;
   struct cpu_core *clast;

   /* Find the right socket */
   slist = s;
   thiss = NULL;
   while ( slist )
   {
      if ( slist->id == r->physical_package_id )
         thiss = slist;

      slist = slist->next;
   }

   if ( NULL == thiss )
      return(1);

   /* There are lots of implications of exiting here - leaving lots of 
      haning memory locations, but basically we gotta cut and run...
      If you can't malloc() then it is over. */
   if ( NULL == (thisc = new_core(r->core_id)) )
      return(1);

   /*** ordered insert ***/

   /* Top of the list (no other items) */
   if ( thiss->clist == NULL )
   {
      thiss->clist = thisc;
      return(0);
   }

   /* Top of the list (sorted first) */
   if ( thiss->clist->id > thisc->id )
   {
      thisc->next = thiss->clist;
      thiss->clist = thisc;
      return(0);
   }

   if ( thiss->clist->id == thisc->id )
   {
      /* A bit wasteful - but we only do it once */
      free(thisc);
      return(0);
   }

   /* into the middle */
   clast = thiss->clist;
   clist = clast->next;
   while ( clist )
   {
      if ( clist->id > thisc->id )
      {
         thisc->next = clast->next;
         clast->next = thisc;
         return(0);
      }

      if ( clist->id == thisc->id )
      {
         free(thisc);
         return(0);
      }

      clast = clist;
      clist = clist->next;
   }

   /* Now at the end */
   clast->next = thisc;
   return(0);
}

/* ========================================================================= */
/* Name: new_thread
 * Notes: See the new_[socket|core|thread] description above
 */
struct cpu_thread *new_thread(int id)
{
   struct cpu_thread *t;

   if ( NULL == ( t = (struct cpu_thread *)malloc(sizeof(struct cpu_thread)) ) )
      return(NULL);

   t->id = id;
   t->cs = NULL;
   t->next = NULL;

   return(t);
}

/* ========================================================================= */
/* Name: insert_thread
 * Notes: See the insert_[socket|core|thread] description above
 */
int insert_thread(struct cpu_socket *s, struct rawsyscpu *r)
{
   struct cpu_socket *slist;  /* for walking through list */
   struct cpu_socket *thiss;  /* The socket we belong on */
   struct cpu_core *clist;    /* For walking through the list */
   struct cpu_core *thisc;    /* The core we belong on */

   struct cpu_thread *thist;    /* The item we are working with */
   struct cpu_thread *tlist;
   struct cpu_thread *tlast;

   /* Find the right socket */
   slist = s;
   thiss = NULL;
   while ( slist )
   {
      if ( slist->id == r->physical_package_id )
         thiss = slist;

      slist = slist->next;
   }

   if ( thiss == NULL )
      return(1);

   /* Find the right core */
   clist = thiss->clist;
   thisc = NULL;
   while( clist )
   {
      if ( clist->id == r->core_id )
         thisc = clist;

      clist = clist->next;
   }

   if ( thisc == NULL )
      return(1);


   /* There are lots of implications of exiting here - leaving lots of 
      haning memory locations, but basically we gotta cut and run...
      If you can't malloc() then it is over. */
   if ( NULL == (thist = new_thread(r->logical_id)) )
      return(1);


   /*** ordered insert ***/

   /* Top of the list (no other items) */
   if ( thisc->tlist == NULL )
   {
      thisc->tlist = thist;
      return(0);
   }

   /* Top of the list (sorted first) */
   if ( thisc->tlist->id > thist->id )
   {
      thist->next = thisc->tlist;
      thisc->tlist = thist;
      return(0);
   }

   if ( thisc->tlist->id == thist->id )
   {
      /* SHOULD NEVER BE HERE! */
      free(thist);
      return(0);
   }

   /* into the middle */
   tlast = thisc->tlist;
   tlist = tlast->next;
   while ( tlist )
   {
      if ( tlist->id > thist->id )
      {
         thist->next = tlast->next;
         tlast->next = thist;
         return(0);
      }

      if ( tlist->id == thist->id )
      {
         /* SHOULD NEVER BE HERE! */
         free(thist);
         return(0);
      }

      tlast = tlist;
      tlist = tlist->next;
   }

   /* Now at the end */
   tlast->next = thist;
   return(0);
}

/* ========================================================================= */
/* Name: insert_cpustat
 * Description: Given a complete tree, associate the cpustat structures
 * Parameters: A complete tree of CPU information
 *             A list of cpuastats (from the same parent cpustats struct)
 * Returns: 0 on success, 1 on failure
 * Side effects: Makes all the links
 * Notes: Ideally... this could be called with a single cpustats struct
 *        so that the "same struct integrity" is enforced. This is not
 *        explicitly done because this is an internal function that has a
 *        very limited "call dependency". (It is only called by BuildCPUTree())
 *        So... As far as external API users are concerned, the link cannot
 *        be made across multiple parent structs.
 */
int insert_cpustat(struct cpu_socket *cpu_tree, struct cpustat *cs)
{
   struct cpu_socket *thiss;
   struct cpu_core *thisc;
   struct cpu_thread *thist;

   thiss = cpu_tree;
   while(thiss)
   {
      thisc = thiss->clist;
      while(thisc)
      {
         thist = thisc->tlist;
         while(thist)
         {
            if ( thist->id == cs->instance )
            {
               thist->cs = cs;
               return(0);
            }
            thist = thist->next;
         }
         thisc = thisc->next;
      }
      thiss = thiss->next;
   }

   return(1);
}

/* ========================================================================= */
/* Name: BuildCPUTree
 * Description: Given a cpustats struct, build and "link" the CPU tree. 
 * Notes: This is an externally referenced API - read the full description in
 *        cpustat.h.
 */
int BuildCPUTree(struct cpustats *cpus)
{
   struct rawsyscpu *rscl;
   struct rawsyscpu *thisrsc;
   struct cpu_socket *cpu_tree;
   struct cpustat *thiscpus;
   int i;

   /* Make sure we are dealing with a real struct here */
   if ( NULL == cpus )
      return(1);

   /* Read in a linked list of CPU info */
   if (NULL == (rscl = read_sys_cpus()))
   {
      return(1);
   }

   /* Build out the cpu_sockets */
   thisrsc = rscl;
   cpu_tree = NULL;
   while( thisrsc )
   {
      cpu_tree = insert_socket(cpu_tree, thisrsc);
      thisrsc = thisrsc->next;
   }

   /* Build out the cpu_cores */
   thisrsc = rscl;
   while( thisrsc )
   {
      if(insert_core(cpu_tree, thisrsc))
         return(1);

      thisrsc = thisrsc->next;
   }

   /* Build out the cpu_threads */
   thisrsc = rscl;
   while( thisrsc )
   {
      if(insert_thread(cpu_tree, thisrsc))
         return(1);

      thisrsc = thisrsc->next;
   }

   thiscpus = cpus->cpulist;
   i = 0;
   while ( i < cpus->cpucnt )
   {
      if(insert_cpustat(cpu_tree, &thiscpus[i]))
         return(1);

      i++;
   }

   /* Only insert the tree when complete -> 100% successful */
   cpus->cpu_tree = cpu_tree;

   return(0);
}

















































































#ifdef NOT_USED_ANY_MORE

/* ========================================================================= */
struct cpustats *ParseCPUStats(struct cpustats *cpus, void *statbuf)
{
   char line[256];
   struct bufhandle bh;
   int cpulncnt = 0;

   if ( cpus == NULL )
   {
      bh.buf = statbuf;
      bh.readptr = statbuf;

      while(get_cpu_line(line, 256, &bh))
      {
         cpulncnt++;
      }

      cpus = alloc_cpustats(cpulncnt);
   }

   /* Now start reading in data */
   bh.buf = statbuf;
   bh.readptr = statbuf;

   while(get_cpu_line(line, 256, &bh))
   {
      parse_cpu_line(cpus, line);
   }

   return(cpus);
}

/* ========================================================================= */
unsigned long GetCPUData(void *buf, unsigned long bufsz)
{
   int fd;
   void *junkbuf = NULL;
   size_t got;
   size_t totalgot;
   size_t rv;

   if ( -1 == (fd = open("/proc/stat", O_RDONLY)) )
   {
      fprintf(stderr, "ERROR: Unable to open CPU stat file.\n");
      return(0);
   }

   if ( NULL == buf )
   {
      /* The user wants to know the data size only */

      bufsz = 4096; /* Set to a page */

      if ( NULL == (junkbuf = malloc(bufsz)) )
      {
         fprintf(stderr, "ERROR: Unable to allocate a temp buffer.\n");
         return(0);
      }

      buf = junkbuf;
   }


   totalgot = 0;
   while( 0 != (got = read(fd, buf + totalgot, bufsz)) )
   {
      if ( got == -1 )
      {
         /* Could be valid. */
         fprintf(stderr, "ERROR: read() on CPU stat file failed.\n");
         return(0);
      }

      totalgot += got;

   }

   close(fd);

   if ( junkbuf )
   {
      if ( 0 == (rv = 4096 * ( totalgot / 4096 )) )
         rv = 4096;

      free(junkbuf);
   }
   else
      rv = totalgot;

   return(rv);
}
#endif
