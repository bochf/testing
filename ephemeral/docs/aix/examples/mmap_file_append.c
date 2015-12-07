#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>

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

   data = (char *)mmap(NULL, 26, PROT_READ | PROT_WRITE, MAP_FILE, fd, 0);
   if ( NULL == data )
   {
      fprintf(stderr, "ERROR: Unable to map opened file to a memory location.\n");
      return(1);
   }

   close(fd);

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
   munmap(data, 26);

   /* Leave main() */
   return(0);
}
