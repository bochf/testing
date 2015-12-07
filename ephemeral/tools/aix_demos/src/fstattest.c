/* fstattest.c:  Test case for {DRQS 54536119 <go>} and {DRQS 53163888 <go>}
 * Complaint was that fstat did not faithfully return the size of a file after
 * a long run, on both Solaris and AIX.  Long runs of this program on AIX seem to disprove it...
 *
 * The program creates 3 PIDs:  a stat module, an fstat module and a monitoring/update module.
 * (The monitoring module is the parent PID.)
 * The 3 are linked by a shared memory segment containing the tracking structures.
 * 
 * While the fstat and stat modules are polling the watched files each second, by file descriptor 
 * and by file name, respectively,  the update module looks for discrepencies between them.
 * Periodically, the update tool will post small updates to the watched files
 * on a per-file interval (in seconds) specified on the command-line.
 *
 * To build:
 *	/bb/util/version12-032014/usr/vac/bin/xlc -qarch=pwr6 -qtune=pwr7 -qdfp -qstrict -o fstattest fstattest.c -lpthread -lrts
 *
 *
 * Usage: 
 *      fstattest file1:update_interval1 file2:update_interval2 ...
 *
 * fstattest will alert on its own.  If used in conjunction with AIX trace, fstattest will stop
 * the trace if it detects any discrepency.
 *
 * Author:  Eugene Schulman <ESchulman6@bloomberg.net>, R&D Systems Performance (G325)
 */

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <libgen.h>
#include <stdarg.h>
#include <inttypes.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#define _Q(x) #x
#define QUOTE(x) _Q(x)
#define LONGIO 1000000000

struct stat_with_timestamp
{
	pthread_mutex_t m;
	int ready;
	struct timespec t;
	struct stat s;
};

typedef struct tracker
{
	int active;
	int fd_r;
	int fd_w;
	char *fn;
	struct timespec interval, last_update;
	struct stat_with_timestamp s_stat;
	struct stat_with_timestamp s_fstat;
} track_t;


char *progname;
int shmid=0;
void *shmbuf=NULL;
pid_t p_me=0;               /* Which PID I am */
pid_t p_monitor=0;          /* PID #1, The parent, runs the a_monitor method */
pid_t p_stat=0;             /* PID #2, A child, runs the a_stat method */
pid_t p_fstat=0;            /* PID #3, A child, runs the a_fstat method */



void warn_t(struct timespec * t,const char *fmt, ...)
{
	va_list ap;
	char tmbuf[32];
	strftime(tmbuf,sizeof(tmbuf),"%H:%M:%S",localtime(&(t->tv_sec)));
	fprintf(stderr,"%s.%03u %s: ",tmbuf, t->tv_nsec/1000000L, progname);
	va_start(ap,fmt);
	vfprintf(stderr,fmt,ap);
	va_end(ap);
	fflush(stderr);
}


/* DIE: Exit with a string and an exit code */
/* Usage:  die(EXIT_SUCCESS,"test %d",2); */
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

void a_stat(track_t *track, unsigned int tracked, struct timespec *delay)
{
	int i, e;
	struct timespec before,after;
	/* Get rid of the read & write file descriptors */
	clock_gettime(CLOCK_REALTIME,&before);
	warn_t(&before,"STARTING A_STAT\n");

	for (i=0; i<tracked; i++)
	{
		close(track[i].fd_w);
		close(track[i].fd_r);
	}

	for ( ; ; nanosleep(delay, NULL))
	{
		for (i=0; i<tracked; i++)
		{
			pthread_mutex_lock(&(track[i].s_stat.m));
			__lwsync();
			track[i].s_stat.ready=0;
/*	
clock_gettime(CLOCK_REALTIME,&before);
warn_t(&before,"POLL STAT\n");
*/
			clock_gettime(CLOCK_REALTIME,&before);
			e=stat(track[i].fn, &(track[i].s_stat.s));
			clock_gettime(CLOCK_REALTIME,&after);
			track[i].s_stat.t=after;
	
			if (e>=0)
				track[i].s_stat.ready=1;
			else
				warn_t(&after, "stat(\"%s\",..) failed: %s, error code %d\n", track[i].fn, strerror(errno), errno);

			if (track[i].s_stat.ready && ((after.tv_sec - before.tv_sec)*1000000000L + after.tv_nsec - before.tv_nsec > LONGIO))
				warn_t(&after,"stat(\"%s\",..) long I/O > " QUOTE(LONGIO) "nsec\n", &after, track[i].fn);
			
			__lwsync();
			pthread_mutex_unlock(&(track[i].s_stat.m));
			/*track[i].s_stat.ready=1;*/
			__lwsync();
		}
	}
}


void a_fstat(track_t *track, unsigned int tracked, struct timespec *delay)
{
	int i, e;
	struct timespec before,after;
	/* Get rid of the write file descriptors */
	clock_gettime(CLOCK_REALTIME,&before);
	warn_t(&before,"STARTING A_FSTAT\n");

	for (i=0; i<tracked; i++)
		close(track[i].fd_w);

	for ( ; ; nanosleep(delay, NULL))
	{
		for (i=0; i<tracked; i++)
		{
			pthread_mutex_lock(&(track[i].s_fstat.m));
			__lwsync();
			track[i].s_fstat.ready=0;
/*
clock_gettime(CLOCK_REALTIME,&before);
warn_t(&before,"POLL FSTAT\n");
*/
			clock_gettime(CLOCK_REALTIME,&before);
			e=fstat(track[i].fd_r, &(track[i].s_fstat.s));
			clock_gettime(CLOCK_REALTIME,&after);
			track[i].s_fstat.t=after;
	
			if (e>=0)
				track[i].s_fstat.ready=1;
			else
				warn_t(&after, "fstat(%d,..) failed for \"%s\": %s, error code %d\n", track[i].fd_r, track[i].fn, strerror(errno), errno);

			if (track[i].s_fstat.ready && ((after.tv_sec - before.tv_sec)*1000000000L + after.tv_nsec - before.tv_nsec > LONGIO))
				warn_t(&after,"fstat(%d,..) for \"%s\" long I/O > " QUOTE(LONGIO) "nsec\n", track[i].fd_r, track[i].fn);
			__lwsync();

			pthread_mutex_unlock(&(track[i].s_fstat.m));
			__lwsync();
		}	
	}
}

void a_monitor(track_t *track, unsigned int tracked, struct timespec *delay)
{
	struct timespec now;
	char x='.';
	int i;

	/* Allow the first cycle of stats to complete */
	nanosleep(delay, NULL);
	nanosleep(delay, NULL);

clock_gettime(CLOCK_REALTIME,&now);
warn_t(&now,"STARTING A_MONITOR\n");

	/* Get rid of the read file descriptors */
	for (i=0; i<tracked; i++)
		close(track[i].fd_r);

	warn_t(&now, "TRACE ON\n");
	trcon();

	for ( ; ; nanosleep(delay, NULL))
	{
		clock_gettime(CLOCK_REALTIME,&now);
/*
warn_t(&now,"POLL A_MONITOR\n");
*/
		for (i=0; i<tracked; i++)
		{
			/* Look for discrepencies */
			pthread_mutex_lock(&(track[i].s_stat.m));
			pthread_mutex_lock(&(track[i].s_fstat.m));
			if (track[i].active && track[i].s_stat.ready && track[i].s_fstat.ready)
			{
				if (track[i].s_stat.s.st_dev != track[i].s_fstat.s.st_dev)
				{
					trcoff();
					warn_t(&now, "stat & fstat dev different for \"%s\" %lu vs. %lu\n", track[i].fn, track[i].s_stat.s.st_dev, track[i].s_fstat.s.st_dev);
					warn_t(&now, "TRACE OFF\n");
				}
				if (track[i].s_stat.s.st_ino != track[i].s_fstat.s.st_ino)
				{
					trcoff();
					warn_t(&now, "stat & fstat inode different for \"%s\" %" PRIu64 " vs. %" PRIu64 "\n", track[i].fn, track[i].s_stat.s.st_ino, track[i].s_fstat.s.st_ino);
					warn_t(&now, "TRACE OFF\n");
				}
				if ((track[i].s_stat.s.st_size > track[i].s_fstat.s.st_size + 1) || (track[i].s_fstat.s.st_size > track[i].s_stat.s.st_size + 1))
				{
					trcoff();
					warn_t(&now, "stat & fstat size different for \"%s\" %" PRIu64 " vs. %" PRIu64 "\n", track[i].fn, track[i].s_stat.s.st_size, track[i].s_fstat.s.st_size);
					warn_t(&now, "TRACE OFF\n");
				}
				if ((track[i].s_stat.t.tv_sec > track[i].s_fstat.t.tv_sec + 2*delay->tv_sec) || (track[i].s_fstat.t.tv_sec > track[i].s_stat.t.tv_sec + 2*delay->tv_sec)) 
				{
					trcoff();
					warn_t(&now, "stat & fstat times differ by more than 2 intervals for \"%s\", %lu vs. %lu\n", track[i].fn, track[i].s_stat.t.tv_sec, track[i].s_fstat.t.tv_sec);
					warn_t(&now, "TRACE OFF\n");
				}
			}
			pthread_mutex_unlock(&(track[i].s_fstat.m));
			pthread_mutex_unlock(&(track[i].s_stat.m));
			
			/* Now do updates... */
			if (track[i].last_update.tv_sec==0 && track[i].last_update.tv_nsec==0)
			{
				track[i].last_update=now;
				/*warn_t(&now, "\"%s\" init\n", track[i].fn);*/
			}
			else if (((now.tv_sec - track[i].last_update.tv_sec)*1000000000LL + now.tv_nsec - track[i].last_update.tv_nsec) > (track[i].interval.tv_sec*1000000000LL + track[i].interval.tv_nsec))  /* Time for update */
			{
/*warn_t(&now, "\"%s\" About to write\n", track[i].fn);*/
				if (write(track[i].fd_w,&x,sizeof(x))>0)
				{
					/*warn_t(&now, "Wrote 1 to fd=%d for \"%s\"\n", track[i].fd_w, track[i].fn);*/
					track[i].last_update=now;
				}
				else
					warn_t(&now, "Could not write 1 to fd=%d for \"%s\": %s, error code %d\n", track[i].fd_w, track[i].fn, strerror(errno), errno);
			}
		}
	}
}


void cleanup_monitor(void)
{
	if (p_stat)   kill (SIGKILL, p_stat);
	if (p_fstat)  kill (SIGKILL, p_fstat);
	_exit(EXIT_FAILURE);
}

pid_t launch_pid(pid_t *p, void (*fxn)(track_t *, unsigned int, struct timespec*), track_t *track, unsigned int tracked, struct timespec *delay)
{
	if (p_me==p_monitor)
	{
		fflush(stdout);
		fflush(stderr);
		*p=fork();
		if (*p<0)
			die(EXIT_FAILURE,"%s: fork failed: %s, error code %d\n",progname,strerror(errno),errno);
		else if (*p==0)
		{
			p_me = *p = getpid();
			fxn(track, tracked, delay);  /* Child launches routine;  should not return */
		}
	}
}

int main(int argc, char **argv, char **envp)
{
	unsigned int tracked;
	struct timespec t_delay = { 1, 0 }; /* 1 sec */
	track_t *track;
	char *tok;
	char iobuf1[BUFSIZ];
	char iobuf2[BUFSIZ];
	pthread_mutexattr_t mutex_attr;
	
	int i, e;

	progname=basename(argv[0]);

	if (argc<2)
	{
		printf("Usage:  %s  <file1:interval1>  <file2:interval2> ...\n");
		exit(EXIT_SUCCESS);
	}
	
	setvbuf(stdout,iobuf1,_IOLBF,sizeof(iobuf1));
	setvbuf(stderr,iobuf2,_IOLBF,sizeof(iobuf2));
	atexit(&cleanup_monitor);
	/* Create shared memory space between parent (stat test) and child (fstat test) */
	if ((shmid=shmget(IPC_PRIVATE, sizeof(track_t)*argc, IPC_CREAT|IPC_EXCL|SHM_PIN|S_IRUSR|S_IWUSR))<0)
		die(EXIT_FAILURE,"%s: shmget failed: %s, error code %d\n",progname,strerror(errno),errno);
	if ((shmbuf=shmat(shmid, NULL, 0))==(void*)(-1))
		die(EXIT_FAILURE,"%s: shmat failed: %s, error code %d\n",progname,strerror(errno),errno);
	track=(track_t *)shmbuf;

	/* Initialize mutex attribute */
	pthread_attr_init(&mutex_attr);
	pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);

	/* Lazy argument parsing */
	/* Point right back to argv */
	tracked=argc-1;
	for (i=0; i<tracked; i++)
	{
		double dval;
		char *strtod_endp;
		track[i].active=0;
		track[i].fn = strtok(argv[i+1],":");  /* 1st word is the filepath */
		tok=strtok(NULL,":");                 /* 2nd word is the interval (seconds) */
		if (tok)
		{
			dval=strtod(tok,&strtod_endp);
			if (dval>0 && strtod_endp && !*strtod_endp) /* Valid positive double */
			 { } /* Great */
			else /* Invalid double */
				die(EXIT_FAILURE,"%s: Bad command line interval \"%s\"\n",tok);
		}
		else dval=1.0;
		track[i].interval.tv_sec =(uint64_t)dval;
		fprintf(stderr,"YO!\n");
		track[i].interval.tv_nsec=(uint64_t)(dval - track[i].interval.tv_sec*1000000000.0);
		fprintf(stderr,"BLOW!\n");

		if ((track[i].fd_w = open(track[i].fn,  O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR))<0)
			die(EXIT_FAILURE,"%s: Cannot open \"%s\" for writing\n",track[i].fn);

		if ((track[i].fd_r = open(track[i].fn,  O_RDONLY))<0)
			die(EXIT_FAILURE,"%s: Cannot open \"%s\" for reading\n",track[i].fn);

		track[i].last_update.tv_sec=0;
		track[i].last_update.tv_nsec=0;
		track[i].active=1;
		pthread_mutex_init(&(track[i].s_stat.m), &mutex_attr);
		pthread_mutex_init(&(track[i].s_fstat.m),&mutex_attr);

		printf("\"%s\", interval %" PRIu64 ".%03" PRIu64 ", fd_r=%d, fd_w=%d\n",
		 track[i].fn, (uint64_t)track[i].interval.tv_sec, (uint64_t)(track[i].interval.tv_nsec/1000000LL), track[i].fd_r, track[i].fd_w);
	}
	fprintf(stderr,"Tracked=%d\n",tracked);

	
	p_me=p_monitor=getpid();
	launch_pid(&p_stat,  &a_stat, track, tracked, &t_delay);  /* Create PID #2 - the a_stat engine */
	launch_pid(&p_fstat, &a_fstat, track, tracked, &t_delay); /* Create PID #3 - the a_fstat engine */
	shmctl(shmid,IPC_RMID,shmbuf);
	a_monitor(track, tracked, &t_delay);                      /* PID #1 - Parent - the monitoring engine */
}
