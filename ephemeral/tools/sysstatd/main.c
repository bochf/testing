#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "options.h"
#include "iobuf.h"
#include "cfgfile.h"
#include "stdioio.h"
#include "version.h"
#include "forest.h"
#include "urlparse.h"
#include "support.h"
#include "httpd.h"
#include "fileio.h"
#include "daemonize.h"

void about(void);
void help(void);

int (*data_to_buffer)(struct iobuf *, struct dataitem *);

/*** Globals ***/
int errorto = ERROR_MSG_STDERR;
/* The following are counters for service requests. Used in coredata.c,
   httpd.c, and here in this file. */
/* You could put a failure count here as well. */
unsigned long customers_served = 0;
pthread_mutex_t cs_lock = PTHREAD_MUTEX_INITIALIZER;

/* ========================================================================= */
int main(int argc, char *argv[])
{
   struct options *o;
   struct iobuf *ob;
   struct cfoptions *cfo;
   struct query *q;
   struct httpdsrv *hsrv;
   struct forest *trees;
   int stand;

   /* Read in command line options */
   if ( NULL == (o = ReadOptions(argc, argv)) )
      return(1);

   /* Handle the basic / early termination options */
   if ( o->bAbout )
   {
      about();
      return(0);
   }

   if ( o->bHelp )
   {
      help();
      return(0);
   }

   if ( NULL == o->url )
         printf("sysstatd version %s starting.\n", VERSION_STRING);

   /* Read in config file */
   if ( NULL == (cfo = ReadConfigFile(o->config_file)) )
      return(1);

   if ( SaveCFileToOptions(o, cfo) )
      return(1);

   if ( o->bDebug )
      DbgDumpConfig(o);

   if ( NULL == o->url ) /* Run as daemon */
   {
      if ( 0 == o->bForeground )
         MakeDaemon();

      if ( NULL == ( hsrv = HTTPDInit(o) ) )
         return(1);

      HTTPDStart(hsrv);

      return(0);
   }

   if ( o->url )
   {
      /* Don't bother taking a lock. We are the only consumer. */
      customers_served++;

      ob = NewIOBuf(o->cf_obufsz);

      if ( NULL == ( q = ParseURLString(NULL, o->url) ) )
         return(1);

      stand = TREE_NONE;
      switch ( q->tree )
      {
      case TREE_ROOT:
         stand = ROOT_TREE;
         break;
      case TREE_BBNV:
         stand = BBENV_TREE;
         break;
      case TREE_PROC:
      case TREE_QCCR:
         /* No tree to build - these are dynamic trees */
         break;
      case TREE_HRDW:
         stand = HARDWARE_TREE;
         break;
      default:
         error_msg("ERROR: Request was made for an unsupported tree.");
         return(1);
         break;
      }

      /* NOTE: RenderTree will generate dynamic trees if necessary */
      trees = PlantForest(stand, o);

      RenderTree(ob, q, trees, o, NULL);

      /*** Conditionally dump based on output method ***/
      if ( o->dump_file )
         DumpFile(o->dump_file, ob);
      else
         DumpStdIO(ob);
   }

   return(0);
}

/* ========================================================================= */
void about(void)
{
   printf("sysstatd - RESTful System Status Daemon\n");
   printf("  Version: %s\n", VERSION_STRING);
   printf("    Aylan Kurdi <refugeeatthree@martyrscemetery.tr>\n");
   printf("    William Favorite <wfavorite2@bloomberg.net>\n");
   fflush(stdout);
}

/* ========================================================================= */
void help(void)
{
   printf("sysstatd - RESTful System Status Daemon\n");
   printf("  Version: %s\n", VERSION_STRING);
   printf("  Usage: sysstatd [[-f <config_file>]|[-u <url>]]\n");
   printf("  Options:\n");
   printf("    -a              Show \"about\" info (and exit)\n");
   printf("    -F              Run web server in foreground (default = background)\n");
   printf("    -f <filename>   Use (alternate) config file (not /etc/sysstatd.conf)\n");
   printf("    -h              Show help (and exit)\n");
   printf("    -o <filename>   Dump -u output to a file\n");
   printf("    -u <uri>        Run on command line with request URI\n");
   
   fflush(stdout);
}
