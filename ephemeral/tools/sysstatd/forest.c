#include <stdlib.h>

#include "forest.h"
#include "coredata.h"
#include "bbenv.h"
#include "version.h"

#include "yamlfmtr.h"
#include "jsonfmtr.h"
#include "support.h"
#include "proctree.h"
#include "qcconr.h"

#ifdef PORT_Linux
#include "Linux/osspecific.h"
#endif

#ifdef PORT_AIX
#include "AIX/osspecific.h"
#endif

#ifdef PORT_SunOS
#include "SunOS/osspecific.h"
#endif

/* Discussion:
   Forest knows the trees.... specifically how to initialize them and if they
   require refreshing prior to a data pull. For this reason, all code to
   handle these actions should be called from here.

   Forest (typically) does not know about OSS (operating system specific)
   items. The OSS will need to provide common naming for each item that
   is desired. Some variation in the trees is expected.
*/

/* ========================================================================= */
struct dataitem *GrowMainTree(void)
{
   struct dataitem *thisdata;
   struct dataitem *trnkdata;
   struct dataitem *rootdata;
   struct dataitem *brnhdata;

   /* Initialize all core data */
   if ( InitCoreData() ) /* Found in coredata.h/c */
      return(NULL);

   if ( InitOSSCoreData() ) /* Found in the OSS library */
      return(NULL);

   /* First - version string */
   rootdata = NewDataItem("VersionString", GetVersion);

   /* Branch for uname data */
   brnhdata = NewDataBranch("uname", NULL, NULL);
   trnkdata = brnhdata;
   rootdata->next = brnhdata;
   thisdata = NewDataItem("uname_s", GetUname_s);
   brnhdata->sublist = thisdata;
   thisdata->next = NewDataItem("uname_p", GetUname_p);
   thisdata = thisdata->next;
   thisdata->next = NewDataItem("uname_r", GetUname_r);
   thisdata = thisdata->next;
   thisdata->next = NewDataItem("uname_n", GetUname_n);

   thisdata = NewDataItem("TimeStamp", GetTimestamp);
   trnkdata->next = thisdata;
   trnkdata = trnkdata->next;

   thisdata->next = NewDataItem("UptimeSecs", GetUptimeSecs);
   thisdata = thisdata->next;
   thisdata->next = NewDataItem("ProcCount", GetProcCount);
   thisdata = thisdata->next;
   thisdata->next = NewDataItem("ServicedRequests", GetServed);
   thisdata = thisdata->next;

   /* Refresh data - only to insure that it can be done. (Determine
      potential failures up front.) */
   if( RefreshOSSCoreData() )
   {
      /* Bail without free() because the failure is likely not a 
         memory related issue, and we will be exiting soon. The only
         reason to free memory when we are about to exit is to insure
         that our error message is printed. */
      return(NULL);
   }

   /* Nothing is ever "refreshed" in coredata.h/c at this time. */

   return(rootdata);
}

/* ========================================================================= */
struct dataitem *GrowBBEnvTree(void)
{
   struct dataitem *thisdata;
   struct dataitem *trnkdata;
   struct dataitem *rootdata;
   struct dataitem *brnhdata;

   /* Init data here */
   /* NOTE: There is no init or refresh at this time */

   /* Build the directory branch */
   thisdata = NewDataItem("/bb", Get_bb);
   brnhdata = thisdata;
   thisdata->next = NewDataItem("/bb/bin", Get_bb_bin);
   thisdata = thisdata->next;
   thisdata->next = NewDataItem("/bb/sys", Get_bb_sys);
   thisdata = thisdata->next;
   thisdata->next = NewDataItem("/bb/data", Get_bb_data);
   thisdata = thisdata->next;
   thisdata->next = NewDataItem("/bb/cores", Get_bb_cores);
   thisdata = thisdata->next;
   thisdata->next = NewDataItem("/bb/util", Get_bb_util);
   thisdata = thisdata->next;
   thisdata->next = NewDataItem("/bb/bigmem", Get_bb_bigmem);
   thisdata = thisdata->next;
   thisdata->next = NewDataItem("/bb/admin", Get_bb_admin);
   thisdata = thisdata->next;
   thisdata->next = NewDataItem("/bb/pm", Get_bb_pm);
   thisdata = thisdata->next;
   trnkdata = NewDataBranch("Directories", brnhdata, NULL);

   /* One time set of the root */
   rootdata = trnkdata;

   /* Build the files branch */
   thisdata = NewDataItem("/bb/bin/ethdv.dta", Get_ethdv_dta);
   brnhdata = thisdata;
   thisdata->next = NewDataItem("/bb/bin/comdbg64", Get_comdbg64);
   thisdata = thisdata->next;
   thisdata->next = NewDataItem("/bb/bin/timserv.tsk", Get_timserv);
   thisdata = thisdata->next;
   thisdata->next = NewDataItem("/bb/bin/bbpy", Get_bbpy);
   thisdata = thisdata->next;
   thisdata->next = NewDataItem("/HOST.bypass", Get_bypass_file);
   thisdata = thisdata->next;
   thisdata->next = NewDataItem("/bb/bin/varldb", Get_varldb);
   thisdata = thisdata->next;
   thisdata->next = NewDataItem("/tmp/console.fifo", Get_console_fifo);
   thisdata = thisdata->next;
   thisdata->next = NewDataItem("/bb/bin/rpml", Get_rpml);
   thisdata = thisdata->next;
   trnkdata->next = NewDataBranch("Files", brnhdata, NULL);
   trnkdata = trnkdata->next;

   /* Register (remotely) and lay in the process data */
   brnhdata = RegisterBBProcItems();
   trnkdata->next = NewDataBranch("Processes", brnhdata, EnumerateMatches);

   return(rootdata);
}

/* ========================================================================= */
struct dataitem *GrowHardwareTree(void)
{
   struct dataitem *thisdata;
   struct dataitem *trnkdata;
   struct dataitem *rootdata;
   struct dataitem *brnhdata;

   if ( InitOSSHardwareData())
      return(NULL);

   /* Build the CPU branch */
   thisdata = NewDataItem("Logical CPUs", GetCPULogicalCount);
   brnhdata = thisdata;
   thisdata->next = NewDataItem("Threads per CPU", GetCPUThreadCount);
   thisdata = thisdata->next;
   thisdata->next = NewDataItem("Cores", GetCPUCoreCount);
   thisdata = thisdata->next;
   thisdata->next = NewDataItem("Sockets", GetCPUSocketCount);
   thisdata = thisdata->next;
   thisdata->next = NewDataItem("CPU Speed", GetCPUMHz);
   thisdata = thisdata->next;
#ifdef PORT_Linux
   thisdata->next = NewDataItem("BogoMIPS", GetCPUBogomips);
   thisdata = thisdata->next;
#endif
   trnkdata = NewDataBranch("CPU", brnhdata, NULL);

   /* One time set of the root */
   rootdata = trnkdata;

   return(rootdata);
}

/* ========================================================================= */
int RenderTree(struct iobuf *obuf,
               struct query *q,
               struct forest *f,
               struct options *o,
               int *httprc)
{
   int junk;
   int (*data_to_buffer)(struct iobuf *, struct dataitem *);

   /* The caller may not care. So make a /dev/null for this value */
   if ( NULL == httprc )
      httprc = &junk;

   /* Conditionally set the desired output format */
   switch ( q->format )
   {
   case FORMAT_JSON:
      data_to_buffer = DataToBufJSON;
      break;
   case FORMAT_XML:
   case FORMAT_CSV:
      error_msg("WARNING: Chosen output format not supported at this time.");
      /* STUB: Fall-through for now. */
   case FORMAT_YAML:
      data_to_buffer = DataToBufYAML;
      break;
   case FORMAT_NONE:
   default:
      error_msg("ERROR: Problems parsing output format from URL.");
      return(1);
      break;
   }
      
   /* Now call the chosen render function with the appropriate tree */
   switch ( q->tree )
   {
   case TREE_BBNV:
      /* There is no refresh for bbenv at this time */
      data_to_buffer(obuf, f->bbenv);
      break;
   case TREE_HRDW:
      if (RefreshOSSHardwareData())
         return(1);
      data_to_buffer(obuf, f->hardware);
      break;
   case TREE_PROC:
      /* I am different. I am a dynamic tree! */
      DynamicProcTree(obuf, q->format);
      break;
   case TREE_QCCR:
      /* I am different. I am a dynamic tree! */
      DynamicQCTree(obuf, q, o);
      break;
   case TREE_ROOT:
   default:
      /* Nothing to refresh in coredata.c/h */
      if(RefreshOSSCoreData())
         return(1);
      data_to_buffer(obuf, f->core);
      break;
   }

   switch ( IOBufError(obuf) )
   {
   case IOBUF_ERR_NONE:
      *httprc = 200;
      break;
   case IOBUF_ERR_OVRFLOW:
      *httprc = 500;
      error_msg("ERROR: Output buffer size exceeded.");
      break;
   case IOBUF_ERR_NOFILE:
      *httprc = 404;
      error_msg("ERROR: Required file not found.");
      break;
   default:
      *httprc = 500;
      error_msg("ERROR: Unhandled exception processing tree.");
      break;
   }

   return(0);
}

/* ========================================================================= */
struct forest *PlantForest(unsigned int stand, struct options *o)
{
   struct forest *f;

   if ( NULL == (f = (struct forest *)malloc(sizeof(struct forest))) )
   {
      error_msg("ERROR: Unable to allocate memory for forest.");
      return(NULL);
   }
   
   f->core = NULL;
   f->bbenv = NULL;
   f->hardware = NULL;

   if ( stand & ROOT_TREE )
   {
      if ( NULL == (f->core = GrowMainTree()) )
         return(NULL);
   }

   if ( stand & BBENV_TREE )
   {
      if ( NULL == (f->bbenv = GrowBBEnvTree()) )
         return(NULL);
   }

   if ( stand & HARDWARE_TREE )
   {
      if ( NULL == (f->hardware = GrowHardwareTree()) )
         return(NULL);
   }

   return(f);
}







