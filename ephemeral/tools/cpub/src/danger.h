#ifndef DANGER_H
#define DANGER_H

/* This file defines all the danger mode limits. */


/* This is the maximum number of active providers */
#define MAX_ACTIVE_PROVIDERS 3

/* This is the maximum quad count */
/* DISABLED!
#define MAX_QUAD_COUNT 10
*/

/* This is the minimum interval time */
/* NOTE: 950 is a recognized threshold for averaging (to a second). It is 
         recognized as a second for averaging purposes. The point of 950 is
         to *intentionally* introduce skew while allowing averaging. The
         averaging threshold does NOT have anything (directly) to do with
         danger mode. Of course you do not average at this level, it is just
         that for averaging purposes 950 ms = 1 s. */
#define MIN_RUN_EVERY 950

#endif
