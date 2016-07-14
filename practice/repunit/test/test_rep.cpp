#include <repunit.h>
#include <gtest/gtest.h>
#include <tuple>
#include <cstdint>
#include <climits>
#include <cmath>
#include <stdexcept>
#include <iostream>
#include <ctime>
#include <cstdlib>

using namespace std;
using namespace ::testing;

class TestRepunit : public TestWithParam<uint64_t>
{
  protected:
    void SetUp() {
      srand(time(NULL));
    }
    
    uint64_t rep(const uint32_t k) {
      // validate input
      if (k > log10(ULLONG_MAX)) {
        throw overflow_error("exceed max value");
      }

      uint64_t result = 1;
      for (uint32_t i = 0; i < k; ++i) {
        result *= 10;
      }
      --result;
      result /= 9;
    }
};

TEST_P(TestRepunit, Basic)
{
  uint64_t n = GetParam();
  uint32_t  k = leastK(n);

  if ((n % 2 == 0) || (n % 5 == 0)) {
    EXPECT_EQ(0, k);
  }
  else {
    EXPECT_NE(0, k) << " n=" << n << ", k=" << k;
    /*
    try {
      EXPECT_EQ(0, rep(k) % n) << " n=" << n << ", k=" << k;
    }
    catch (overflow_error e) {
      cerr << e.what() << ", k=" << k << endl;
    }
    */
  }
}

TEST_F(TestRepunit, Random) {
  uint64_t n = (rand() % 1000000) * (rand() % 10000000) + rand() % 10; // n <= 10 ^ 13
  if (n % 5 == 0)
  n--;
  if (n % 2 == 0)
  n--;
  
  uint32_t k = leastK(n);
  EXPECT_NE(0, k) << "(" << n << ", " << k;
}

INSTANTIATE_TEST_CASE_P(TestRep, TestRepunit, Range((uint64_t)0, (uint64_t)100000));
//INSTANTIATE_TEST_CASE_P(TestRep, TestRepunit, Range((uint64_t)97059, (uint64_t)97060));

int main(int argc, char * argv[])
{
  InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}


