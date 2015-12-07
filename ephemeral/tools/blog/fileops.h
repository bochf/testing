#ifndef FILEOPS_H
#define FILEOPS_H

#include <sys/types.h>

/* ========================================================================= */
/* Name:
 * Description:
 * Input:
 * Returns:
 * Side effects:
 * Notes:
 */
int OpenNewTrunc(char *filename);

/* ========================================================================= */
/* Name:
 * Description:
 * Input:
 * Returns:
 * Side effects:
 * Notes:
 */
int OpenExistingForWrite(char *filename);

int OpenExistingForMmap(char *filename);




/* OpenAppend is a problem - These files are size limited so simply 
   appending is a problem. lseek() is probably more appropriate */

/* ========================================================================= */
/* Name:
 * Description:
 * Input:
 * Returns:
 * Side effects:
 * Notes: This only happens in ONE case - append in r mode.
 */
int OpenExistAppend(char *filename);

/* ========================================================================= */
/* Name:
 * Description:
 * Input:
 * Returns:
 * Side effects:
 * Notes:
 */
ssize_t Write(int fd, void *buf, size_t n_bytes);

/* ========================================================================= */
/* Name:
 * Description:
 * Input:
 * Returns:
 * Side effects:
 * Notes:
 */
ssize_t Read(int fd, void *buf, size_t n_bytes);

/* ========================================================================= */
/* Name:
 * Description:
 * Input:
 * Returns:
 * Side effects:
 * Notes:
 */
int Unlink(char *filename);

/* ========================================================================= */
/* Name:
 * Description:
 * Input:
 * Returns:
 * Side effects:
 * Notes:
 */
size_t GetFileSize(const char *filename);



#endif
