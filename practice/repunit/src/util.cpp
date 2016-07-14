#include <util.h>

#include <cstdint>
#include <cassert>

uint8_t getPair(const uint8_t n, const uint8_t p)
{
  assert(n > 0);
  assert(n < 10);
  assert(n % 2 != 0);
  assert(n % 5 != 0);
  assert(p < 10);

  switch(n) {
    case 1: {
      return p;
    }
    break;
    case 3: {
      switch(p) {
        case 0:
          return 0;
        case 1:
          return 7;
        case 2:
          return 4;
        case 3:
          return 1;
        case 4:
          return 8;
        case 5:
          return 5;
        case 6:
          return 2;
        case 7:
          return 9;
        case 8:
          return 6;
        case 9:
          return 3;
        default:
          assert(0);
      }
    }
    break;
    case 7: {
      switch(p) {
        case 0:
          return 0;
        case 1:
          return 3;
        case 2:
          return 6;
        case 3:
          return 9;
        case 4:
          return 2;
        case 5:
          return 5;
        case 6:
          return 8;
        case 7:
          return 1;
        case 8:
          return 4;
        case 9:
          return 7;
        default:
          assert(0);
      }
    }
    break;
    case 9: {
      return (10 - p) % 10;
    }
    break;
    default:
      assert(0);
  }
}

uint8_t removeOnes(uint64_t &n)
{
  uint8_t result = 0;
  while (n != 0) {
    uint8_t remainder = n % 10;
    if (remainder != 1) {
      break;
    }
    else {
      n /= 10;
      result ++;
    }
  }
  return result;
}
