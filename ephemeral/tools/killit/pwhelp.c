#include "pwhelp.h"


uid_t GetUIDbyName(char *name)
{
   struct passwd *p;

   if ( NULL == name )
      return(-1);
   
   if ( name[0] == 0 )
      return(-1);

   if (NULL == (p = getpwnam(name)))
      return(-1);

   return(p->pw_uid);
}
