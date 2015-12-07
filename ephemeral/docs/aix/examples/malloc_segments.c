
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define MEG_PER_MALLOC 256
#define BYTES_PER_MALLOC MEG_PER_MALLOC * 1024 * 1024

int main(int argc, char *argv[])
{
   int mcnt;
   char *data;
   char *dp;
   long i;

   if ( argc != 2 )
   {
      fprintf(stderr, "ERROR: Memory usage not defined. Pass a numeric to run.\n");
      return(1);
   }

   printf("LDR_CNTRL = %s\n", getenv("LDR_CNTRL"));

   mcnt = atoi(argv[1]);
   if ( mcnt > 0 )
      printf("malloc()ing %d %dM chunk%s\n", mcnt, MEG_PER_MALLOC, mcnt > 1 ? "s." : ".");
   else
      printf("Not malloc()ing any (additional) memory.\n");
   fflush(stdout);



   i = 0;
   while ( i < mcnt )
   {
      printf("  malloc(%dM) = ", MEG_PER_MALLOC);
      fflush(stdout);

      data = malloc(BYTES_PER_MALLOC);
      printf("0X%lX\n", (long)data);
      if ( data != NULL )
         printf("Touching...");
      fflush(stdout);

      if ( data != NULL )
      {
         dp = data;
         while ( dp < data + BYTES_PER_MALLOC )
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
         }

         printf("Done.\n");
         fflush(stdout);
      }

      i++;
   }

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
