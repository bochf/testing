#ifndef THREADS_H
#define THREADS_H

#include "options.h"
#include "ringbuf.h"

int LaunchThreads(struct options *o, struct ringbuf *rb);




#endif
