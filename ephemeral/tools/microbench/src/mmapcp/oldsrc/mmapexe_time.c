#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h> /* mmap() is defined in this header */
#include <fcntl.h>
#include <time.h>

#define CLOCK CLOCK_REALTIME
/* #define CLOCK CLOCK_REALTIME_COARSE */
/* #define CLOCK CLOCK_MONOTONIC */
/* #define CLOCK CLOCK_MONOTONIC_COARSE */
/* #define CLOCK CLOCK_PROCESS_CPUTIME_ID */
/* #define CLOCK CLOCK_THREAD_CPUTIME_ID */

#define NS_PER_SECOND 1000000000LL
#define TIMESPEC_TO_NS( aTime ) ( ( NS_PER_SECOND * ( ( long long int ) aTime.tv_sec ) ) + aTime.tv_nsec )

int err_quit(char *msg)
{
    printf(msg);
    return 0;
}


long long int gettime (void)
{

   struct timespec t;
   int rcode;
   if ((rcode= clock_gettime( CLOCK, &t )) <0) {
       printf("error getting time! (%i)\n", rcode);
       
   }

  return TIMESPEC_TO_NS( t );

}


long long int starttime(void)
{

  return gettime();

}


long long int elapsedtime(long long int startt, long long int *total)
{

  long long int endtime=gettime();

  long long int ldiff=endtime - startt;
  *total= *total + ldiff;
  return ldiff;
}

struct timespec lLastTime;
struct timespec lCurrentTime;


long long int  lstart,lend,lDifference, ltotal=0;

int main (int argc, char *argv[])
{
 int fdin, fdout;
 char *src, *dst;
 struct stat statbuf;
 int mode = 0x0777;

 if (argc != 3)
   err_quit ("usage: a.out <fromfile> <tofile>\n");

 /* open the input file */
 
  lstart=starttime();

  if ((fdin = open (argv[1], O_RDONLY)) < 0)
   {printf("can't open %s for reading\n", argv[1]);
    return 0;
   }
  lDifference=elapsedtime(lstart, &ltotal);

 printf( "open  %s in (usecs): %lld\n", argv[1], lDifference  / 1000  );

 /* open/create the output file */
lstart=starttime(); 
if ((fdout = open (argv[2], O_RDWR | O_CREAT | O_TRUNC, mode )) < 0)//edited here
   {printf ("can't create %s for writing\n", argv[2]);
    return 0;
   }

 lDifference=elapsedtime(lstart, &ltotal);
 
 printf( "open  %s in (usecs): %lld\n", argv[2], lDifference  / 1000  );
 
/* find size of input file */
 if (fstat (fdin,&statbuf) < 0)
   {printf ("fstat error\n");
    return 0;
   }

 /* go to the location corresponding to the last byte */
 if (lseek (fdout, statbuf.st_size - 1, SEEK_SET) == -1)
   {printf ("lseek error\n");
    return 0;
   }

 /* write a dummy byte at the last location */
 if (write (fdout, "", 1) != 1)
   {printf ("write error\n");
     return 0;
   }

 /* mmap the input file */
lstart=starttime();  
if ((src = mmap (0, statbuf.st_size, PROT_READ, MAP_SHARED, fdin, 0))
   == (caddr_t) -1)
   {printf ("mmap error for input");
    return 0;
   }
 lDifference=elapsedtime(lstart, &ltotal);

 printf( "mmap  %s in (usecs): %lld\n", argv[1], lDifference  / 1000  );


/* mmap the output file */
 lstart=starttime();

 if ((dst = mmap (0, statbuf.st_size, PROT_READ | PROT_WRITE,

   MAP_SHARED, fdout, 0)) == (caddr_t) -1)
   {printf ("mmap error for output\n");
    return 0;
   }
 lDifference=elapsedtime(lstart, &ltotal);
 printf( "mmap  %s in (usecs): %lld\n", argv[2], lDifference  / 1000  );



 /* this copies the input file to the output file */
lstart=starttime();

 memcpy (dst, src, statbuf.st_size);
// return 0;
 
 lDifference=elapsedtime(lstart, &ltotal);
 printf( "memcpy in (usecs): %lld\n", lDifference  / 1000  );

  
 printf( "total time  (usecs): %lld\n", ltotal  / 1000  );

 // sync();
  fsync(fdout);
return 0;
} /* main */

//end of code
