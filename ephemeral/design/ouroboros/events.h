#ifndef EVENTS_H
#define EVENTS_H

#include <pthread.h>

#define EVENT_NORMAL_OPS   0  /* No event, no problems, normal operations    */
#define EVENT_WRITE_DONE   1  /* The writer is done writing all blocks       */
#define EVENT_READ_ERROR   2  /* The reader thread had a read error          */
#define EVENT_WRITE_ERROR  3  /* The writer thread had a write error         */
#define EVENT_SLOW_QFLUSH  4  /* The EOF object could not be placed in time  */ 
#define EVENT_FAIL_QFLUSH  5  /* The EOF object couldn't be placed for an
                                 extended / reasonable period of time.       */

struct tevent
{
   pthread_mutex_t elock;
   pthread_cond_t  econd;
   int             event;
};

/* ========================================================================= */
struct tevent *InitEventFramework(void);

/* ========================================================================= */
int PostEvent(struct tevent *t, int event);

/* ========================================================================= */
int GetNextEvent(struct tevent *t);

#endif
