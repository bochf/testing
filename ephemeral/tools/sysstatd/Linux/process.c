#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <assert.h>
#include <string.h>

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

   p->pid = -1; /* First request should be impossible number */

   p->procdir = NULL; /* Linux uses this as indicator of start */

   return(p);
}

/* ========================================================================= */
int StartProcessEntry(struct procitem *p)
{
   assert ( NULL != p );

   /* Not used specifically by this implementation */
   p->pid = -1;
   /* This is */
   p->procdir = NULL;

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
         /* Even though this is a dynamic and rapidly changing directory,
            we should expect consistent reads here. The following line is
            for debuggery.  
            perror("DEBUG: ");  */
         error_msg("ERROR: Failed to properly read the /proc directory.");
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
unsigned long long parse_status_kbval(char *line)
{
   unsigned long long val;

   val = 0;

   if ( NULL == line )
      return(val);

   if ( 0 == line[0] )
      return(val);

   while (( *line != ':' ) && ( *line != 0 ))
      line++;

   if ( *line == ':' )
      line++;
   else
      return(val);

   while (( *line == ' ' ) || ( *line == '\t' ))
      line++;

   while (( *line >= '0' ) && ( *line <= '9' ))
   {
      val *= 10;
      val += *line - '0';

      line++;
   }

   return(val);
}

/* ========================================================================= */
int fill_procitem(struct procitem *p)
{
   FILE *f;
   char filename[256]; /* Stealing length from dirent.h - way longer than required */
   char line[256];     /* Also extremely large buffer - for CPU line needs */
   char *name;
   struct stat s;

   /* Build the file name (for status file) */
   sprintf(filename, "/proc/%s/status", p->direntry.d_name);

   /* Save off the pid value */
   p->pid = atol(p->direntry.d_name);

   /* There is no "real" start time in seconds in the /proc/PID files.
      The "alternative" is to stat the stat file and get time from there. */
   if( 0 != stat(filename, &s))
      return(1);

   p->stime = s.st_ctim.tv_sec;

   if ( NULL == ( f = fopen(filename, "r") ) )
   {
      /* This is *highly likely* to be an *expected* error condition. The
         directory structure changes regularly as PIDs come and go. The 
         point here is to exit gracefully and move on. There is no need to
         generate an error message / notification.
      */
      return(1);
   }

   while ( fgets(line, 256, f) )
   {
      /* The Name line is rather unique. No elaborate test required */
      if (( line[0] == 'N' ) &&
          ( line[4] == ':' ))
      {
         name = &line[5];
         
         while (( ' ' == *name ) ||( '\t' == *name ))
            name++;

         strcpy(p->pname, name);
         chomp(p->pname);
      }

      /* VmRSS */
      if (( line[0] == 'V' ) &&
          ( line[2] == 'R' ))
      {
         p->K_rss = parse_status_kbval(line);
      }

      /* VmSize */
      if (( line[0] == 'V' ) &&
          ( line[2] == 'S' ) &&
          ( line[3] == 'i' ))
      {
         p->K_size = parse_status_kbval(line);
      }

   }

   fclose(f);

   return(0);
}
