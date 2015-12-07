#ifndef GATHER_H
#define GATHER_H

#include "datastruct.h"
#include "options.h"

/* Call me *once* to set up the data structures */
struct interrupts *InitIntrData(struct options *o);

/* Call me to refresh (and rotate) the data */
int SampleStats(struct interrupts *intr);

#endif
