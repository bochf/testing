#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <signal.h>


#include "datahandler.h"
#include "cfileops.h"
#include "rfileops.h"

#define INPUT_BUF_SIZE 4096

extern struct cfile *gcf;
extern struct rfile *grf;

void handle_flushquit(int unused)
{
  if ( gcf )
  {
    UpdateCFileHeader(gcf);
  }

  if ( grf )
  {
    DumpMetaDataFile(grf);
  }

  exit(0);
}

void handle_flush(int unused)
{
   off_t ereiam;

   if ( gcf )
   {
      if(-1 != (ereiam = lseek(gcf->fd, 0, SEEK_CUR)))
      {
         UpdateCFileHeader(gcf);

         if(ereiam != lseek(gcf->fd, ereiam, SEEK_SET))
         {
            gcf->fbits = ( gcf->fbits & CLOG_LSEEK_FAIL );
         }
      }
   }

   if ( grf )
   {
      DumpMetaDataFile(grf);
   }
}

int HandleStdinError(void)
{
  /* This is basically stubbed for now */
  fprintf(stderr, "ERROR: Failure on stdin read().\n");
  return(0);
}

int HandleFileOutError(void)
{
  /* This is basically stubbed for now */
  fprintf(stderr, "ERROR: blog failure on file write(). Data lost.\n");
  return(0);
}

int HandleStdoutError(void)
{
  /* This is basically stubbed for now */
  fprintf(stderr, "ERROR: Failure on stdout write().\n");
  return(0);
}

char *new_buffer(void);


/* ========================================================================= */
int ClogSession(struct options *o)
{
  char *buf;
  struct cfile *c;
  ssize_t got;

  if ( NULL == (buf = new_buffer()) )
    return(1);

  if (o->bmmap)
  {
    if ( NULL == (c = NewCircularMmap(o)))
      return(1);
  }
  else
  {
    if ( NULL == (c = NewCircularFile(o)))
      return(1);
  }

  /* Set up signal handling */
  gcf = c;
  signal(SIGHUP, handle_flush);
  signal(SIGINT, handle_flush);
  signal(SIGQUIT, handle_flush);

  while(0 != (got = read(0, buf, INPUT_BUF_SIZE)))
  {
     if ( got > 0 )
     {
        /* Echo this to stdout if requested */
        if(o->bStdout)
        {
           if ( got != write(1, buf, got) )
              HandleStdoutError();
        }
      
        if (o->bmmap)
        {
           if ( got != WriteToCircularMmap(c, buf, got) )
              HandleFileOutError();
        }
        else
        {
           if ( got != WriteToCircularFile(c, buf, got) )
              HandleFileOutError();
        }
     }
     else
        HandleStdinError();
  }

  if (o->bmmap)
     CloseCircularMmap(c);
  else
     CloseCircularFile(c);

  return(0);
}

/* ========================================================================= */
int ClogAppendSession(struct options *o)
{
  char *buf;
  struct cfile *c;
  ssize_t got;

  if ( NULL == (buf = new_buffer()) )
    return(1);

  if (o->bmmap)
  {
    if ( NULL == (c = OpenCircularMmap(o)))
      return(1);
  }
  else
  {
    if ( NULL == (c = OpenCircularFile(o)))
      return(1);
  }

  /* Set up signal handling */
  gcf = c;
  signal(SIGHUP, handle_flush);
  signal(SIGINT, handle_flush);
  signal(SIGQUIT, handle_flush);

  while(0 != (got = read(0, buf, INPUT_BUF_SIZE)))
  {
    if ( got > 0 )
    {
       /* Echo this to stdout if requested */
       if(o->bStdout)
       {
          if ( got != write(1, buf, got) )
             HandleStdoutError();
       }
      
       if (o->bmmap)
       {
          if ( got != WriteToCircularMmap(c, buf, got) )
             HandleFileOutError();
       }
       else
       {
          if ( got != WriteToCircularFile(c, buf, got) )
             HandleFileOutError();
       }
    }
    else
      HandleStdinError();
  }

  if (o->bmmap)
    CloseCircularMmap(c);
  else
    CloseCircularFile(c);

  return(0);
}

/* ========================================================================= */
int RlogSession(struct options *o)
{
  struct rfile *rf;
  char *buf;
  ssize_t got;

  if ( NULL == (rf = NewRFile(o)) )
     return(1);
       
  if ( NULL == (buf = new_buffer()) )
     return(1);

  /* Set up signal handling */
  grf = rf;
  signal(SIGHUP, handle_flush);
  signal(SIGINT, handle_flush);
  signal(SIGQUIT, handle_flush);

  while(0 != (got = read(0, buf, INPUT_BUF_SIZE)))
  {
     if ( got > 0 )
     {
        /* Echo this to stdout if requested */
        if(o->bStdout)
        {
           if ( got != write(1, buf, got) )
              HandleStdoutError();
        }
      
        if ( got != WriteToRotatingFile(rf, buf, got) )
           HandleFileOutError();
     }
     else
        HandleStdinError();
  }

  DumpMetaDataFile(rf);

  return(0);
}


/* ========================================================================= */
int RlogAppendSession(struct options *o)
{
  struct rfile *rf;
  char *buf;
  ssize_t got;

  if ( NULL == (rf = AppendRFile(o)) )
    return(1);

  if ( NULL == (buf = new_buffer()) )
    return(1);

  /* Set up signal handling */
  grf = rf;
  signal(SIGHUP, handle_flush);
  signal(SIGINT, handle_flush);
  signal(SIGQUIT, handle_flush);

  while(0 != (got = read(0, buf, INPUT_BUF_SIZE)))
  {
    if ( got > 0 )
    {
      /* Echo this to stdout if requested */
      if(o->bStdout)
      {
	if ( got != write(1, buf, got) )
	  HandleStdoutError();
      }
      
      if ( got != WriteToRotatingFile(rf, buf, got) )
	HandleFileOutError();
    }
    else
      HandleStdinError();
  }

  DumpMetaDataFile(rf);

  return(0);
}


/* ========================================================================= */
/* Kind of pointless to turn this into a funcition - but I did anyways */
char *new_buffer(void)
{
  void *buf;
  
  if ( NULL == (buf = malloc(INPUT_BUF_SIZE)) )
  {
    fprintf(stderr, "ERROR: Unable to allocate memory.\n");
  }

  return((char *)buf);
}
