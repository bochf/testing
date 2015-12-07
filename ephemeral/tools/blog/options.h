#ifndef OPTIONS_H
#define OPTIONS_H

#define RLOG_TYPE_NUMERICAL 1
#define RLOG_TYPE_TIMESTAMP 0

struct options
{
  int bDump;                    /* Dump the contents of a circular file */
  int bHelp;                    /* Show help, exit                      */
  int bAbout;                   /* Show about, exit                     */
  unsigned long long bytesSize; /* Limit file(s) to this size           */
  char *filename;               /* Name of circular file or basename of
				   rotating files                       */
  int bDumpHeader;              /* Semi-documented circular file header
				   dumper                               */
  int bDumpMetadata;            /* Semi-doucmented metadata file dumper */
  char *description;            /* Description for the circular file    */
  int eRlogType;                /* The kind of extension on the rolling 
				   log file - appended to filename      */
  int iRolling;                 /* 1 = rolling, 0 = circular            */
  int bStdout;                  /* tee to stdout                        */
  int bmmap;                    /* Use mmap() - only for circular mode  */
  /* Not handled yet */
  int bAppend;

  /* Not for human consumption */
  int bDebug;
};

/* ========================================================================= */
/* Name:
 * Description:
 * Input:
 * Returns:
 * Side effects:
 * Notes:
 */
struct options *ParseOptions(int argc, char *argv[]);



#endif
