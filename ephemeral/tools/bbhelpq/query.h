#ifndef QUERY_H
#define QUERY_H

/* compile.h contains the db format info / offsets */
#include "compile.h"
#include "options.h"

/* ===========================================================================
 * Name: HandleQueries
 * Description: All queries are managed in this call
 * Paramaters: 
 * Returns: 
 * Side effects: Dumps to stdout
 * Notes: Exit after calling this (but release the db first)
 *        Will skip out immediately if you are compiling... So you can always
 *        call it, even when not querying (read: compiling).
 */
int HandleQueries(struct database *db, struct options *o); 




char *GetEntryItemOffset(void *el, int ol);



#endif
