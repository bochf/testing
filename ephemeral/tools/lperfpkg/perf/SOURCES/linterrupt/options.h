#ifndef OPTIONS_H
#define OPTIONS_H

#define DEFAULT_INTERVAL 1

struct options
{
   int bAbout;                /* Show about info and exit                    */
   int bHelp;                 /* Show help/usage info and exit               */
   int bError;                /* There was an error on input                 */
   int bTimestamp;            /* Show a timestamp for each iteration         */
   int bHeader;               /* Show header info or not (default = yes)     */
   int bCollapse;             /* Collapse interrupt lines with no data       */
   int bPivot;                /* Pivot the chart to go vertical              */
   int bMonochrome;           /* Display in monochrome (no-heat map)         */
   int bNames;                /* Display interrupt names (Default: on)       */
   int bShortNames;           /* Display a shortened interrupt name          */
   int bSkipLOC;              /* Skip LOC stats                              */
   int bCPUStats;             /* Show CPU stats                              */

   int bDebug;                /* Show some extra debuggery data              */

   int interval;              /* Time between samplings */
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


#endif
