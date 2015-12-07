#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
   int bd;
   int bdp;
   char *page;
   int i;
   ssize_t wrote;
   ssize_t bd_wrote;
   ssize_t bdp_wrote;

   unlink("BIG_DATA");
   unlink("BIG_DATA_plus4k");

   if ( -1 == (bd = (open("BIG_DATA", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH))))
   {
      /* Catch errors and exit */
      fprintf(stderr, "ERROR: Failed to open \"BIG_DATA\".\n");
      return(1);
   }

   if ( -1 == (bdp = (open("BIG_DATA_plus4k", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH))))
   {
      /* Catch errors and exit */
      fprintf(stderr, "ERROR: Failed to open \"BIG_DATA_plus4k\".\n");
      return(1);
   }

   if ( NULL == (page = (char *)malloc(4096)) )
   {
      fprintf(stderr, "ERROR: Unable to allocate memory.\n");
      return(1);
   }

   i = 0;
   while(i < 4096)
   {
      page[i] = 'A' + (i % 26);
      i++;
   }

   bd_wrote = 0;
   bdp_wrote = 0;

   printf("Writing"); fflush(stdout);
   while ( bd_wrote < 256 * 1024 * 1024 )
   {
      wrote = write(bd, page, 4096);
      if ( wrote > 0 )
         bd_wrote = bd_wrote + wrote;

      wrote = write(bdp, page, 4096);
      if ( wrote > 0 )
         bdp_wrote = bdp_wrote + wrote;

      printf("."); fflush(stdout);
   }

   wrote = write(bdp, page, 4096);
   if ( wrote > 0 )
      bdp_wrote = bdp_wrote + wrote;

   printf("Done.\n"); fflush(stdout);

   printf("BIG_DATA        ==> %ld.\n", bd_wrote);
   printf("BIG_DATA_plus4k ==> %ld.\n", bdp_wrote);

   close(bd);
   close(bdp);

   return(0);
}
