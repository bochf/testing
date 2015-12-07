#ifndef MUNGE_H
#define MUNGE_H

#include "gather.h"
#include "options.h"


/* Add the desired extended data into the cpu_list structure */
int InitMungeData(struct options *o, struct cpu_list *c);

int CalcLatest(struct options *o, struct cpu_list *c);

#endif
