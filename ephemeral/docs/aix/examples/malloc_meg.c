
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define BYTES_PER_MEG 1024 * 1024

int main(int argc, char *argv[])
{
   int mcnt;
   char *data;
   char *dp;
   char *nextdot;
   long i;
   long successm;
   long long pointer; /* Strictly to beat compiler warnings */

   if ( argc != 2 )
   {
      fprintf(stderr, "ERROR: Memory usage not defined. Pass a numeric to run.\n");
      return(1);
   }

   printf("LDR_CNTRL = %s\n", getenv("LDR_CNTRL"));

   mcnt = atoi(argv[1]);
   if ( mcnt > 0 )
      printf("malloc()ing %dM of memory.\n", mcnt);
   else
      printf("Not malloc()ing any (additional) memory.\n");
   fflush(stdout);

   i = 0;
   successm = 0;
   while ( i < mcnt )
   {
      printf("  malloc(%d) = ", BYTES_PER_MEG);
      fflush(stdout);

      data = malloc(BYTES_PER_MEG);
      pointer = (long)data; /* long == long long in 64 bit */
#ifdef __64BIT__
      printf("0X%016llX\n", pointer);
#else
      printf("0X%08llX\n", pointer);
#endif
      fflush(stdout);

      if ( data != NULL )
      {
         printf("  Touching");
         successm++;

         dp = data;
         nextdot = data + 65536;
         while ( dp < data + BYTES_PER_MEG )
         {
            dp[0] = 0;
            dp[1] = 1;
            dp[2] = 2;
            dp[3] = 3;
            dp[4] = 4;
            dp[5] = 5;
            dp[6] = 6;
            dp[7] = 7;
            dp = dp + 4096;

            if ( dp >= nextdot )
            {
               printf(".");
               fflush(stdout);
               nextdot += 65536;
            }
         }

         printf("Done.\n");
         fflush(stdout);
      }
      else
      {
         printf("  Failed on malloc().\n");
         /* Cheap way to exit the loop */
         i = mcnt;
      }

      i++;
   }

   printf("malloc()'d %ld Meg of (additional) memory.\n", successm);
   printf("=========================================\n");
   printf("Run: svmon -P %ld\n", (long)getpid());
   printf("Run: procmap -S %ld\n", (long)getpid());
   printf("=========================================\n");
   printf("Hit return to exit.\n");
   fflush(stdout);
   getchar();

   /* Leave main() */
   return(0);
}
