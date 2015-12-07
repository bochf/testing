#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

#include "daemonize.h"

/* ========================================================================= */
int MakeDaemon(void)
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
      close(i++);

   /* Ignore HUPs */
   signal(SIGHUP, SIG_IGN);

   return(0);
}

