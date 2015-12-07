#ifndef CHEADER_H
#define CHEADER_H

#include <stdint.h>
#include <time.h>
#include "options.h"

/* Header of the c(ircular )log file

   VERSION 1
   +-----+-----+-----+-----+
   | 'C' | 'L' | 'O' | 'G' |
   +-----+-----+-----+-----+
   | Ver |DataS|FBITS|DSize|
   +-----+-----+-----+-----+
   |                       |
   |         HEAD          |
   |                       |
   +-----------------------+
   |                       |
   |         TAIL          |
   |                       |
   +-----------------------+
   |                       |
   |         RECV          |
   |                       |
   +-----------------------+
   |                       |
   |         SIZE          |
   |                       |
   +-----------------------+
   |                       |
   |      First Write      |
   |                       |
   +-----------------------+
   |                       |
   |       Last Write      |
   |                       |
   +-----------------------+

   Ver   - Version of the header
   DataS - Offset that data starts
   FBITS - Flag bits
      0x80 - lseek() failure
	   0x40 -
	   0x20 -
	   0x10 -
      0x08 -
	   0x04 -
	   0x02 -
	   0x01 - Endian: 1 = big, 0 = little
   DSize - Description size
*/
#define FBITS_ENDIAN_MASK 0x01
#define FBITS_ANYERR_MASK 0xF0
#define CLOG_LSEEK_FAIL 0x80

#define OFFSET_MAGIC    0
#define OFFSET_VERSION  4
#define OFFSET_DATASTRT 5
#define OFFSET_ENDIAN   6
#define OFFSET_DESCSZ   7
#define OFFSET_HEADPTR  8
#define OFFSET_TAILPTR  16
#define OFFSET_RECVPTR  24
#define OFFSET_FILSIZE  32
#define OFFSET_FRSTWRT  40
#define OFFSET_LSTWRIT  48
#define OFFSET_MINDATA  56
#define OFFSET_DESCRIP  OFFSET_MINDATA

#define HEADER_VERSION 1
#define POINTER_SIZE   8
#define TIME_SIZE      8

struct cfile
{
  /* Header info */
  unsigned char magic[4];
  unsigned char version;
  unsigned char data_start;
  unsigned char fbits;
  unsigned char description_size;
  char *description;
  int64_t first_write;
  int64_t last_write;

  char *filename;
  /* All of these offsets are in the file - not the data area */
  uint64_t head;            /* Where writing ends               */
  uint64_t tail;            /* Where writing starts             */
  uint64_t here;            /* Where we last wrote to           */
  uint64_t size;            /* Size of the file                 */
  uint64_t recv;            /* Bytes received - required for 
			                      wrap detection                   */

  void *mmcfile;

  int fd;
};

/* =========================================================================
 * Name: NewCFileStruct
 * Description: Create and optionally intialize the cfile struct
 * Paramaters: options struct, and initialize flag
 * Returns: cfile struct pointer on success, NULL on failure
 * Side effects:
 * Notes: 1) Do not fprintf() on error, the function will do it for you.
 *        2) filename is always set (regardless of initialize flag)
 *        3) Magic and version number are also set regardless of flag
 */
#define EMPTY_CFILE_STRUCT 0
#define LOAD_CFILE_STRUCT  1
struct cfile *NewCFileStruct(struct options *o, int initialize);

/* =========================================================================
 * Name: NewCFileHeader
 * Description: Open the file, write the initial header
 * Paramaters:
 * Returns:
 * Side effects:
 * Notes:
 */
int NewCFileHeader(struct cfile *c);
int NewMMCFileHeader(struct cfile *c);

/* =========================================================================
 * Name: ReadCFileHeader
 * Description: Open the file, read the header
 * Paramaters:
 * Returns:
 * Side effects:
 * Notes:
 */
int ReadCFileHeader(struct cfile *c);


int UpdateCFileHeader(struct cfile *c);




#endif
