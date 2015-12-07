#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <dirent.h>

#include "cpuinfo.h"
#include "proccmn.h"




/* ========================================================================= */
int is_cpu_dir(char *d_name)
{
   if ( NULL == d_name )
      return(0);

   /* Step through character by character */
   if ( 'c' != d_name[0] )
      return(0);

   if ( 'p' != d_name[1] )
      return(0);

   if ( 'u' != d_name[2] )
      return(0);

   if ( ( '0' > d_name[3] ) || ( '9' < d_name[3] ) )
      return(0);

   return(1);
}

/* ========================================================================= */
int get_physical_id(char *d_name)
{
   FILE *f;
   char filename[64];
   int core_id;

   if ( NULL == d_name )
      return(-2);

   sprintf(filename, "/sys/devices/system/cpu/%s/topology/core_id", d_name);

   if ( NULL == ( f = fopen(filename, "r") ) )
   {
      return(-3);
   }

   if ( 1 != fscanf(f, "%d", &core_id) )
      core_id = -4;

   fclose(f);

   return(core_id);
}

/* ========================================================================= */
int GetSMTValue(void)
{
   DIR *sdsc;
   struct dirent *de;
   int gotid;       /* The last ID we got */
   int targetid;    /* The first non-0 ID we got, and will continue to look for */
   int cntid;       /* The count of targetid */

   if ( NULL == ( sdsc = opendir("/sys/devices/system/cpu") ) )
      return(GSMTV_ERR_NODIR);

   cntid = 0;
   targetid = -1;
   while( NULL != (de = readdir(sdsc)) )
   {
      if ( is_cpu_dir(de->d_name) )
      {
         if ( 0 <= ( gotid = get_physical_id(de->d_name) ) )
         {
            if ( -1 == targetid )
            {
               targetid = gotid;
            }

            if ( targetid == gotid )
               cntid++;
         }
      }
   }

   closedir(sdsc);
   return(cntid);
}





















































unsigned long GetCPUInfo(void *buf, unsigned long bufsz)
{
   return(ReadProcFile(buf, bufsz, "/proc/cpuinfo"));
}

struct cpuinfo *ParseCPUInfo(struct cpuinfo *cpui, void *statbuf)
{





   return(NULL);
}

