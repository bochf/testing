/* mkfragfile.c: Creates a file that will be heavily fragmented on JFS2.
 * Part of the test case for {DRQS 490960142}.
 *
 * To build:
 *      /bb/util/version12-032014/usr/vac/bin/xlc -q64 -qarch=pwr6 -qtune=pwr7 -qstrict -O3 -o mkfragfile mkfragfile.c
 *
 * Usage:    mkfragfile <filename> <size>
 * Expectation:  A frag file as large as 5GB will take 30-50 seconds to unlink,
 *               locking out JFS2 filesystem activity on the filesystem root inode
 *               and the parent directory inode.
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>

int sort_rand(void *a, void *b)
{
	register long r=random();
	//printf("r=%lu\n",r);
	if      (r >= 0xD0000000) return 1;
	else if (r <= 0x30000000) return -1;
	else		          return 0;
}

int main(int argc, char **argv, char **envp)
{
	int fd;              /* File descriptor of the target frag file */
	off_t fd_size;       /* Size of frag file to construct */
	off_t *blk, *hack;   /* Internal: 
	                      *   blk:  array of per-block offsets
	                      *   hack: array of per-odd-block offsets (for sorting) */
	unsigned long blocks;/* Count of 512B blocks for this file */
	char fill[512];      /* Fill data */
	char *strtoul_endp;  /* Working */
	unsigned long i;     /* Working */
	char iobuf[BUFSIZ];
	struct timespec last, now;

	setvbuf(stdout, iobuf, _IOFBF, sizeof(iobuf));
	if (argc<3 || !argv[1] || !argv[2])
	{
		fprintf(stderr, "Usage: %s <filename> <size>\n",
		 basename(argv[0]));
		return EXIT_FAILURE;
	}
	fd_size=strtoul(argv[2],&strtoul_endp,10);

	blocks=fd_size/512;
	blk=malloc(sizeof(off_t)*blocks);
	hack=malloc(sizeof(off_t)*blocks/2);
	if (!blk)
	{
		fprintf(stderr, "malloc(%lu) failed: %s, exit code %d\n",
		 sizeof(off_t)*blocks, strerror(errno), errno);
		return EXIT_FAILURE;
	}
	if (!hack)
	{
		fprintf(stderr, "malloc(%lu) failed: %s, exit code %d\n",
		 sizeof(off_t)*blocks, strerror(errno), errno);
		return EXIT_FAILURE;
	}

	/* Work on the odd numbered blocks */
	for (i=0; i < blocks / 2; i++)
		hack[i]=i*512UL*2; /*Assign the file offset*/

	/* Randomize the offsets of the odd numbered blocks */
	srandom(time(NULL));
	qsort (hack, blocks / 2, sizeof(off_t), sort_rand);

	/* Load the offsets for odds and placeholders for evens */
	for (i=0; i< blocks / 2; i++)
	{
		blk[i*2]=hack[i];
		blk[i*2+1]=0UL;
	}
	free(hack);

	fd=open(argv[1], O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU|S_IRWXG);
	if (fd<0)
	{
		fprintf(stderr, "open \"%s\" failed: %s, exit code %d\n",
		 argv[1], strerror(errno), errno);
		return EXIT_FAILURE;
	}

	memset(fill,sizeof(fill),0xff);
	clock_gettime(CLOCK_REALTIME,&last);
	for (i=0; i< blocks; i++)
	{
		clock_gettime(CLOCK_REALTIME,&now);
		if (blk[i] && (now.tv_sec - last.tv_sec)*1000000000UL + now.tv_nsec - last.tv_nsec > 250000000UL)
		{
			printf("Block %010lu @ Offset %010lu  (%04.1lf%%)     \r",i, blk[i], 100.0*i/blocks);
			fflush(stdout);
			last=now;
		}
		if (blk[i])
			pwrite(fd, fill, 512, blk[i]);
	}
	printf("Block %010lu @ Offset %010lu  (%05.1lf%%)     \n",i, blk[i], 100.0*i/blocks);
	return EXIT_SUCCESS;
}
