#ifndef COMPILE_H
#define COMPILE_H

#include <time.h>

#include "options.h"

#define DB_FILENAME "bbhelp.db"

#define DB_VERSION 2

#define DB_ENTRY_START 16      /* Offset from dblocation where entries start */
#define DB_META_START  0
#define DB_META_SIZE   16
#define DB_ENTRY_SIZE  480    /* Size of a single db entry                  */
#define DB_ENTRY_NAME  0      /* Offset of the name entry                   */
#define DB_ENTRY_DESC  96     /* Offset of the description entry            */
#define DB_ENTRY_KEYS  192    /* Offset of the keys entry                   */
#define DB_ENTRY_LOCN  288    /* Offset of the location entry               */
#define DB_ENTRY_PFRM  384    /* Offset of the platform entry               */


/*
  --------------------------------------------------------------------------------- 
  Here is the Version 1 database layout:

  +---------------------+  dblocation + DB_META_START 
  | Metadata            |
  +---------------------+  dblocation + DB_ENTRY_START
  | DB Entry            |
  +---------------------+  dblocation + DB_ENTRY_START + DB_ENTRY_SIZE
  /                     /
  +---------------------+  dblocation + DB_ENTRY_START + (DB_ENTRY_SIZE * n)
  | DB Entry            |
  +---------------------+

  Here is the Metadata layout
  string[6]   magic = "bbhelp"
  byte        endian   1=big ; 0=little
  byte        version  


  Here is the DB entry layout
  string[96]   name    <dbentrystart> + DB_ENTRY_NAME
  string[96]   desc    <dbentrystart> + DB_ENTRY_DESC
  string[96]   keys    <dbentrystart> + DB_ENTRY_KEYS
  string[96]   locn    <dbentrystart> + DB_ENTRY_LOCN
  string[96]   pfrm    <dbentrystart> + DB_ENTRY_PFRM
*/


/* How much memory to reserve - a statistical determination - NOT A MAXIMUM number   */
#define DBV2_META_RESERVE  1024
#define DBV2_ENTRY_RESERVE 1024
/* These are offsets FROM THE BASE MEMORY LOCATION                                   */
#define DBV2_META_START    0
#define DBV2_META_MAGIC    0   /* Offset to magic[6]                                 */
#define DBV2_META_ENDIAN   6   /* Offset to endian byte                              */
#define DBV2_META_VERSION  7   /* Offset to version byte                             */
#define DBV2_META_ENTRIES  8
#define DBV2_META_RSRVD    10  /* Reserved. = 0xFF as additional magic/recognition   */
#define DBV2_META_CTIME    12  /* Offset to compile time signed long (32 bit/4byte)  */

#define DBV2_ENTRY_START   16  /* Same header size as V 1 (offset from file start)   */
/* The remainder are offsets FROM THE ENTRY START POINT                              */
#define DBV2_ENTRY_FIRSTD  20  /* Offset for the first data write                    */
#define DBV2_ENTRY_SIZE    0   /* Entry offset to size uint16_t                      */
#define DBV2_ENTRY_NAME    2   /* Entry offset to name uint16_t                      */
#define DBV2_ENTRY_DESC    4   /* Entry offset to description offset uint16_t        */
#define DBV2_ENTRY_KEYS    6   /* Entry offset to keys offset uint16_t               */
#define DBV2_ENTRY_LOCN    8   /* Entry offset to location offset uint16_t           */
#define DBV2_ENTRY_PFRM    10  /* Entry offset to platform offset uint16_t           */
#define DBV2_ENTRY_OWNR    12  /* Entry offset to owner offset uint16_t              */
#define DBV2_ENTRY_FILE    14  /* Entry offset to filename offset uint16_t           */
#define DBV2_ENTRY_RSRV    16  /* Entry offset to reserved item offset uint16_t      */
#define DBV2_ENTRY_BITF    18  /* Entry offset to bitfield uint16_t                  */
/* These are bitmask options and value defines for the bitfield                      */
/* This is how we encode it, it just so happens that this (currently) matches the
   values defined for the value in the struct. */
#define DBV2_ENTRY_IMPACT_MASK 0x0003 /* Mask for the impact part */
#define DBV2_ENTRY_IMPACT_NONE 0x0000 /* Value for no impact */
#define DBV2_ENTRY_IMPACT_LOWI 0x0001 /* Value for low impact */
#define DBV2_ENTRY_IMPACT_MEDI 0x0002 /* Value for medium impact */
#define DBV2_ENTRY_IMPACT_HIGH 0x0003 /* Value for high impact */
/*
  --------------------------------------------------------------------------------- 
  Here is the Version 2 database layout:

  +---------------------+  dblocation + DBV2_META_START 
  | Metadata            |
  +---------------------+  dblocation + DBV2_ENTRY_START
  | DB Entry            |
  +---------------------+  dblocation + DBV2_ENTRY_START + (last DBV2_ENTRY_SIZE location)
  /                     /
  +---------------------+  There is no formula to get here. You have to walk the entire
  | DB Entry            |         list of items to find this location. You can 'quick walk'
  +---------------------+         by skipping through the DBV2_ENTRY_SIZE items.

  Here is the Metadata layout
  string[6]   magic = "bbhelp"
  byte        endian   1=big ; 0=little
  byte        version  
  time_t      time compiled ---> time(); NOTE: This is int32_t, not 64bit Linux/Darwin/etc..

  Here is the DB entry layout
  +--------+--------+--------+--------+
  |  size of entry  |       20        | <----- The value in the DBV2_ENTRY_NAME location
  +--------+--------+--------+--------+        is always 16. The remaining items vary.
  | offset to desc  |  offset to keys |
  +--------+--------+--------+--------+
  | offset to loc   |  off to uname-s |
  +--------+--------+--------+--------+
  | offset to ownr  | offset to filen |
  +--------+--------+--------+--------+
  | offset to ????  | impact bitfield | <----- 0xDEAD (hard valued for now)
  +--------+--------+--------+--------+
  |   'b'  |   'b'  |   'h'  |   'e'  | <-----  DBV2_ENTRY_NAME references a uint16_t that 
  +--------+--------+--------+--------+         referenes the start of this string
  |   'l'  |   'p'  |   'q'  |    0   |
  +--------+--------+--------+--------+
  |   'A'  |   ' '  |   't'  |   'o'  | <-----  DBV2_ENTRY_DESC references a uint16_t that 
  +--------+--------+--------+--------+         references the start of this string.
  |   'o'  |   'l'  |   ' '  |   'f'  |
  +--------+--------+--------+--------+
  |   'o'  |   'r'  | ...             |
  +--------+--------+--------+--------+

  All strings are NULL terminated - but sizes can be derived - if required
*/





/* mmap() file operations / cases

             existing file
                Y     N
             +-----+-----+
           Y |SWAP |MOVE |
   compile   +-----+-----+
           N |FILE |ANON |
             +-----+-----+

 NOOP - No op - meaning: Not determined yet
 SWAP - Create mmap() of temp file
        Fill the temp file
        Swap old for new, unlink the old
 MOVE - Create mmap() of temp file
        Fill the temp file
        Move temp file into place
 FILE - mmap() an existing file
 ANON - mmap() anonymous/private memory
*/
#define MMFILE_NOOP 0
#define MMFILE_SWAP 1
#define MMFILE_MOVE 2
#define MMFILE_FILE 3
#define MMFILE_ANON 4

/* This is a giant struct (no mallocs for the file name) because I want
   to free all in one call. The void pointer references a mmap() location
   so it does not get free()d. 

   This is the grand unifying struct for all database related calls. It is
   very central to the entire application.

   There will only be one of these at a time.
*/
struct database
{
   char full_dbfilename[256];
   char full_dbtempname[256];
   int reserved_entries;
   int actual_entries;
   int mmfileop;        /* See MMFILE_* defines above */
   void *dblocation;
   size_t dbsize;        /* This is projected, using for mmap() calls... */
   size_t total_written; /* This is the actual ammount of data written */
   time_t tcompiled;
   char dbversion;
};


/* ===========================================================================
 * Name: LoadDatabase
 * Description: Does whatever it takes to load the database
 * Paramaters: Input options struct
 * Returns: database struct (see above)
 * Side effects: Reads in, or loads existing database.
 * Notes: 
 */
struct database *LoadDatabase(struct options *o);

/* ===========================================================================
 * Name: CloseDatabase
 * Description: Closes the db by munmap(), file swaps (if applicable), and free
 * Paramaters: database struct
 *             input options
 * Returns: 
 * Side effects: May replace your (compiled) database file, generates I/O
 * Notes: 
 */
int CloseDatabase(struct database *db, struct options *o);

#endif
