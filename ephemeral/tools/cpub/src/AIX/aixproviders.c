
#include "aixproviders.h"
#include "vminfoprov.h"
#include "lpcputprov.h"
#include "processprov.h"
#include "lpdadapter.h"

int AllOSRegister(struct proot *p)
{
   int x;

   x = 0;

   x += VMInfoRegister(p);
   x += LPCPUTRegister(p);
   x += ProcessRegister(p);
   x += LPDiskAdapterRegister(p);

   return(x);
}


