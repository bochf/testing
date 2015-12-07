#ifndef QCCONR_H
#define QCCONR_H

/* This concept of a "dynamic" or "pass-thru" tree came from proctree.h/c.
   Original implementation notes can be found there.
*/

#include "iobuf.h"
#include "urlparse.h" /* Used primarily for q->format */
#include "options.h"

/* =========================================================================
 * Name: 
 * Description: 
 * Paramaters: 
 * Returns: 
 * Side Effects: 
 * Notes: 
 */
int DynamicQCTree(struct iobuf *ob, struct query *q, struct options *o);


#endif
