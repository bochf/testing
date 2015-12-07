#ifndef SLURP_H
#define SLURP_H

/* cmditem is an *intermediate* data location. If it is all good, *then*
   we copy it again into place. */
struct cmditem
{
   char name[96];
   char desc[96];
   char keys[96];
   char locn[96];
   char pfrm[96];
   char ownr[96];
   char file[96];

   char impact; /* STUB - currently #defined in compile.h */

   int itemsread;       /* How many single line items read */
   int fileok;          /* If the file was well formatted */
};

/* 0 on successful read */
/* ===========================================================================
 * Name: ReadCmdItemFile
 * Description: Read/parse an input file into a more usable struct
 * Paramaters: verbose (0 if querying, non-0 if compiling)
 * Returns: 0 on success, non-0 on failure
 * Side effects: Reads a file into the cmditem struct
 * Notes: 
 */
int ReadCmdItemFile(struct cmditem *ci, char *filename, int verbose);

#endif
