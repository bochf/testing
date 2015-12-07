#ifndef MUNGE_H
#define MUNGE_H

#include "datastruct.h"


/* I do basic calculations */
int MungeData(struct interrupts *intr);

/* I do additional calculations required for heat map etc... */
int SecondMunge(struct interrupts *intr);

int MungeCPU(struct interrupts *intr);

#endif
