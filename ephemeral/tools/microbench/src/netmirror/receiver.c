#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "receiver.h"
#include "nethelp.h"

#include "../lib/verbosity.h"

extern int g_stop;

/* ========================================================================= */
int is_all_addr(char *addr)
{
   if ( NULL == addr )
      return(1);

   if ( 0 == strcmp(addr, "*") )
      return(1);

   if ( ( addr[0] == 'A' ) || ( addr[0] == 'a' ) )
   {
      if ( ( addr[1] == 'L' ) || ( addr[1] == 'l' ) )
      {
         if ( ( addr[2] == 'L' ) || ( addr[2] == 'l' ) )
            return(1);
      }
   }

   return(0);
}

/* ========================================================================= */
int mirror_stream(int sd)
{
   uint32_t multiplier;
   uint32_t addr;
   uint16_t port;
   uint16_t bsize;

   ssize_t got;

   /* Used to print the sink address */
   const char *addrstr;
   char abuf[INET_ADDRSTRLEN];

   /* Items for data transfer */
   char *buff;
   int go;

   /* Return trip items */
   int rd;
   struct sockaddr_in sin;
   const struct sockaddr *sa;



   DebugMessage("Connection established.\n");

   /* Fork off the child to do the work */
   switch(fork())
   {
   case 0:
      break;
   case -1:
      exit(1); /* The coward's approach */
      break;
   default:
      return(0); /* This is the parent - go back to listening */
      break;
   }

   /* We are the child - start working */
   got = recv(sd, &multiplier, sizeof(multiplier), 0);

   if ( got == 4 )
   {
      multiplier = ntohl(multiplier);
      DebugMessage("Mirror multiplier is %lu.\n", multiplier);
   }
   else
   {
      ErrorMessage("ERROR: Problems parsing multiplier input to mirror.\n");
      return(1);
   }

   VerboseMessage("Reciever running as a...");
   if ( multiplier )
      VerboseMessage("Mirror.\n");
   else
      VerboseMessage("Sink.\n");
      

   got = recv(sd, &addr, sizeof(addr), 0);

   if ( got == 4 )
   {
      addrstr = inet_ntop(AF_INET,
                          &addr,
                          abuf,
                          INET_ADDRSTRLEN);
      DebugMessage("Sink address is %s.\n", addrstr);
   }
   else
   {
      ErrorMessage("ERROR: Problems parsing sink address input to mirror.\n");
      return(1);
   }
      
   got = recv(sd, &port, sizeof(port), 0);

   if ( got == 2 )
   {
      port = ntohs(port);
      DebugMessage("Sink port is %d.\n", port);
   }
   else
   {
      ErrorMessage("ERROR: Problems parsing sink port input to mirror.\n");
      return(1);
   }
      
   got = recv(sd, &bsize, sizeof(bsize), 0);

   if ( got == 2 )
   {
      bsize = ntohs(bsize);
      DebugMessage("Optimal read/block size is %d.\n", bsize);
   }
   else
   {
      ErrorMessage("ERROR: Problems parsing block size input to mirror.\n");
      return(1);
   }
      
   DebugMessage("Allocating memory for transfer buffer...");
   if ( NULL == (buff = (char *)malloc(bsize)) )
   {
      DebugMessage("Failed.\n");
      return(1);
   }
   DebugMessage("Done.\n");

   rd = -1;  /* Default, assume no return connection */
   if ( multiplier )
   {
      DebugMessage("Setting up return socket...");

      /* STUB: Assumes the TCP case */
      if ( -1 == (rd = socket(AF_INET, SOCK_STREAM, 0)) )
      {
         DebugMessage("Failed.\n");
         ErrorMessage("ERROR: Unable to establish socket for return data.\n");
         return(1);
      }
      DebugMessage("Done.\n");

      sin.sin_family = AF_INET;
      sin.sin_port = port;
      sin.sin_addr.s_addr = addr;
      sa = (struct sockaddr *)&sin;

      DebugMessage("Connecting to sink...");
      if ( 0 != connect(rd, sa, sizeof(sin)) )
      {
         DebugMessage("Failed.\n");
         ErrorMessage("ERROR: Unable to connect to sink.\n");
         close(rd);
         rd = -1;

         /* STUB: return(1); <---- Commented out for now */
      }
      DebugMessage("Done.\n");
   }














   go = 1;
   while ( go )
   {
      got = Recv(sd, buff, bsize, 0);

      if ( -1 == got )
      {
         /* NOTE: Errors here are basically unrecoverable. We are not in
                  situations were the error would be survivable (such
                  as EAGAIN (we block), or EINVAL (no OOB data)). */

         ErrorMessage("ERROR: Unable to recv() on a socket.\n");
         exit(1);
      }

      if ( got == 0 )
      {
         DebugMessage("Detected a shutdown on the connection.\n");
         go = 0;
      }

      if ( got == bsize )
      {
         DebugMessage("+");
         if ( rd >= 0 )
         {
            /* STUB: Values not checked */
            Send(rd, buff, bsize, 0);
            DebugMessage("-");
         }
      }
   }

   DebugMessage("Closing the connection.\n");
   close(sd);

   DebugMessage("Mirror child process is exiting.\n");

   exit(0);
}

/* ========================================================================= */
int RunAsReceiver(char *listen_address, char *listen_protocol, char *listen_port)
{
   int socktype;
   struct addrinfo hint;
   struct addrinfo *ailist, *aip;
   struct sockaddr_in *sinp, bindto;
   const char *addr;
   //char *local_addr;
   int e;
   int sd;
   int cc; /* Client connection from accept() */
   char abuf[INET_ADDRSTRLEN];
   char cpsocktype[24];
   char cpprotocol[24];
   char cpfamily[24];

   if ( is_all_addr(listen_address) )
      listen_address = NULL;

   DebugMessage("Establishing a receiver (mirror) on the following:\n");
   DebugMessage(" Address : %s\n", listen_address == NULL ? "all" : listen_address);
   DebugMessage(" Protocol: %s\n", listen_protocol);
   DebugMessage(" Port    : %s\n", listen_port);

   /* Parse the proto string - Raw is our initialized / invalid value */
   if ( SOCK_RAW == ( socktype = ParseType(listen_protocol) ) )
   {
      ErrorMessage("ERROR: Unable to parse the protocol string.\n");
      return(1);
   }

   if ( listen_address )
   {
      /* Build our listener description information */
      hint.ai_flags = AI_CANONNAME;
      hint.ai_family = AF_INET;
      hint.ai_socktype = socktype;
      hint.ai_protocol = 0;
      hint.ai_addrlen = 0;
      hint.ai_canonname = NULL;
      hint.ai_addr = NULL;
      hint.ai_next = NULL;
      
      if ( 0 != ( e = getaddrinfo(listen_address, listen_port, &hint, &ailist) ) )
      {
         ErrorMessage("ERROR: Unable to getaddrinfo() for receiver address/service.\n");
         return(1);
      }

      DebugMessage("%s resolved to:\n", listen_address);
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

      /* The compiler is worried that some variables are not initialized.
         The successful return from getaddrinfo() tells that there is
         something there. This following statement just clarifies that to
         the compiler. */
      if ( NULL == ailist )
         return(1);

      bindto.sin_family = AF_INET;
      bindto.sin_port = sinp->sin_port;
      bindto.sin_addr = sinp->sin_addr;
   }
   else
   {
      bindto.sin_family = AF_INET;
      bindto.sin_port = htons(atoi(listen_port));
      bindto.sin_addr.s_addr = htonl(INADDR_ANY);
   }

   VerboseMessage("Establishing a listener for the mirror process.\n");

   DebugMessage("Creating a socket...");
   if ( 0 > (sd = socket(AF_INET, socktype, 0)) )
   {
      DebugMessage("Failed.\n");
      ErrorMessage("ERROR: Unable to create socket.\n");
      return(1);
   }
   DebugMessage("Done.\n");
   

   DebugMessage("Binding to a specific port/address...");
   if ( 0 > bind(sd, (struct sockaddr *)&bindto, sizeof(bindto)))
   {
      DebugMessage("Failed.\n");
      ErrorMessage("ERROR: Unable to bind to port %s.\n", listen_port);
      return(1);
   }
   DebugMessage("Done.\n");


   DebugMessage("Beginning to listen for incomming connections...");
   if ( 0 > listen(sd, 1) )
   {
      DebugMessage("Failed.\n");
      ErrorMessage("ERROR: Unable to listen for incomming connections.\n");
      return(1);
   }
   DebugMessage("Done.\n");

   /* STUB: 2nd param to accept() should be used to get address. This can
      STUB:   be used for verbose reporting of the connection. */

   while ( 0 == g_stop )
   {
      /* STUB: This g_stop will not work - on its own. accept() will block,
         STUB:    and we will not pop out of accept() until another connection
         STUB:    is attempted (and handled). */
      if ( 0 <= (cc = accept(sd, NULL, NULL)) )
         mirror_stream(cc);
   }


   DebugMessage("Shutting down the listening connection...");
   close(sd);
   DebugMessage("Done.\n");




















   return(0);
}
