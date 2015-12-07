#ifndef YAMLFMTR_H
#define YAMLFMTR_H

#include "iobuf.h"
#include "datatree.h"

#define YAML_INDENT_SIZE 2
#define YAML_INDENT_CHAR ' '

/* =========================================================================
 * Name: DataToBufYAML
 * Description: Write a data tree to the output buffer
 * Paramaters: 
 * Returns: 
 * Side Effects: 
 * Notes: This writes a (non-dynamic) tree to the output buffer. If the
 *        write was successful (the buffer was not too small, etc...) then
 *        it can be sent to the client.
 */
int DataToBufYAML(struct iobuf *ob, struct dataitem *dt);

#endif
