#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#include "fileops.h"
#include "rfileops.h"


/* ========================================================================= */
int DumpMetaDataFile(struct rfile *rf)
{
  int fd;
  char *filename;

  char MAGIC[8];
  int16_t maxfiles;
  int16_t nextfilenum;
  size_t maxfilesize;
  size_t bytesrecvd;
  int16_t filecount;
  int16_t filenlen;
  int desclen;
  int16_t twobytezero;
  int32_t fourbytezero;

  struct rfilelist *rfl;
  struct rfilelist *rflstop;

  assert(NULL != rf);

  /* Set all values */
  MAGIC[0] = 'R';
  MAGIC[1] = 'L';
  MAGIC[2] = 'O';
  MAGIC[3] = 'G';
  MAGIC[4] = 1;                             /* Version */
  MAGIC[5] = rf->bfErrors;                  /* Bitfield */
                                            /* Note: I am mapping a 16 bit int into a char. This
                                               is kind of a problem... except for the fact that
                                               we are only using 4 bits of the bit field. */
  MAGIC[6] = rf->eRlogType;                 /* Rolling file type */
  if (rf->description)
    MAGIC[7] = strlen(rf->description);     /* Description length */
  else
    MAGIC[7] = 0;
  desclen = MAGIC[7];

  maxfiles = rf->maxfile;
  nextfilenum = rf->nextfilenum;
  maxfilesize = rf->max_file_size;
  bytesrecvd = rf->bytes_recvd;
  filecount = 3;
  twobytezero = 0;

  /* Begin writing the file */
  if ( NULL == ( filename = (char *)malloc(strlen(rf->basename) + 10 )) )
  {
    fprintf(stderr, "ERROR: Failure to allocate memory.\n");
    return(1);
  }

  sprintf(filename, "%s.metadata", rf->basename);

  if (-1 == (fd = OpenNewTrunc(filename)))
  {
    fprintf(stderr, "ERROR: Unable to open %s.\n", filename);
    free(filename);
    return(1);
  }

  /* Offset 0 */
  if ( 8 != Write(fd, &MAGIC, 8) )
  {
    fprintf(stderr, "ERROR: Unable to write to %s.\n", filename);
    close(fd);
    return(1);
  }

  /* Offset 8 */
  if ( 2 != Write(fd, &maxfiles, 2) )
  {
    fprintf(stderr, "ERROR: Unable to write to %s.\n", filename);
    close(fd);
    return(1);
  }

  if ( 2 != Write(fd, &nextfilenum, 2) )
  {
    fprintf(stderr, "ERROR: Unable to write to %s.\n", filename);
    close(fd);
    return(1);
  }

  /* Offset 12 */
  if ( 8 != Write(fd, &maxfilesize, 8) )
  {
    fprintf(stderr, "ERROR: Unable to write to %s.\n", filename);
    close(fd);
    return(1);
  }

  /* Offset 20 */
  if ( 8 != Write(fd, &bytesrecvd, 8) )
  {
    fprintf(stderr, "ERROR: Unable to write to %s.\n", filename);
    close(fd);
    return(1);
  }

  /* Offset 28 */
  filecount = 0;

  if ( 2 != Write(fd, &filecount, 2) )
  {
    fprintf(stderr, "ERROR: Unable to write to %s.\n", filename);
    close(fd);
    return(1);
  }

  if ( 2 != Write(fd, &twobytezero, 2) )
  {
    fprintf(stderr, "ERROR: Unable to write to %s.\n", filename);
    close(fd);
    return(1);
  }

  rfl = rf->filelist->next;
  rflstop = NULL;

  while ( rflstop != rfl )
  {
    if ( NULL == rflstop )
      rflstop = rfl;

    if ( rfl->bFileExists )
    {
      filenlen = strlen(rfl->filename);

      if ( 2 != Write(fd, &filenlen, 2) )
      {
	fprintf(stderr, "ERROR: Unable to write to %s.\n", filename);
	close(fd);
	return(1);
      }

      if ( filenlen != Write(fd, rfl->filename, filenlen) )
      {
	fprintf(stderr, "ERROR: Unable to write to %s.\n", filename);
	close(fd);
	return(1);
      }
      
      filecount++; /* we will come back for this later */
    }
      
    rfl = rfl->next;
  }

  /* This is like a terminating zero - the next filename is 0 bytes long */
  if ( 2 != Write(fd, &twobytezero, 2) )
  {
    fprintf(stderr, "ERROR: Unable to write to %s.\n", filename);
    close(fd);
    return(1);
  }

  /* Timestamp - reserved */
  fourbytezero = 0;
  if ( 4 != Write(fd, &fourbytezero, 4) )
  {
    fprintf(stderr, "ERROR: Unable to write to %s.\n", filename);
    close(fd);
    return(1);
  }

  /* Timestamp - reserved */
  fourbytezero = 0;
  if ( 4 != Write(fd, &fourbytezero, 4) )
  {
    fprintf(stderr, "ERROR: Unable to write to %s.\n", filename);
    close(fd);
    return(1);
  }

  if ( desclen != Write(fd, &rf->description, desclen) )
  {
    fprintf(stderr, "ERROR: Unable to write to %s.\n", filename);
    close(fd);
    return(1);
  }

  lseek(fd, 28, SEEK_SET);
  if ( 2 != Write(fd, &filecount, 2) )
  {
    fprintf(stderr, "ERROR: Unable to write to %s.\n", filename);
    close(fd);
    return(1);
  }

  close(fd);

  return(0);
}




/* ========================================================================= */
struct rfile *LoadMetaDataFile(char *basename)
{
  int fd;
  struct rfile *rf;
  char MAGIC[8];
  int desclen;
  int16_t maxfiles;
  int16_t nextfilenum;
  size_t maxfilesize;
  size_t bytesrecvd;
  int16_t filecount;
  int16_t twobytezero;
  int32_t fourbytezero;
  int16_t filenlen;
  char readbuf[1024];
  struct rfilelist *head;
  struct rfilelist *here;
  struct rfilelist *last;
  int filename_len;
  int i;

  char *mdfilename;

  assert(NULL != basename);

  if (NULL == (mdfilename = GetMetaDataFile(basename)))
     return(NULL);

  if (NULL == (rf = (struct rfile *)malloc(sizeof(struct rfile))))
  {
    fprintf(stderr, "ERROR: Unable to allocate memory for rotating file metadata.\n");
    return(NULL);
  }

  rf->basename = GetBaseName(basename);
  rf->file_part = GetFilePart(rf->basename);
  rf->dir_part = GetDirPart(rf->basename);
  
  if ( -1 == (fd = open(mdfilename, O_RDONLY)) )
  {
    fprintf(stderr, "ERROR: Unable to open %s.\n", mdfilename);
    free(rf);
    return(NULL);
  }

  /* Offset 0 */
  if ( 8 != Read(fd, &MAGIC, 8) )
  {
    fprintf(stderr, "ERROR: Unable to read from %s.\n", mdfilename);
    close(fd);
    free(rf);
    return(NULL);
  }

  if ( ( MAGIC[0] != 'R' ) ||
       ( MAGIC[1] != 'L' ) ||
       ( MAGIC[2] != 'O' ) ||
       ( MAGIC[3] != 'G' ) )
  {
    fprintf(stderr, "ERROR: File format of %s not understood.\n", mdfilename);
    close(fd);
    free(rf);
    return(NULL);
  }

  rf->bfErrors = MAGIC[5];
  rf->eRlogType = MAGIC[6];
  desclen = MAGIC[7];

  if ( MAGIC[4] != 1 )
  {
    fprintf(stderr, "ERROR: Version %d metadata files not understood.\n", MAGIC[4]);
    close(fd);
    free(rf);
    return(NULL);
  }

  /*
  fprintf(stderr, "Magic = \"%c%c%c%c\"\n", MAGIC[0], MAGIC[1], MAGIC[2], MAGIC[3]);
  fprintf(stderr, "Version = %d\n", MAGIC[4]);
  fprintf(stderr, "Error bits = 0X%02X\n", MAGIC[5]); 
  fprintf(stderr, "File type = %s\n", MAGIC[6] == RLOG_TYPE_NUMERICAL ? "Numerical" : "Timestamp");
  fprintf(stderr, "Description length = %d\n", MAGIC[7]);
  
  */
  if ( 2 != Read(fd, &maxfiles, 2) )
  {
    fprintf(stderr, "ERROR: Unable to read from %s.\n", mdfilename);
    close(fd);
    free(rf);
    return(NULL);
  }

  if ( 2 != Read(fd, &nextfilenum, 2) )
  {
    fprintf(stderr, "ERROR: Unable to read from %s.\n", mdfilename);
    close(fd);
    free(rf);
    return(NULL);
  }

  /* Offset 12 */
  if ( 8 != Read(fd, &maxfilesize, 8) )
  {
    fprintf(stderr, "ERROR: Unable to read from %s.\n", mdfilename);
    close(fd);
    free(rf);
    return(NULL);
  }

  /* Offset 20 */
  if ( 8 != Read(fd, &bytesrecvd, 8) )
  {
    fprintf(stderr, "ERROR: Unable to read from %s.\n", mdfilename);
    close(fd);
    free(rf);
    return(NULL);
  }

  if ( 2 != Read(fd, &filecount, 2) )
  {
    fprintf(stderr, "ERROR: Unable to read from %s.\n", mdfilename);
    close(fd);
    free(rf);
    return(NULL);
  }

  if ( 2 != Read(fd, &twobytezero, 2) )
  {
    fprintf(stderr, "ERROR: Unable to read from %s.\n", mdfilename);
    close(fd);
    free(rf);
    return(NULL);
  }

  /*
  fprintf(stderr, "Max files in rotation = %d\n", maxfiles);
  fprintf(stderr, "Next file number to use = %d\n", nextfilenum);
  fprintf(stderr, "Maximum file size = %llu\n", (unsigned long long)maxfilesize);
  fprintf(stderr, "Bytes received = %llu\n", (unsigned long long)bytesrecvd);
  fprintf(stderr, "Files in use = %d\n", filecount);
  */

  /* Set the rfile values */
  rf->eRlogType = MAGIC[6];
  rf->bytes_recvd = bytesrecvd;
  rf->nextfilenum = nextfilenum;
  rf->max_file_size = maxfilesize;
  rf->maxfile = maxfiles;
  if ( desclen > 0 )
  {
    if ( NULL == (rf->description = (char *)malloc(desclen + 1)) )
    {
      fprintf(stderr, "ERROR: Failed to allocate memory for rotating file metadata.\n");
      close(fd);
      free(rf);
      return(NULL);
    }
  }
  else
    rf->description = NULL;
  rf->bDeleteLastFile = 0;
  rf->bfErrors = RLOG_ERR_NOERRS;
  rf->bytes_lastfile = 0;
  rf->cur_fd = -1;

  /* However it was passed - yank the .metadata and add whatever extension to be used 
     this should be sufficient for either mode */
  filename_len = strlen(mdfilename) + 10;

  i = rf->maxfile + 1;
  head = NULL;
  last = NULL;
  here = NULL;
  while(i)
  {
    if ( NULL == ( here = (struct rfilelist *)malloc(sizeof(struct rfilelist))) )
    {
      fprintf(stderr, "ERROR: Unable to allocate memory.\n");
      free(rf);
      close(fd);
      return(NULL);
    }

    if ( NULL == head )
      head = here;

    if ( NULL == (here->filename = (char *)malloc(filename_len)) )
    {
      fprintf(stderr, "ERROR: Unable to allocate memory.\n");
      free(rf);
      close(fd);
      return(NULL);
    }

    if ( NULL == (here->fullfn = (char *)malloc(filename_len)) )
    {
      fprintf(stderr, "ERROR: Unable to allocate memory.\n");
      free(rf);
      close(fd);
      return(NULL);
    }
    
    here->filename[0] = 0;
    here->fullfn[0] = 0;
    here->bFileExists = 0;
    here->next = last;

    last = here;
    i--;
  }

  head->next = last;   /* Close the loop */



  /* rf->filelist = last; / * Attach the loop to the main structure */


  if ( 2 != Read(fd, &filenlen, 2) )
  {
    fprintf(stderr, "ERROR: Unable to read from %s.\n", mdfilename);
    free(rf);
    close(fd);
    return(NULL);
  }

  /*
  if ( filenlen > 0 )
    fprintf(stderr, "Files in rotation:\n");
  */
  while ( filenlen > 0 )
  {    
    if ( filenlen != Read(fd, readbuf, filenlen) )
    {
      fprintf(stderr, "ERROR: Unable to read from %s.\n", mdfilename);
      free(rf);
      close(fd);
      return(NULL);
    }

    readbuf[filenlen] = 0;

    strcpy(here->filename, readbuf);
    strcpy(here->fullfn, rf->dir_part);
    strcat(here->fullfn, "/");
    strcat(here->fullfn, readbuf);
    here->bFileExists = 1;

    /* fprintf(stderr, "  %s\n", readbuf); */

    if ( 2 != Read(fd, &filenlen, 2) )
    {
      fprintf(stderr, "ERROR: Unable to read from %s.\n", mdfilename);
      free(rf);
      close(fd);
      return(NULL);
    }
    last = here;
    here = here->next;
  }

  rf->filelist = last;

  /* Timestamp - reserved */
  if ( 4 != Read(fd, &fourbytezero, 4) )
  {
    fprintf(stderr, "ERROR: Unable to read from  %s.\n", mdfilename);
    free(rf);
    close(fd);
    return(NULL);
  }

  /* Timestamp - reserved */
  if ( 4 != Read(fd, &fourbytezero, 4) )
  {
    fprintf(stderr, "ERROR: Unable to read from  %s.\n", mdfilename);
    free(rf);
    close(fd);
    return(NULL);
  }


  if ( desclen > 0 )
  {
    if ( desclen != Read(fd, readbuf, desclen) )
    {
      fprintf(stderr, "ERROR: Unable to read from %s.\n", mdfilename);
      free(rf);
      close(fd);
      return(NULL);
    }
    readbuf[desclen] = 0;
    strcpy(rf->description, readbuf);

  }

  close(fd);

  return(rf);
}




