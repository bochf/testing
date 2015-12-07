#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "sink.h"

#include "../lib/verbosity.h"



/* ========================================================================= */
int daemonize(int reader, int writer)
{
   int i;

   /* Do the first fork */
   if(fork() != 0)
      exit(0);
    
   /* We are now running the child */
   /* Become the session leader */
   setsid();
  
   /* Fork again */
   if(fork() != 0)
      exit(0);
    
   /* Set ourself to a base set of settings */
   chdir("/");
   /* It may be appropriate to chdir("/bb/pm") here... even if it fails. */
   umask(0);

   /* Close any files that may be open */
   i = 0;
   while(i <= 256)
   {
      if ( ( i != reader ) && ( i != writer ) )
         close(i);

      i++;
   }

   /* Ignore HUPs */
   signal(SIGHUP, SIG_IGN);


   return(0);
}

/* ========================================================================= */
int send_parent_msg(int pd, char *msgstr)
{
   return(write(pd, msgstr, strlen(msgstr))); 
}

/* ========================================================================= */
int parent_says_stop(int outof)
{
   struct timeval t;
   fd_set readfds;
   char cp[2];

   t.tv_sec = 0;
   t.tv_usec = 0;

   FD_ZERO(&readfds);
   FD_SET(outof, &readfds);

   select(outof + 1, &readfds, NULL, NULL, &t);

   if ( FD_ISSET(outof, &readfds) )
   {
      read(outof, cp, 1);

      if ( cp[0] == 'S' )
         return(1);
   }

   return(0);
}

/* ========================================================================= */
int SendStopSink(int into)
{
   return(write(into, "S", 1));
}

/* ========================================================================= */
int CheckForChildMessages(int outof,
                          unsigned long mswait,
                          char *cp)
{
   struct timeval t;
   fd_set readfds;
   ssize_t got;

   t.tv_sec = 0;
   t.tv_usec = mswait * (unsigned long)1000;

   FD_ZERO(&readfds);
   FD_SET(outof, &readfds);

   select(outof + 1, &readfds, NULL, NULL, &t);

   if ( FD_ISSET(outof, &readfds) )
   {
      got = read(outof, cp, 80);
      if ( got > 0 )
      {
         cp[got] = 0;
         return((int)got);
      }

      return(0);
   }

   return(0);
}


/* ========================================================================= */
struct pc_comm *LaunchAsSink(char *listen_address,
                             char *listen_protocol,
                             char *listen_port)
{
   struct pc_comm *p;
   int go;
   int bs_counter;

   DebugMessage("Launching a sink process on the following:\n");
   DebugMessage(" Address : %s\n", listen_address);
   DebugMessage(" Protocol: %s\n", listen_protocol);
   DebugMessage(" Port    : %s\n", listen_port);

   if ( NULL == ( p = (struct pc_comm *)malloc(sizeof(struct pc_comm)) ) )
   {
      ErrorMessage("ERROR: Unable to allocate pipe struct.\n");
      return(NULL);
   }

   /* Create a pipe to talk back to our parent */
   if ( pipe(p->msg_pipe) == -1 )
   {
      ErrorMessage("ERROR: Unable to create a message pipe.\n");
      /* This leaks memory - but the ship is going down anyways */
      return(NULL);
   }

   /* Create a pipe to listen for stop commands from parent */
   if ( pipe(p->ctl_pipe) == -1 )
   {
      ErrorMessage("ERROR: Unable to create a message pipe.\n");
      return(NULL);
   }

   /* --- Spawn a child --- */
   if ( fork() != 0 )
   {
      /* We are the parent */
      close(p->msg_pipe[1]); /* We read, not write to msg */
      p->outof = p->msg_pipe[0];
      close(p->ctl_pipe[0]); /* We write, not read from ctl */
      p->into = p->ctl_pipe[1];
      
      return(p); /* Return if we are the parent */
                 /* The child lives on to become a daemon */
   }

   /* We are the child */
   close(p->msg_pipe[0]); /* We write to, not read from msg */
   p->into = p->msg_pipe[1];
   close(p->ctl_pipe[1]); /* We read from, not write to ctl */
   p->outof = p->ctl_pipe[0];

   /* --- Become a daemon --- */
   daemonize(p->into, p->outof);

   /* We are now a daemon - (direct) console messages are not possible. */

   send_parent_msg(p->into, "Properly daemonized.");


   /* Stubbed */
   go = 1;
   bs_counter = 0;
   while ( go )
   {
      sleep(1); /* A stand in for receiving the data */
      
      if ( parent_says_stop(p->outof) )
         go = 0;

      bs_counter++;

      if ( bs_counter % 3 == 0 )
         send_parent_msg(p->into , "Some kind of event happened (this is a test).");
   }

   /* A special message to the parent - exiting */
   send_parent_msg(p->into, ".");

   if ( go == 0 )
      exit(0);

   /* This is unreachable - just to make the compiler happy */

   return(NULL);
}





