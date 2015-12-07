/* For fprintf() and printf() calls */
#include <stdio.h>
/* For the open() call */
#include <fcntl.h>
/* For the close() call */
#include <unistd.h>
/* For the mmap() and munmap() calls */
#include <sys/types.h>
#include <sys/mman.h>

int main(int argc, char *argv[])
{
   int fd;
   char *data;
   int i;

   /* open() the file to get a file descriptor. Insure that our open() and mmap()
      access types are the same (both read-only). In this case DATA_FILE contains
      26 bytes of upper-case ASCII characters.
      Specifically: ABCDEFGHIJKLMNOPQRSTUVWXYZ
   */
   if ( -1 == (fd = (open("DATA_FILE", O_RDONLY))) )
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
        MAP_PRIVATE <-- will be for our use only (not subject to external changes)
        fd        <---- file descriptor from open()
        0         <---- begin mapping at offset 0 (the beginning) of the file
   */
   data = (char *)mmap(NULL, 26, PROT_READ, MAP_FILE | MAP_PRIVATE, fd, 0);
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
   printf("Character number %d of the alphabet is %c.\n", i + 1, data[i]);

   /* Unmap the memory location, ignore any errors (we are about to exit) */
   munmap(data, 26);

   /* Leave main() */
   return(0);
}
