#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <assert.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "compile.h"
#include "filefmt.h"
#include "endian.h"
#include "slurp.h"

/* ========================================================================= */
/* Prototyped - so that I do not have to focus on ordering....
   the file is getting kind of large. */
int readv2_metadata(struct database *db);  /* mmap() to struct */
int writev2_metadata(struct database *db); /* struct to mmap() */


/* ========================================================================= */
size_t factor_page_size(size_t requested)
{
   /* Commentary... I can determine page size... or just round up to the
      largest possible size (likely). That is 64K. Because we are dealing
      with a reasonable ammount of memory, and an even smaller rounding
      value, we can use this safely without fear of much wastage. */
   unsigned long pages;

   pages = requested / (64 * 1024);
   if ( pages * (64 * 1024) < requested ) /* resolve rounding errors */
      pages++;

   return(pages * (64 * 1024));
}


/* ========================================================================= */
/* -1 on error */
int count_entries(struct options *o)
{
   DIR *dir;
   struct dirent *de;
   int items;

   if ( NULL == ( dir = opendir(o->bbhelpdir) ) )
      return(-1);

   items = 0;
   while ( NULL != (de = readdir(dir)) )
   {
      if ( 0 == strcmp(de->d_name, ".") ||
           0 == strcmp(de->d_name, "..") ||
           de->d_name == strstr(de->d_name, DB_FILENAME) )
         continue;

      items++;
   }
   
   closedir(dir);
      
   return(items);
}

/* ========================================================================= */
struct database *init_db_struct(void)
{
   struct database *db;

   if ( NULL == ( db = (struct database *)malloc(sizeof(struct database)) ) )
      return(NULL);

   db->reserved_entries = 0;
   db->actual_entries = 0;
   db->mmfileop = MMFILE_NOOP;
   db->dbsize = 0;
   db->total_written = DBV2_ENTRY_START; /* Used for V2+. Start with header size */
   db->dblocation = NULL;
   db->dbversion = DB_VERSION;
   db->full_dbtempname[0] = 0; /* Set to nothing */
   db->full_dbfilename[0] = 0; /* Set to nothing */
   return(db);
}

/* ========================================================================= */
uint16_t drop_in_string(struct database *db, void *thisentry, char *str, unsigned int wheretype)
{
   uint16_t written;
   uint16_t *record_size;   /* DBV2_ENTRY_SIZE (For the whole record) */
   uint16_t *szfield;       /* The size field for this specific write */
   char *writehere;

   written = 0;               /* We have not written anything */
   record_size = thisentry;   /* Find the record size. We use this to calculate
                                 the location of the next write */


   szfield = thisentry + wheretype;               /* Set the *location* of the value  */
   writehere = (char *)(thisentry + *record_size);/* Set the write location           */
   strcpy(writehere, str);                        /* Actually write                   */
   written = strlen(str);                         /* Account for the string size      */
   writehere[written] = 0;                        /* Pedantic */
   written++;                                     /* Account for the NULL termination */
   *szfield = written;                            /* Record how much we wrote         */
   *record_size += written;                       /* Add our latest write to the total*/

   return(written);
}


/* ========================================================================= */
int load_into_memory(struct database *db, struct options *o)
{
   struct dirent *de;
   struct cmditem ci;
   char fullfn[256];
   DIR *dir;
   int items;
   void *thisentry;
   void *lastentry;
   uint16_t *dbe_size;
   uint16_t *dbe_rsrv;
   uint16_t *dbe_bitf; /* Bitfield */

   uint16_t totalwsize;
   uint16_t alignment;

   if ( o->bDebug )
      fprintf(stderr, "DEBUG: Into load_into_memory();\n");

   thisentry = db->dblocation + DBV2_ENTRY_START;
   lastentry = NULL;


   if ( NULL == ( dir = opendir(o->bbhelpdir) ) )
      return(-1);

   items = 0;
   while ( NULL != (de = readdir(dir)) )
   {
      if ( 0 == strcmp(de->d_name, ".") ||
           0 == strcmp(de->d_name, "..") ||
           de->d_name == strstr(de->d_name, DB_FILENAME) )
         continue;

      strcpy(fullfn, o->bbhelpdir);
      strcat(fullfn, "/");
      strcat(fullfn, de->d_name);

      /* The -c(ompile) option sets the verbose flag */
      if ( 0 == ReadCmdItemFile(&ci, fullfn, o->bCompile))
      {
         /* The read call passed, but the file may still be bad */
         if ( (ci.itemsread == REQUIRED_KEYS) && ci.fileok )
         {
            /* I use strcpy because the memory should be nulled out on
               allocation, and this will leave the empty space as all nulls.
               A memcpy may move in un-initialized junk that is messy /
               undesirable. */

            if ( o->bDebug )
            {
               fprintf(stderr, "DEBUG: Adding entry %d into DB.\n", db->actual_entries);
               fprintf(stderr, "DEBUG: thisentry = 0X%lX\n", (unsigned long)thisentry);

            }

            totalwsize = DBV2_ENTRY_FIRSTD; /* Include the header write */
            dbe_size = thisentry + DBV2_ENTRY_SIZE;
            dbe_rsrv = thisentry + DBV2_ENTRY_RSRV;
            dbe_bitf = thisentry + DBV2_ENTRY_BITF;

            *dbe_size = DBV2_ENTRY_FIRSTD; /* Set this so our first write offset 
                                             is at the proper location */
            /* These MUST be called in order */
            totalwsize += drop_in_string(db, thisentry, ci.name, DBV2_ENTRY_NAME);
            totalwsize += drop_in_string(db, thisentry, ci.desc, DBV2_ENTRY_DESC);
            totalwsize += drop_in_string(db, thisentry, ci.keys, DBV2_ENTRY_KEYS);
            totalwsize += drop_in_string(db, thisentry, ci.locn, DBV2_ENTRY_LOCN);
            totalwsize += drop_in_string(db, thisentry, ci.pfrm, DBV2_ENTRY_PFRM);
            totalwsize += drop_in_string(db, thisentry, ci.ownr, DBV2_ENTRY_OWNR);
            totalwsize += drop_in_string(db, thisentry, ci.file, DBV2_ENTRY_FILE);
            *dbe_rsrv = 0xDEAD;

            /* This is kind of brute force... Potentially optimize later */
            *dbe_bitf = 0x0000; /* Clear it */
            *dbe_bitf = ci.impact; /* <---------- It just so happens that values match bitfiled */
            /* Any additional entries in bitfiled and this code will need to change */


            alignment = totalwsize % 4;
            if ( alignment > 0 )
            {
               totalwsize += (4 - alignment);
               *dbe_size += (4 - alignment);
            }

            lastentry = thisentry;
            thisentry = thisentry + totalwsize;

            /* Need to update size in the db struct so we can truncate
               the file on exit. */
            db->total_written += totalwsize;

            items++;
            db->actual_entries++;

            if ( o->bDebug )
               fprintf(stderr, "DEBUG: Done adding entry into DB. Size = %d\n", totalwsize);

         }
      }
   }
   
   closedir(dir);
      

   if ( o->bDebug )
      fprintf(stderr, "DEBUG: Leaving load_into_memory();\n");


   return(items);
}

/* ========================================================================= */
struct database *LoadDatabase(struct options *o)
{
   struct database *db;
   struct stat sb;
   char full_dbfilename[256];
   char full_dbtempname[256];
   size_t totaldbsize;
   int fd;
   int mode;
   char zerobyte = 0;

   if ( o->bDebug )
      fprintf(stderr, "DEBUG: Into LoadDatabase();\n");

   /* ------------------------------------------------------------ */
   assert(o != NULL);
   assert(o->bbhelpdir != NULL);

   if ( NULL == (db = init_db_struct()) )
      return(NULL);

   /* ------------------------------------------------------------ */
   if ( 0 != access(o->bbhelpdir, F_OK) )
   {
      fprintf(stderr, "ERROR: The helpdir \"%s\" does not exist.\n", o->bbhelpdir);
      return(NULL);
   }

   if ( 0 != access(o->bbhelpdir, R_OK | X_OK) )
   {
      fprintf(stderr, "ERROR: Unable to read helpdir \"%s\".\n", o->bbhelpdir);
      return(NULL);
   }

   if ( o->bCompile )
   {
      /* Go ahead and test for write, since we are testing now */
      if ( 0 != access(o->bbhelpdir, W_OK) )
      {
         fprintf(stderr, "ERROR: Unable to write to helpdir \"%s\".\n", o->bbhelpdir);
         return(NULL);
      }
   }      

   /* ------------------------------------------------------------ */
   /*** Let's determine our plan for file operations ***/
   strcpy(full_dbfilename, o->bbhelpdir);
   strcat(full_dbfilename, "/");
   strcat(full_dbfilename, DB_FILENAME);
   /* This is for temp file creation - if needed */
   sprintf(full_dbtempname, "%s.%ld", full_dbfilename, (long)getpid());

   if ( o->bCompile )
   {
      if ( 0 == access(full_dbfilename, F_OK) )
      {
         /* File exists, and we are compiling a new one */
         db->mmfileop = MMFILE_SWAP;
      }
      else
      {
         /* File does not exist, we are compilng a new one */
         db->mmfileop = MMFILE_MOVE;
      }
   }
   else
   {
      if ( 0 == access(full_dbfilename, F_OK) )
      {
         /* File exists, so we will use it */
         db->mmfileop = MMFILE_FILE;
      }
      else
      {
         /* File does not exist, we are runtime compilng a new one */
         db->mmfileop = MMFILE_ANON;
      }
   }

   if ( o->bForceSrc ) /* Forcing a read of source files (runtime compile) */
   {
      /* This is not compatible with -c (and is caught/ignored early)... so
         the only thing we may be flipping here is MMFILE_FILE to MMFILE_ANON. */
      db->mmfileop = MMFILE_ANON;
   }

   /* ------------------------------------------------------------ */
   /*** Determine counts - even if we do not use them */
   db->reserved_entries = count_entries(o);
   totaldbsize = (db->reserved_entries * DBV2_ENTRY_RESERVE) + DBV2_META_RESERVE;

   /* Rounding this up to fix an issue on solaris. It did not fix the issue, but
      I like keeping this a factor of page sizes. */
   totaldbsize = factor_page_size(totaldbsize);

   /* ------------------------------------------------------------ */
   /*** Now do our actual mmap() operations ***/
   if ( db->mmfileop == MMFILE_FILE )
   {
      /* Open an existing file */
      if ( -1 == (fd = open(full_dbfilename, O_RDONLY)) )
      {
         fprintf(stderr, "ERROR: Unable to open database file (%s).\n", full_dbfilename);
         return(NULL);
      }

      if ( -1 == fstat(fd, &sb) )
      {
         fprintf(stderr, "ERROR: Unable to stat database file (%s).\n", full_dbfilename);
         return(NULL);
      }
      db->dbsize = sb.st_size;

      if ( MAP_FAILED == (db->dblocation = mmap(NULL,
                                                db->dbsize,
                                                PROT_READ,
                                                MAP_PRIVATE,
                                                fd,
                                                0)))
      {
         fprintf(stderr, "ERROR: Unable to map the database file.\n");
         return(NULL);
      }

      close(fd);

      /* Read metadata from mmap()ed region into the struct where it is
         commonly referenced. We could use it in the mmap()... but all
         other instances use the struct. */
      if(readv2_metadata(db))
      {
         /* The most basic cleanup - just munmap(), let exit() handle rest */
         /* The error messages were already written by read_metadata() */
         munmap(db->dblocation, db->dbsize);
         free(db);

         return(NULL);
      }
   }

   if ( ( db->mmfileop == MMFILE_SWAP ) || ( db->mmfileop == MMFILE_MOVE ) )
   {
      /* Open a temp file */
      /* These are both compile options, so be verbose */
      printf("Reserving space for %d entries.\n", db->reserved_entries);
      printf("Resulting file size will be %ld bytes.\n", (long)totaldbsize);
      fflush(stdout);

      db->dbsize = totaldbsize;

      /* Copy off the names - for later file swapping */
      strcpy(db->full_dbtempname, full_dbtempname);
      strcpy(db->full_dbfilename, full_dbfilename);

      mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
      if ( -1 == (fd = open(full_dbtempname, O_CREAT | O_RDWR, mode)) )
      {
         fprintf(stderr, "ERROR: Unable to open temp database file (%s).\n", full_dbtempname);
         return(NULL);
      }

      if ( totaldbsize - 1 != lseek(fd, db->dbsize - 1, SEEK_SET) )
      {
         fprintf(stderr, "ERROR: Unable to position file pointer in temp DB file.\n");
         unlink(full_dbtempname);
         close(fd);
         return(NULL);
      }

      if ( 1 != write(fd, &zerobyte, 1) )
      {
         fprintf(stderr, "ERROR: Unable to write to offset in temp DB file.\n");
         unlink(full_dbtempname);
         close(fd);
         return(NULL);
      }

      if ( o->bDebug )
         fprintf(stderr, "DEBUG: mmap(NULL, %lld, PROT_READ|PROT_WRITE, MAP_SHARED, %d, 0);",
                 (long long)db->dbsize,
                 fd);


      if ( MAP_FAILED == (db->dblocation = mmap(NULL,
                                                db->dbsize,
                                                PROT_READ | PROT_WRITE,
                                                MAP_SHARED,
                                                fd,
                                                0)))
      {
         fprintf(stderr, "ERROR: Unable to map the temp database file.\n");
         unlink(full_dbtempname);
         close(fd);
         return(NULL);
      }

      if ( o->bDebug )
         fprintf(stderr, " = 0X%lX\n",
                 (long)db->dblocation);

      
      close(fd);
   }

   if ( db->mmfileop == MMFILE_ANON )
   {
      db->dbsize = factor_page_size(totaldbsize);

      /* mmap anonymous */
      if ( MAP_FAILED == ( db->dblocation = mmap(NULL,
                                                 db->dbsize,
                                                 PROT_READ | PROT_WRITE,
                                                 MAP_PRIVATE | MAP_ANON,
                                                 -1,
                                                 0)))
      {
         fprintf(stderr, "ERROR: Unable to \"allocate\" memory for runtime compile.\n");
         return(NULL);
      }
   }

   /* We only load from individual files - if we are NOT using a DB */
   if ( db->mmfileop != MMFILE_FILE )
      load_into_memory(db, o);


   if ( ( db->mmfileop == MMFILE_SWAP ) || ( db->mmfileop == MMFILE_MOVE ) )
   {
      if ( o->bDebug )
         fprintf(stderr, "DEBUG: msync(0X%lX, %lld, MS_ASYNC);",
                 (long)db->dblocation,
                 (long long)db->dbsize);
                 

      /* Background the sync */
      msync(db->dblocation, totaldbsize, MS_ASYNC);
   }

   if ( o->bDebug )
      fprintf(stderr, "DEBUG: Leaving LoadDatabase();\n");

   return(db);
}

/* ========================================================================= */
int writev2_metadata(struct database *db)
{
   char tmpbyte;
   int16_t tmpshort;
   int32_t tmplong;
   time_t compiletime;

   memcpy(db->dblocation, "bbhelp", 6);       /* Magic */

   tmpbyte = GetEndianValue();
   memcpy(db->dblocation + DBV2_META_ENDIAN, &tmpbyte, 1);   /* Endianness (compiled under) */

   tmpbyte = DB_VERSION;
   memcpy(db->dblocation + DBV2_META_VERSION, &tmpbyte, 1);   /* File version */

   tmpshort = db->actual_entries;
   memcpy(db->dblocation + DBV2_META_ENTRIES, &tmpshort, 2);  /* Entries in this file */

   tmpshort = 0xFACE;
   memcpy(db->dblocation + DBV2_META_RSRVD, &tmpshort, 2);  /* Entries in this file */

   time(&compiletime);
   tmplong = (int32_t)compiletime; /* Cast/assign here - safe until 2038 or so */
   memcpy(db->dblocation + DBV2_META_CTIME, &tmplong, 4);  /* Current time */

   return(0);
}

/* ========================================================================= */
int readv2_metadata(struct database *db)
{
   char magic[6];
   char fileendian;
   char appendian;
   char version;
   int16_t entries;
   int32_t tcompiled;

   memcpy(magic, db->dblocation, 6);       /* Magic */
   if ( ( magic[0] != 'b' ) ||
        ( magic[1] != 'b' ) ||
        ( magic[2] != 'h' ) ||
        ( magic[3] != 'e' ) ||
        ( magic[4] != 'l' ) ||
        ( magic[5] != 'p' ) )
   {
      fprintf(stderr, "ERROR: The database file is not recognized. (Bad magic)\n");
      return(1);
   }

   appendian = GetEndianValue();
   memcpy(&fileendian, db->dblocation + DBV2_META_ENDIAN, 1);   /* Endianness (compiled under) */

   if ( appendian != fileendian )
   {
      fprintf(stderr, "ERROR: The database file is not supported. (Endian mismatch)\n");
      return(1);
   }

   memcpy(&version, db->dblocation + DBV2_META_VERSION, 1);   /* File version */
   if ( version != DB_VERSION )
   {
      fprintf(stderr, "ERROR: The database file is not supported. (Version mismatch)\n");
      return(1);
   }

   memcpy(&entries, db->dblocation + DBV2_META_ENTRIES, 2);  /* Entries in this file */
   db->actual_entries = entries;

   memcpy(&tcompiled, db->dblocation + DBV2_META_CTIME, 4);  /* time_t we compiled the file */
   db->tcompiled = tcompiled;
   
   return(0);
}
   
/* ========================================================================= */
int CloseDatabase(struct database *db, struct options *o)
{
   int fd;

   /* Check our input */
   assert(NULL != db);
   assert(NULL != o);

   if ( o->bDebug )
      fprintf(stderr, "DEBUG: Into CloseDatabase();\n");


   /* Write out the header info - As long as we did not mmap() DB or anonymous */
   /* The mmap()d DB is read-only. The anonymous mapping will be deleted */
   if (( db->mmfileop != MMFILE_FILE ) && ( db->mmfileop != MMFILE_ANON ))
   {
      writev2_metadata(db);
   }

   /* First, lets resolve the mapping */
   munmap(db->dblocation, db->dbsize);


   /* If we compiled, then truncate to actual size */
   if (( db->mmfileop == MMFILE_SWAP ) || ( db->mmfileop == MMFILE_MOVE ))
   {
      printf("Trimming underlying file to appropriate size.\nOpening file...");
      if ( -1 != (fd = open(db->full_dbtempname, O_WRONLY)) )
      {
         printf("Done.\nTruncating file...");
         if ( -1 == ftruncate(fd, db->total_written) )
         {
            printf("Failed.\n");
            fprintf(stderr, "ERROR: Unable to truncate temp compile file.\n");
            close(fd);
            unlink(db->full_dbtempname);
            free(db);
            return(1);
         }
         
         printf("Done.\nClosing file...");
         close(fd);
         printf("Done.\n");
      }
      else
      {
         printf("Failed.\n");
         fprintf(stderr, "ERROR: Unable to re-open temp compile file for truncate.\n");
         unlink(db->full_dbtempname);
         free(db);
         return(1);
      }
   }

   /* Some file manipulation */
   if ( db->mmfileop == MMFILE_SWAP ) 
   {
      if(0 == unlink(db->full_dbfilename))
      {
         rename(db->full_dbtempname, db->full_dbfilename);
      }
      else
      {
         fprintf(stderr, "ERROR: Failed to remove old database.\n");
         unlink(db->full_dbfilename);
         free(db);
         return(1);
      }
   }

   if ( db->mmfileop == MMFILE_MOVE )
   {
      if(0 != rename(db->full_dbtempname, db->full_dbfilename))
      {
         fprintf(stderr, "ERROR: Failed to rename new database.\n");
         unlink(db->full_dbtempname);
         free(db);
         return(1);
      }

   }

   /* Free the database struct */
   free(db);

   return(0);
}
