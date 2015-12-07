#include "datastruct.h"

/* ========================================================================= */
inline int is_not_loc_intr(struct interrupt *one_intr)
{
   /* Most all will be eliminated on the first test. That is why I wrote
      it with three separate tests. */
   if ( one_intr->tag[0] != 'L' )
      return(1);

   if ( one_intr->tag[1] != 'O' )
      return(1);

   if ( one_intr->tag[2] != 'C' )
      return(1);

   return(0);
}
