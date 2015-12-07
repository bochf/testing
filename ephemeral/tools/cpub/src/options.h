#ifndef OPTIONS_H
#define OPTIONS_H

/* NOTE - Command line options are *intentionally light*. Most items are
          passed through the config file. */

/* conf_args (Config File Arguments) are passed via the command line *after*
   the config file. They may or may not have dashes... the point is that they
   are passed raw and substituted for $ARGx arguments in the config file.
*/
struct conf_args
{
   char *argv;

   struct conf_args *next;
};

struct options
{
   int bAbout;                /* Show about info and exit                    */
   int bHelp;                 /* Show help/usage info and exit               */
   int bError;                /* There was an error on input                 */
   int bVerbose;              /* Probably not used!!!                        */
   int bDanger;               /* User acknowledges danger                    */
   int bListProv;             /* List providers and exit                     */
   int bListAllD;             /* List all data items and exit                */
   int bListPD;               /* List all data items for (*provider)         */
   int bListTypes;            /* Modifies list functionality to include type */

   char *provider;            /* Provider name for bListPD                   */
   char *confile;             /* The name of the config file                 */

   struct conf_args *calist;  /* The list of post config file arguments      */
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
 * Name: GetConfArgument
 * Description: Return (a reference to) the Nth argument
 * Paramaters: Filled options struct, Argument number (0 is config file)
 * Returns: Pointer to char string or NULL on not found. (such as index out
 *          of bound).
 * Side Effects: None
 */
char *GetConfArgument(struct options *o, int argn);


#endif
