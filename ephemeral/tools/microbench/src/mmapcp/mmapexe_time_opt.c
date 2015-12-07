/* this is an enanchment test used  for redhat 
reclaim test

to compile with library

 gcc -O2 -o mmapexe mmapexe_time_opt.c ../lib/libmb.a -lrt

*/

/* This is required for clock_gettime() warning/declaration on Solaris */
#ifdef PORT_SunOS
#define __EXTENSIONS__
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h> /* mmap() is defined in this header */
#include <fcntl.h>
#include <time.h>

#include "../lib/optparser.h"
#include "../lib/verbosity.h"

/* MAX_VALUE_SIZE is defined in optparser.h. It is the largest value allowed.*/
#define MAXL ( MAX_VALUE_SIZE + 1 )

#define CLOCK CLOCK_REALTIME
/* #define CLOCK CLOCK_REALTIME_COARSE */
/* #define CLOCK CLOCK_MONOTONIC */
/* #define CLOCK CLOCK_MONOTONIC_COARSE */
/* #define CLOCK CLOCK_PROCESS_CPUTIME_ID */
/* #define CLOCK CLOCK_THREAD_CPUTIME_ID */

#define NS_PER_SECOND 1000000000LL
#define TIMESPEC_TO_NS( aTime ) ( ( NS_PER_SECOND * ( ( long long int ) aTime.tv_sec ) ) + aTime.tv_nsec )

/* ========================================================================= */
int err_quit(char *msg)
{
    printf(msg);
    return 0;
}

/* ========================================================================= */
long long int gettime (void)
{
   struct timespec t;
   int rcode;
   if ((rcode = clock_gettime( CLOCK, &t )) <0)
   {
      printf("error getting time! (%i)\n", rcode); 
   }

   return TIMESPEC_TO_NS( t );

}

/* ========================================================================= */
long long int starttime(void)
{
  return gettime();
}

/* ========================================================================= */
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

/* ========================================================================= */
int main (int argc, char *argv[])
{
   int fdin, fdout;
   char *src, *dst;
   struct stat statbuf;
   int mode = 0x0777;

   char *filein=malloc(MAXL*sizeof(char)); 
   char *fileout=malloc(MAXL*sizeof(char));

   struct optbase *ob;
 
   /*
     if (argc != 3)
     err_quit ("usage: a.out <fromfile> <tofile>\n");
   */

   /* start acquire list*/
   //commenting out until casting available)

   if(NULL == (ob = ReadOptions(argc, argv)))
      return(1);

#define NEW_WAY
#ifdef NEW_WAY
   {
      /* Scope minl and maxl local to the single place they are used. */

      int minl = 1;  /* The minimum string length for a file name. */
      int maxl = MAXL; /* The maximum string length for a file name. */

      GetOptionValue(ob, "FILEIN", GOV_STRING, filein, &minl, &maxl, NULL);
      GetOptionValue(ob, "FILEOUT", GOV_STRING, fileout, &minl, &maxl, NULL);
      EvalOptions(ob);
   }
#else
   if ( IsInvalidOption(ob, "FILEIN", IVO_NONE) )
   {
      ErrorMessage("ERROR: FILEIN option is missing or invalid.\n");
      return(1);
   }

  
   if ( IsInvalidOption(ob, "FILEOUT", IVO_NONE) )
   {
      ErrorMessage("ERROR: FILEOUT option is missing or invalid.\n");
      return(1);
   }


   if ( GetStrValue(ob, filein, "FILEIN", 1, 60) )
   {
      ErrorMessage("ERROR: Unable to parse FILEIN name.\n");
      return(1);
   }


   if ( GetStrValue(ob, fileout, "FILEOUT", 1, 60) )
   {
      ErrorMessage("ERROR: Unable to parse FILEOUT name.\n");
      return(1);
   }
#endif

#ifdef NEW_WAY
   VerboseMessage( "file in to copy %s \n", filein);
   VerboseMessage( "file in output  %s \n", fileout);
#else
   printf( "file in to copy %s \n", filein);
   printf( "file in output  %s \n", fileout);
#endif

   /* open the input file */
   
   lstart=starttime();
   
   if ((fdin = open (filein, O_RDONLY)) < 0)
   {
#ifdef NEW_WAY
      ErrorMessage("ERROR: Can't open %s for reading\n", filein);
#else
      printf("can't open %s for reading\n", filein);
#endif
      return 0;
   }

   lDifference=elapsedtime(lstart, &ltotal);

   printf( "open  %s in (usecs): %lld\n", filein, lDifference  / 1000  );


   /* open/create the output file */
   lstart=starttime(); 
   if ((fdout = open (fileout, O_RDWR | O_CREAT | O_TRUNC, mode )) < 0)//edited here
   {
      printf ("can't create %s for writing\n", fileout);
      return 0;
   }

   lDifference=elapsedtime(lstart, &ltotal);
 
   printf( "open  %s in (usecs): %lld\n", fileout, lDifference  / 1000  );

   /* find size of input file */
   if (fstat (fdin,&statbuf) < 0)
   {
      printf ("fstat error\n");
      return 0;
   }

   /* go to the location corresponding to the last byte */
   if (lseek (fdout, statbuf.st_size - 1, SEEK_SET) == -1)
   {
      printf ("lseek error\n");
      return 0;
   }

   /* write a dummy byte at the last location */
   if (write (fdout, "", 1) != 1)
   {
      printf ("write error\n");
      return 0;
   }

   /* mmap the input file */
   lstart=starttime();  
   if ((src = mmap (0, statbuf.st_size, PROT_READ, MAP_SHARED, fdin, 0))
       == (caddr_t) -1)
   {
      printf ("mmap error for input");
      return 0;
   }
   
   lDifference=elapsedtime(lstart, &ltotal);

   printf( "mmap  %s in (usecs): %lld\n", filein, lDifference  / 1000  );


   /* mmap the output file */
   lstart=starttime();

   if ((dst = mmap (0, statbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fdout, 0)) == (caddr_t) -1)
   {
      printf ("mmap error for output\n");
      return 0;
   }

   lDifference=elapsedtime(lstart, &ltotal);
   printf( "mmap  %s in (usecs): %lld\n", fileout, lDifference  / 1000  );

   /* this copies the input file to the output file */
   lstart=starttime();

   memcpy (dst, src, statbuf.st_size);
   // return 0;
 
   lDifference=elapsedtime(lstart, &ltotal);
   printf( "memcpy in (usecs): %lld\n", lDifference  / 1000  );

  
   // printf( "total time  (usecs): %lld\n", ltotal  / 1000  );

   // sync();

   lstart=starttime();

   fsync(fdout);
   lDifference=elapsedtime(lstart, &ltotal);
   printf( "sync in (usecs): %lld\n", lDifference  / 1000  );

   printf( "total time  (usecs): %lld\n", ltotal  / 1000  );

   return 0;
} /* main */

//end of code
