#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/shm.h>

int main(int argc, char *argv[])
{
   int fd;
   char *data;

   if ( -1 == (fd = (open("DATA_FILE", O_RDWR))) )
   {
      /* Catch errors and exit */
      fprintf(stderr, "ERROR: Failed to open \"DATA FILE\".\n");
      return(1);
   }

   data = (char *)shmat(fd, NULL, SHM_MAP);
   if ( 0 > data )
   {
      fprintf(stderr, "ERROR: Unable to map opened file to a memory location.\n");
      return(1);
   }


   /* Print a simple statement to show how we access the file data */
   fprintf(stderr, "Creating new characters in the alphabet");
   data[26] = '*';
   fprintf(stderr, ".");
   data[27] = '*';
   fprintf(stderr, ".");
   data[28] = '*';
   fprintf(stderr, ".");
   data[32] = '*';
   fprintf(stderr, ".");
   data[64] = '*';
   fprintf(stderr, ".");
   data[128] = '*';
   fprintf(stderr, "Done.\n");

   /* Unmap the memory location, ignore any errors (we are about to exit) */
   shmdt(data);

   close(fd);

   /* Leave main() */
   return(0);
}
