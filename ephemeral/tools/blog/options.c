
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "options.h"

struct options *init_options(void);
int validate_options(struct options *o);
int parse_bytes(unsigned long long *bytes, char *argv);
int parse_integer(int *i_val, char *argv);
char *parse_description(char *argv);

#define NI_NONE   0
#define NI_SIZE   1
#define NI_ROLL   2
#define NI_DESC   3

/* ========================================================================= */
struct options *ParseOptions(int argc, char *argv[])
{
  struct options *o;
  int a, i;
  int ddError;
  int next_item;

  if(NULL == (o = init_options()))
    return(NULL);

  i = 1;
  while ( i < argc )
  {
    next_item = NI_NONE;

    if ( argv[i][0] == '-' )
    {
      if ( argv[i][1] == '-' )
      {
	ddError = 1;

	if ( 0 == strcmp(argv[i], "--append"))
	{ o->bAppend = 1;  ddError = 0; }

	if ( 0 == strcmp(argv[i], "--dump"))
	{  o->bDump = 1; ddError = 0; }
	
	if ( 0 == strcmp(argv[i], "--about"))
	{  o->bAbout = 1; ddError = 0; }
	
	if ( 0 == strcmp(argv[i], "--dump-header"))
	{  o->bDumpHeader = 1; ddError = 0; }
	
	if ( 0 == strcmp(argv[i], "--dump-metadata"))
	{  o->bDumpMetadata = 1; ddError = 0; }
	
	if ( 0 == strcmp(argv[i], "--help"))
	{  o->bHelp = 1; ddError = 0; }

	if ( ( 0 == strcmp(argv[i], "--mirror-stdout") ) || ( 0 == strcmp(argv[i], "--stdout")) )
	{  o->bStdout = 1; ddError = 0; }

	if ( ( 0 == strcmp(argv[i], "--use-mmap") ) || ( 0 == strcmp(argv[i], "--mmap")) )
	{  o->bmmap = 1; ddError = 0; }

	if ( ddError )
	{
	  fprintf(stderr, "ERROR: %s not understood.\n", argv[i]);
	  if(o->filename)
	    free(o->filename);
	  free(o);
	  return(NULL);
	}
      }
      else
      {
	a = 1;
	while ( argv[i][a] != 0 )
	{
	  switch ( argv[i][a] )
	  {
	  case '+':
	    o->bDebug = 1;
	    break;

	  case 'A':
	    o->bAbout = 1;
	    break;
	  case 'a':
	    o->bAppend = 1;
	    break;
	  case 'C':
	    o->bDumpHeader = 1;
	    break;
	  case 'd':
	    o->bDump = 1;
	    break;
	  case 'D':
	    next_item = NI_DESC;
	    break;
	  case 'h':
	    o->bHelp = 1;
	    break;
	  case 'l':
	    next_item = NI_SIZE;
	    break;
	  case 'm':
	    o->bmmap = 1;
	    break;
	  case 'M':
	    o->bDumpMetadata = 1;
	    break;
	  case 'r':
	    next_item = NI_ROLL;
	    break;
	  case 's':
	    o->bStdout = 1;
	    break;
	  case 't':
	    o->eRlogType = RLOG_TYPE_TIMESTAMP;
	    break;

	  default:
	    fprintf(stderr, "ERROR: -%c is not understood.\n", argv[i][a]);
	    if ( o->filename )
	      free(o->filename);
	    free(o);
	    return(NULL);
	  }
	  a++;
	}
      }
    }
    else
    {
      if ( 0 == strcmp(argv[i], "+") )
      {
	o->bDebug = 1;
      }
      else
      {
	/* Make sure we did not try to previously set filename. Two filenames is 
	   considered an error. */
	if ( NULL != o->filename )
	{
	  fprintf(stderr, "ERROR: Filenames? Argument %s conflicts with %s.\n",
		  o->filename,
		  argv[i]);
	  free(o);
	  return(NULL);
	}

	/* Allocate memory for filename */
	if ( NULL == (o->filename = (char *)malloc(strlen(argv[i]) + 1) ) )
	{
	  fprintf(stderr, "ERROR: Unable to allocate memory.\n");
	  free(o);
	  return(NULL);
	}

	strcpy(o->filename, argv[i]);
      }
    }

    i++;

    switch ( next_item )
    {
    case NI_SIZE:
      if ( parse_bytes(&o->bytesSize, argv[i]) )
      {
	fprintf(stderr, "ERROR: Unable to parse the argument to -l.\n");
	free(o);
	return(NULL);
      }
      i++;
      break;
    case NI_ROLL:
      if ( parse_integer(&o->iRolling, argv[i]) )
      {
	fprintf(stderr, "ERROR: Unable to parse -r argument.\n");
	free(o);
	return(NULL);
      }
      i++;
      break;
    case NI_DESC:
      if(NULL == (o->description = parse_description(argv[i])))
      {
	fprintf(stderr, "ERROR: Unable to parse -D argument.\n");
	free(o);
	return(NULL);
      }
      i++;
      break;
    default:
    case NI_NONE:
      break;
    }      
  }

  if ( validate_options(o) )
  {
    if ( o->filename )
      free(o->filename);
    free(o);
    return(NULL);
  }

  return(o);
}


/* ========================================================================= */
int validate_options(struct options *o)
{
  assert(NULL != o);

  if ( o->bDebug )
  {
    fprintf(stderr, "Debug              : on\n");
    fprintf(stderr, "Help               : %s\n", o->bHelp ? "on" : "off");
    fprintf(stderr, "About              : %s\n", o->bAbout ? "on" : "off");
    fprintf(stderr, "Rolling log        : %d\n", o->iRolling );
    fprintf(stderr, "Mirror stdout      : %s\n", o->bStdout ? "on" : "off");
    fprintf(stderr, "mmap() c-log       : %s\n", o->bmmap ? "on" : "off");
    fprintf(stderr, "filename (base)    : %s\n", o->filename);
    fprintf(stderr, "File size (bytes)  : %llu\n", o->bytesSize);
    fprintf(stderr, "Dump CFile header  : %s\n", o->bDumpHeader ? "yes" : "no");
    fprintf(stderr, "Dump RFile metadata: %s\n", o->bDumpMetadata ? "yes" : "no");
    fprintf(stderr, "-r extension       : %s\n", o->eRlogType == RLOG_TYPE_TIMESTAMP ? "timestamp" : "numerical");
    fprintf(stderr, "Append             : %s\n", o->bAppend ? "append" : "overwrite");
  }

  if ( o->bHelp )
    return(0);

  if ( o->bAbout )
    return(0);

  if ( NULL == o->filename )
  {
    fprintf(stderr, "ERROR: A (base) filename was not specified.\n");
    return(1);
  }

  if ( o->iRolling )
  {
    if ( o->bmmap )
    {
      fprintf(stderr, "NOTE: -m(map()) option is not supported with -r(olling).\n");
      o->bmmap = 0; /* Turning it off should make no difference */
    }

    if ( o->iRolling < 2 )
    {
      fprintf(stderr, "ERROR: Invalid -r(olling) value. Must be >= 2.\n");
      return(1);
    }
  }
  else
  {
    if ( o->eRlogType == RLOG_TYPE_TIMESTAMP )
    {
      fprintf(stderr, "NOTE: -t(imestamp) only works with -r(olling).\n");
      o->eRlogType = RLOG_TYPE_NUMERICAL;
    }

  }

  if ( o->bytesSize < (1024 * 100) )
  {
    fprintf(stderr, "ERROR: Log files of less than 100K are not supported.\n");
    /* Debug mode lets us create smaller files */
    if ( 0 == o->bDebug )
      return(1);
  }


  return(0);
}

/* ========================================================================= */
struct options *init_options(void)
{
  struct options *o;

  if ( NULL == (o = (struct options *)malloc(sizeof(struct options))))
  {
    fprintf(stderr, "ERROR: Unable to allocate memory.\n");
    return(NULL);
  }

  /* Set defaults */
  o->iRolling = 0;
  o->bDump = 0;
  o->bHelp = 0;
  o->bDebug = 0;
  o->bmmap = 0;
  o->bStdout = 0;
  o->bytesSize = 1024 * 1024;
  o->filename = NULL;
  o->bDumpHeader = 0;
  o->bAppend = 0;
  o->bDumpMetadata = 0;
  o->eRlogType = RLOG_TYPE_NUMERICAL;
  o->description = NULL;

  return(o);
}


/* ========================================================================= */
int parse_bytes(unsigned long long *bytes, char *argv)
{
  int i;
  unsigned long long rawnum;

  assert( NULL != bytes );

  i = 0;
  rawnum = 0;
  while ( ( argv[i] >= '0' ) && ( argv[i] <= '9') )
  {
    rawnum *= 10;
    rawnum += argv[i] - '0';
    i++;
  }

  switch ( argv[i] )
  {
  case 'k':
  case 'K':
    rawnum *= 1024;
    break;
  case 'm':
  case 'M':
    rawnum *= 1024;
    rawnum *= 1024;
    break;
  case 'g':
  case 'G':
    rawnum *= 1024;
    rawnum *= 1024;
    rawnum *= 1024;
    break;
  case 'b':
  case 'B':
  case 0:
    break;
  default:
    return(1);
  }

  *bytes = rawnum;
  return(0);
}

/* ========================================================================= */
char *parse_description(char *argv)
{
  int len;
  char *description;

  if ( NULL == argv )
    return(NULL);

  if (argv[0] == 0)
    return(NULL);

  len = strlen(argv);

  if ( len > 128 )
    return(NULL);

  if ( NULL == (description = (char *)malloc(len + 1)))
    return(NULL);

  strcpy(description, argv);

  return(description);
}

/* ========================================================================= */
int parse_integer(int *ival, char *argv)
{
  int i;
  int rval;

  assert( NULL != ival );

  if ( NULL == argv )
    return(1);

  if (argv[0] == 0)
    return(1);

  i = 0;
  rval = 0;
  while ( ( argv[i] >= '0' ) && ( argv[i] <= '9' ) )
  {
    rval *= 10;
    rval += argv[i] - '0';

    i++;
  }

  /* If we are not a terminating char, then we ran into an invalid char */
  if ( argv[i] != 0 )
    return(1);

  *ival = rval;

  return(0);
}
