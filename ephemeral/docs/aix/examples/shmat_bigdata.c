/* For fprintf() and printf() calls */
#include <stdio.h>
/* For the open() call */
#include <fcntl.h>
/* For the close() call */
#include <unistd.h>
/* For the shmat() and shmdt() calls */
#include <sys/shm.h>

int main(int argc, char *argv[])
{
   int fd;
   char *data;
   long i;

   printf("Opening BIG_DATA\n");
   fflush(stdout);

   /* open() the file to get a file descriptor. Insure that our open() and 
      shmat() access types are the same (both read-only).
   */
   if ( -1 == (fd = (open("BIG_DATA", O_RDONLY))) )
   {
      /* Catch errors and exit */
      fprintf(stderr, "ERROR: Failed to open \"DATA FILE\".\n");
      return(1);
   }

   /* shmat() the file:
        fd <---------- file descriptor to map
        NULL <-------- map to any address
        SHM_MAP  <---- will be a file (or'd with)
        SHM_RDONLY <-- will be read only
   */
   data = (char *)shmat(fd, NULL, SHM_MAP | SHM_RDONLY);
   if ( 0 > data )
   {
      fprintf(stderr, "ERROR: Unable to map opened file to a memory location.\n");
      return(1);
   }

   /* Set an arbitrary index.
      NOTE: The index is 0 based, we refer to the data (the alphabet) starting at 1
            so we will do some 0 <---> 1 offset conversions.
   */ 
   i = 8;

   /* Print a simple statement to show how we access the file data */
   printf("Character number %ld of the alphabet is %c.\n", i + 1, data[i]);

   printf("Run: svmon -P %ld\n", (long)getpid());
   printf("Hit return to continue.\n");
   fflush(stdout);
   getchar();

   /* Release the memory location, ignore any errors (we are about to exit) */
   shmdt(data);

   /* Close the file, ignore errors.
      NOTE: We close the fd after we are done accessing the maped data. */
   close(fd);

   printf("Opening BIG_DATA_plus4k\n");
   fflush(stdout);

   if ( -1 == (fd = (open("BIG_DATA_plus4k", O_RDONLY))) )
   {
      fprintf(stderr, "ERROR: Failed to open \"DATA FILE\".\n");
      return(1);
   }

   data = (char *)shmat(fd, NULL, SHM_MAP | SHM_RDONLY);
   if ( 0 > data )
   {
      fprintf(stderr, "ERROR: Unable to map opened file to a memory location.\n");
      return(1);
   }

   i = (256 * 1024 * 1024) + 8;
   printf("Character number %ld of the alphabet is %c.\n", i + 1, data[i]);

   printf("Run: svmon -P %ld\n", (long)getpid());
   printf("Hit return to exit.\n");
   fflush(stdout);
   getchar();


   shmdt(data);
   close(fd);


   /* Leave main() */
   return(0);
}
