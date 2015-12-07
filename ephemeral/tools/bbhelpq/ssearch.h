#ifndef SSEARCH_H
#define SSEARCH_H

/* I was scattering the code with various case conversions and case insensitive
   compares - so I moved it all to one file. This is it. */


/* ===========================================================================
 * Name: MakeString
 * Description: Basic wrapper to allocate and fill operation
 * Paramaters: String to replicate
 * Returns: Pointer to a copy of the string
 * Side effects: Allocates memory
 * Notes: See also MakeUCString()
 */
char *MakeString(char *cpin);

/* ===========================================================================
 * Name: MakeUCString
 * Description: Same as MakeString() but converts the string to upper case
 * Paramaters: String to replicate
 * Returns: Pointer to a copy of the string
 * Side effects: Allocates memory
 * Notes: Used to make strings used for case-insensitive compares
 */
char *MakeUCString(char *cpin);

/* ===========================================================================
 * Name: ConvertToLC
 * Description: Convert a string, in place, to lower case
 * Paramaters: String to be modified
 * Returns: Pointer to the same string passed in
 * Side effects: Potentially odifies the string passed
 * Notes:
 */
char *ConvertToLC(char *cpin);

/* ===========================================================================
 * Name: ConvertToUC
 * Description: Convert a string, in place, to lower case
 * Description: Convert a string, in place, to lower case
 * Paramaters: String to be modified
 * Returns: Pointer to the same string passed in
 * Side effects: Potentially odifies the string passed
 * Notes:
 */
char *ConvertToUC(char *cpin);

#endif
