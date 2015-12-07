#ifndef SCATTER_H
#define SCATTER_H

#include "gather.h"

/* ========================================================================= */
/* NotDisplayable - A boolean test to see if the user's column and SMT inputs
   are compatible with the actual CPU count.*/
int NotDisplayable(struct options *o, struct cpu_list *c);

/* ========================================================================= */
/* DisplayCPU - Actually displays a screen of CPU info */
int DisplayCPU(struct options *o, struct cpu_list *cpul);


#endif
