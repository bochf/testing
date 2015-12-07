#include "endian.h"

/* Can we use this???
#include <byteswap.h>
__bswap_32 (uint32_t input)
__bswap_16 (uint16_t input)
*/

/* ========================================================================= */
int GetEndianValue(void)
{
  int16_t si;
  char *c;

  si = 0x0001;
  c = (char *)&si;
  
  if ( c[1] == 1 )
    return(IS_BIG_ENDIAN);

  return(IS_LITTLE_ENDIAN);
}

/* I had a version with a temp variable. The Internet coughed up this. */
/* ========================================================================= */
int16_t SwapEndian16(int16_t in)
{
   return( ( in << 8 ) | (( in >> 8 ) & 0xFF ) );
}

/* ========================================================================= */
uint16_t SwapEndianU16(uint16_t in)
{
   return( ( in << 8 ) | ( in >> 8 ) );
}

/* ========================================================================= */
int32_t SwapEndian32(int32_t in)
{
   in = ((in << 8) & 0xFF00FF00) | ((in >> 8) & 0xFF00FF ); 
   return (in << 16) | ((in >> 16) & 0xFFFF);
}

/* ========================================================================= */
uint32_t SwapEndianU32(uint32_t in)
{
   in = ((in << 8) & 0xFF00FF00 ) | ((in >> 8) & 0xFF00FF ); 
   return (in << 16) | (in >> 16);
}
