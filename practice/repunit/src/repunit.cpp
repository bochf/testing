#include <repunit.h>
#include <util.h>

#include <cstdint>
#include <iostream>

using namespace std;

uint32_t leastK(const uint64_t n)
{
  /// validate input
  if ((n % 2 == 0) || (n % 5 == 0)) {
    // gdc(n, 10) = 1
    return 0;
  }

  uint32_t k = 0;  /// number of "1" in the repunit
  uint64_t currentSum = 0;  /// the sum of current m * n / 10 ^ k
  /// n is the given number, which m * n is a repunit
  /// m is the multiplicator we try to construct,
  /// let m = a1 + a2*10 + a3*10^2 + a4*10^3 + ..., a[i] is in [0, 9]
  /// if m * n is a repunit, (a[i] * n % 10 + currentSum) % 10 = 1
  /// So we can get a[i] based on the last digit of a[i] * n


  bool isRepunit = false;  /// do we get the repunit of m * n

  uint8_t ones = n % 10;
  while (!isRepunit) {
    uint8_t p = (11 - currentSum % 10) % 10;
    uint8_t a = getPair(ones, p);

    currentSum += n * a;
    k += removeOnes(currentSum);
    /*
    cout << "(" << k << ", " << currentSum;
    for (uint32_t i = 0; i < k; ++i)
      cout << "1";
    cout << endl;
    */

    if (currentSum == 0) {
      isRepunit = true;  // all the digits in sum are "1", we found the repunit
    }
    else if (k >= 1000000) {
      break;
    }
  }

  return (isRepunit ? k : 0);
}
