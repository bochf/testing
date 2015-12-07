#ifndef RMETADATA_H
#define RMETADATA_H



#include "fileops.h"
#include "rfileops.h"



/* ========================================================================= */
int DumpMetaDataFile(struct rfile *rf);
struct rfile *LoadMetaDataFile(char *filename);


  /*
    0 +-----+-----+-----+-----+
      | 'R' | 'L' | 'O' | 'G' |
    4 +-----+-----+-----+-----+
      | Ver |BITS |Rtype|DescL|
    8 +-----+-----+-----+-----+
      | Max Files | Next File |
   12 +-----+-----+-----+-----+
      |                       |
      |    Maximum Filesize   |
      |                       |
   20 +-----------+-----------+
      |                       |  <=== This is not derivable if the file
      |    Bytes Received     |       type is timestamp.
      |                       |
   28 +-----------+-----------+
      | File Cnt  |
   32 +-----------+-----------+
      | NextFile  | Filename....
   XX +-----------+-----------+
      | NextFile  | Filename....
   XX +-----------+-----------+
      | NextFile  | Filename....
   XX +-----------+-----------+
      |     0     | Timestamp-->    <=== 4 bytes (not currently used)
   XX +-----------+-----------+
     -->Reserved  | Timestamp-->    <=== 4 bytes (not currently used)
   XX +-----------+-----------+
     -->Reserved  | Description...
   XX +-----------+-----------+

      Ver(sion) = 1
      BITS
       0x80  -
       0x40  -
       0x20  -
       0x10  -
       0x08  -
       0x04  -
       0x02  -
       0x01  - Endian
      RType
        1 = NUMERICAL
	2 = TIMESTAMP
      DescL <== Description length
      Max Files <== Maximum number of files (not linked list members)
      Next File <== Next number to be used. (0 if timestamp)




       Insert
       - Files created (derived, yes);
       - Bytes recvd - Derivable?
       - Bytes dropped
       - Bytes on disk
       - File sizes? Also derivable, yes?



  */

#endif









