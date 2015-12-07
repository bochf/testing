#ifndef FILEFMT_H
#define FILEFMT_H

/*
  These files contain the keys and basic formatting rules for the source files 
*/


/* This many keys must be read from a file for it to be valid. This is a crappy
   test of a well formatted file. */
#define REQUIRED_KEYS 7

/* Used for pattern matching & template file generation */
#define NAME_LINE_KEY "Name:"
#define DESC_LINE_KEY "Description:"
#define KEYS_LINE_KEY "Keylist:"
#define LOCN_LINE_KEY "Location:"
#define PFRM_LINE_KEY "Platform:"
#define IMPT_LINE_KEY "Impact:"
#define OWNR_LINE_KEY "Owner:"



/* ===========================================================================
 * Name: DumpFileTemplate
 * Description: 
 * Paramaters: 
 * Returns: 
 * Side effects: 
 * Notes: 
 */
int DumpFileTemplate(void);



/* ===========================================================================
 * Name: ParseImpact
 * Description: 
 * Paramaters: 
 * Returns: 
 * Side effects: 
 * Notes: This is used by options.c (for command line input) AND compile.c
 *        (for file input).
 */
#define IMPACT_UNSET  -1    /* Only for the -i option. (when not used) */
#define IMPACT_NONE    0
#define IMPACT_LOW     1
#define IMPACT_MEDIUM  2
#define IMPACT_HIGH    3
#define IMPACT_FAIL    4    /* This means we failed to parse the entry */

int ParseImpact(char *impactstr);


#endif
