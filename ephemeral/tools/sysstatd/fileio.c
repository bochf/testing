#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "fileio.h"
#include "support.h"

int DumpFile(char *filename, struct iobuf *ob)
{
   int fd;

   if ( -1 == (fd = open(filename,
                         O_CREAT | O_WRONLY | O_TRUNC,
                         S_IRUSR | S_IWUSR | S_IRGRP |S_IWGRP | S_IROTH)) )
   {
      error_msg("ERROR: Failed to open %s for output.\n");
      return(1);
   }

   /* STUB: I am not going to spend time here (now). This *should* work. */
   write(fd, ob->buf, ob->eod);

   close(fd);

   return(0);
}


