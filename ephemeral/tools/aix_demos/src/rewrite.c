/* rewrite.c: Copy a file onto itself...
 * Test to see if this will deal with fragmented file business
 * Part of the test case for {DRQS 490960142}.
 *
 * To build:
 *      /bb/util/version12-032014/usr/vac/bin/xlc -q64 -qarch=pwr6 -qtune=pwr7 -qstrict -O3 -bstackpsize:64k -o rewrite rewrite.c
 *
 * Usage:    rewrite <filename>
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

int main(int argc, char **argv, char **envp)
{
	int fd;              /* File descriptor of the target frag file */
	off_t o;             /* File offset */
	size_t szr,szw,todo;
	char fbuf[4*1024*1024];
	struct timespec last, now;
	struct stat st_fd;

	if (argc<2 || !argv[1])
	{
		fprintf(stderr, "Usage: %s <filename>\n",
		 basename(argv[0]));
		return EXIT_FAILURE;
	}
	
	
	fd=open(argv[1], O_RDWR | O_LARGEFILE | O_DIRECT);
	if (fd<0)
	{
		fprintf(stderr, "open \"%s\" failed: %s, exit code %d\n",
		 argv[1], strerror(errno), errno);
		return EXIT_FAILURE;
	}
	fstat(fd,&st_fd);

	clock_gettime(CLOCK_REALTIME,&last);
	for (o=0; ; o+=szr)
	{
		szr=pread(fd,fbuf,sizeof(fbuf),o);
		if (szr < 0)
		{
			fprintf(stderr, "read on \"%s\" failed at offset %lu: %s, exit code %d\n",
			 argv[1], o, strerror(errno), errno);
			return EXIT_FAILURE;
		}
		else if (szr == 0)
			break;

		/* szr > 0 */
		for (todo=szr; todo; todo-=szw)
		{
			szw=pwrite(fd,fbuf+szr-todo,todo,o+szr-todo);
			if (szw<0)
			{
				fprintf(stderr, "write on \"%s\" failed at offset %lu: %s, exit code %d\n",
				 argv[1], o, strerror(errno), errno);
				return EXIT_FAILURE;
			}
			else if (szw==0)
			{
				fprintf(stderr, "zero-byte write on \"%s\" failed at offset %lu\n",
				 argv[1], o);
				return EXIT_FAILURE;
			}
		}
		clock_gettime(CLOCK_REALTIME,&now);
		if ((now.tv_sec - last.tv_sec)*1000000000UL + now.tv_nsec - last.tv_nsec > 250000000UL)
		{
			printf("Progress: %010lu bytes (%04.1lf%%)     \r", o, 100.0 * o / st_fd.st_size);
			fflush(stdout);
			last=now;
		}
	}
	printf("Progress: %010lu bytes (%05.1lf%%)\n", o, 100.0 * o / st_fd.st_size);
	printf("Syncing\n");
	fsync(fd);
	close(fd);
	printf("Done\n");
	return EXIT_SUCCESS;
}
