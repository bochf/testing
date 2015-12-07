#ifndef NETHELP_H
#define NETHELP_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>



int ParseType(char *cpproto);
char *GetSocktype(char *cpin, struct addrinfo *ai);
char *GetProtocol(char *cpin, struct addrinfo *ai);
char *GetFamily(char *cpin, struct addrinfo *ai);


/* Stevens inspired code */
ssize_t Recv(int socket, void *buffer, size_t length, int flags);

/* STUB: Not written yet */
#define Send send

#endif
