#include <cassert>

#include "base_arithmetic.h"

ull convertBaseN(const char * x) {
  const unsigned char maxLength = sizeof(long long) * 8;
  
  unsigned char base10Digit[100];  // buffer of base-10 integer of each digit
  unsigned char length = 0;        // number of digits
  unsigned char baseN = 2;

  const char * p = x;  // point to head of string
  while (*p && length<maxLength) {
    if ('0' <= *p && *p <= '9') {
      base10Digit[length] = *p - '0';
    }
    else if ('A' <= *p && *p <= 'F') {
      base10Digit[length] = *p - 'A' + 10;
    }
    else if ('a' <= *p && *p <= 'f') {
      base10Digit[length] = *p - 'a' + 10;
    }
    else {
      assert(0);
    }
    if (baseN <= base10Digit[length]) {
      baseN = base10Digit[length] + 1;
    }
    ++p;
    ++length;
  }
  
  ull sum = 0;
  for (unsigned char i = 0; i < length; ++i) {
    sum = sum * baseN + base10Digit[i];
  }
  
  return sum;
}
