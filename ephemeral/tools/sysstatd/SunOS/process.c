#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <procfs.h>

#include "process.h"
#include "../support.h"

int fill_procitem(struct procitem *p);

/* ========================================================================= */
struct procitem *InitProcessEntry(struct procitem *p)
{

   if ( NULL == p )
   {
      if ( NULL == ( p = (struct procitem *)malloc(sizeof(struct procitem)) ) )
      {
         error_msg("ERROR: Failed to allocate memory for process list."); 
         return(NULL);
      }
   }

   p->pid = 0; /* First request should be 0 */

   return(p);
}

/* ========================================================================= */
int StartProcessEntry(struct procitem *p)
{
   assert ( NULL != p );

   /* Used specifically by this implementation */
   p->pid = 0;


   return(0);
}

/* ========================================================================= */
int GetProcessEntry(struct procitem *p)
{
   struct dirent *result;
   int read_again;

   assert ( NULL != p );

   if ( NULL == p->procdir )
   {
      if ( NULL == ( p->procdir = opendir("/proc") ) )
      {
         error_msg("ERROR: Unable to open /proc for process listing.");
         return(1);
      }
   }

   read_again = 1;
   while ( read_again )
   {
      if (readdir_r(p->procdir, &p->direntry, &result))
      {
         error_msg("ERROR: Failed to read directory.");
         read_again = 0;
      }
      else
      {
         /* The readdir_r() call was successful */

         /* Check to see that the pointers match - a proper indication of a
            successful directory read. */
         if ( result == &p->direntry )
         {
            /* NOTE: dirent->d_type is NOT PORTABLE across all Linux
                     filesystems! It is not portable to other Unices... but
                     that is not the problem as this is Linux-only code...
                     The problem is that we cannot rely on this across all
                     Linux implementations. */
            if ( is_numeric(p->direntry.d_name) )
            {
               if ( 0 == fill_procitem(p) )
               {
                  /* All is ok, note that we got one via return value */
                  return(1);
               }

               /* If we are here, we hit a directory that no longer exists.
                  Consider this a non-read, and try again. */
            }
         }
         else
         {
            /* Pointer returned from readdir_r did not match. This means that
               the last directory item was read. We are done here. */
            /* Last item was read */
            p->pid = -1; /* Invalid result */
            read_again = 0;
            closedir(p->procdir);
         }
      } /* if ( readdir() ) else ... */
   } /* while(read_again) */

   return(0);
}




/* ========================================================================= */
int fill_procitem(struct procitem *p)
{
   int fd;
   char filename[256]; /* Stealing length from dirent.h - way longer than required */
   struct psinfo psi;
   ssize_t got;
   int rv = 1;

   /* Build the file name (for psinfo file) */
   sprintf(filename, "/proc/%s/psinfo", p->direntry.d_name);

   /* Save off the pid value */
   p->pid = atol(p->direntry.d_name);

   if ( 0 > (fd = open(filename, O_RDONLY)) )
   {
      return(1);
   }

   got = read(fd, &psi, sizeof(struct psinfo));

   if ( got == sizeof(struct psinfo) )
   {
      strcpy(p->pname, psi.pr_fname);
      p->K_size = psi.pr_size;
      p->K_rss  = psi.pr_rssize;
      p->stime  = psi.pr_start.tv_sec;

      rv = 0;
   }

   close(fd);

   return(rv);
}







