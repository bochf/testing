#ifndef OPTIONS_H
#define OPTIONS_H

#define DIRECT_NONE  0
#define DIRECT_READ  1
#define DIRECT_WRITE 2
#define DIRECT_ALL   DIRECT_READ | DIRECT_WRITE

#define DEFAULT_BUFFER_OBJECTS 10
#define DEFAULT_BUFFER_SIZE    4096

/* ========================================================================= */
struct options
{
   /* Major modes of operation */
   int bAbout;                /* Show about info and exit                    */
   int bHelp;                 /* Show help/usage info and exit               */
   int bError;                /* There was an error on input                 */


   int bForeground;           /* Run in foreground even when HTTPd           */
   int bDebug;                /* Show some extra debuggery data              */

   /* Path name of source and destination items */
   char *source;
   char *dest;

   /* Behivous modification / tunables */
   unsigned int buf_rsize;  /* Size of the buffer ring                       */
   long buf_bsize;          /* Size of a buffer object                       */
   int direct;

   /* verbose, timing, status, etc.... */
   int bVerbose;
   int bStats;

};


/* =========================================================================
 * Name: ReadOptions
 * Description: Convert command line options into a usable options struct
 * Paramaters: The same args we pass to main()
 * Returns: Pointer to options struct, NULL on error
 *          NOTE: error here means malloc() or some such... not user input
 *          error. Check the bError member for user error.
 * Side Effects: Allocates memory
 * Notes: See note in "Returns" above.
 */
struct options *ReadOptions(int argc, char *argv[]);

/* =========================================================================
 * Name: DbgDumpConfig
 * Description: Write config options to stdout (and fflush)
 * Paramaters: Options struct
 * Returns: Nothing of value
 * Side Effects: Writes to stdout
 * Notes: 
 */
int DbgDumpConfig(struct options *o);


#endif
