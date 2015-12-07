#ifndef OPTPARSER_H
#define OPTPARSER_H

#include <limits.h>
#include <stdint.h>
#include <time.h>

#define MAX_LABEL_SIZE  64
#define MAX_VALUE_SIZE  64

typedef int8_t mb_bool;

/* ========================================================================= */
struct optpair
{
   char *name;
   char *cpvalue;

   struct optpair *next;
};

#define PARSE_GO    0
#define PARSE_EVAL  1
#define PARSE_NONE  PARSE_GO

/* ========================================================================= */
struct optbase
{
   struct optpair *opstart;

   int action;
   int parse_error;
   int parsed_opts;
};

/* =========================================================================
 * Name: ReadOptions
 * Description: Called by your module to read in a standardized config
 * Paramaters: Just pass on what was given by main()
 * Returns: An optbase struct that contains all parameters for the module
 *          or NULL on error
 * Side Effects: Will allocate memory
 * Notes: 
 */
struct optbase *ReadOptions(int argc, char *argv[]);

/* =========================================================================
 * Name: EvalOptions
 * Description: Called after all options have been parsed & retrieved
 * Paramaters: The optbase struct returned by ReadOptions()
 * Returns: Nothing. Calls exit(0) if user supplied "eval" instead of "go"
 * Side Effects: May exit the program
 * Notes: This simply will exit with "success" message if the params were
 *        properly parsed and the user terminated with "eval".
 *
 *        Call it thusly:
 *           ob = ReadOptions(argc, argv);
 *           GetOptionValue(ob, ....);
 *           GetOptionValue(ob, ....);
 *           GetOptionValue(ob, ....);
 *           EvalOptions(ob);   <---- Only after all GetOptionValue() are done.
 */
void EvalOptions(struct optbase *ob);


/* ========================================================================= */
/* ========================== NEW - USE THIS =============================== */
/* ========================================================================= */

/* =========================================================================
 * Name: GetOptionValue
 * Description: Read in a option value to a specific data type
 * Paramaters: optbase - Links you to the data parsed by ReadOptions()
 *             String - The name of the data label to read
 *             int - The data type (see defines below)
 *             <type> pointer - The data item to fill
 *             <type> - Lower acceptable bound
 *             <type> - Upper acceptable bound
 *             <type> - Default data to use if not found. Can be NULL.
 * Returns: 0 on successful parse, exits on failure.
 * Side Effects: May exit(), may call printf(stderr)
 * Notes: limits.h is included in this header for use of MAX and MIN defines
 *        for the upper and lower bounds.
 *
 *        All the void pointers are the same data type as specified in
 *        data_type, UNLESS it is for a string. Then lower_bound and
 *        upper_bound are ints that specify *string length*.
 *
 *        Time parses with a factor (S/M/H/D) but always works
 *        in (positive) seconds.
 *
 *        The call to GetOptionValue() will (conditionally) print a message
 *        to stderr, and will exit() if an error is encountered.
 *
 *        If no key-value pair is found, and default is set to non-NULL,
 *        then the value of the default is used. If default is NULL, and the
 *        key-value pair is not found, then GetOptionValue() will exit() as
 *        an error.
 *
 *        The idea is that GetOptionValue() can be called and the return
 *        value/exception handling is not required, as the call will exit()
 *        and (optionally) handle the error message for you.
 */
#define GOV_NONE     0
#define GOV_UINT8    1
#define GOV_INT8     2
#define GOV_UINT16   3
#define GOV_INT16    4
#define GOV_UINT32   5
#define GOV_INT32    6
#define GOV_UINT64   7
#define GOV_INT64    8
#define GOV_TIME     9
#define GOV_STRING   10   /* char* */
#define GOV_UCSTRING 11   /* char* */
#define GOV_BOOLEAN  12   /* mb_bool */
int GetOptionValue(struct optbase *ob,
                   char *pair_name,
                   int data_type,
                   void *data,
                   void *lower_bound,
                   void *upper_bound,
                   void *default_value);






/* ========================================================================= */
/* ========================== OLD - DEPRECATED ============================= */
/* ========================================================================= */

/* ========================================================================= */
/* These can be or'd - but may not make sense in all combinations */
#define IVO_NONE       0    /* Just check for existence                      */
#define IVO_EXISTS     IVO_NONE
#define IVO_BOOLEAN    1    /* T/F/1/0/true/false - case insensitive         */
#define IVO_NUMERIC    2    /* Contains only (-)0123456789                   */
#define IVO_NNNUMERIC  4    /* Non-negative numeric (a positive number)      */

/* =========================================================================
 * Name: IsInvalidOption
 * Description: Check a named option against one or more test requirements
 * Paramaters: optbase struct returned from ReadOptions()
 *             String - The label (option) name (case insensitive)
 *             int - (#define) IWO_????? test (can be or'd for multiple tests)
 * Returns: non-zero if the test fails, 0 if it passes
 * Side Effects: None
 * Notes: Call it thusly:
 *
 *        if ( IsInvalidOption(ob, "MY_OPTION", IVO_NUMERIC) )
 *        {
 *           ErrorMessage("ERROR: MY_OPTION is not numeric.");
 *           exit(1);
 *        }
 *
 *        The label name is not case sensitive.
 */
int IsInvalidOption(struct optbase *ob, char *label, int test_type);

/* =========================================================================
 * Name: Get[*]Value <---- Multiple functions, similar structure
 * Description: Get the value of a label, store in a specific type
 * Paramaters: optbase - Links you to the data parsed by ReadOptions()
 *             <type> pointer - The data item to fill
 *             String - The name of the data label to read
 *             <type> - Lower acceptable bound
 *             <type> - Upper acceptable bound
 * Returns: 0 on successful parse, 1 on failure (Failure means label was not
 *          found or value was out of range.) The data to be filled is only
 *          modified on a return value of 0.
 * Side Effects: None.
 * Notes: limits.h is included in this header for use of MAX and MIN defines
 *        for the upper and lower bounds.
 *
 *        The GetStrValue() and GetSTRValue() functions return string values.
 *        Str is mixed case (as is), while STR forces the results to all
 *        upper case. The lower and upper_bound values relate to the length
 *        of the string. upper_bound cannot exceed MAX_VALUE_SIZE.
 *
 *        GetTimeValue() parses time with a factor (S/M/H/D) but always works
 *        in (positive) seconds.
 */
int GetULValue(struct optbase *ob,
               unsigned long *setval,
               char *pair_name,
               unsigned long lower_bound,
               unsigned long upper_bound);

int GetUSValue(struct optbase *ob,
               unsigned short *setval,
               char *pair_name,
               unsigned short lower_bound,
               unsigned short upper_bound);

int GetUCValue(struct optbase *ob,
               unsigned char *setval,
               char *pair_name,
               unsigned char lower_bound,
               unsigned char upper_bound);

int GetIValue(struct optbase *ob,
              int *setval,
              char *pair_name,
              int lower_bound,
              int upper_bound);

int GetStrValue(struct optbase *ob,
                char *setval,
                char *pair_name,
                int lower_bound,
                int upper_bound);

int GetSTRValue(struct optbase *ob,
                char *setval,
                char *pair_name,
                int lower_bound,
                int upper_bound);

int GetTimeValue(struct optbase *ob,
                 unsigned long *setval,
                 char *pair_name,
                 unsigned long lower_bound,
                 unsigned long upper_bound);

int GetBoolValue(struct optbase *ob,
                 int *setval,
                 char *pair_name);

#endif
