#ifndef IOBUF_H
#define IOBUF_H

#include <stdlib.h>
#include <stdarg.h>

#define DEFAULT_BUFFER_SIZE 10240
#define MINIMUM_BUFFER_SIZE 2048   /* Just enough to handle errors and/or
                                      any request. */

#define IOBUF_ERR_NONE     0
#define IOBUF_ERR_OVRFLOW  1
#define IOBUF_ERR_NOFILE   2  /* 404: File not found. (currently for the
                                      QC Conr tree) */

/* ========================================================================= */
struct iobuf
{
   size_t size;        /* Static - size of the buffer                        */
   size_t eod;         /* How much data is in the buffer                     */

   unsigned int error; /* Used to pass errors back to the rest of the app    */

   char *buf;          /* The buffer itself                                  */
};

/* =========================================================================
 * Name: NewIOBuf
 * Description: 
 * Paramaters: 
 * Returns: 
 * Side Effects: 
 * Notes: 
 */
struct iobuf *NewIOBuf(size_t size);

/* =========================================================================
 * Name: ResetIOBuf
 * Description: Reset error, EOD (end of data) marker and prepare for new data
 * Paramaters: 
 * Returns: 
 * Side Effects: 
 * Notes: Call this to clean the buffer for a new tree layout
 */
int ResetIOBuf(struct iobuf *iob);

/* =========================================================================
 * Name: IOBufWrite
 * Description: A IOBuf version of sprintf()
 * Paramaters: Same as sprintf() except param 1 is an iobuf struct
 * Returns: Same value as sprintf()
 * Side Effects: Fills iobuf, but also updates the eod value. 
 * Notes: 
 */
int IOBufWrite(struct iobuf *iob, char *fmt, ...);

/* =========================================================================
 * Name: IOBufError
 * Description: Return a simple boolean if an error was experienced
 * Paramaters: 
 * Returns: 1 - an error was encountered, 0 - no errors were encountered
 * Side Effects: None
 * Notes: There is only one error at this time, and that is buffer overflow.
 */
int IOBufError(struct iobuf *iob);

#endif
