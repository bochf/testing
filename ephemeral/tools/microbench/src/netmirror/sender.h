#ifndef SENDER_H
#define SENDER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

/* This is the default operation size if none is specified using the 
   set OPTIMAL_BLOCK = option. */
#define DEFAULT_OPTIMAL_BLOCK 1024

struct roundtrip
{

   /* These are the receiver params */
   struct addrinfo *ralist;
   int rtype;     /* needed? */
   int rprotocol; /* needed? */

   unsigned long multiplier;
   unsigned short opt_block;

   unsigned long run_seconds;

   /* These are the "sink" params */
   struct addrinfo *salist;
   int stype;     /* needed? */
   int sprotocol; /* needed? */
   int sport;

   /* The socket descriptor */
   int sd;



};




/* =========================================================================
 * Name: 
 * Description: Initialize the roundtrip data structure
 * Paramaters: 
 * Returns: 
 * Side Effects: 
 * Notes: 
 */
struct roundtrip *InitRoundTrip(void);

/* =========================================================================
 * Name: InitSender
 * Description: Set up the sender information
 * Paramaters: 
 * Returns: 
 * Side Effects: 
 * Notes: 
 */
int InitSender(struct roundtrip *rt, char *cphost, char *cpproto, char *cpport);

/* =========================================================================
 * Name: 
 * Description: Set up the receiver information
 * Paramaters: 
 * Returns: 
 * Side Effects: 
 * Notes: 
 */
int InitReceiver(struct roundtrip *rt,
                 char *cphost,
                 char *cpproto,
                 char *cpport,
                 unsigned long multiplier);



int StartSender(struct roundtrip *rt);













#endif

