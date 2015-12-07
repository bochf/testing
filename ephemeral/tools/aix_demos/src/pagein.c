/* @(#) Demo program to force page-ins on demand across the memory space
 * {DRQS 53708560 <go>}
 * 
 * Author:  Eugene Schulman,  R&D Systems Performance (G325)
 *
 * Demo is compiled as follows:
 *    32-bit
 *  	/bb/util/version12-032014/usr/vac/bin/xlc -qarch=pwr6 -qtune=pwr7 -o pagein pagein.c
 *    64-bit
 *    /bb/util/version12-032014/usr/vac/bin/xlc -q64 -qarch=pwr6 -qtune=pwr7 -o pagein pagein.c
 *    
 * 
 * Note that paging-in the entire memory space takes time!  The test timings are approx 7us / page.
 * For a 3.25GB 32-bit address space, consisting of only 4kB pages, this could take the calling thread
 * approx 6 seconds to execute.
 *
 * Author:  Eugene Schulman <ESchulman6@bloomberg.net>, R&D Systems Performance (G325)
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <inttypes.h>
#include <procinfo.h>
#include <sys/types.h>
#include <sys/vminfo.h>
#include <sys/mman.h>
#include <sys/shm.h>
#ifndef __LDINFO_PTRACE64__
# define __LDINFO_PTRACE64__ 1
#endif
#include <sys/ldr.h>

#define PAGE_TOUCH_RDONLY 0
#define PAGE_TOUCH_RDWR   1
#define LOADQUERY_BUFFER 16384


extern _text;
extern _etext;
extern _data;
extern _edata;


typedef struct my_region
{
	void *addr;
	size_t size;
} my_region_t;


/* Reference all of the pages of a range */
/* This is the action packed part of the program */
unsigned long long pg_ref(void *p_start, void *p_end, size_t pg_size, int pg_opt_write)
{
	register char x;
	register char *p_seek;
	register unsigned long long pg_cnt=0;
	struct vm_page_info vinfo;
	static size_t pg_size_default;
	
	/* If not supplied, derive a fixed page size based on the first page in the range using VM_PAGE_INFO.
	 * It's possible to have mixed page sizes within a segment.
	 * The code is not handling this case yet.
	 * A more conservative derivation would be to use VMINFO_GETPSIZES and select the smallest size.
	 */
	if (!pg_size)
	{
		vinfo.addr=(uint64_t)p_start;
		if (vmgetinfo(&vinfo, VM_PAGE_INFO, sizeof(vinfo))==0)
		{
			pg_size=vinfo.pagesize;
		}
		else
		{
			/* Failsafe method - system's "fixed" pagesize value */
			/* Warning message? */
	 		fprintf(stderr,"vmgetinfo returned %s, error code %d.  Defaulting to sysconf.\n",strerror(errno),errno);
			if (!pg_size_default) pg_size_default=sysconf(_SC_PAGE_SIZE);
			pg_size=pg_size_default;
		}
	}	
	for (p_seek=p_start;  p_seek < p_end;  p_seek+=pg_size, ++pg_cnt)
	{
	   /* Read.  Force page-in; trigger copy-on-ref after forks (depending on vmm_fork_policy tuning) */	
		x=*p_seek;  
		/* Write.  Force page-in; trigger copy-on-write after forks; and for mmap memory, take page control */
		if (pg_opt_write) *p_seek=x;
	}
	return pg_cnt;
}

int stats_checkpoint_initial(struct procentry64 *b)
{
	pid_t p_idx=getpid();
	int e;
	if ((e=getprocs64(b, sizeof(*b), NULL, 0, &p_idx, 1)) >= 0)
		{} /* Great */
	else
 		fprintf(stderr,"getprocs64 returned %s, error code %d.  Continuing\n",strerror(errno),errno);
	return e;
}

int stats_checkpoint(pid_t p_idx, uint64_t pg_cnt, struct procentry64 *a, struct procentry64 *b)
{
	int e;
	if ((e=getprocs64(b, sizeof(*b), NULL, 0, &p_idx, 1)) >= 0)
		printf("Pages scanned=%" PRIu64 ",  Minor page faults=%" PRIu64 ",  Major page faults=%" PRIu64 "\n",
		 pg_cnt, b->pi_minflt - a->pi_minflt,  b->pi_majflt - a->pi_majflt);
	else
 		fprintf(stderr,"getprocs64 returned %s, error code %d.  Continuing\n",strerror(errno),errno);
	return e;
}

int pgin_all(my_region_t *Caller_Supplied_Regions,  int lock_in_memory)
{
	pid_t mypid=getpid();
	/* For data (heap) section */
	void *my_edata;
	/* For Loader / Shared lib section */
	struct ld_xinfo *p_ld_xinfo;
	char *ar;
	char ldbuf[LOADQUERY_BUFFER];
	/* For stats */
	uint64_t pg_cnt;
	struct procentry64 p[6];

	/* Initial stats checkpoint */
	stats_checkpoint_initial(&p[0]);

 	/*****************************
	 * FAST PAGE-IN DIRECTIVE    *
 	 *****************************
	 */
	 if (lock_in_memory)
	 {
	 	/* Shortcut ! */
	 	if (mlockall(MCL_CURRENT|MCL_FUTURE) >= 0)
		{
			/* Great.  mlockall forces the page-ins across the address space.  Page-outs will be resisted.  OK for a small memory size.
			 * For a large memory size relative to the host system, don't lock... use lock_in_memory=0. */
			printf ("MLOCKALL\n");
			stats_checkpoint(mypid, pg_cnt, &p[0], &p[1]);
			return 0;
		}
		else
	 		fprintf(stderr,"Page locking in memory was requested.  mlockall returned %s, error code %d.  Continuing...\n",strerror(errno),errno);
			/* Fall through into the soft handler */
	}
 
 	/*****************************
	 * MANUAL PAGE-IN ALGORITHM  *
 	 *****************************
	 */

	/* Text region */
	/* The Loader section will also cover text */
	printf ("TEXT: pg_ref(_text=0x%p, _etext=0x%p, %d, %d)\n", &_text, &_etext, 0, PAGE_TOUCH_RDONLY);
	pg_cnt=pg_ref(&_text, &_etext, 0, PAGE_TOUCH_RDONLY);

	stats_checkpoint(mypid, pg_cnt, &p[0], &p[1]);



	/* Data region */
	my_edata=sbrk(0);
	printf ("DATA: pg_ref(_data=0x%p, _edata=0x%p, %d, %d)\n", &_data, my_edata, 0, PAGE_TOUCH_RDONLY);
	pg_ref(&_data, my_edata, 0, PAGE_TOUCH_RDONLY);           /* Multi-threaded: read-only- not thread-safe  */
	/*pg_cnt=pg_ref(&_data, my_edata, 0, PAGE_TOUCH_RDWR);*/  /* Single-threaded: read-write */

	stats_checkpoint(mypid, pg_cnt, &p[1], &p[2]);



	/* Loader / Shared Library text & data regions */
	if ((loadquery(L_GETXINFO, (void*)ldbuf, sizeof(ldbuf)))<0)
	{
		if (errno==ENOMEM)
	 		fprintf(stderr,"loadquery returned %s, error code %d.  Increase the static buffers or recode to make it dynamic.\n",strerror(errno),errno);
		else
	 		fprintf(stderr,"loadquery returned %s, error code %d.\n",strerror(errno),errno);
		return EXIT_FAILURE;
	}

	for (p_ld_xinfo=(struct ld_xinfo*) ldbuf;  p_ld_xinfo;  p_ld_xinfo = (p_ld_xinfo->ldinfo_next ? (void*)p_ld_xinfo + p_ld_xinfo->ldinfo_next : NULL))
	{
		ar = (char*)p_ld_xinfo + p_ld_xinfo->ldinfo_filename;
		printf ("%s:  file=\"%s\"  member=\"%s\"  text=0x%015" PRIX64 "  text_len=0x%016" PRIX64 ",  data=0x%015" PRIX64 "  data_len=0x%016" PRIX64 ",  tdata=0x%015" PRIX64 "  tdata_len=0x%016" PRIX64 ",  tbss_len=0x%016" PRIX64 "\n",
		 "LOAD+SHLIB",
		 ar,
		 ar + strlen(ar) + 1,
		 (ptr64_t)p_ld_xinfo->ldinfo_textorg,
		 (uint64_t)p_ld_xinfo->ldinfo_textsize,
		 (ptr64_t)p_ld_xinfo->ldinfo_dataorg,
		 (uint64_t)p_ld_xinfo->ldinfo_datasize,
		 (ptr64_t)p_ld_xinfo->ldinfo_tdataorg,
		 (uint64_t)p_ld_xinfo->ldinfo_tdatasize,
		 (uint64_t)p_ld_xinfo->ldinfo_tbsssize);
		printf ("text\n");
		pg_cnt=pg_ref((void*)(p_ld_xinfo->ldinfo_textorg), (void*)(p_ld_xinfo->ldinfo_textorg) + p_ld_xinfo->ldinfo_textsize, 0, PAGE_TOUCH_RDONLY);
		stats_checkpoint(mypid, pg_cnt, &p[2], &p[3]);

		printf ("data\n");
		pg_cnt=pg_ref((void*)(p_ld_xinfo->ldinfo_dataorg), (void*)(p_ld_xinfo->ldinfo_dataorg) + p_ld_xinfo->ldinfo_datasize, 0, PAGE_TOUCH_RDONLY);
		stats_checkpoint(mypid, pg_cnt, &p[3], &p[4]);
		
		memcpy(&p[2],&p[4],sizeof(p[2])); /* Reinit p[2] for next loop */
	}
	


	/* Stack */
	/* Skipping this - low quantity / low value */
	;



	/* Caller Supplied Regions
	 * Includes shared memory, i.e. shmget/shmat and mmap
	 * There is no C API method to derive shared memory mappings using
	 * public interfaces, up to the time of writing, July 2014 / AIX 7.1 TL3.
	 * svmon -P <pid> & procmap -S <pid>  derive this using the 
	 * private, undocumented kernel performance extensions, specifically 
    * getvsidsandprocl_pid() and ptx_getsegstat().
	 * Until IBM supplies a public API or guidance on the private interfaces
	 * these cannot be derived.   No consideration is given to calling out
	 * externally to svmon, because this call is way too slow.
	 *
	 * In the meantime, the caller can identify the shared segments to map.
	 */
	 if (Caller_Supplied_Regions)
	 {
	 	int i;
	 	for (i=0; Caller_Supplied_Regions[i].addr; i++)
		{
			printf("CALLER SUPPLIED REGION #%d:  0x%p - 0x%p\n", i, (void*)(Caller_Supplied_Regions[i].addr), (void*)(Caller_Supplied_Regions[i].addr) + Caller_Supplied_Regions[i].size);
			pg_cnt=pg_ref((void*)(Caller_Supplied_Regions[i].addr), (void*)(Caller_Supplied_Regions[i].addr) + Caller_Supplied_Regions[i].size, 0, PAGE_TOUCH_RDONLY);

			stats_checkpoint(mypid, pg_cnt, &p[4], &p[5]);
			memcpy(&p[4],&p[5],sizeof(p[4])); /* Reinit p[4] for next loop */
		}
	 }
	 
}

int main(int argc, char **argv, char **envp)
{
	/* THIS SHARED REGION SETUP IS FOR DEMO PURPOSES ONLY.  YOUR IMPLEMENTATION SHOULD SIZE THE MY_REGION_T ARRAY AS APPROPRIATE (WITH ADDR/SIZE), AND WITH ONE NULL ENTRY TO TERMINATE */
	int shm1id;
	void *shm1addr;
	my_region_t shmregions[2]={ {NULL,0}, {NULL,0} };
	int lock_in_memory=1;
	if ((shm1id=shmget(IPC_PRIVATE, 65536, IPC_CREAT|IPC_EXCL|S_IRUSR)) >= 0)  /* SHM Key */
	{
		if ((shm1addr=shmat(shm1id, NULL, SHM_RDONLY)) != (void*)(-1))         /* SHM Addr */
		{
			shmctl(shm1id, IPC_RMID, shm1addr);                                 /* SHM clean up... pended */
			shmregions[0].addr=shm1addr;
			shmregions[0].size=65536;
		}		
	}
	/* END OF SHARED REGION SETUP FOR DEMO */

	/* HEAP REGION SETUP FOR DEMO PURPOSES */
	malloc(1048576);
	/* END OF HEAP REGION SETUP FOR DEMO PURPOSES */

	/* pgin_all coordinates the calls to pg_ref */
	pgin_all(shmregions, lock_in_memory);  /* Covers text, data, shared libs, and the regions supplied */
}
