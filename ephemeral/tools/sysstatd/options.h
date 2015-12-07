#ifndef OPTIONS_H
#define OPTIONS_H

/* ========================================================================= */
struct options
{
   int bAbout;                /* Show about info and exit                    */
   int bHelp;                 /* Show help/usage info and exit               */
   int bError;                /* There was an error on input                 */
   int bForeground;           /* Run in foreground even when HTTPd           */
   int bDebug;                /* Show some extra debuggery data              */

   char *config_file;
   char *url;
   char *dump_file;

   /* Items validated and passed from the config file */
   char *cf_host;           /* Address to listen on                          */
   char *cf_port;           /* Port to listen on                             */
   int cf_hst;              /* HTTPd server threads                          */
   unsigned long cf_obufsz; /* Output buffer size                            */
   unsigned long cf_ibufsz; /* Input buffer size                             */
   char *cf_msgfile;        /* Where to send errors / messages               */
   char *cf_accessfile;     /* Where to log requests to the server           */
   char *cf_qcfiledir;      /* Where to find the QC results file             */
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
