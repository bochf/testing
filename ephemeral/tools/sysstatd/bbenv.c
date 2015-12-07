#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "bbenv.h"
#include "defaults.h"
#include "support.h"
#include "datatree.h"

#ifdef PORT_Linux
#include "Linux/osspecific.h"
#endif

#ifdef PORT_AIX
#include "AIX/osspecific.h"
#endif

#ifdef PORT_SunOS
#include "SunOS/osspecific.h"
#endif

/*
  NOTE:
   - The process list works by registering a local list of process
     names that will be pattern matched against the process list.
   - The process list is kept in the tree, but it is also kept in
     the local list. The local list is of struct bbproc items. The
     top of that list is BBP_listtop. This is statically defined
     with the STATIC_BBP() macro.
   - To prevent multiple requests stomping all over the list, a 
     lock is required on the whole list modification code. The
     only other alternative is to make it all local. That would
     be too problematic. So the lock method is used.
   - Elsewhere, process listing is a dynamic tree. Here it is a
     set of defined processes we are looking for, but still
     something kind of dynamic.
   - A new process (to watch) must be registered with the following
     macros:
      STATIC_BBP() <---------- Creates the pointer to the data
      BBPROCESS_GETTOR() <---- Creates a gettor function that
                               uses the pointer above
      REGISTER_PROC() <------- Registers a local bbproc struct
                               that is part of a list, and is
                               assigned to the individual
                               pointer defined in STATIC_BBP()
      NEW_DATA_ITEM() <------- Places the item in the tree
                               and registers the gettor func
*/
      

/* Struct used for process list walking/pattern matching */
struct bbproc
{
   char pname[MAX_PNAME_LEN];
   unsigned long cnt;

   struct bbproc *next;
};

/* Globals for this section */
static pthread_mutex_t bbproc_lock = PTHREAD_MUTEX_INITIALIZER;

/* Each proc name gets a BBProc pointer */
#define STATIC_BBP(CNAME) static struct bbproc *BBP_##CNAME = NULL;

STATIC_BBP(listtop)
STATIC_BBP(comdbg64)
STATIC_BBP(ibig_tsk)
STATIC_BBP(tkrm_tsk)
STATIC_BBP(rpml)
STATIC_BBP(remcon2)
STATIC_BBP(pipeboy_tsk)
STATIC_BBP(varldb)
STATIC_BBP(bb_filexferd2)
STATIC_BBP(tickhash_tsk)
STATIC_BBP(timserv_tsk)
STATIC_BBP(appsshd)
STATIC_BBP(comdb2_tsk)
STATIC_BBP(varmon_tsk)
STATIC_BBP(basgwyh_tsk)
STATIC_BBP(prctmgr_tsk)



/* ========================================================================= */
/* Prototypes                                                                */
struct bbproc *register_proc(char *procname);
long write_ul_value(char *status, unsigned long size, struct bbproc *finding);



/* There is a file_exists() in support.h/c. It is a basic file test. I will
   re-implement here as a more robust version as it has multiple use cases
   with differing options here (in this file) */

/* ========================================================================= */
#define FI_NONE  -1
#define FI_UNK    0
#define FI_DIR    1
#define FI_FILE   2
#define FI_FIFO   3
#define FI_LINK   4

#define FI_MOUNT  10  /* A proposal */
#define FI_ERROR  88

int file_is_a(char *filename)
{
   struct stat sb;

   /* Bad things that must stop us from continuing */
   assert( NULL != filename );
   assert( filename[0] != 0 );

   /* See header file for notes */
   if ( 0 == lstat(filename, &sb) )
   {
      if ( S_ISDIR(sb.st_mode) )
         return(FI_DIR);

      if ( S_ISFIFO(sb.st_mode) )
         return(FI_FIFO);

      if ( S_ISREG(sb.st_mode) )
         return(FI_FILE);

      if ( S_ISLNK(sb.st_mode) )
         return(FI_LINK);

      return(FI_UNK);
   }
   else
   {
      if ( errno == ENOENT )
         return(FI_NONE);
   }

   return(FI_ERROR);
}

/* ========================================================================= */
long write_finding(char *status, unsigned long size, int finding)
{
   long rv;

   switch (finding)
   {
   case FI_NONE:
      if ( size > 4 )
      {
         strcpy(status, "NONE");
         rv = 4;
      }
      else
         rv = 0;
      break;
   case FI_UNK:
      if ( size > 6 )
      {
         strcpy(status, "Exists");
         rv = 6;
      }
      else
         rv = 0;
      break;
   case FI_DIR:
      if ( size > 9 )
      {
         strcpy(status, "Directory");
         rv = 9;
      }
      else
         rv = 0;
      break;
   case FI_FILE:
      if ( size > 4 )
      {
         strcpy(status, "File");
         rv = 4;
      }
      else
         rv = 0;
      break;
   case FI_FIFO:
      if ( size > 4 )
      {
         strcpy(status, "Pipe");
         rv = 4;
      }
      else
         rv = 0;
      break;
   case FI_LINK:
      if ( size > 4 )
      {
         strcpy(status, "Link");
         rv = 4;
      }
      else
         rv = 0;
      break;
   case FI_ERROR:
   default:
      if ( size > 5 )
      {
         strcpy(status, "Error");
         rv = 5;
      }
      else
         rv = 0;
      break;
   }

   return(rv);
}

/* ========================================================================= */
/* The gettor is completely re-entrant and thread safe. There is no
   initialization required. */
long Get_bb(char *status, unsigned long size)
{
   return(write_finding(status, size, file_is_a("/bb")));
}

long Get_bb_bin(char *status, unsigned long size)
{
   return(write_finding(status, size, file_is_a("/bb/bin")));
}

long Get_bb_sys(char *status, unsigned long size)
{
   return(write_finding(status, size, file_is_a("/bb/sys")));
}

long Get_bb_data(char *status, unsigned long size)
{
   return(write_finding(status, size, file_is_a("/bb/data")));
}

long Get_bb_pm(char *status, unsigned long size)
{
   return(write_finding(status, size, file_is_a("/bb/pm")));
}

long Get_bb_bigmem(char *status, unsigned long size)
{
   return(write_finding(status, size, file_is_a("/bb/bigmem")));
}

long Get_bb_cores(char *status, unsigned long size)
{
   return(write_finding(status, size, file_is_a("/bb/cores")));
}

long Get_bb_util(char *status, unsigned long size)
{
   return(write_finding(status, size, file_is_a("/bb/util")));
}

long Get_bb_admin(char *status, unsigned long size)
{
   return(write_finding(status, size, file_is_a("/bb/admin")));
}

/* ========================================================================= */
long Get_ethdv_dta(char *status, unsigned long size)
{
   return(write_finding(status, size, file_is_a("/bb/bin/ethdv.dta")));
}

long Get_comdbg64(char *status, unsigned long size)
{
   return(write_finding(status, size, file_is_a("/bb/bin/comdbg64")));
}

long Get_timserv(char *status, unsigned long size)
{
   return(write_finding(status, size, file_is_a("/bb/bin/timserv.tsk")));
}

long Get_bbpy(char *status, unsigned long size)
{
   return(write_finding(status, size, file_is_a("/bb/bin/bbpy")));
}

long Get_varldb(char *status, unsigned long size)
{
   return(write_finding(status, size, file_is_a("/bb/bin/varldb")));
}

long Get_rpml(char *status, unsigned long size)
{
   return(write_finding(status, size, file_is_a("/bb/bin/rpml")));
}

long Get_console_fifo(char *status, unsigned long size)
{
   return(write_finding(status, size, file_is_a("/tmp/console.fifo")));
}

long Get_bypass_file(char *status, unsigned long size)
{
   char filename[128]; /* Lazy way of managing potential overflow */
   char hostname[64];
   int i;

   if (gethostname(hostname, 64))
      return(0);  /* STUB: I have no idea if this is appropriate */

   /* This is a sloppy way of turning "server.domain.com" into "server" */
   i = 0;
   while( hostname[i] != 0 )
   {
      if ( hostname[i] == '.' )
         hostname[i] = 0;

      i++;
   }

   sprintf(filename, "/%s.bypass", hostname);

   return(write_finding(status, size, file_is_a(filename)));
}

/* ========================================================================= */
long EnumerateMatches(char *status, unsigned long size)
{
   struct procitem p;
   struct bbproc *thisp;

   /* Take the big lock */
   pthread_mutex_lock(&bbproc_lock);

   /* Initialize all counters */
   thisp = BBP_listtop;
   while ( thisp )
   {
      thisp->cnt = 0;

      thisp = thisp->next;
   }

   /* Walk the process list */
   InitProcessEntry(&p);
   StartProcessEntry(&p);

   while( GetProcessEntry(&p) )
   {
      thisp = BBP_listtop;
      while ( thisp )
      {
         if ( 0 == strcmp(thisp->pname, p.pname) )
            thisp->cnt++;

         thisp = thisp->next;
      }
   }

   /* Keep the lock! */
   return(0);
}

/* ========================================================================= */
struct bbproc *register_proc(char *procname)
{
   struct bbproc *thisbbp;

   if ( NULL == (thisbbp = (struct bbproc *)malloc(sizeof(struct bbproc))) )
   {
      error_msg("ERROR: Failed to allocate memory for a BB process item.");
      return(NULL);
   }

   /* Initialize all values */
   strcpy(thisbbp->pname, procname);
   thisbbp->cnt = 0;
   thisbbp->next = NULL;

   /* Put it in the list */
   thisbbp->next = BBP_listtop;
   BBP_listtop = thisbbp;

   return(thisbbp);
}

/* ========================================================================= */
long write_ul_value(char *status, unsigned long size, struct bbproc *finding)
{
   if ( size < 5 )
      return(0);

   return(sprintf(status, "%lu", finding->cnt));
}

/* ========================================================================= */
/* Define the gettor functions. They must be before RegisterBBProcItems()    */
#define BBPROCESS_GETTOR(P_NAME) long GetBBP_##P_NAME(char *status, unsigned long size) \
                                 {                                                      \
                                    return(write_ul_value(status, size, BBP_##P_NAME)); \
                                 }

BBPROCESS_GETTOR(comdbg64)
BBPROCESS_GETTOR(ibig_tsk)
BBPROCESS_GETTOR(tkrm_tsk)
BBPROCESS_GETTOR(rpml)
BBPROCESS_GETTOR(remcon2)
BBPROCESS_GETTOR(pipeboy_tsk)
BBPROCESS_GETTOR(varldb)
BBPROCESS_GETTOR(bb_filexferd2)
BBPROCESS_GETTOR(tickhash_tsk)
BBPROCESS_GETTOR(timserv_tsk)
BBPROCESS_GETTOR(appsshd)
BBPROCESS_GETTOR(comdb2_tsk)
BBPROCESS_GETTOR(varmon_tsk)
BBPROCESS_GETTOR(basgwyh_tsk)
BBPROCESS_GETTOR(prctmgr_tsk)


long GetBBP_Total(char *status, unsigned long size)
{
   struct bbproc *thisp;
   unsigned long total = 0;

   /* Walk and enumerate */
   thisp = BBP_listtop;
   while ( thisp )
   {
      total += thisp->cnt;
      thisp = thisp->next;
   }
 
   pthread_mutex_unlock(&bbproc_lock);

   /* Sanity check on size */
   if ( size < 6 )
      return(0);

   return(sprintf(status, "%lu", total));
}

/* ========================================================================= */
/* These macros could be combined */
#define REGISTER_PROC(PATTERN, CNAME);  if ( NULL == (BBP_##CNAME = register_proc(PATTERN)) ) { return(NULL); }
#define FIRSTDATA_ITEM(PATTERN, CNAME); if ( NULL == (thisdata  = NewDataItem(PATTERN, GetBBP_##CNAME)) ) { return(NULL); } \
                                        brnhdata = thisdata;
#define NEW_DATA_ITEM(PATTERN, CNAME);  if ( NULL == (thisdata->next = NewDataItem(PATTERN, GetBBP_##CNAME)) ) { return(NULL); } \
                                        thisdata = thisdata->next;

struct dataitem *RegisterBBProcItems(void)
{
   struct dataitem *thisdata;
   struct dataitem *brnhdata;

   REGISTER_PROC("comdbg64", comdbg64);
   REGISTER_PROC("ibig.tsk", ibig_tsk);
   REGISTER_PROC("trkm.tsk", tkrm_tsk);
   REGISTER_PROC("rpml", rpml);
   REGISTER_PROC("remcon2", remcon2);
   REGISTER_PROC("pipeboy.tsk", pipeboy_tsk);
   REGISTER_PROC("varldb", varldb);
   REGISTER_PROC("bb_filexferd2", bb_filexferd2);
   REGISTER_PROC("tickhash.tsk", tickhash_tsk);
   REGISTER_PROC("timserv.tsk", timserv_tsk);
   REGISTER_PROC("appsshd", appsshd);
   REGISTER_PROC("comdb2.tsk", comdb2_tsk);
   REGISTER_PROC("varmon.tsk", varmon_tsk);
   REGISTER_PROC("basgwyh.tsk", basgwyh_tsk);
   REGISTER_PROC("prctmgr.tsk", prctmgr_tsk);
   /* You do NOT register a proc for "total" is is not a process */

   FIRSTDATA_ITEM("comdbg64", comdbg64);
   NEW_DATA_ITEM("ibig.tsk", ibig_tsk);
   NEW_DATA_ITEM("tkrm.tsk", tkrm_tsk);
   NEW_DATA_ITEM("rpml", rpml);
   NEW_DATA_ITEM("remcon2", remcon2);
   NEW_DATA_ITEM("pipeboy.tsk", pipeboy_tsk);
   NEW_DATA_ITEM("varldb", varldb);
   NEW_DATA_ITEM("bb_filexferd2", bb_filexferd2);
   NEW_DATA_ITEM("tickhash.tsk", tickhash_tsk);
   NEW_DATA_ITEM("timserv.tsk", timserv_tsk);
   NEW_DATA_ITEM("appsshd", appsshd);
   NEW_DATA_ITEM("comdb2.tsk", comdb2_tsk);
   NEW_DATA_ITEM("varmon.tsk", varmon_tsk);
   NEW_DATA_ITEM("basgwyh.tsk", basgwyh_tsk);
   NEW_DATA_ITEM("prctmgr.tsk", prctmgr_tsk);
   NEW_DATA_ITEM("Total", Total); /* This must always be the last registered */

   return(brnhdata);
}







