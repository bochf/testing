#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "nethelp.h"

/* ========================================================================= */
int ParseType(char *cpproto)
{
   int type = SOCK_RAW;

   if (( cpproto[0] == 'T' ) || ( cpproto[0] == 't' ))
   {
      if (( cpproto[1] == 'C' ) || ( cpproto[1] == 'c' ))
      {
         if (( cpproto[2] == 'P' ) || ( cpproto[2] == 'p' ))
         {
            type = SOCK_STREAM;
         }
      }
   }
   
   if (( cpproto[0] == 'U' ) || ( cpproto[0] == 'u' ))
   {
      if (( cpproto[1] == 'D' ) || ( cpproto[1] == 'd' ))
      {
         if (( cpproto[2] == 'P' ) || ( cpproto[2] == 'p' ))
         {
            type = SOCK_DGRAM;
         }
      }
   }

   return(type);
}

/* ========================================================================= */
char *GetSocktype(char *cpin, struct addrinfo *ai)
{
   switch(ai->ai_socktype)
   {
   case SOCK_STREAM:
      strcpy(cpin, "stream");
      break;
   case SOCK_DGRAM:
      strcpy(cpin, "datagram");
      break;
   case SOCK_SEQPACKET:
      strcpy(cpin, "seqpacket");
      break;
   case SOCK_RAW:
      strcpy(cpin, "raw");
      break;
   default:
      sprintf(cpin, "UNK(%d)", ai->ai_socktype);
      break;
   }

   return(cpin);
}

/* ========================================================================= */
char *GetProtocol(char *cpin, struct addrinfo *ai)
{
   switch(ai->ai_protocol)
   {
   case 0:
      strcpy(cpin, "default");
      break;
   case IPPROTO_TCP:
      strcpy(cpin, "tcp");
      break;
   case IPPROTO_UDP:
      strcpy(cpin, "udp");
      break;
   case IPPROTO_RAW:
      strcpy(cpin, "raw");
      break;
   default:
      sprintf(cpin, "UNK(%d)", ai->ai_protocol);
      break;
   }

   return(cpin);
}

/* ========================================================================= */
char *GetFamily(char *cpin, struct addrinfo *ai)
{
   switch(ai->ai_family)
   {
   case AF_INET:
      strcpy(cpin, "inet");
      break;
   case AF_INET6:
      strcpy(cpin, "inet6");
      break;
   case AF_UNIX:
      strcpy(cpin, "unix");
      break;
   default:
      sprintf(cpin, "UNK(%d)", ai->ai_family);
      break;
   }

   return(cpin);
}

/* ========================================================================= */
ssize_t Recv(int socket, void *buffer, size_t length, int flags)
{
   /* Not as smoothe as Stevens - We could derive some of these values
      rather than keeping redundant variables. */
   ssize_t got;
   ssize_t get;
   ssize_t rv;

   get = length;
   got = 0;

   while(get > 0)
   {
      rv = recv(socket, (char *)((unsigned long)buffer + got), get, flags);

      if ( -1 == rv )
      {
         if ( EINTR != errno )
            return(-1);
      }

      if ( 0 == rv )
         return(0);

      if ( 0 < rv )
      {
         got += rv;
         get = length - got;
      }

      /* A nutty edge condidtion */
      if ( 0 > get )
         return(-1);

   }

   return(got);
}
