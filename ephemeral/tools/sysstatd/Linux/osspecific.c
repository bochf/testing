#include "osspecific.h"

/* ========================================================================= */
/* Just a wrapper to call all the sub-functions */
int RefreshOSSCoreData(void)
{
   int rv = 0;

   rv |= RefreshBasic();

   return(rv);
}

/* ========================================================================= */
/* DO NOT expect your data to be correct after the Init. It *may* be for some
   items. The RefreshCoreData() needs to be called to insure that it is
   initialized to the values - not just initialization of the structures. */
int InitOSSCoreData(void)
{
   int rv = 0;
   
   rv |= InitBasic();

   return(rv);
}

/* ========================================================================= */
int InitOSSHardwareData(void)
{
   int rv = 0;
   
   rv |= EnumerateCPUs(); /* Call it here to create the data */

   return(rv);
}

/* ========================================================================= */
int RefreshOSSHardwareData(void)
{
   int rv = 0;
   
   rv |= EnumerateCPUs(); /* Call it here to create the data */

   return(rv);
}
