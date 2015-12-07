#ifndef WMCOMMON_H
#define WMCOMMON_H

#include <time.h>

#include "../providers.h"
#include "../configfile.h"

/* ========================================================================= */
#define FS_EXISTS 0   /* File exists and is read/write-able                  */
#define FS_LOCKED 1   /* Exists, but cant be read/written                    */
#define FS_NOFILE 2   /* File does not exist but is create-able              */
#define FS_SERROR 3   /* Some sort of error happend - ABORT                  */

int file_state(char *filename);


/* ========================================================================= */
int touch_file(char *filename);



/* =========================================================================
 * Name: factor_data
 * Description: Convert the data into a factored string representation
 * Paramaters: string to fill (char[32])
 *             pitem to represent
 * Returns: 0 on success, 1 on failure
 * Side Effects: Modifies the input string
 * Notes: See the code for error conditions... but basically an error is 
 *        because the value cannot be factored.
 */
int factor_data(char *factstr, struct pitem *pi);

/* =========================================================================
 * Name: LastMidnight
 * Description: Find the most recent (past) midnight
 * Paramaters: None
 * Returns: The unix time_t for the midnight in *local* time
 * Side Effects: None
 * Notes: Used to calculate re-occouring alarms
 */
time_t LastMidnight(void);

/* =========================================================================
 * Name: GetEndianess
 * Description: Determine what endian architecture we are running on
 * Paramaters: None
 * Returns: One of the defines below
 * Side Effects: None
 * Notes: Used primarily by BCD writer (hence the BCDH_ define)
 */
#define BCDH_LITTLE_ENDIAN 1
#define BCDH_BIG_ENDIAN    0
int GetEndianness(void);

#endif
