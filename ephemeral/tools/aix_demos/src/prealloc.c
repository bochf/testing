#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <memory.h>
#include <errno.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>


char *progname;

/*  Use an appropriate logging routine instead */
#ifndef die
# define die(...)  _die(__FILE__,__LINE__,"die",__VA_ARGS__)
#endif
void _die(const char *file,int line,const char *fxn,int e, const char *fmt, ...)
{
	va_list ap;
	fprintf(stderr,"%s: FATAL: ",progname);
	va_start(ap,fmt);
	vfprintf(stderr,fmt,ap);
	va_end(ap);
	fprintf(stderr,"\nStopped at %s:%d\n",file,line);
	fflush(stderr);
	exit(e);
}

int prealloc_aix(int fd, off_t preallocsz)
{
	char altbuf[1048576];
	int e;
	off_t fp, ftodo, fdone;
	/* Test that the file descriptor is writable */
	switch (e=posix_fallocate(fd, 0, preallocsz))
	{
		case 0:  /* Successful allocation */
		return EXIT_SUCCESS;

		case ENOTSUP:
		/* posix_fallocate not supported on this filesystem - switch to manual method */
		memset(altbuf,'\0',sizeof(altbuf));
		for (ftodo=preallocsz, fp=0; ftodo>0 ; ftodo-=fdone, fp+=fdone)
		{
			fdone=pwrite(fd, altbuf, ftodo>sizeof(altbuf)?sizeof(altbuf):ftodo, fp);
			if (fdone<0)
				die (EXIT_FAILURE, "Couldn't write %lu preallocation bytes to file descriptor %d: %s, error code %d\n", preallocsz, fd, strerror(errno), errno);
		}
		return EXIT_SUCCESS;
		

		default:
		die (EXIT_FAILURE, "Couldn't preallocate %lu bytes to file descriptor %d: %s, error code %d\n", preallocsz, fd, strerror(e), e);
	}
}

int proveit(int fd, off_t preallocsz)
{
	char altbuf[1048576];
	int e;
	off_t fp, ftodo, fdone;
	memset(altbuf,'\0',sizeof(altbuf));
	for (ftodo=preallocsz, fp=0; ftodo>0 ; ftodo-=fdone, fp+=fdone)
	{
		fdone=pwrite(fd, altbuf, ftodo>sizeof(altbuf)?sizeof(altbuf):ftodo, fp);
		if (fdone<0)
			die (EXIT_FAILURE, "proveit(): Couldn't write %lu bytes to file descriptor %d, offset %lu: %s, error code %d\n", preallocsz, fd, fp, strerror(errno), errno);
		fsync_range(fd,O_NOCACHE,fp,ftodo>sizeof(altbuf)?sizeof(altbuf):ftodo);
	}
	return EXIT_SUCCESS;
}

int main(int argc, char **argv, char **envp)
{
	int fd;
	off_t offset;
	char *strtoul_endp;
	progname=strdup(basename(argv[0]));
	if (!progname) progname=argv[0];

	if (argc!=3)
	{
		fprintf(stderr,"Usage:\n\t%s <filename> <offset>\n\n",progname);
		return EXIT_SUCCESS;
	}
	fd=open(argv[1],O_WRONLY|O_LARGEFILE|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
	if (fd<0)
		die (EXIT_FAILURE, "Couldn't open \"%s\" for writing: %s, error code %d\n", argv[1], strerror(errno), errno);

	offset=strtoul(argv[2],&strtoul_endp,10);
	if (!(offset< 1UL<< 44 && strtoul_endp && !*strtoul_endp)) /* invalid offset */
		die (EXIT_FAILURE,"Invalid file offset value \"%s\".  Range is 0 to %lu.\n",argv[2], 1UL<<44 );
	
	/* return prealloc_aix(fd, offset); */
	fprintf (stderr,"Starting pre-allocation\n");
	prealloc_aix(fd, offset);
	fprintf (stderr,"Pre-allocation done\n");
	fprintf (stderr,"Double checking allocation\n");
	proveit(fd, offset);
	
}
