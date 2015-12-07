#ifndef DYNTREE_H
#define DYNTREE_H

#include "iobuf.h"

/* NOTE: This is support code for the dynamic tree rendering code. Now that
         there is more than one source file / tree that renders dynamic / 
         pass-thru trees, I moved the support code into its own file.

         As a result... these not-so-public functions break my standard
         naming convention.
*/

/* ========================================================================= */
int indent_json(struct iobuf *ob, int indent);

/* ========================================================================= */
int indent_yaml(struct iobuf *ob, int indent);

#endif
