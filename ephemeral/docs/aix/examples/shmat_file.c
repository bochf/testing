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
   int i;

   /* open() the file to get a file descriptor. Insure that our open() and shmat()
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

   /* shmat() the file:
        fd <---------- file descriptor to map
        NULL <-------- map to any address
        SHM_MAP  <---- will be a file (or'd with)
        SHM_RDONLY <-- will be read only
   */
   data = (char *)shmat(fd, NULL, SHM_MAP | SHM_RDONLY);
   if ( (void *)-1 == data )
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
   printf("Character number %d of the alphabet is %c.\n", i + 1, data[i]);

   /* Release the memory location, ignore any errors (we are about to exit) */
   shmdt(data);

   /* Close the file, ignore errors.
      NOTE: We close the fd after we are done accessing the maped data. */
   close(fd);

   /* Leave main() */
   return(0);
}
