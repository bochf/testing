#ifndef DATAHANDLER_H
#define DATAHANDLER_H

#include "options.h"
#include "filefinder.h"

/* ========================================================================= */
/* Name:
 * Description:
 * Input:
 * Returns:
 * Side effects:
 * Notes:
 */
int ClogSession(struct options *o);

/* ========================================================================= */
/* Name:
 * Description:
 * Input:
 * Returns:
 * Side effects:
 * Notes:
 */
int RlogSession(struct options *o);

/* ========================================================================= */
/* Name:
 * Description:
 * Input:
 * Returns:
 * Side effects:
 * Notes:
 */
int ClogAppendSession(struct options *o);

/* ========================================================================= */
/* Name:
 * Description:
 * Input:
 * Returns:
 * Side effects:
 * Notes:
 */
int RlogAppendSession(struct options *o);

#endif
