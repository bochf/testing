#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

#include "../mainhelp.h"
#include "../providers.h"

#include "wmcommon.h"

/* ========================================================================= */
int file_state(char *filename)
{
   /* Bad values should not make it here */
   assert( NULL != filename );
   assert( filename[0] != 0 );

   /* Does the file exist */
   if ( -1 == access(filename, F_OK) )
   {
      if(errno == ENOENT)
         return(FS_NOFILE); /* No file by that name */

      return(FS_SERROR); /* Some kind of error */
   }

   /* File exists. Now see what access we have */
   if ( access(filename, R_OK | W_OK) )
      return(FS_EXISTS);

   /* Fall-through exists !writable */
   return(FS_LOCKED);
}

/* ========================================================================= */
int touch_file(char *filename)
{
   int fd;

   /* Bad values should not make it here */
   assert( NULL != filename );
   assert( filename[0] != 0 );

   if ( -1 == (fd = creat(filename, 0666)) )
      return(1);

   close(fd);

   return(0);
}

/* ========================================================================= */
int factor_data(char *factstr, struct pitem *pi)
{
   int64_t data;
   int64_t sign;

   char FACTOR[] = "BKMGTPEZY";
   int f = 0;
   int e = 0;

   /* Insert into the data value */
   switch ( pi->data_type )
   {
   case PDT_INT8:
      data = *((int8_t *)pi->data_ptr);
      break;
   case PDT_UINT8:
      data = *((uint8_t *)pi->data_ptr);
      break;
   case PDT_INT16:
      data = *((int16_t *)pi->data_ptr);
      break;
   case PDT_UINT16:
      data = *((uint16_t *)pi->data_ptr);
      break;
   case PDT_INT32:
      data = *((int32_t *)pi->data_ptr);
      break;
   case PDT_UINT32:
      data = *((uint32_t *)pi->data_ptr);
      break;
   case PDT_INT64:
      data = *((int64_t *)pi->data_ptr);
      break;
   case PDT_UINT64:
      /* Do not factor if it wont fit into data type */
      if ( *((uint64_t *)pi->data_ptr) >= 0x8000000000000000 )
         return(1);
      data = *((uint64_t *)pi->data_ptr);
      break;
   default:
      return(1); /* other types are not supported */
      /* Unreachable */
      break;
   }

   /* Convert to bytes (if not decimal) */
   switch ( pi->fact_from )
   {
   case FACT_FROM_KBYTES:
      data = data * 1024;
      break;
   case FACT_FROM_4KPAGE:
      data = data * 4096;
      break;
   case FACT_FROM_8KPAGE:
      data = data * 8192;
      break;
   case FACT_FROM_64KPAGE:
      data = data * 65536;
      break;
   case FACT_FROM_BYTES:
   case FACT_FROM_DECIMAL:
   default:
      break;
   }

   /* Some providers may pass signed data. Others may not.
      What this means:
      - Some data may be of signed type, and may be signed.
      - Some data may be of unsigned type, and may have a "sign bit" saved
        in the form of a sign flag. (WHY?! you ask. Because the native data
        type for something like memory is likely an unsigned type. But if we
        do a diff it can go negative. So we save the data in the native type,
        but save the fact that the diff went negative in another flag value)
      So this is what we do:
      - If signed, strip off the sign (will be re-applied later)
      - If unsigned but sign flag is set then keep that sign
      In both cases (out of band sign, and in-band sign) the sign value is
      saved and unsigned data is passed into the process that follows.
   */

   /* Take off the sign */
   sign = 1;
   if ( data < 0 )
   {
      sign = -1;
      data *= -1;
   }

   /* Make note of the sign if it was saved elsewhere */
   if ( pi->sign_flag )
      sign = -1;

   /* Build the string representation */
   if ( pi->fact_to == FACT_TO_AUTOB )
   {
      f = 0;
      while(data >= 10000)
      {
         data = data / 1024;
         f++;
      }

      /* Put the sign back on */
      data *= sign;

      sprintf(factstr, "%lld %c", (long long)data, FACTOR[f]);
      return(0);
   }

   if ( pi->fact_to == FACT_TO_AUTOD )
   {
      e = 0;
      while(data >= 10000)
      {
         data = data / 10;
         e++;
      }

      /* Put the sign back on */
      data *= sign;

      sprintf(factstr, "%lldE%d", (long long)data, e);
      return(0);
   }

   if ( pi->fact_to == FACT_TO_HEX )
   {
      /* Put the sign back on */
      data *= sign;

      sprintf(factstr, "0X%llX", (long long)data);
      return(0);
   }

   return(1);
}

/* ========================================================================= */
time_t LastMidnight(void)
{
   time_t now;
   struct tm *midnight;

   time(&now);

   midnight = localtime(&now);

   midnight->tm_sec = 0;
   midnight->tm_min = 0;
   midnight->tm_hour = 0;

   return(mktime(midnight));

}

/* ========================================================================= */
int GetEndianness(void)
{
   uint16_t ashort;
   char *twobytes;

   ashort = 1;
   twobytes = (char *)&ashort;

   if ( twobytes[0] == 1 )
      return(BCDH_LITTLE_ENDIAN);

   return(BCDH_BIG_ENDIAN);
}
