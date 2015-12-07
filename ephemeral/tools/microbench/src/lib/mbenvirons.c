
#ifdef PORT_SunOS
#define _XOPEN_SOURCE 600
#define __EXTENSIONS__
#endif

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "mbenvirons.h"
#include "verbosity.h"

/* ========================================================================= */
int ZombieRepellent(void)
{
   int rv;
   struct sigaction act;

   VerboseMessage("Disabling zombie children...");
   VerboseFlush();

   memset (&act, 0, sizeof (struct sigaction));
   act.sa_flags = SA_NOCLDWAIT;

   /* Advanced Unix Programming 2nd ed. tells me that sigaction() is more
      portable than just ignoring SIGCHLD signals. */
   /* Param 1: The signal
      Param 2: The action to perform on that signal
      Param 3: What it was previously
   */
   rv = sigaction(SIGCHLD, &act, NULL);

   VerboseMessage("Done.\n");

   return(rv);
}

