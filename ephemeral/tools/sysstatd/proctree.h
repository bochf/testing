#ifndef PROCTREE_H
#define PROCTREE_H

#include "iobuf.h"
#include "urlparse.h" /* Used to decode "int format" */

/* NOTE:  proctree.h/c is a special provider. It creates dynamic trees
          and does not have "gettors" for a well defined tree. The 
          solution is to pass the buffer and the format request to the
          dynamc tree function and it will be created directly from the
          data. */

int DynamicProcTree(struct iobuf *ob, int format);



#endif
