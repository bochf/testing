#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <cstdint>
#include <chrono>

#include <gtest/gtest.h>
#include <shard_mod.h>

using namespace std;
using namespace ::testing;

/**
 * tuple of testing parameters
 * <0>: number of elements, 2^n
 * <1>: initial number of buckets
 * <2>: increase or decrease bucket
 */
typedef tuple<int, int, bool> ShardParams;

class TestShard : public TestWithParam<ShardParams>
{
  public:
    static void run(IShard * shard, const ShardParams& params)
    {
			const size_t n = 1 << get<0>(params); // number of elements
			const size_t bucket1 = get<1>(params); // initial buckets
			const size_t bucket2 = bucket1 + get<2>(params) ? 1 : -1; // changed buckets

			ASSERT_FALSE(shard == NULL);
			ASSERT_GT(n, 1000);
			ASSERT_LT(n, 20000000);  // total elements to be tested between (1000, 20M)
			ASSERT_GT(bucket1, 10);
			ASSERT_LT(bucket1, 1000000);  // total buckets between (10, 1M)

			// record start time
			auto start = chrono::high_resolution_clock::now();

			// initialize sharding
			// using accumulating number [0..n) as key
			// save the hashed sharding index in the array
			vector<int32_t> index;
			for (size_t key = 0; key < n; ++key) {
				index.push_back(shard->hash(key, bucket1));
			}

			size_t moved = 0;  // statstic how many elements need to be moved, e.g. get new sharding index while the number of buckets changed
			for (size_t key = 0; key < n; ++key) {
				int32_t newIndex = shard->hash(key, bucket2);
				if (newIndex != index[key]) {
					++moved;
					index[key] = newIndex;
				}
			}
			// record end time
			auto finish = chrono::high_resolution_clock::now();
			// calculate elapsed time
			chrono::nanoseconds ns = chrono::duration_cast<chrono::nanoseconds>(finish - start);
			cout << n << ", " << bucket1 << ", " << bucket2 << ", " << moved << ", " << ns.count() << endl;
    }
};

TEST_P(TestShard, Mod)
{
	ShardMod shard;
  run(&shard, GetParam());
}

INSTANTIATE_TEST_CASE_P(Sharding,
                        TestShard,
                        Combine(
                          Range(10, 20),
                          Values(20, 40, 80, 160, 320, 640, 1280, 2560, 5120, 10240,
                              20480),
                          Bool()
                        )
                       );

int main(int argc, char * argv[])
{
  InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
