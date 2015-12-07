
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "verbosity.h"

#include "../version.h"

static int verbose_value;

/* ========================================================================= */
int VerboseMessage(const char *format, ...)
{
   va_list ap;

   if ( verbose_value < VL_VERBOSE )
      return(0);

   va_start(ap, format);
   vfprintf(stdout, format, ap);
   va_end(ap);

   return(0);
}

/* ========================================================================= */
int VerboseFlush(void)
{
   if ( verbose_value < VL_VERBOSE )
      return(0);

   fflush(stdout);

   return(0);
}

/* ========================================================================= */
int ErrorMessage(const char *format, ...)
{
   va_list ap;

   if ( verbose_value < VL_NORMALV )
      return(0);

   va_start(ap, format);
   vfprintf(stderr, format, ap);
   va_end(ap);

   return(0);
}

/* ========================================================================= */
int DebugMessage(const char *format, ...)
{
   va_list ap;

   if ( verbose_value != VL_DBG_MSG )
      return(0);

   va_start(ap, format);
   vfprintf(stderr, format, ap);
   va_end(ap);

   return(0);
}

/* ========================================================================= */
int SetVerbosityLevel(int vlevel)
{
   int oldvlevel;

   oldvlevel = verbose_value;

   switch ( vlevel )
   {
   case VL_SILENCE:
   case VL_NORMALV:
   case VL_VERBOSE:
   case VL_DBG_MSG:
      verbose_value = vlevel;
      /* No break - This is an intentional fall-through */
   case VL_JSTASKN:
      return(oldvlevel);
   default:
      break;
   }

   return(-1);
}

/* ========================================================================= */
int ReportStart(char *module_name,
                char *module_version,
                char *module_description)
{
   if ( verbose_value < VL_VERBOSE )
      return(0);

   if ( NULL == module_name )
      return(1);

   printf("MicroBench module %s - ", module_name);

   if ( NULL == module_version )
      printf("version %s starting.\n", VERSION_STRING);
   else
      printf("version %s starting.\n", module_version);

   if ( module_description )
      printf("   %s\n", module_description);

   return(0);
}
