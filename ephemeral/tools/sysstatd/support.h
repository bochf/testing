#ifndef SUPPORT_H
#define SUPPORT_H

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


/* ========================================================================= */
#define DEBUG_STUB(...) fprintf(stderr, __VA_ARGS__)

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
 * Notes: chomp() returns a status int. Chomp() returns the line.
 *        Chomp() is designed to be used inline. 
 */
int chomp(char *line);
char *Chomp(char *line);

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

/* STUB: int set_errorto(int newerrorto); */

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
 *
 */
int is_numeric(char *cpin);  /* Considers positive and negative as valid */
int is_pnumeric(char *cpin); /* Only considers positive values as valid  */

/* =========================================================================
 * Name: StrStr
 * Description: A non-case-sensitive version of strstr() 
 * Paramaters: haystack and needle (like strstr)
 * Returns: pointer to location of match, or NULL for no match
 * Side Effects: None
 * Notes: This is a slightly less naive approach than the original code.
 *        This implemetation is NOT CONSISTENT WITH strstr() in that it does
 *        not return haystack if needle is an empty string or if both are
 *        empty strings.
 */
char *StrStr(char *haystack, char *needle);



#endif

