#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>

#include "coreprov.h"
#include "mainhelp.h"

/* These values are placed in pitem->sioffset. sioffset is typically used
   for struct math. Here it is used as a different kind of "pointer" to 
   the data. */
#define CT_24HHMMSS   1
#define CT_TIMEVAL    2
#define CT_UTIME      3
#define CT_CTIME      4
#define CT_NCTIME     5
#define CT_ICOUNT     6
#define CT_TIMESPEC   7
#define CT_ECHOARG    8


/* ========================================================================= */
static struct provider *this_provider;

#ifdef _TIMESPEC
static const char *diname[] = {"24hhmmss",   "timeval",    "utime",     \
                               "ctime",      "nctime",     "icount",    \
                               "timespec",   "echoarg",                 \
                               NULL};
#else
static const char *diname[] = {"24hhmmss",   "timeval",    "utime",     \
                               "ctime",      "nctime",     "icount",    \
                               "echoarg",                               \
                               NULL};
#endif


#ifdef _TIMESPEC
static const int ditype[]  =  {PDT_STRING,   PDT_TIMEVAL,   PDT_INT64,  \
                               PDT_STRING,   PDT_STRING,    PDT_UINT64, \
                               PDT_TIMESPEC, PDT_STRING,                \
                              };
#else
static const int ditype[]  =  {PDT_STRING,   PDT_TIMEVAL,   PDT_INT64,  \
                               PDT_STRING,   PDT_STRING,    PDT_UINT64, \
                               PDT_STRING,                              \
                              };
#endif                               


static time_t now;    /* enabled with get_time  */
static struct tm ts;  /* enabled with get_ltime */
static int get_time;
static int get_ltime;

/* ========================================================================= */
int core_update(int interval)
{
   struct pitem *this_pitem;
   char *ctimerp;
   uint64_t *ullptr;
   int64_t *llptr;
   int rv;

   rv = 0; /* Assume success on start */

   /* Only get time once */
   if ( get_time )
      time(&now);

   /* Only get ltime once */
   if ( get_ltime )
      localtime_r(&now, &ts);

   /* Walk the u(pdate )i(tem) list */
   this_pitem = this_provider->ui_list;
   while ( this_pitem )
   {
      switch( this_pitem->sioffset )
      {
      case CT_24HHMMSS:
         sprintf((char *)this_pitem->data_ptr,
                 "%02d%02d%02d",
                 ts.tm_hour,
                 ts.tm_min,
                 ts.tm_sec);
         break;
      case CT_UTIME:
         llptr = this_pitem->data_ptr;
         *llptr = now;
         break;
      case CT_CTIME:
         ctimerp = ctime(&now);
         strcpy((char *)this_pitem->data_ptr, ctimerp);
         chomp((char *)this_pitem->data_ptr);
         break;
      case CT_NCTIME:
         /* Similar to: http://en.wikipedia.org/wiki/ISO_8601 */
         sprintf((char *)this_pitem->data_ptr,
                 "%4d-%02d-%02dT%02d%02d%02d",
                 ts.tm_year + 1900,
                 ts.tm_mon + 1,
                 ts.tm_mday,
                 ts.tm_hour,
                 ts.tm_min,
                 ts.tm_sec);
         break;
      case CT_ICOUNT:
         ullptr = this_pitem->data_ptr;
         *ullptr += 1;
         break;
#ifdef _TIMESPEC
      case CT_TIMESPEC:
         /* Super hi-res */
         clock_gettime(CLOCK_MONOTONIC, (struct timespec *)this_pitem->data_ptr);
         break;
#endif
      case CT_TIMEVAL:
         /* Hi-res */
         gettimeofday((struct timeval *)this_pitem->data_ptr, NULL);
         break;
      case CT_ECHOARG:
         /* Nothing to do here */
         break;
      default:
         /* Unknown type - This is an assert issue really */
         rv = 1;
         break;
      }

      this_pitem = this_pitem->next_ui;
   }

   return(rv);
}

/* ========================================================================= */
int core_list(int dflag)
{
   int icount = 0;

   while ( diname[icount] )
   {
      DumpQuadData(dflag, ditype[icount]);
      printf("core::%s\n", diname[icount]);

      icount++;
   }

   return(icount);
}

/* ========================================================================= */
struct pitem *core_enablepitem(struct qparts *qp)
{
   struct pitem *this_pitem;
   void *data_ptr;
   uint64_t *ictrptr;

   /* Make sure we were called with the right quad (provider) name */
   if ( 0 != strcmp(qp->pname, "core") )
      return(NULL);

   this_pitem = NULL; /* Not parsed at this time */

   /* ------------------------------------------------------- */
   if ( 0 == strcmp(qp->iname, "24hhmmss") )
   {
      if ( NULL == (data_ptr = malloc(12)) )
      {
         fprintf(stderr, "ERROR: Unable to allocate memory for a data structure.\n");
         return(NULL);
      }

      /* Allocate pitem */
      if ( NULL == (this_pitem = NewPItem(qp, PDT_STRING, data_ptr)) )
         return(NULL);

      /* Set the type - sioffset is not used in the "traditional" sense here */
      this_pitem->sioffset = CT_24HHMMSS;

      get_time = 1; /* This requires a time() call */
      get_ltime = 1; /* This requires a localtime() call */
   }

   /* ------------------------------------------------------- */
   if ( 0 == strcmp(qp->iname, "echoarg") )
   {
      if ( NULL == (data_ptr = malloc(strlen(qp->iargs) + 1)) )
      {
         fprintf(stderr, "ERROR: Unable to allocate memory for a data structure.\n");
         return(NULL);
      }

      strcpy((char *)data_ptr, qp->iargs);

      /* Allocate pitem */
      if ( NULL == (this_pitem = NewPItem(qp, PDT_STRING, data_ptr)) )
         return(NULL);

      /* Set the type - sioffset is not used in the "traditional" sense here */
      this_pitem->sioffset = CT_ECHOARG;
   }

   /* ------------------------------------------------------- */
   if ( 0 == strcmp(qp->iname, "timeval") )
   {
      if ( NULL == (data_ptr = malloc(sizeof(struct timeval))) )
      {
         fprintf(stderr, "ERROR: Unable to allocate memory for a data structure.\n");
         return(NULL);
      }

      /* Allocate pitem */
      if ( NULL == (this_pitem = NewPItem(qp, PDT_TIMEVAL, data_ptr)) )
         return(NULL);

      /* Set the type - sioffset is not used in the "traditional" sense here */
      this_pitem->sioffset = CT_TIMEVAL;
   }

   /* ------------------------------------------------------- */
   if ( 0 == strcmp(qp->iname, "utime") )
   {
      if ( NULL == (data_ptr = malloc(sizeof(int64_t))) )
      {
         fprintf(stderr, "ERROR: Unable to allocate memory for a data structure.\n");
         return(NULL);
      }

      /* Allocate pitem */
      if ( NULL == (this_pitem = NewPItem(qp, PDT_INT64, data_ptr)) )
         return(NULL);

      /* Set the type - sioffset is not used in the "traditional" sense here */
      this_pitem->sioffset = CT_UTIME;

      get_time = 1; /* This requires a time() call */
   }

   /* ------------------------------------------------------- */
   if ( 0 == strcmp(qp->iname, "ctime") )
   {
      if ( NULL == (data_ptr = malloc(32)) )
      {
         fprintf(stderr, "ERROR: Unable to allocate memory for a data structure.\n");
         return(NULL);
      }

      /* Allocate pitem */
      if ( NULL == (this_pitem = NewPItem(qp, PDT_STRING, data_ptr)) )
         return(NULL);

      /* Set the type - sioffset is not used in the "traditional" sense here */
      this_pitem->sioffset = CT_CTIME;

      get_time = 1; /* This requires a time() call */
      get_ltime = 1; /* This requires a localtime() call */
   }

   /* ------------------------------------------------------- */
   if ( 0 == strcmp(qp->iname, "nctime") )
   {
      if ( NULL == (data_ptr = malloc(32)) )
      {
         fprintf(stderr, "ERROR: Unable to allocate memory for a data structure.\n");
         return(NULL);
      }

      /* Allocate pitem */
      if ( NULL == (this_pitem = NewPItem(qp, PDT_STRING, data_ptr)) )
         return(NULL);

      /* Set the type - sioffset is not used in the "traditional" sense here */
      this_pitem->sioffset = CT_NCTIME;

      get_time = 1; /* This requires a time() call */
      get_ltime = 1; /* This requires a localtime() call */
   }

   /* ------------------------------------------------------- */
   if ( 0 == strcmp(qp->iname, "icount") )
   {
      if ( NULL == (data_ptr = malloc(sizeof(uint64_t))) )
      {
         fprintf(stderr, "ERROR: Unable to allocate memory for a data structure.\n");
         return(NULL);
      }

      /* Initialize the counter */
      ictrptr = data_ptr;
      *ictrptr = 0;
      
      /* Allocate pitem */
      if ( NULL == (this_pitem = NewPItem(qp, PDT_UINT64, data_ptr)) )
         return(NULL);

      /* Set the type - sioffset is not used in the "traditional" sense here */
      this_pitem->sioffset = CT_ICOUNT;
   }


   /* ------------------------------------------------------- */
   if ( 0 == strcmp(qp->iname, "timespec") )
   {
#ifdef _TIMESPEC
      if ( NULL == (data_ptr = malloc(sizeof(struct timespec))) )
      {
         fprintf(stderr, "ERROR: Unable to allocate memory for a data structure.\n");
         return(NULL);
      }

      /* Allocate pitem */
      if ( NULL == (this_pitem = NewPItem(qp, PDT_TIMESPEC, data_ptr)) )
         return(NULL);

      /* Set the type - sioffset is not used in the "traditional" sense here */
      this_pitem->sioffset = CT_TIMESPEC;
#else
      fprintf(stderr, "ERROR: The timespec type is not supported on this platform.\n");
      return(NULL);
#endif
   }

   /* NOTE - diff is simply not supported here! */
   if ( ShouldDiff(qp) )
   {
      fprintf(stderr, "ERROR: The diff argument is not supported on a core provider data item.\n");
      this_pitem = NULL; /* Consider this NOT parsed */
   }
   
   if ( ShouldPSAvg(qp) )
   {
      fprintf(stderr, "ERROR: The psavg argument is not supported on a core provider data item.\n");
      this_pitem = NULL; /* Consider this NOT parsed */
   }

   if ( NULL == this_pitem ) /* Used as a "is parsed" flag */
   {
      fprintf(stderr, "ERROR: Unable to register the \"%s\" item in the core provider.\n",
              qp->iname);
      return(NULL);
   }

   /* Activate the provider - Go hot */
   this_provider->update_required = 1;

   /* Shove this into our update item list */
   this_pitem->next_ui = this_provider->ui_list;
   this_provider->ui_list = this_pitem;

   /* Item registered */
   return(this_pitem);
}

/* ========================================================================= */
int CoreRegister(struct proot *p)
{
   /* No assert for:
      timeval  - Uses same type throughout
      timespec - Uses same type throughout
      string   - A string is a string
      icount   - Uses a stdint.h type
   */
   assert(sizeof(time_t) == sizeof(int64_t));


   /* There is no init (malloc()) of data here. There are two (static) globals
      that are set here in place of an init funciton. */
   get_time = 0;   /* Should we call time() on update? */
   get_ltime = 0;  /* Should we call localtime() on update? */

   /* Register the provider */
   if ( NULL == (this_provider = RegisterProvider(p, "core",
                                                  core_update,
                                                  core_list,
                                                  core_enablepitem)) )
      return(1); /* Failure condition from RegisterProvider() */

   /* Exit on success */
   return(0);

}
