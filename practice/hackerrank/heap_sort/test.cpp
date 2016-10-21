#include <gtest/gtest.h>

#include <heap_sort.h>
#include <vector>
#include <cstdlib>
#include <iostream>

using namespace std;
using namespace ::testing;

TEST(TestHeapSort, sort)
{
  srand(time(NULL));

  vector<int> v;
  for (int i = 0; i < 100; ++i) {
    v.push_back(rand() % 1000);
  }

  HeapSort::sort(v);

  for (int i = 0; i < v.size() - 1; ++i) {
    EXPECT_LE(v[i], v[i + 1]) << "i=" << i;
  }
}