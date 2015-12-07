#ifndef FOREST_H
#define FOREST_H

#include "datatree.h"
#include "urlparse.h"
#include "iobuf.h"
#include "options.h"

/* ========================================================================= */
struct forest
{
   struct dataitem *core;
   struct dataitem *bbenv;
   struct dataitem *hardware;
};

/* =========================================================================
 * Name: PlantForest
 * Description: Create all (or some) of the static trees
 * Paramaters: enum of what trees to create
 *             options struct of app options
 * Returns: A pointer to a forest struct on success. NULL on failure.
 * Side Effects: Allocates memory, may fork/pipe, numerous system calls.
 * Notes: You only "plant" static trees. There is no planting of dynamic
 *        trees. They are rendered on the fly so there is no need to 
 *        initialize these.
 */
#define CLEAR_CUT     0
#define ROOT_TREE     1
#define BBENV_TREE    2
#define PERF_TREE     4
#define HARDWARE_TREE 8
#define RAINFOREST  (ROOT_TREE | BBENV_TREE | PERF_TREE | HARDWARE_TREE)
struct forest *PlantForest(unsigned int stand, struct options *o);

/* =========================================================================
 * Name: GrowMainTree
 * Description: Create the "core" / main tree
 * Paramaters: 
 * Returns: The root of the tree structure
 * Side Effects: Allocates memory, makes syscalls, may fork() for some
 *               system data.
 * Notes: This function calls popen() which means open() with a pipe. This
 *        is hardly ideal, but only happens during the tree initialization.
 */
struct dataitem *GrowMainTree(void);

/* =========================================================================
 * Name: GrowBBEnvTree
 * Description: Create the tree related to the BB Environment
 * Paramaters: 
 * Returns: The root of the tree
 * Side Effects: Allocates memory
 * Notes: 
 */
struct dataitem *GrowBBEnvTree(void);

/* =========================================================================
 * Name: GrowHardwareTree
 * Description: Grow the hardware tree
 * Paramaters: 
 * Returns: The root of the tree
 * Side Effects: 
 * Notes: 
 */
struct dataitem *GrowHardwareTree(void);

/* =========================================================================
 * Name: RenderTree
 * Description: Render the appropriate tree into the output buffer
 * Paramaters: 
 * Returns: 
 * Side Effects: 
 * Notes: This converts the tree request into an actual textual
 *        representation that can be sent by the httpd server.
 */
int RenderTree(struct iobuf *obuf,
               struct query *q,
               struct forest *f,
               struct options *o,
               int *httprc);

#endif
