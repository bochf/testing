#ifndef HTTPD_H
#define HTTPD_H

#include "options.h"
#include "datatree.h"
#include "forest.h"

struct httpdsrv
{
   /*** Items read from config file (or other locations) ***/
   struct options *o;

   /*** Items uniquely related to this tread ***/
   int lsock;   /* Listening socket */
   struct sockaddr *laddr;   /* Listening socket */


   pthread_t *tid_list;

   /* This is the queue for the worker/socket */
   pthread_cond_t *qready;
   pthread_mutex_t *qlock;
   int squeue;
   int trun;  /* Not protected - this is a simple flag */


   /* Plant the trees here */
   struct forest *trees;
};

/* =========================================================================
 * Name: HTTPDInit
 * Description: 
 * Paramaters: 
 * Returns: 
 * Side Effects: 
 * Notes: 
 */
struct httpdsrv *HTTPDInit(struct options *o);

/* =========================================================================
 * Name: HTTPDStart
 * Description: 
 * Paramaters: 
 * Returns: 
 * Side Effects: 
 * Notes: 
 */
int HTTPDStart(struct httpdsrv *h);

#endif
