/* mmaprandom.c 
 * Test case for {DRQS 70699072}
 

Issue Abstract
An investigation was done in {fifw DRQS 70699072 <go>} into I/O blasts that 
occur regularly at certain times of day on BVAL AIX boxes.  

- Cause of the I/O blast
The root cause is that some BVAL apps are writing out files using mmap/shmat 
writes or highly random write patterns.  Specific root cause writers, files, and
the write characteristics have been identified in the DRQS.

The issue is not the volume of data written to the file.  Rather, it is the high
I/O operation rate.

On AIX, mmap/shmat modified pages get flushed in small 4kB page quanta, and so 
there are a lot of I/O operations generated when a large file is heavily 
modified.

- Detail
When committing modifications to a shmat/mmap file, the page-out interaction is
only favorable for the I/O subsystem if the pages can be coalesced into large
swquential chunks.  This issue presents a case where the coalesce quality is poor.
Furthermore, if the application doesn't coordinate, the flush happens en masse
during syncd runs.   Translates to tens/hundreds of thousands of I/O Operations
per second.

The problem with the "en masse" scenario is seen at the driver / VMM layer.
There is a mandatory portion of individual I/O operation handling that executes
in the interrupt context, which in these quantities, outcompetes all application
threads on all CPUs.

- Detection
The blasts are most easily detected by PowerPath latency alarms that are posted 
to the system logs.

- Effect of the I/O blast
High-priority tkrm threads are getting delayed by significant I/O blasts, 
causing latency & risking data loss.  

The desired outcome would be to make the writing more architecturally friendly 
for the UNIX flavors - and specifically AIX, to avoid the bursts, and the 
disruption to tkrm.  For example, this might include building the target file 
image in heap memory and then writing it out in sequential chunks, rather than 
mmap/shmat write.   


 * Test generator
 * 1. Generate the map file (by posix_fallocate, or zeroing, or both)
 * 2. Optionally flush the cache
 * 3. Map the file
 * 4. Optionally warm the cache by (a) read the mapped space sequentially, or  (b) write sequentially
 * 5. Random write the entire mapped space
 * 6. Control the commit with (a) syncvfs, (b) msync in chunks, or (c) fsync_range in chunks
 

Usage: mmaprandom [-h?AZRQMF] [-h|--help] [-?|--usage]
  [-f|--file=STRING] [-s|--size=LONG]
  [-S|--seq-file=STRING]
  [-A|--fallocate] [-Z|--zeros] [-R|--random-allocate]
  [-Q|--flush]
  [--no-warmup] [--warmup-sequential-read] [--warmup-sequential-write]
  [-K|--chunk-size=LONG] [-M|--msync] [-F|--fsync]


Example:
	mmaprandom --file /bb/pm/foo --size 33554432 --seq-file=/bb/pm/foo.seq
	 --fallocate --zeros --flush --warmup-sequential-read --fsync


Build with:
    /bb/util/version12-102014/usr/vac/bin/xlc -q64 -o mmaprandom -g -qarch=pwr6 -qtune=pwr7
	  -I/home/eschulma/sw/include mmaprandom.c -L/home/eschulma/sw/lib -lpopt -liconv


AUTHOR
   Eugene Schulman <eschulman6@bloomberg.net>
   Bloomberg LP, R&D Performance Engineering (G325)
   120 Park Ave, New York, NY 10017


REFERENCES
   {DRQS 70699072 <go>}
   {DRQS 72400030 <go>}
   {DRQS 72400858 <go>}


SEE ALSO
   http://www-01.ibm.com/support/knowledgecenter/ssw_aix_71/com.ibm.aix.base/kc_welcome_71.htm?lang=en
	fsync_range(2), mmap(2), msync(2), posix_fallocate(1), read(2), shmat(2), syncd(8), write(2)

 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <inttypes.h>
#include <procinfo.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/syncvfs.h>
#include <sys/systemcfg.h>
#include <sys/time.h>
#include <sys/vminfo.h>
#include <sys/trcmacros.h>
#include <sys/trchkid.h>
#include <popt.h>
#define ZERO_BLKSZ ((size_t)4194304)

char *progname;
pid_t mypid;


struct options
{
	int help;
	int usage;
	char *fn, *fn_seq;
	off_t sz;
	int fallocate;
	int zeros;
	int randomalloc;
	int flush;
	int pi_seq;
	size_t chunksize;
	int msync;
	int fsync;
};

int get_my_procentry( struct procentry64 *pe)
{
	int e;
	pid_t proc_idx = mypid;
	e = getprocs64( pe, sizeof(*pe), NULL, NULL, &proc_idx, 1);
	if (e == 1)
		return e;
	else if (e < 0)
	{
		fprintf(stderr, "%s: FATAL: getprocs64 error: %s, error code %d\n",
		 progname, strerror(errno), errno);
		exit( EXIT_FAILURE ); 
	}
	else if (e==0)
	{
		fprintf(stderr, "%s: FATAL: getprocs64 error: Couldn't find my own PID??\n");
		exit( EXIT_FAILURE ); 
	}
	else if (e > 1)
	{
		fprintf(stderr, "%s: FATAL: getprocs64 error: Too many PIDs returned ??\n");
		exit( EXIT_FAILURE ); 
	}
	else
	{
		fprintf(stderr, "%s: FATAL: getprocs64_wrap should never get to %s:%d\n",
		 progname, __FILE__, __LINE__);
		exit( EXIT_FAILURE );
	}
}

int opts_process( struct options *opts, int opts_argc, char **opts_argv )
{
	struct poptOption option_tab[]=
	{
		{ "help", 'h', POPT_ARG_NONE, &(opts->help), 0, "Help", NULL},
		{ "usage", '?', POPT_ARG_NONE, &(opts->usage), 0, "Usage", NULL},
		{ "file", 'f', POPT_ARG_STRING, &(opts->fn), 0, "File name", NULL},
		{ "size", 's', POPT_ARG_LONG, &(opts->sz), 0, "File size", NULL},
		{ "seq-file", 'S', POPT_ARG_STRING, &(opts->fn_seq), 0, "Sequence file", NULL},
		{ "fallocate", 'A', POPT_ARG_NONE, &(opts->fallocate), 0, "Use posix_fallocate", NULL},
		{ "zeros", 'Z', POPT_ARG_NONE, &(opts->zeros), 0, "Write zeros to file", NULL},
		{ "random-allocate", 'R', POPT_ARG_NONE, &(opts->randomalloc), 1, "Random pre-allocation", NULL},
		{ "flush", 'Q', POPT_ARG_NONE, &(opts->flush), 0, "Flush cache", NULL },
		{ "no-warmup", '', POPT_ARG_VAL, &(opts->pi_seq), 0, "Do not warm up the cache", NULL },
		{ "warmup-sequential-read", '', POPT_ARG_VAL, &(opts->pi_seq), 1, "Warm up cache by reading mapping sequentially", NULL },
		{ "warmup-sequential-write", '', POPT_ARG_VAL, &(opts->pi_seq), 2, "Warm up cache by writing mapping sequentially", NULL },
		{ "chunk-size", 'K', POPT_ARG_LONG, &(opts->chunksize), 0, "Chunk size", NULL },
		{ "msync", 'M', POPT_ARG_NONE, &(opts->msync), 0, "msync in chunks", NULL },
		{ "fsync", 'F', POPT_ARG_NONE, &(opts->fsync), 0, "fsync in chunks", NULL },
		POPT_TABLEEND
	};

	poptContext cx;
	int rc;
	const char *fmt1     = "  - %s\n";
	const char *fmt2_str = "  - %-30s \"%s\"\n";
	const char *fmt2_ull = "  - %-30s %" PRIu64 "\n";

	/* Clear all options */
	memset(opts, '\0', sizeof(*opts));
	/* Prepare to parse options */
	cx = poptGetContext( NULL, opts_argc, (const char**)opts_argv, option_tab, 0 );
	/* Parse options */
	while ((rc = poptGetNextOpt( cx )) != -1)
	{
		switch (rc)
		{
			case POPT_ERROR_NOARG:
			case POPT_ERROR_BADOPT:
			case POPT_ERROR_BADQUOTE:
			case POPT_ERROR_BADNUMBER:
			case POPT_ERROR_OVERFLOW:
			fprintf(stderr, "%s: FATAL: Argument %s, %s\n", progname, poptBadOption(cx, 0), poptStrerror(rc));
			exit( EXIT_FAILURE );

			case POPT_ERROR_ERRNO:
			fprintf(stderr, "%s: FATAL: Argument %s, %s, %s, errno=%d\n", progname, poptBadOption(cx, 0), poptStrerror(rc), strerror(errno), errno);
			exit( EXIT_FAILURE );
			
			default:
			break;
			/* All of the other options should fall through */
		}
	}

	if (opts->help)
	{
		poptPrintUsage( cx, stderr, 0 );
		exit( EXIT_SUCCESS );
	}

	poptFreeContext( cx );

	if (! opts->fn || ! *(opts->fn))
	{
		fprintf(stderr,"%s: Filename required.  (--file parameter.)\n", progname);
		exit(EXIT_FAILURE);
	}
	if (! opts->fn_seq || ! *(opts->fn_seq))
	{
		char fnbuf[PATH_MAX];
		snprintf(fnbuf, sizeof(fnbuf), "%s.seq", opts->fn);
		opts->fn_seq = (char*) strdup(fnbuf);
		if (!strcmp(opts->fn, opts->fn_seq))
		{
			fprintf(stderr,"%s: FATAL: Could not construct unique sequence file name.\n", progname);
			exit(EXIT_FAILURE);
		}
	}

	if (! opts->fallocate && ! opts->zeros && ! opts->randomalloc)
	{
		fprintf(stderr,"%s: At least one of --fallocate or --zeros or --random-allocate required.\n", progname);
		exit(EXIT_FAILURE);
	}
	else if (opts->fallocate && opts->randomalloc)
	{
		fprintf(stderr,"%s: --fallocate and --random-allocate are incompatible\n", progname);
		exit(EXIT_FAILURE);
	}

	if (opts->msync && opts->fsync)
	{
		fprintf(stderr,"%s: --msync & --fsync are not compatible.\n", progname);
		exit(EXIT_FAILURE);
	}

	printf("Action Plan:\n");
	printf("\n == File locations and sizes ==\n");
	if (opts->fn)
		printf(fmt2_str, "Filename:", opts->fn);
	if (opts->sz)
		printf(fmt2_ull, "File Size:", (uint64_t)opts->sz);
	else
	{
		opts->sz = 768 * 1024 * 1024;
		printf(fmt2_ull, "File Size, default:", (uint64_t)opts->sz);
	}
	if (opts->fn_seq)
		printf(fmt2_str, "Sequence filename:", opts->fn_seq);

	printf("\n == Allocation Phase ==\n");
	if (opts->fallocate)
		printf(fmt1, "Allocate map file with posix_fallocate()");
	if (opts->zeros)
		printf(fmt1, "Initialize map file with zeros");
	if (opts->randomalloc)
		printf(fmt1, "Initialize map file random contents, random block order");

	printf("\n == Cache cooling & warmup options ==\n");
	if (opts->flush)
		printf(fmt1, "Flush cache after init");
	if (opts->pi_seq == 0)
		printf(fmt1, "Random page-in");
	else if (opts->pi_seq == 1)
		printf(fmt1, "Sequential read page-in");
	else if (opts->pi_seq == 2)
		printf(fmt1, "Sequential write page-in");

	printf("\n == Random Write Phase  (no options) ==\n");

	printf("\n == Commit Phase ==\n");
	if (opts->msync || opts->fsync)
	{
		if (opts->chunksize)
			printf(fmt2_ull, "Commit chunk size", (uint64_t)opts->chunksize);
		else
		{
			opts->chunksize = 128 * 1024;
			printf(fmt2_ull, "Commit chunk size, default:", (uint64_t)opts->chunksize);
		}
	}	
	if (opts->msync)
		printf(fmt1, "msync() commit method");
	else if (opts->fsync)
		printf(fmt1, "fsync() commit method");
	else
		printf(fmt1, "syncvfs() commit method (default)");

	printf("\n");
}

void fd_fallocate( char *fn, int fd, off_t sz )
{
	int e;
	if ((e = posix_fallocate(fd, 0, sz)))
	{
		switch (e)
		{
			case 0:
			return;

			case ENOTSUP:
			fprintf(stderr, "%s: warning: posix_fallocate() failed against \"%s\": %s, error code %d.  Continuing...\n",
			 progname, fn, strerror(e), e);
			break;

			default:
			fprintf(stderr, "%s: FATAL: posix_fallocate() failed against \"%s\": %s, error code %d.\n",
			 progname, fn, strerror(e), e);
			exit( EXIT_FAILURE );
		}
	}
}

void fd_zero( char *fn, int fd, off_t sz )
{
	void *z;
	off_t of;
	ssize_t written;
	size_t target_write_sz;
	if ( ! (z = calloc( 1, ZERO_BLKSZ )))
	{
		fprintf(stderr, "%s: FATAL: Could not allocate %" PRIu64 " bytes for buffer: %s, error code %d\n",
		 progname, (uint64_t)ZERO_BLKSZ, strerror(errno), errno);
		exit( EXIT_FAILURE ); 
	}
	for (of=0  ;  of < sz  ;  of += written)
	{
		target_write_sz  =  (sz-of > ZERO_BLKSZ) ? ZERO_BLKSZ : sz-of;
		if ((written = write( fd, z, target_write_sz )) < 0)
		{
			fprintf(stderr, "%s: FATAL: Write %" PRIu64 " bytes to \"%s\" (FD %d) failed at offset %" PRIu64 ": %s, error code %d\n",
			 progname, (uint64_t)target_write_sz, fn, fd, (uint64_t)of, strerror(errno), errno);
			exit( EXIT_FAILURE ); 
		}
	}
	free(z);
}

void fd_randomalloc( char *fn, int fd, off_t sz, off_t *seq_list, size_t pgsz, char *pattern_buf, size_t pattern_buf_sz)
{
	uint64_t pages = (uint64_t) ((uint64_t)sz / (uint64_t)pgsz);
	uint64_t i;
	ssize_t written;
	size_t target_write_sz;

	if (pgsz <= pattern_buf_sz)
		target_write_sz = pgsz;
	else
	{
		fprintf(stderr, "%s: FATAL: Page size %" PRIu64 " is larger than pattern buffer size %" PRIu64 "!  Fix the code!\n",
		 progname, pgsz, pattern_buf_sz);
		exit( EXIT_FAILURE );
	}

	for (i=0  ;  i < pages  ;  i++ )
	{
		if ((written = pwrite( fd, pattern_buf, target_write_sz, (off_t)(seq_list[i] * pgsz) )) < 0)
		{
			fprintf(stderr, "%s: FATAL: Write %" PRIu64 " bytes to \"%s\" (FD %d) failed at offset %" PRIu64 ": %s, error code %d\n",
			 progname, (uint64_t)target_write_sz, fn, fd, (uint64_t)seq_list[i], strerror(errno), errno);
			exit( EXIT_FAILURE ); 
		}
	}
}

void noop_read( unsigned char *x, volatile unsigned char *y )
 { *y = *x; }

void noop_write( volatile unsigned char *x, volatile unsigned char *y )
 { *x = *y; } 


int gen_seq ( char *fn_seq, off_t **seq_list_p, off_t sz, size_t pgsz)
{
	int fd;
	off_t i,j,swap;
	uint64_t pages = (uint64_t) ((uint64_t)sz / (uint64_t)pgsz);
	off_t *seq_list;
	ssize_t e;
	struct stat s;

	if (fn_seq)
	{
		fd = open( fn_seq, O_RDONLY | O_LARGEFILE);
		if (fd < 0  && errno != ENOENT)
		{
			fprintf(stderr, "%s: FATAL: Could not open \"%s\" for reading: %s, error code %d\n",
			 progname, fn_seq, strerror(errno), errno);
			exit( EXIT_FAILURE ); 
		}
		else if (fd >= 0)
		{
			if (fstat(fd, &s) < 0)
			{
				fprintf(stderr, "%s: FATAL: Could not fstat \"%s\": %s, error code %d\n",
				 progname, progname, strerror(errno), errno);
				exit( EXIT_FAILURE ); 
			}
			if (s.st_size != sizeof(off_t) * pages)
			{
				printf("Progress:  (!) Random sequence file is stale, will rebuild.\n");
				fd=-1;
			}
		}
	}

	if (fn_seq && fd >= 0)
	{
		printf("Progress:  Open map sequence file \"%s\"\n", fn_seq);
		if ((seq_list = shmat( fd, NULL, SHM_MAP | SHM_RDONLY )) >= 0)
		{
			printf("Progress:  Sequence file mapped\n");
		}
		else
		{
			fprintf(stderr, "%s: FATAL: Could not map \"%s\": %s, error code %d\n",
			 progname, fn_seq, strerror(errno), errno);
			exit( EXIT_FAILURE ); 
		}
	}
	else  /* No filename or no existing sequence file */
	{
		if ((fd = open( fn_seq, O_RDWR | O_CREAT | O_TRUNC | O_LARGEFILE, S_IRUSR | S_IWUSR)) >= 0)
			printf("Progress:  Create new map sequence file \"%s\"\n", fn_seq);
		else
		{
			fprintf(stderr, "%s: FATAL: Could not open \"%s\" for writing: %s, error code %d\n",
			 progname, fn_seq, strerror(errno), errno);
			exit( EXIT_FAILURE ); 
		}
		
		if ( ! (seq_list = (off_t*) malloc(pages * sizeof(off_t))))
		{
			fprintf(stderr, "%s: FATAL: Could not allocate %" PRIu64 " bytes for buffer: %s, error code %d\n",
			 progname, (uint64_t)(pages * sizeof(off_t)), strerror(errno), errno);
			exit( EXIT_FAILURE ); 
		}
		/* Create linear sequence */
		for (i=0; i < pages; i++)
			seq_list[i] = i;

		/* Randomize the list */
		srandom(time(NULL));
		for (i=pages-1 ; i > 0 ; --i)
		{
			j = random() % (i+1);
			/* Swap */
			swap = seq_list[i];
			seq_list[i] = seq_list[j];
			seq_list[j] = swap;
		}
		if (( e = write( fd, seq_list, (size_t)(pages * sizeof(off_t)) )) < 0)
		{
			fprintf(stderr, "%s: FATAL: Write %" PRIu64 " bytes to \"%s\": %s, error code %d\n",
			 progname, (uint64_t)(pages*sizeof(off_t)), fn_seq, strerror(errno), errno);
			exit( EXIT_FAILURE ); 
		}
	}
	/* Apparently, must leave the fd open to maintain a shmat mapping */
	// close(fd);
	*seq_list_p = seq_list;
	/* There is no error checking done on the mappings in either case */
	return 0;
}

int ntimersub_wrap( struct timespec *t2, struct timespec *t1, struct timespec *t_diff)
{
	ntimersub( *t2, *t1, *t_diff );
	return 0;
}

int main (int argc, char **argv, char **envp)
{
	struct options o;
	struct options *opts = &o;
	int fd;
	void *map;
	off_t *seq_list;
	unsigned char *p;
	volatile unsigned char _discard;
	size_t pgsz;
	// struct vm_page_info vpi;
	struct procentry64 myproc[2];
	int e;
	uint64_t i, pages, target_page, chunks;
	struct timespec t[2], t_diff;
	unsigned char pattern_buf[4096];
	int sync_method_confirmed;

	#define TSTAMP1  (clock_gettime(CLOCK_REALTIME, &(t[0])))

	#define TSTAMP2  (clock_gettime(CLOCK_REALTIME, &(t[1])))
 
	#define TDIFF_NS ( ntimersub_wrap( &t[1], &t[0], &t_diff ),  \
	 (t_diff.tv_sec * 1000000000 + t_diff.tv_nsec) )

	#define TDIFF_US ( ntimersub_wrap( &t[1], &t[0], &t_diff ),  \
	 ((t_diff.tv_sec * 1000000000 + t_diff.tv_nsec)/1000) )

	
	mypid = getpid();
	progname = (char*)strdup((char*)basename(argv[0]));
	opts_process(opts, argc, argv );


	srandom(time(NULL));
	for (i=0 ; i < sizeof(pattern_buf) ; i++ )
		pattern_buf[i] = (unsigned char)(random() & 0xff);
	printf("Progress:  Pattern buffer built\n");

	/* The order in which pages are to be written is prescribed by 
	 * seq_list.  The seq_list is a complete array of pages numbers in
	 * the mapping.  HOWEVER, the ordering of the pages has been randomized.
	 *
	 * It would be better to derive the exact page size with vmgetinfo, but 
	 * that's not available until the file is mapped.  So continue with 
	 * the correct assumption that mappings back to filecache are using
	 * standard 4kB pages.
	 */
	pgsz = sysconf(_SC_PAGESIZE);
	gen_seq( opts->fn_seq, &seq_list, opts->sz, pgsz );
	printf("Progress:  Random Sequence ready\n");
	

	TSTAMP1;
	if ((fd = open( opts->fn, O_RDWR | O_CREAT | O_TRUNC | O_LARGEFILE, S_IRUSR | S_IWUSR )) >= 0)
	{
		TSTAMP2;
		printf("Progress:  Map \"%s\" opened  (%" PRIu64 "us)\n", opts->fn, (uint64_t) TDIFF_US);
	}
	else
	{
		fprintf(stderr, "%s: FATAL: Could not open \"%s\" for read/write: %s, error code %d\n",
		 progname, opts->fn, strerror(errno), errno);
		exit( EXIT_FAILURE ); 
	}

	if (opts->sz && opts->fallocate)
	{
		TSTAMP1;
		fd_fallocate( opts->fn, fd, opts->sz );
		TSTAMP2;
		printf("Progress:  Map file allocated via fallocate (%" PRIu64 "us)\n", (uint64_t) TDIFF_US);
	}

	if (opts->sz && opts->zeros)
	{
		TSTAMP1;
		fd_zero( opts->fn, fd, opts->sz );
		TSTAMP2;
		printf("Progress:  Map file zeroed out  (%" PRIu64 "us)\n", (uint64_t) TDIFF_US);
	}

	if (opts->sz && opts->randomalloc)
	{
		TSTAMP1;
		fd_randomalloc( opts->fn, fd, opts->sz, seq_list, pgsz, pattern_buf, sizeof(pattern_buf) );
		TSTAMP2;
		printf("Progress:  Map file initialized, randomly written (%" PRIu64 "us)\n", (uint64_t) TDIFF_US);
	}

	sync_method_confirmed=0;
	if (opts->msync)
	{
		if (msync( map, pgsz, MS_SYNC ) == 0)
			sync_method_confirmed=1;
		else
		{
			printf("Progress:  (!) Test of msync bombed on the mapped pages, switching to fsync.\n");
			opts->msync=0;
			opts->fsync=1;
		}
	}
	if (opts->fsync)
	{
		if (fsync_range( fd, O_DSYNC, 0, pgsz ) == 0)
			sync_method_confirmed=1;
		else
		{
			printf("Progress:  (!) Test of fsync bombed on the mapped pages, defaulting to syncvfs.\n");
			opts->fsync=0;
		}
	}

	if (opts->flush)
	{
		TSTAMP1;
		fsync_range( fd, O_NOCACHE, 0, opts->sz );
		TSTAMP2;
		printf("Progress:  fsync_range - clear cache  (%" PRIu64 "us)\n", (uint64_t) TDIFF_US);
	}

	TSTAMP1;
	if ((map = shmat( fd, NULL, SHM_MAP )) >= 0)
	{
		TSTAMP2;
		printf("Progress:  File mapped  (%" PRIu64 "us)\n", (uint64_t) TDIFF_US);
	}
	else
	{
		fprintf(stderr, "%s: FATAL: Could not map \"%s\": %s, error code %d\n",
		 progname, opts->fn, strerror(errno), errno);
		exit( EXIT_FAILURE ); 
	}
	
	/* The actual page size, only available after the mapping is made. */
	/*
	memset( &vpi, '\0', sizeof(vpi) );
	vpi.addr = (uint64_t)map;
	if (vmgetinfo( &vpi, VM_PAGE_INFO, sizeof(vpi) ) < 0)
	{
		fprintf(stderr, "%s: FATAL: Could not get page size of \"%s\" mapping: %s, error code %d\n",
		 progname, opts->fn, strerror(errno), errno);
		exit( EXIT_FAILURE ); 
	}
	pgsz = vpi.pagesize;
	*/


	if (opts->pi_seq)
	{
		get_my_procentry( &(myproc[0]) );
		TSTAMP1;
		for (p=(unsigned char*)map;  p < (unsigned char *)map + opts->sz ; p += pgsz)
		{
			if (opts->pi_seq == 1)
			{
				TRCHKL2T( HKWD_USER8 | 0x1, (unsigned long)((void*)p-map), (unsigned long)p );
				noop_read ( p, &_discard );
			}
			else if (opts->pi_seq == 2)
			{
				TRCHKL2T( HKWD_USER8 | 0x2, (unsigned long)((void*)p-map), (unsigned long)p );
				noop_write ( p, &_discard );
			}
		}
		TSTAMP2;
		get_my_procentry( &(myproc[1]) );

		printf("Progress:  File paged-in sequentially for %s,  "
		"%" PRIu64 " minflt, "
		"%" PRIu64 " majflt  "
		 "(%" PRIu64 "us)\n",
		 opts->pi_seq == 1 ? "read" : "write",
		 myproc[1].pi_minflt - myproc[0].pi_minflt,
		 myproc[1].pi_majflt - myproc[0].pi_majflt,
		 (uint64_t) TDIFF_US);
	}



	/* Now, make a lot of changes!
	 * The order in which pages are to be written is prescribed by 
	 * seq_list.
	 */

	printf("Progress:  About to apply LOTS of changes to the mapping.\n");
	pages = (uint64_t) ((uint64_t)opts->sz / (uint64_t)pgsz);

	get_my_procentry( &(myproc[0]));
	TSTAMP1;
	for (i=0 ; i < pages ; i++)
	{
		if (i%1000 == 0)
			printf("            working... [Page %" PRIu64 "]  (%4.1lf%%)\n", i, (double)(100ULL*(i+1)/pages));
		target_page = seq_list[i];
		//fprintf(stderr, "DEBUG: i=%" PRIu64 " target_page=%" PRIu64 "\n", i, (uint64_t)target_page);
		TRCHKL2T( HKWD_USER8 | 0x3, (unsigned long)(map + pgsz*target_page), sizeof(pattern_buf) );
		//memcpy( map + pgsz*target_page,  pattern_buf, sizeof(pattern_buf));
		//memcpy( map + pgsz*target_page,  pattern_buf, 16);
		*(uint64_t*)(map + pgsz*target_page) = *(uint64_t*)pattern_buf;
	}
	TSTAMP2;
	get_my_procentry( &(myproc[1]));

	printf("Progress:  Applied LOTS of changes to the mapping,  "
	 "%" PRIu64 " minflt, "
	 "%" PRIu64 " majflt  "
	 "(%" PRIu64 "us)\n",
	 myproc[1].pi_minflt - myproc[0].pi_minflt,
	 myproc[1].pi_majflt - myproc[0].pi_majflt,
	 (uint64_t) TDIFF_US);
	

	printf("Progress:  sync in 5 seconds !\n");
	sleep(5);


	chunks = (uint64_t) ((uint64_t)opts->sz / (uint64_t)opts->chunksize);
	if (opts->msync)
	{
		for (i=0 ; i < chunks ; i++)
		{
			if (i%1000 == 0 || i==(chunks-1))
				printf("Progress:  msyncing chunk %" PRIu64 " of %" PRIu64". (%4.1lf%%)\n", i+1, chunks, (double)(100ULL*(i+1)/chunks));
			msync(map + (i * opts->chunksize), opts->chunksize, MS_SYNC);
		}
	}
	else if (opts->fsync)
	{
		for (i=0 ; i < chunks ; i++)
		{
			if (i%1000 == 0 || i==(chunks-1))
				printf("Progress:  fsyncing chunk %" PRIu64" of %" PRIu64". (%4.1lf%%)\n", i+1, chunks, (double)(100ULL*(i+1)/chunks));
			fsync_range(fd, O_DSYNC, (off_t)(i * opts->chunksize), opts->chunksize);
		}
	}
	else
	{
		printf("Progress:  syncvfs.  Good luck!\n");
		syncvfs( "jfs2", FS_SYNCVFS_FSTYPE );
	}
	close( fd );
}
