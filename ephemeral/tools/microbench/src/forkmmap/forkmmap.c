#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#include "../lib/optparser.h"
#include "../lib/verbosity.h"

#define LEADING_DIR "mmodir"
#define MMO_PREFIX  "FILE"

/* ========================================================================= */
/* This struct is kind of pointless. The mmap code does not use it. Instead
   it relies on the distinctive naming convention of the files - all of which
   is defined and enforced out of this source file. */
struct mmo
{
   char *filename;
   int fd;

   struct mmo *next;
};

/* ========================================================================= */
struct mmo *create_mmo_struct()
{
   struct mmo *m;

   if ( NULL == ( m = (struct mmo *)malloc(sizeof(struct mmo)) ) )
      return(NULL);

   if ( NULL == ( m->filename = (char *)malloc(32) ) )
      return(NULL);

   m->filename[0] = 0;
   m->fd = -1;
   
   return(m);
}

/* ========================================================================= */
int create_mmo_file(struct mmo *m, unsigned long num, unsigned long size)
{
   unsigned char nothing = 0;

   sprintf(m->filename, "%s/%s%06lu.mmo", LEADING_DIR, MMO_PREFIX, num);

   /* Yank anything that might exist */
   unlink(m->filename); /* Don't bother looking at return code */

   if ( -1 == (m->fd = open(m->filename, O_WRONLY | O_CREAT, 0777)) )
      return(1);
      
   if ( -1 == lseek(m->fd, size - 1, SEEK_SET) )
      return(1);

   if (1 != write(m->fd, &nothing, 1))
      return(1);

   close(m->fd);

   return(0);
}


/* ========================================================================= */
int create_mmo_directory(char *dirname)
{
   /* Create the directory if it does not exist */
   printf("Checking for mmap object directory...");
   fflush(stdout);
   if ( 0 != access(dirname, F_OK) )
   {
      printf("None.\nCreating mmap object directory...");
      fflush(stdout);

      if (mkdir(dirname, 0777))
      {
         printf("Failed.\n");
         fflush(stdout);
         return(1);
      }
      else
      {
         printf("Done.\n");
         fflush(stdout);
         return(0);
      }
   }
   printf("Found one.\n");
   fflush(stdout);

   /* Dir exists, make sure that is has proper permissions */

   printf("Checking access on mmap object directory...");
   fflush(stdout);
   if ( 0 != access(dirname, R_OK | W_OK | X_OK) )
   {
      printf("Failed.\n");
      fflush(stdout);
      return(1);
   }

   /* All is OK. */
   printf("Good.\n");
   fflush(stdout);
   return(0);
}

/* ========================================================================= */
/* The point here is simply to round up on the page size. */
unsigned long get_object_size(unsigned long size)
{
   unsigned long pagecnt;

   pagecnt = (size / 4096);

   if ( size % pagecnt > 0 )
      pagecnt++;

   pagecnt++; /* Throw on an extra page. This is masking a bug in the 
                 mmap->touch->munmap sequence. */

   return(pagecnt * 4096);

}


/* ========================================================================= */
struct mmo *create_mmos(unsigned long count, unsigned long size)
{
   struct mmo *mml;
   struct mmo *mmothis;

   unsigned long dotval;

   /* Derive the number to draw a dot on */
   dotval = count / 20;
   
   mml = NULL;

   if ( create_mmo_directory(LEADING_DIR) )
   {
      fprintf(stderr, "ERROR: Unable to create mmap() file object dir.\n");
      return(NULL);
   }

   printf("Creating mmap objects");
   while(count > 0)
   {
      if ( NULL == (mmothis = create_mmo_struct()) )
      {
         printf("Failed.\n");
         fflush(stdout);
         return(NULL);
      }
   
      if ( create_mmo_file(mmothis, count, size) )
      {
         printf("Failed.\n");
         fflush(stdout);
         return(NULL);
      }

      mmothis->next = mml;
      mml = mmothis;

      if ( count % dotval == 0 )
         printf(".");
      fflush(stdout);
      count--;
   }

   printf("Done.\n");
   fflush(stdout);

   return(mml);
}


struct mmlayout
{
   unsigned long pid;
   unsigned long pidnum;
   unsigned long thisop;
};

/* ========================================================================= */
int mmap_ops(unsigned long mmsize, unsigned long fwanted, unsigned long pidnum, unsigned long opsppid)
{
   char filename[32];
   unsigned long filenum;
   int fd;
   struct mmlayout *mmop;
   void *mmloc;
   unsigned long thispid;

   thispid = getpid();

   while(opsppid)
   {
      filenum = ((pidnum + opsppid) % fwanted) + 1;

      sprintf(filename, "%s/%s%06lu.mmo", LEADING_DIR, MMO_PREFIX, filenum);

      if ( -1 == (fd = open(filename, O_RDWR)) )
         return(7);


      if ( MAP_FAILED == (mmloc = (struct mmlayout *)mmap(NULL,
                                                          mmsize,
                                                          PROT_READ | PROT_WRITE,
                                                          MAP_SHARED,
                                                          fd,
                                                          0)))
      {
         return(8);
      }

      close(fd);

      mmop = (struct mmlayout *)((unsigned long)mmloc + (pidnum * sizeof(struct mmlayout)));

      mmop->pid = thispid;
      mmop->pidnum = pidnum;
      mmop->thisop++;

      if ( 0 != munmap(mmloc, 4096) )
      {
         perror("CHILD");
         return(9);
      }

      opsppid--;
   }

   return(0);
}





/* ========================================================================= */
int main(int argc, char *argv[])
{
   unsigned long cwanted;
   unsigned long fwanted;
   unsigned long opsppid;
   struct mmo *mmlist;
   unsigned long pidnum;
   unsigned long mmsize;
   unsigned long ccreated;
   pid_t pid;
   pid_t *pidlist;
   int have_children;
   int rv;


   struct optbase *ob;

   /* Read options into array from input file */
   if(NULL == (ob = ReadOptions(argc, argv)))
      return(1);

   /* Do some basic validation of the expected values */
   if ( IsInvalidOption(ob, "FORK_CHILDREN", IVO_NNNUMERIC) )
   {
      ErrorMessage("ERROR: FORK_CHILDREN option is missing or invalid.\n");
      return(1);
   }

   if ( IsInvalidOption(ob, "MMAP_FILE_COUNT", IVO_NNNUMERIC) )
   {
      ErrorMessage("ERROR: MMAP_FILE_COUNT option is missing or invalid.\n");
      return(1);
   }

   if ( IsInvalidOption(ob, "OPS_PER_PID", IVO_NNNUMERIC) )
   {
      ErrorMessage("ERROR: OPS_PER_PID option is missing or invalid.\n");
      return(1);
   }

   /* Pull in the values (as native types) with range validation */
   if ( GetULValue(ob, &cwanted, "FORK_CHILDREN", 1, USHRT_MAX) )
   {
      ErrorMessage("ERROR: Problems parsing FORK_CHILDREN value.\n");
      return(1);
   }

   if ( GetULValue(ob, &fwanted, "MMAP_FILE_COUNT", 1, LONG_MAX) )
   {
      ErrorMessage("ERROR: Problems parsing MMAP_FILE_COUNT value.\n");
      return(1);
   }

   if ( GetULValue(ob, &opsppid, "OPS_PER_PID", 1, LONG_MAX) )
   {
      ErrorMessage("ERROR: Problems parsing OPS_PER_PID value.\n");
      return(1);
   }

   DebugMessage("Arguments I got         :   %d\n", argc);
   DebugMessage("Processes to create     :   %lu\n", cwanted);
   DebugMessage("Files to create         :   %lu\n", fwanted);
   DebugMessage("Ops per PID             :   %lu\n", opsppid);

   mmsize = get_object_size((cwanted + 1) * sizeof(struct mmlayout));

   if ( NULL == (mmlist = create_mmos(fwanted, mmsize)) )
      return(1);

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
   pidnum = 0;
   while(cwanted > ccreated)
   {
      pid = fork();
      switch( pid )
      {
      case 0:
         if(mmap_ops(mmsize, fwanted, pidnum, opsppid))
            return(5);
         return(0);
      case -1:
         ErrorMessage("Failed to fork(). Giving up.\n");
         return(1);
      default:

         pidlist[ccreated] = pid;
         ccreated++;
         break;
      }

      pidnum++; /* This will be forever frozen in the child after the fork */
    
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

               if ( 0 == WEXITSTATUS(rv) )
                  VerboseMessage("#");
               else
                  VerboseMessage("%d", WEXITSTATUS(rv));

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

































