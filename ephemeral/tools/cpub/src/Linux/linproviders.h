#ifndef LINPROVIDERS_H
#define LINPROVIDERS_H

#include "../providers.h"

/* Public - called to register all Linux providers in one call */
int AllOSRegister(struct proot *p);




/* Private - Available only to the providers */
/* ========================================================================= */
/* About using these functions
   Why?
    - Because opening a /proc file, reading, and parsing is fairly common.
    - Because fscanf() may not be optimal buffer size. (We want this to be
      a SINGLE read - ALWAYS.) We want the read time to MINIMAL.
   Which ones?
    - The best is probably InitReadBuff(), FillReadBuff(), and Kill...().
   Why more than one?
    - The support was initially "stubbed" in with minimal long term thought
      for the single instance that was under development. As needs grew, the
      API matured.
    - The old was kept because it served the existing code... but also because
      it allowed for buffer reuse - and that MIGHT be desired.
   What are some things I should know?
    - These do NOT generate error messages. They do report errors (in the
      return values). The calling function should generate a message if
      appropriate.
      Hint: It is always appropriate to error and not continue.
    - The *struct* buffer is NOT a buffer. It *contains* the buffer. It also
      includes (offset) pointers, size info, and the file name. Unlike a
      FILE "handle", it does not contain an open file descriptor. The file is
      copied into the buffer and the file is immediately closed. You can then
      spend the time required to parse it out - with the file closed.
*/
struct buffer
{
   char *filename;            /* The name of the file to read                */

   char *buf;                 /* Pointer to the data we will work with       */
   unsigned long index;       /* Offset we are working with. Set to 0 when 
                                 the file is read. Incremented as lines are
                                 peeled out of the buffer. This is effectively
                                 a file pointer.                             */
   unsigned long got;         /* Size of the last read (from a file)         */
   unsigned long bsize;       /* Size of the buffer                          */
};

/* ========================================================================= */
/* =============================== USE THESE =============================== */
/* ========================================================================= */


/* =========================================================================
 * Name: InitReadBuff
 * Description: Allocate a read buffer based on a file name
 * Paramaters: Name of the /proc file, how much it may (be expected to) grow
 * Returns: Pointer to buffer structure, or NULL on error
 * Side Effects: Opens file, determines size to allocate buffer
 * Notes: To be used with FillReadBuff()
 */
struct buffer *InitReadBuff(char *filename, int pct_growth);


/* =========================================================================
 * Name: FillReadBuff
 * Description: Fill the buffer with the latest file contents
 * Paramaters: The buffer (filename is in the buffer)
 * Returns: 0 on success, 1 on failure
 * Side Effects: Reads a file (specified in InitReadBuff()) and modifies
 *    the "buffer" struct that was passed.
 * Notes: Can only be used with InitReadBuff()
 */
int FillReadBuff(struct buffer *b);


/* =========================================================================
 * Name: KillReadBuff
 * Description: Free memory associated with the (read) buffer structure
 * Paramaters: The buffer struct to free
 * Returns: Basically nothing
 * Side Effects: Will free() the memory
 * Notes: This is likely never to be called. Cleanup is not really a 
 *        priority as we would then immediately exit.
 */
int KillReadBuff(struct buffer *b);

/* =========================================================================
 * Name: GetNextLine
 * Description: Copy the next line into the first (line) pointer
 * Paramaters: line - buffer to fill
 *             bufs - the buffer structure to read from
 *             line_len - the length of the buffer to fill
 * Returns: 1 on success, 0 on failure / end of file
 * Side Effects: Mofifies the buffer->index value
 *               Will overwrite contents of the "line" buffer
 * Notes: 
 */
int GetNextLine(char *line, struct buffer *bufs, int line_len);

/* =========================================================================
 * Name: PtrNextLine
 * Description: Get a pointer to the next line (instead of copying it)
 * Paramaters: bufs - a buffer structure
 * Returns: Pointer to line, or NULL on end of file
 * Side Effects: Mofifies the buffer->index value
 * Notes: UNTESTED! - Has not been used at this time.
 */
char *PtrNextLine(struct buffer *bufs);

/* ========================================================================= */
/* =============================== NOT THESE =============================== */
/* ========================================================================= */

/* NOTE: These are marked "deprecated" for now... BUT... they may just be
         the most appropriate when a single buffer is used for multiple 
         files - such as a proc.pid.stat provider that uses the same buffer
         to read in more than one file.
*/


/* =========================================================================
 * Name: InitProcBuff   --== DEPRECATED ==--
 * Description: Initialize the struct to buffer input
 * Paramaters: Size of the buffer required
 * Returns: Pointer to the buffer struct
 * Side Effects: Allocates memory
 * Notes: The memory returned is NOT the buffer. It is a struct that
 *        contains the buffer and some pointers used for reading.
 * !!!--> Use only with PullNewBuff()
 */
struct buffer *InitProcBuff(unsigned long size);

/* =========================================================================
 * Name: PullNewBuff   --== DEPRECATED ==--
 * Description: Reads the supplied filename into the buffer
 * Paramaters: Buffer (struct) to fill, filename to read from
 * Returns: 0 on success, non-0 on failure
 * Side Effects: Will overwrite buffer contents with that of the file
 * Notes: Ignores the filename member of the struct.
 * !!!--> Use only with InitProcBuff()
 */
int PullNewBuff(struct buffer *b, char *filename);

/* =========================================================================
 * Name: GetOptimalBufferSize
 * Description: Will determine the optimal buffer size for a file
 * Paramaters: filename and potential growth of the data over its life
 * Returns: The size on success, 0 on failure
 * Side Effects: Opens and reads the file
 * Notes: Several...
 *   1. You cannot simply stat() a file in /proc/ for size. This is a method
 *      to sample the file to get the size. It should NOT be called more than
 *      once on a file - unless something bad happened and we need to re-
 *      establish the file size.
 *   2. The pct_growth paramater is how much you expect the data to
 *      (potentially) grow. This is expreseed as an *integer*. So a 50%
 *      growth would be expressed as 50.
 *   3. The returned size will be rounded up to the "page size" as determined
 *      by the SIMPLE_BUFFER_SIZE #define. THIS IS NOT A TRUE PAGE SIZE. It 
 *      is a size that factors into all pages that might be found on Linux
 *      systems (this includes LoP) that is reasonable for this kind of
 *      allocation / usage. (Currently it is 4k - not too small, not too
 *      large.) The SIMPLE_BUFFER_SIZE is a *multiple* of the end size!
 */
unsigned long GetOptimalBufferSize(char *filename, int pct_growth);





#endif
