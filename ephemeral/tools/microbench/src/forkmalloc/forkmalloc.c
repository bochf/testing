#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "../lib/optparser.h"
#include "../lib/verbosity.h"

/* ========================================================================= */
int meg_use(int touch_loop)
{
   unsigned char *cptr;
   unsigned char v;
   int i;

   if(NULL == (cptr = (unsigned char *)malloc(1024 * 10240)))
   {
      /* perror("\nmeg_heap"); */
      return(1);
   }

   v = (unsigned char)touch_loop;
   while ( v > 0 )
   {
      i = 0;
      while ( i < 1024 * 10240 )
      {
         cptr[i] = v;
         i++;
      }
      v--;
   }
   return(0);
}

/* ========================================================================= */
int hog_it_up(unsigned long size, int touch_loop)
{
   while(size > 0)
   {
      meg_use(touch_loop);
      size--;
   }

   return(0);
}

/* ========================================================================= */
int main(int argc, char *argv[])
{
   unsigned long cwanted;
   unsigned long ccreated;
   unsigned long msize;
   pid_t pid;
   pid_t *pidlist;
   int have_children;
   int rv;
   int touch_loop;
   struct optbase *ob;


   if(NULL == (ob = ReadOptions(argc, argv)))
      return(1);


   /* Do some basic validation of the expected values */
   if ( IsInvalidOption(ob, "FORK_CHILDREN", IVO_NNNUMERIC) )
   {
      ErrorMessage("ERROR: FORK_CHILDREN option is missing or invalid.\n");
      return(1);
   }

   if ( IsInvalidOption(ob, "MEG_TO_MALLOC", IVO_NNNUMERIC) )
   {
      ErrorMessage("ERROR: MEG_TO_MALLOC option is missing or invalid.\n");
      return(1);
   }

   if ( IsInvalidOption(ob, "TOUCH_COUNT", IVO_NNNUMERIC) )
   {
      ErrorMessage("ERROR: TOUCH_COUNT option is missing or invalid.\n");
      return(1);
   }

   /* Retrieve the values */
   if ( GetULValue(ob, &cwanted, "FORK_CHILDREN", 1, USHRT_MAX) )
   {
      ErrorMessage("ERROR: Problems parsing FORK_CHILDREN value.\n");
      return(1);
   }

   if ( GetULValue(ob, &msize, "MEG_TO_MALLOC", 10, USHRT_MAX) )
   {
      ErrorMessage("ERROR: Problems parsing FORK_CHILDREN value.\n");
      return(1);
   }

   msize = msize / 10;

   if ( GetIValue(ob, &touch_loop, "TOUCH_COUNT", 1, UCHAR_MAX) )
   {
      ErrorMessage("ERROR: Problems parsing FORK_CHILDREN value.\n");
      return(1);
   }
  
   ReportStart("forkmalloc", NULL, "A module that malloc()s and exercises memory.");
   /* Some reporting of status */
   DebugMessage("Arguments I got         :   %d\n", argc);
   DebugMessage("Processes to create     :   %lu\n", cwanted);
   DebugMessage("10M mallocs to perform  :   %lu\n", msize);
   DebugMessage("Times to touch a byte   :   %d\n", touch_loop);

   if ( NULL == (pidlist = malloc(sizeof(pid_t) * cwanted)) )
   {
      ErrorMessage("ERROR: Unable to allocate memmory for pid list.\n");
      return(1);
   }

   /* zero out the list */
   ccreated = 0;
   while ( ccreated < cwanted )
      pidlist[ccreated++] = 0;
      
   /* Make some children */
   ccreated = 0;
   while(cwanted > ccreated)
   {
      pid = fork();
      switch( pid )
      {
      case 0:
         hog_it_up(msize, touch_loop);
         sleep(4); /* Just hang to hold the memory */
         return(0);
      case -1:
         ErrorMessage("Failed to fork(). Giving up.\n");
         return(1);
      default:

         pidlist[ccreated] = pid;
         ccreated++;
         break;
      }
    
      usleep(1000);
   }
   
   VerboseMessage("Processes created       :   %ld\n", ccreated);
   VerboseMessage("Processes reaped\n");
   VerboseFlush();

   have_children = 1;
   while(have_children)
   {
      have_children = 0;
      ccreated = 0;
      while ( ccreated < cwanted )
      {
         if( pidlist[ccreated] != 0 )
         {
            pid = pidlist[ccreated];
            if (waitpid(pid, &rv, WNOHANG) == pid)
            {
               if (WIFEXITED(rv))
                  pidlist[ccreated] = 0;

               VerboseMessage("#");
               VerboseFlush();
            }
            else
               have_children = 1;
         }
         ccreated++;
      }

   }

   VerboseMessage("\n");
               
   return(0);

}
