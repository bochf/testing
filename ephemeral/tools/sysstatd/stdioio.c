#include <unistd.h>

#include "iobuf.h"

int DumpStdIO(struct iobuf *ob)
{

   write(1, ob->buf, ob->eod);


   return(0);

}




