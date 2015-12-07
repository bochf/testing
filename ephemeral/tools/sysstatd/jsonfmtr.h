#ifndef JSONFMTR_H
#define JSONFMTR_H

#include "iobuf.h"
#include "datatree.h"

#define JSON_INDENT_SIZE 2
#define JSON_INDENT_CHAR ' '

/* =========================================================================
 * Name: 
 * Description: 
 * Paramaters: 
 * Returns: 
 * Side Effects: 
 * Notes: 
 */
int DataToBufJSON(struct iobuf *ob, struct dataitem *dt);

#endif
