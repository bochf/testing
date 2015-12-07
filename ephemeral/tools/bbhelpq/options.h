#ifndef OPTIONS_H
#define OPTIONS_H

#define DEFAULT_BBHELPDIR "/usr/local/share/bbhelpdir"

/* Options for the -l (qList) option */
#define LIST_NONE 0
#define LIST_NAME 1
#define LIST_KEYS 2
#define LIST_PLFM 3
#define LIST_DESC 4
#define LIST_PERR 5 /* Parse error */

struct options
{
   int bDumpTemplate;  /* -n */
   int bVersion;       /* -v */
   int iHelp;          /* -h(h)(h) */
   int bCompile;       /* -c */
   int bDebug;         /* -+  <---- Undocumented */
   int bError;         /* ERROR (unrecognized option) */
   int iImpact;        /* -i */
   int bDecompile;     /* -o */
   int bForceSrc;      /* -r */
   int qList;          /* -l */
   char *tooldump;     /* -t -- Acts as flag and string holder */
   char *systype;      /* -p -- toupper(uname -s); - Used to filter results */
   char *keycons;      /* -k -- Key constraint. Acts as flag and string  */

   /* Not settable from command line, but derived in ReadOptions() */
   char *bbhelpdir;    /* No trailing slash */
};

/* ===========================================================================
 * Name: ReadOptions
 * Description: Same old function, different application
 * Paramaters: 
 * Returns: See the above critical struct
 * Side effects: mallocs
 * Notes: 
 */
struct options *ReadOptions(int argc, char *argv[]);

#endif
