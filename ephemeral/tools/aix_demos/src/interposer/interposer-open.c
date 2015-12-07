#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>


const char *___interposer_open_version="INTERPOSER: open() v0.1.\n";
static int (*___interposer_libc_open)(const char *, int, ...)=NULL;
static int (*___interposer_libc_open64)(const char *, int, ...)=NULL;

int open(const char *filename, int flags, ...)
{
	va_list ap;
	mode_t mode;
	unsigned short dio_attempt=0;
	struct stat s;
	int fd;

	const char ___bb_path[]="/bb/";
	const char ___interposer_open_msg0[]="INTERPOSER: Engaged open().\n";
	const char ___interposer_open_msg3[]="INTERPOSER: FAILED TO LOAD SYMBOL /usr/lib/libc.a::open()\n";
	/* write(2,___interposer_open_msg0,sizeof(___interposer_open_msg0)); */

	if (! ___interposer_libc_open)
	{
		if (!( ___interposer_libc_open = (int(*)(const char*,int,...))dlsym(RTLD_NEXT,"open"))) 
		{
			write(2,___interposer_open_msg3,sizeof(___interposer_open_msg3));
			_exit(EXIT_FAILURE);
		}
	}

	/* Where to attempt to apply DIO */
	if (
	 !strncmp(filename, ___bb_path, sizeof(___bb_path)-1)  /* Apply DIO on a files only inside /bb application areas */
	 &&  !strstr(filename, ".dta")                         /* Never to comdbg data  */
	 &&  !strstr(filename, ".ix")                          /* Never to comdbg index */
	 &&  !strstr(filename, ".mmap")                        /* Never to BAS mmap     */
	 &&  !strstr(filename, "act.log")                      /* Never to act.log      */
	 )
	{
		/* Include only regular files */
		if (lstat(filename, &s)==0 && S_ISREG(s.st_mode))
			dio_attempt = 1;
		else if ((flags & O_CREAT) && errno==ENOENT)
			dio_attempt = 1;
	}

	if (flags & O_CREAT)
	{
		va_start(ap, flags);
		mode=va_arg(ap,mode_t);
		va_end(ap);	
		if (dio_attempt)
		{
			fd = ___interposer_libc_open(filename, flags | O_DIRECT | O_NSHARE, mode);
			if (fd >= 0  ||  errno != ETXTBSY)
				return fd;
			/* else fall through */
		}
		return ___interposer_libc_open(filename,flags,mode);
	}
	else
	{
		if (dio_attempt)
		{
			fd = ___interposer_libc_open(filename, flags | O_DIRECT | O_NSHARE);
			if (fd >= 0  ||  errno != ETXTBSY)
				return fd;
			/* else fall through */
		}
		return ___interposer_libc_open(filename,flags);
	}
}

int open64(const char *filename, int flags, ...)
{
	va_list ap;
	mode_t mode;
	unsigned short dio_attempt=0;
	struct stat64 s;
	int fd;

	const char ___bb_path[]="/bb/";
	const char ___interposer_open_msg0[]="INTERPOSER: Engaged open().\n";
	const char ___interposer_open_msg3[]="INTERPOSER: FAILED TO LOAD SYMBOL /usr/lib/libc.a::open()\n";
	/* write(2,___interposer_open_msg0,sizeof(___interposer_open_msg0)); */

	if (! ___interposer_libc_open64)
	{
		if (!( ___interposer_libc_open64 = (int(*)(const char*,int,...))dlsym(RTLD_NEXT,"open64"))) 
		{
			write(2,___interposer_open_msg3,sizeof(___interposer_open_msg3));
			_exit(EXIT_FAILURE);
		}
	}

	/* Where to attempt to apply DIO */
	if (
	 !strncmp(filename, ___bb_path, sizeof(___bb_path)-1)  /* Apply DIO on a files only inside /bb application areas */
	 &&  !strstr(filename, ".dta")                         /* Never to comdbg data  */
	 &&  !strstr(filename, ".ix")                          /* Never to comdbg index */
	 &&  !strstr(filename, ".mmap")                        /* Never to BAS mmap     */
	 &&  !strstr(filename, "act.log")                      /* Never to act.log      */
	 )
	{
		/* Include only regular files */
		if (lstat64(filename, &s)==0 && S_ISREG(s.st_mode))
			dio_attempt = 1;
		else if ((flags & O_CREAT) && errno==ENOENT)
			dio_attempt = 1;
	}


	if (flags & O_CREAT)
	{
		va_start(ap, flags);
		mode=va_arg(ap,mode_t);
		va_end(ap);	
		if (dio_attempt)
		{
			fd = ___interposer_libc_open64(filename, flags | O_DIRECT | O_NSHARE , mode);
			if (fd >= 0  ||  errno != ETXTBSY)
				return fd;
			/* else fall through */
		}
		return ___interposer_libc_open64(filename,flags,mode);
	}
	else  /* ! O_CREAT */
	{
		if (dio_attempt)
		{
			fd = ___interposer_libc_open64(filename, flags | O_DIRECT | O_NSHARE );
			if (fd >= 0  ||  errno != ETXTBSY)
				return fd;
			/* else fall through */
		}
		return ___interposer_libc_open64(filename,flags);
	}
}
