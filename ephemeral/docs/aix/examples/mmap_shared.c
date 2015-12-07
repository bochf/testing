#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>

int main(int argc, char *argv[])
{
   int fd;
   char *data;
   int i;
   int go, c;
   char write_val;

   if ( -1 == (fd = (open("DATA_FILE", O_RDWR))) )
   {
      /* Catch errors and exit */
      fprintf(stderr, "ERROR: Failed to open \"DATA FILE\".\n");
      return(1);
   }

   /* mmap() the file:
        NULL      <---- to any address
        26        <---- 26 bytes (will be rounded up to page size)
        PROT_READ <---- access will be read-only
        MAP_FILE  <---- will be a file (or'd with)
        MAP_SHARED <--- self-explanatory
        fd        <---- file descriptor from open()
        0         <---- begin mapping at offset 0 (the beginning) of the file
   */
   data = (char *)mmap(NULL, 26, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, fd, 0);
   if ( NULL == data )
   {
      fprintf(stderr, "ERROR: Unable to map opened file to a memory location.\n");
      return(1);
   }

   /* Close the file, ignore errors.
      NOTE: We close the fd before we start to access the mmap()ed data. */
   close(fd);

   /* Set an arbitrary index.
      NOTE: The index is 0 based, we refer to the data (the alphabet) starting at 1
            so we will do some 0 <---> 1 offset conversions.
   */ 
   i = 8;

   /* Print a simple statement to show how we access the file data */
   printf("Grabbed character at %d ==> %c.\n", i, data[i]);

   printf("Run: svmon -P %d\n", getpid());

   write_val = (getpid() % 26) + 'a';

   go = 1;
   c = 0;
   while ( go )
   {
      if ( c != '\n' )
         printf("(q)uit, (r)ead, or (w)rite : ");
      fflush(stdout);

      c = getchar();

      switch(c)
      {
      case 'q':
      case 'Q':
      case 'E':
      case 'e':
         go = 0;
         break;
      case 'r':
      case 'R':
         printf("Grabbed character at %d ==> %c.\n", i, data[i]);
         break;
      case 'w':
      case 'W':
         data[i] = write_val;
         printf("Wrote character at %d ==> %c.\n", i, data[i]);
         break;
      case '\n':
         break;
      default:
         printf("Unknown option - not handled.\n");
         break;
      }
   }

   /* Put it back the way we found it */
   data[i] = 'I';

   /* Unmap the memory location, ignore any errors (we are about to exit) */
   munmap(data, 26);

   /* Leave main() */
   return(0);
}
