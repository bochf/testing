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
   
   /* The same struct is used in two trees. InitBasic() can be called twice
      without issue. */
   rv |= InitBasic();

   return(rv);
}

/* ========================================================================= */
int RefreshOSSHardwareData(void)
{
   int rv = 0;
   
   /* The same struct / data is used in two trees. It should never be asked
      to refresh twice on a single call. */
   rv |= RefreshBasic();

   return(rv);
}

