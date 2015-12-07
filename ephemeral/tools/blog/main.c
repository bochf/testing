#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include "options.h"
#include "cfileops.h"
#include "version.h"
#include "datahandler.h"

#include "filefinder.h"

#include "rfileops.h"
#include "rmetadata.h"

/* ========================================================================= */
struct cfile *gcf;
struct rfile *grf;

/* ========================================================================= */
void usage(void);
void about(void);

/* ========================================================================= */
int main(int argc, char *argv[])
{
  struct options *o;

  /* Set our globals to NULL */
  gcf = NULL;
  grf = NULL;

#ifndef BIN_NAME
  CompileFailNoBinaryNameDefined();
#endif

  /* We want a 64 bit value for size_t - insure it is so */
  assert(8 == sizeof(size_t));

  /* Read options from the command line into a manageable struct */
  if ( NULL == (o = ParseOptions(argc, argv)) )
    return(1);

  /* Does user want usage info? */
  if ( o->bHelp )
  {
    usage();
    return(0);
  }

  /* Does user want about info? */
  if ( o->bAbout )
  {
    about();
    return(0);
  }

  if ( o->bAppend )
  {
    if ( o->iRolling )
    {
      if ( IsCircularFile(o->filename) )
      {
	fprintf(stderr, "WARNING: Unable to convert circular file to rolling - staying circular.\n");
	return(ClogAppendSession(o));
      }

      return(RlogAppendSession(o));
    }

    /* No type is specified - assume circular */
    if ( IsCircularFile(o->filename) )
    {
      return(ClogAppendSession(o));
    }
    else
    {
      if(NULL != (LoadMetaDataFile(o->filename)) )
	return(RlogAppendSession(o));

      return(ClogSession(o));
    }
  }

  if ( o->bDumpHeader )
    return(DumpHeader(o));

  if ( o->bDumpMetadata )
  {
    struct rfile *rf;

    if ( NULL == (rf = LoadMetaDataFile(o->filename)) )
      return(1);

    return(PrintMetaData(rf));
  }

  if ( o->bDump )
  {
    if ( IsCircularFile(o->filename) )
      return(DumpCircularFile(o));
    else
      return(DumpRotatingFile(o));
  }

  if ( o->iRolling )
    return(RlogSession(o));

  return(ClogSession(o));
}

/* ========================================================================= */
void usage(void)
{
  printf("%s - Circular logger\n", BIN_NAME);
  printf("  Version: %s\n", VERSION_STRING);
  printf("  Usage: %s <options> <filename>\n", BIN_NAME);
  printf("  Options:\n");
  printf("    -A           Print information about blog, and exit\n");
  printf("    -a           Append new data to existing files\n");
  printf("    -d           Dump the contents of a previously stored log\n");
  printf("    -D <string>  Associate a description with this log\n");
  printf("    -C           Dump the header of a circular file\n");
  printf("    -l <size>    Limit files to size\n");
  printf("    -M           Dump the contents of a metadata file\n");
  printf("    -m           Use mmap() for the circular log file\n");
  printf("    -r <count>   Generate \"count\" (rolling) logs instead of single circular log\n");
  printf("    -s           Mirror input to stdout\n");
  printf("    -t           Use timestamp extension on -r files\n");

  printf("  Double-dash aliases:\n");
  printf("    -s   --stdout --mirror-stdout\n");
  printf("    -h   --help\n");
  printf("    -A   --about\n");
  printf("    -C   --dump-header\n");
  printf("    -M   --dump-metadata\n");
  printf("    -d   --dump\n");
  printf("    -m   --mmap --use-mmap\n");
}

/* ========================================================================= */
void about(void)
{
  printf("%s - Version: %s\n", BIN_NAME, VERSION_STRING);
  printf(" Isambard Brunel  <ikbrunel@broadgage.com>\n");
  printf(" William Favorite <wfavorite2@bloomberg.net>\n");
}
