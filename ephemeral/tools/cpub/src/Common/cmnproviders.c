
#include "cmnproviders.h"
#include "filestatp.h"
#include "fileopsp.h"
#include "mountdf.h"

int CmnPRegister(struct proot *p)
{
   int x;

   x = 0;

   x += FileStatRegister(p);
   x += FileOpsRegister(p);
   x += MountDFRegister(p);

   return(x);
}


