#include <util.h>
#include <gtest/gtest.h>
#include <tuple>
#include <cstdint>

using namespace std;
using namespace ::testing;

class TestGetPair : public TestWithParam<tuple<uint8_t, uint8_t> >
{
};

TEST_P(TestGetPair, ValidInput)
{
  uint8_t n = get<0>(GetParam());
  uint8_t p = get<1>(GetParam());
  uint8_t m = getPair(n, p);
  EXPECT_EQ(n * m % 10, p) << "(" << m << ", " << n << ", " << p << ")";
}

namespace
{
const uint8_t N[] = {1, 3, 7, 9};
const uint8_t P[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

INSTANTIATE_TEST_CASE_P(TestUtil, TestGetPair, Combine(ValuesIn(N), ValuesIn(P)));
}

class TestRemoveOnes : public TestWithParam<uint64_t>
{
};

TEST_P(TestRemoveOnes, Basic)
{
  uint64_t before = GetParam();
  uint64_t after  = before;
  uint8_t k = removeOnes(after);

  EXPECT_NE(1, after % 10);  // all the "1" at tail were removed, last digit should not be "1"

  // revert
  for (uint8_t i = 0; i < k; ++i) {
    after = after * 10 + 1;
  }
  EXPECT_EQ(before, after);
}

INSTANTIATE_TEST_CASE_P(TestUtil, TestRemoveOnes, Range((uint64_t)0, (uint64_t)100000));

int main(int argc, char * argv[])
{
  InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}


