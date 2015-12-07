#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sender.h"
#include "nethelp.h"
#include "../lib/verbosity.h"


/* This will be a two part setup.
   First part:
      Establish who / how we will talk to the responder

   Second part:
      Establish what will be sent back
*/


/* ========================================================================= */
struct roundtrip *InitRoundTrip(void)
{
   struct roundtrip *rt;

   if ( NULL == (rt = (struct roundtrip *)malloc(sizeof(struct roundtrip))) )
      return(NULL);

   /* Initialize all values */

   return(rt);
}


/* ========================================================================= */
int InitSender(struct roundtrip *rt, char *cphost, char *cpproto, char *cpport)
{
   int type;
   struct addrinfo hint;
   struct addrinfo *ailist, *aip;
   struct sockaddr_in *sinp;
   const char *addr;
   int e; /* STUB: This does not appear to be used */
   char abuf[INET_ADDRSTRLEN];
   char cpsocktype[24];
   char cpprotocol[24];
   char cpfamily[24];

   /* Parse the proto string */
   if ( SOCK_RAW == ( type = ParseType(cpproto) ) ) /* Raw was our initialized / invalid value */
   {
      ErrorMessage("ERROR: Unable to parse the protocol string.\n");
      return(1);
   }

   /* Build our connection description information */
   hint.ai_flags = AI_CANONNAME;
   hint.ai_family = AF_INET;
   hint.ai_socktype = type;
   hint.ai_protocol = 0;
   hint.ai_addrlen = 0;
   hint.ai_canonname = NULL;
   hint.ai_addr = NULL;
   hint.ai_next = NULL;

   if ( 0 != ( e = getaddrinfo(cphost, cpport, &hint, &ailist) ) )
   {
      ErrorMessage("ERROR: Unable to getaddrinfo() for receiver address/service.\n");
      return(1);
   }

   DebugMessage("%s resolved to:\n", cphost);
   DebugMessage("   %-12s %-16s %-8s %-12s %-12s %-6s\n",
                "name",
                "addr",
                "port",
                "socktype",
                "protocol",
                "family");
   aip = ailist;
   while ( aip )
   {
      sinp = (struct sockaddr_in *)aip->ai_addr;

      addr = inet_ntop(AF_INET, &sinp->sin_addr, abuf, INET_ADDRSTRLEN);

      DebugMessage("   %-12s %-16s %-8d %-12s %-12s %-6s\n",
                   aip->ai_canonname ? aip->ai_canonname : "-",
                   addr,
                   ntohs(sinp->sin_port),
                   GetSocktype(cpsocktype, aip),
                   GetProtocol(cpprotocol, aip),
                   GetFamily(cpfamily, aip));

      aip = aip->ai_next;
   }

   rt->ralist = ailist;
   rt->rtype = type;
   rt->rprotocol = 0;


   return(0);
}

/* ========================================================================= */
int InitReceiver(struct roundtrip *rt, char *cphost, char *cpproto, char *cpport, unsigned long multiplier)
{
   int type;
   struct addrinfo hint;
   struct addrinfo *ailist, *aip;
   struct sockaddr_in *sinp;
   const char *addr;
   int e; /* STUB: This does not appear to be used */
   char abuf[INET_ADDRSTRLEN];
   char cpsocktype[24];
   char cpprotocol[24];
   char cpfamily[24];

   
   if ( 0 == ( rt->multiplier = multiplier ) )
      return(0);

   /* Parse the proto string */
   if ( SOCK_RAW == ( type = ParseType(cpproto) ) ) /* Raw was our initialized / invalid value */
   {
      ErrorMessage("ERROR: Unable to parse the protocol string.\n");
      return(1);
   }

   /* Build our connection description information */
   hint.ai_flags = AI_CANONNAME;
   hint.ai_family = AF_INET;
   hint.ai_socktype = type;
   hint.ai_protocol = 0;
   hint.ai_addrlen = 0;
   hint.ai_canonname = NULL;
   hint.ai_addr = NULL;
   hint.ai_next = NULL;

   if ( 0 != ( e = getaddrinfo(cphost, cpport, &hint, &ailist) ) )
   {
      ErrorMessage("ERROR: Unable to getaddrinfo() for sink address/service.\n");
      return(1);
   }

   DebugMessage("%s resolved to:\n", cphost);
   DebugMessage("   %-12s %-16s %-8s %-12s %-12s %-6s\n",
                "name",
                "addr",
                "port",
                "socktype",
                "protocol",
                "family");
   aip = ailist;
   while ( aip )
   {
      sinp = (struct sockaddr_in *)aip->ai_addr;

      addr = inet_ntop(AF_INET, &sinp->sin_addr, abuf, INET_ADDRSTRLEN);

      DebugMessage("   %-12s %-16s %-8d %-12s %-12s %-6s\n",
                   aip->ai_canonname ? aip->ai_canonname : "-",
                   addr,
                   ntohs(sinp->sin_port),
                   GetSocktype(cpsocktype, aip),
                   GetProtocol(cpprotocol, aip),
                   GetFamily(cpfamily, aip));

      aip = aip->ai_next;
   }

   rt->salist = ailist;
   
   return(0);
}

/* ========================================================================= */
int StartSender(struct roundtrip *rt)
{
   struct sockaddr_in *sa;
   char cpfamily[24];
   int sd;
   int crv; /* connect return value */
   int e;
   uint32_t addr;   /* Address of the sink */
   uint32_t mult;   /* Multiplier (send back mult x packets for each incomming) */
   uint16_t port;   /* Port of the sink server */
   uint16_t opbs;   /* Optimal block size */

   if ( rt->multiplier > 0 )
   {
      VerboseMessage("Collecting header information for return data.\n");

      if ( rt->salist->ai_family != AF_INET )
      {
         /* Doh! We were expecting IPv4.
            The following message is cryptic but that is OK. We will
            likely never encounter it. */
         ErrorMessage("ERROR: family = %s != inet. (Not IPv4 as expected.)\n",
                 GetFamily(cpfamily, rt->salist));
         return(1);
      }

      sa = (struct sockaddr_in *)rt->salist->ai_addr;
      addr = (uint32_t)sa->sin_addr.s_addr;
      mult = htonl(rt->multiplier);
      port = sa->sin_port;
      opbs = htons(rt->opt_block);
   }
   else
   {
      VerboseMessage("The send header will be all 0s. No return trip requested.\n");

      mult = 0;
      addr = 0;
      port = 0;
      opbs = 0;
   }

   /* The TCP case */
   if ( -1 == (sd = socket(rt->ralist->ai_family, rt->ralist->ai_socktype, 0)) )
   {
      ErrorMessage("ERROR: Unable to create a socket for outbound connection.\n");
      return(1);
   }

   if ( -1 == ( crv = connect(sd, rt->ralist->ai_addr, rt->ralist->ai_addrlen) ) )
   {
      e = errno;
      ErrorMessage("ERROR: connect() failed.\n");
      ErrorMessage("       %s\n", strerror(e));
      return(1);
   }

   DebugMessage("send_MULT[%08lX]  ntoh(%08lX)...",
                (unsigned long)mult,
                (unsigned long)ntohl(mult));
   Send(sd, &mult, 4, 0); /* STUB: Return value?! */
   DebugMessage("Done.\n");

   DebugMessage("send_ADDR[%08lX]  ntoh(%08lX)...",
                (unsigned long)addr,
                (unsigned long)ntohl(addr));
   Send(sd, &addr, 4, 0); /* STUB: Return value?! */
   DebugMessage("Done.\n");

   DebugMessage("send_PORT[%04X]      ntoh(%u)...",
                (unsigned int)port,
                (unsigned int)ntohs(port)); 
   Send(sd, &port, 2, 0); /* STUB: Return value?! */
   DebugMessage("Done.\n");

   DebugMessage("send_OPBS[%04X]      ntoh(%u)...",
                (unsigned int)opbs,
                (unsigned int)ntohs(opbs)); 
   Send(sd, &opbs, 2, 0); /* STUB: Return value?! */
   DebugMessage("Done.\n");

   rt->sd = sd;

   return(0);
}

