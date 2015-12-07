#ifndef MAINHELP_H
#define MAINHELP_H

#include <stdarg.h>

/* This file contains functions that are kind of common and might be used
   anywhere. They DO NOT FOLLOW the common (William) convention of beginning
   with a capital letter ("exported" functions == MyFunction(); "local" to
   the file functions == my_function();

   These are used *primarily* by the config file parsing

   Many are used to clean up input lines. They should be called in an order.
   chomp() <----- Trim off (UNIX) EOL marker
                  BEFORE [   some data in a # line \n]
                  AFTER  [   some data in a # line ]
   clamp() <----- Trim off trailing comments
                  BEFORE [   some data in a # line ]
                  AFTER  [   some data in a ]
   wstrim() <---- Trim off trailing white space
                  BEFORE [   some data in a ]
                  AFTER  [   some data in a]
   
*/





/* =========================================================================
 * Name: wstrim
 * Description: Trim whitespace off the end of a line
 * Paramaters: A line to trim
 * Returns: 0 on success, non-0 on failure
 * Side Effects: Will turn the left-most white-space into a string term (0)
 * Notes: This function does not treat EOL as white space. chomp() first
 *        before you call this.
 */
int wstrim(char *line);


/* =========================================================================
 * Name: chomp
 * Description: Based on the perl version of chomp()
 * Paramaters: A line to chomp()
 * Returns: 0 on success, non-0 on failure
 * Side Effects: Will turn the trailing \n's (all of them) to 0's
 * Notes: 
 */
int chomp(char *line);


/* =========================================================================
 * Name: clamp
 * Description: Trim off any traling comments 
 * Paramaters: A line to trim
 * Returns: 0 on success, non-0 on any failure
 * Side Effects: Will turn # (and trailing content) into termination (0)
 * Notes: clamp() will ignore "#" - if it is in (double?) quotes
 */
int clamp(char *line);


/* =========================================================================
 * Name: unlead
 * Description: Give a pointer to the first non-whitespace character 
 * Paramaters: Line to "advance"
 * Returns: Pointer to first non-whitespace
 * Side Effects: None
 * Notes: Here is an example:
 *         Given a pointer to this line: [   Some data]
 *         The pointer to (the same data) is returned: [Some data]
 *        unlead refers to: Remove (un) the lead(ing) white space. 
 */
char *unlead(char *line);


/* =========================================================================
 * Name: StrStr
 * Description: A non-case-sensitive version of strstr() 
 * Paramaters: haystack and needle (like strstr)
 * Returns: Pointer to the *original* data!
 * Side Effects: None - SEE NOTES!
 * Notes: This is a very inefficient variant... It is not a clean/
 *        new implementation. Instead, it allocates a string, converts
 *        it to all upper-case then sends to strstr(). It DOES free the
 *        memory when it is done (no side-effects). The intent here is to 
 *        use this very infrequently (such as when parsing a few items from
 *        an input file).
 * 
 *        Also note: It does pointer math and gives you a pointer to the
 *        original/unmodified data.
 *
 *        If you have to do lots of case insensitive compares... DO NOT USE
 *        THIS. Save a case-specific version of your pattern and then convert
 *        the changing/dynamic to that case for the compare.
 */
char *StrStr(char *haystack, char *needle);

/* =========================================================================
 * Name: StrCmp
 * Description: A non-case-sensitive version of strcmp() 
 * Paramaters: s1 and s2 (like strcmp())
 * Returns: 0 on match, 1 on !match
 * Side Effects: None
 * Notes: This is more efficient than StrStr(), but probably far from optimal.
 *           It is designed to be called more frequently than StrStr().
 *        No memory is allocated.
 *        The strcmp() behaviour of more/less is not honoured. Either you get
 *           a 0 or a 1.
 */
int StrCmp(char *s1, char *s2);

/* =========================================================================
 * Name: mkstring
 * Description: Allocate (malloc()) memory and fill it with a string
 * Paramaters: A string
 * Returns: Pointer to a newly malloc()d string, NULL on malloc() failure
 *          or error
 * Side Effects: Allocates memory
 *        This will print to stderr on a malloc() failure.
 * Notes: This is just a shortcut on a malloc() and strcpy() with error check
 */
char *mkstring(char *cpin);

/* =========================================================================
 * Name: dotdotdot_string || string_dotdotdot
 * Description: Make a string for printing with "least significant" truncated
 * Paramaters: string to modify (what will be printed)
 *             string source (the data to be truncated)
 *             size of the output (including the ...)
 *          NOTE: The size is the size of the string. If it is the 
 * Returns: 0 on success, 1 on failure (will assert() on failure conditions
 *          so checking is unnessary)
 * Side Effects: modifies the "set" string
 * Notes: dotdotdot_string: Truncate the beginning of the line (if necessary)
 *        to fit into the desired line size.
 *        string_dotdotdot: Truncate the end of the line (if necessary) to fit
 *        into the desired line size.
 */
int dotdotdot_string(char *cpset, char *cpsrc, int size);
int string_dotdotdot(char *cpset, char *cpsrc, int size);

/* =========================================================================
 * Name: is_[p]numeric
 * Description: Validate that a string is a numeric
 *              the "p" version does not accept negative values
 * Paramaters: A string
 * Returns: 1 if all numeric, 0 if not (NULL, empty, error == not)
 * Side Effects: None
 * Notes: Call it thusly:
 *     if ( is_numeric(my_string) )
 *     {
 *        printf("It is a number!\n");
 *     }
 *     else
 *     {
 *        printf("It is not a number!\n");
 *     }
 */
int is_numeric(char *cpin);  /* Considers positive and negative as valid */
int is_pnumeric(char *cpin); /* Only considers positive values as valid  */


/* =========================================================================
 * Name: error_msg
 * Description: Write an error message *somewhere*
 * Paramaters: Same args that printf() would use (100 char limit)
 * Returns: 0 on success, non-0 on some failure
 * Side Effects: Will (attempt to) write somewhere
 * Notes:
 *
 * Rules for writing...
    - Do NOT include the CR. One will be added for you. This is intended
      to enforce the concept that you cannot write in parts. Put the whole
      message in one call.
    - Do not include timestamps. Some destinations will stamp for you, others
      will get a timestamp (if appropriate).
    - Include the ERROR:, WARNING:, or NOTE: leading tag (if appropriate).
      one WILL NOT be added for you.

 *  The places this may write:
     - stderr       <------ Supported!
     - act.log      <------ Not supported!
     - syslog       <------ Not supported!
     - a log file   <------ Not supported!
 */
#define ERROR_MSG_STDERR   1
#define ERROR_MSG_LOGFIL   2
#define ERROR_MSG_ACTLOG   4
#define ERROR_MSG_SYSLOG   8

int error_msg(const char *format, ...);


/* =========================================================================
 * Name: ALOpenPipe
 * Description: Open a pipe on act.log (if not already open)
 * Paramaters: The "tag" the application will be using for this log
 * Returns: 0 on success, non-0 on some failure
 * Side Effects: Will signal console.fifo to open a pipe
 * Notes: The "tag" is the name of the fifo and the "app" name in the
 *        act.log line. This is set explicitly by the calling function
 *        because multiple instances of cpub might be running.
 */
#define DEFAULT_ACTLOG_TAG "CPUB"

int ALOpenPipe(char *tag);
int ALPutLine(char *tag, const char *format, ...);
int ALClosePipe(char *tag);


/* =========================================================================
 * Name: file_exists
 * Description: Simple boolean (int) check for a file
 * Paramaters: The filename
 * Returns: 1 if file exists, 0 if not
 * Side Effects: None (will call stat())
 * Notes: Invaild (ie: NULL) input triggers an assert(). When there is an
 *        error, the assumption is that the file exists. The alternative is
 *        to return -1 (or something like that) - and this does not do that.
 *
 *        This is a relatively robust implementation of a file check. BUT it
 *        does not (immediately) detect a file deletion in our "across the
 *        river" NFS implementation. The stat() call succeds when an
 *        immediately following open() will not.
 */
int file_exists(char *filename);

#endif
