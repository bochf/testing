#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "filefinder.h"
#include "fileops.h"

/* Concepts:
     basename - The input stripped of any extension. Includes dir and file parts
     file_part - Just the file part, stripped of any extension
     dir_part - Just the directory, no trailing slash
*/




/* Tells if the input string has one of the rlog extensions:
    .[0-9][0-9][0-9][0-9]
    .YYMMDD-HHMMSSa
    .metadata
*/
int has_number_extension(char *input);
int has_timestamp_extension(char *input);
int has_metadata_extension(char *input);

/* ========================================================================= */
char *GetMetaDataFile(char *input)
{
  char *basename;
  char *mdname;

  if (NULL == input)
  {
    fprintf(stderr, "ERROR: No file name given to determine metadata file.\n");
    return(NULL);
  }

  if ( NULL == (basename = GetBaseName(input)) )
  {
    fprintf(stderr, "ERROR: Unable to determine basename of %s.\n", input);
    return(NULL);
  }

  if(NULL == (mdname = (char *)malloc(strlen(basename) + 10)))
  {
    fprintf(stderr, "ERROR: Unable to allocate memory for filename.\n");
    return(NULL);
  }

  strcpy(mdname, basename);
  strcat(mdname, ".metadata");

  free(basename);  /* Not that it matters much, but no excessive leaking */

  return(mdname);
}

/* ========================================================================= */
int IsCircularFile(char *input)
{
  int fd;
  char MAGIC[4];

  if(0 == access(input, F_OK))
  {
    if ( -1 == (fd = open(input, O_RDONLY)) )
    {
      fprintf(stderr, "ERROR: Unable to open %s.\n", input);
      return(0);
    }

    if ( 4 != Read(fd, &MAGIC, 4) )
    {
      fprintf(stderr, "ERROR: Unable to read from %s.\n", input);
      close(fd);
      return(0);
    }

    if ( (MAGIC[0] == 'C') &&
	 (MAGIC[1] == 'L') &&
	 (MAGIC[2] == 'O') &&
	 (MAGIC[3] == 'G') )
      {
	close(fd);
	return(1);
      }
  }

  return(0);
}

/* ========================================================================= */
char *GetBaseName(char *input)
{
  int namelen;
  char *rv;

  if(NULL == input)
    return(NULL);

  if(input[0] == 0)
    return(NULL);

  namelen = strlen(input);
  
  if ( NULL == (rv = (char *)malloc(namelen + 1) ))
  {
    fprintf(stderr, "ERROR: Unable to allocate memory.\n");
    return(0);
  }

  strcpy(rv, input);

  if ( has_metadata_extension(input) )
  {
    rv[namelen - 9] = 0;
  }

  if ( has_timestamp_extension(input) )
  {
    rv[namelen - 14] = 0;
  }

  if ( has_number_extension(input) )
  {
    rv[namelen - 5] = 0;
  }

  return(rv);
}


/* ========================================================================= */
char *GetDirPart(char *input)
{
  int i;
  int lastslash;
  char *dirname;

  i = 0;
  lastslash = -1;
  while(input[i] != 0)
  {
    if(input[i] == '/')
      lastslash = i;

    i++;
  }

  if (-1 == lastslash)
  {
    if(NULL == (dirname = (char *)malloc(2)))
      return(NULL);

    strcpy(dirname, ".");

    return(dirname);
  }

  if (0 == lastslash)
  {
    if(NULL == (dirname = (char *)malloc(2)))
      return(NULL);

    strcpy(dirname, "/");

    return(dirname);
  }

  if(NULL == (dirname = (char *)malloc(lastslash + 2)))
    return(NULL);
  
  i = 0;
  while(i < lastslash)
  {
    dirname[i] = input[i];
    i++;
  }
  dirname[i] = 0;

  return(dirname);
}

/* ========================================================================= */
char *GetFilePart(char *input)
{
  int i;
  int lastslash;
  char *filename;

  i = 0;
  lastslash = -1;
  while(input[i] != 0)
  {
    if(input[i] == '/')
      lastslash = i;

    i++;
  }

  if (-1 == lastslash)
  {
    return(input);
  }

  if ( NULL == (filename = (char *)malloc(strlen(input + lastslash) + 2)))
  {
    return(NULL);
  }

  lastslash++;  /* get off the actual slash */
  i = 0;
  while ( input[lastslash + i] != 0 )
  {
    filename[i] = input[lastslash + i];
    i++;
  }
  filename[i] = 0;

  return(filename);
}

/* ========================================================================= */
int has_number_extension(char *input)
{
  int namelen;
  int mylamecheck;
  int i;

  assert(NULL != input);

  if ( 5 < ( namelen = strlen(input) ) )
  {
    if ( input[namelen - 5] != '.' )
      return(0);

    mylamecheck = 0;
    i = 4;

    while ( i > 0 )
    {
      if ( ( input[namelen - i] >= '0' ) && ( input[namelen - i] <= '9' ) )
	mylamecheck++;
  
      i--;
    }

    if (mylamecheck == 4)
      return(1);
  }
 
  return(0);
}

/* ========================================================================= */
int has_timestamp_extension(char *input)
{
  int namelen;
  int mylamecheck;
  int i;

  assert(NULL != input);

  if ( 14 < ( namelen = strlen(input) ) )
  {
    if ( input[namelen - 14] != '.' )
      return(0);

    mylamecheck = 0;
    i = 13;

    while ( i > 0 )
    {
      if ( i == 7 )
      {
	if ( input[namelen - i] == '-' ) 
	  mylamecheck++;
      }
      else
      {
	if ( ( input[namelen - i] >= '0' ) && ( input[namelen - i] <= '9' ) )
	  mylamecheck++;
      }

      i--;
    }

    if (mylamecheck == 13)
      return(1);
  }
 
  return(0);
}

/* ========================================================================= */
int has_metadata_extension(char *input)
{
  int namelen;

  assert(NULL != input);

  if ( 9 < ( namelen = strlen(input) ) )
  {
    if ( input[namelen - 9] != '.' )
      return(0);

    if ( input[namelen - 8] != 'm' )
      return(0);

    if ( input[namelen - 7] != 'e' )
      return(0);

    if ( input[namelen - 6] != 't' )
      return(0);

    if ( input[namelen - 5] != 'a' )
      return(0);

    if ( input[namelen - 4] != 'd' )
      return(0);

    if ( input[namelen - 3] != 'a' )
      return(0);

    if ( input[namelen - 2] != 't' )
      return(0);

    if ( input[namelen - 1] != 'a' )
      return(0);

    return(1);
  }
 
  return(0);
}

