#include "endian.h"

int IsBigEndian(void)
{
  short int si;
  char *c;

  si = 0x0001;
  c = (char *)&si;
  
  if ( c[1] == 1 )
    return(1);

  return(0);
}

int IsSameEndian(int file)
{
  int system;

  system = IsBigEndian();

  if ( system == file )
    return(1);

  return(0);
}

char GetEndianValue(void)
{
  if ( IsBigEndian() )
    return(1);

  return(0);
}


uint64_t endian_swap_u64(uint64_t value)
{
   /* This is a "progressive" swapping rather than an "explicit" re-ordering.
      I was HEAVILY "inspired" by someone elses code found on the internet. */
   value = ((value << 8) & 0xFF00FF00FF00FF00 ) | ((value >> 8) & 0x00FF00FF00FF00FF );
   value = ((value << 16) & 0xFFFF0000FFFF0000 ) | ((value >> 16) & 0x0000FFFF0000FFFF );
   return (value << 32) | (value >> 32);
}


