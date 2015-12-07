
#include "solproviders.h"
#include "kstatprov.h"

int AllOSRegister(struct proot *p)
{
   int x;

   x = 0;

   x += KstatRegister(p);

   return(x);
}


