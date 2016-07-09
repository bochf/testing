#include <vector>
#include <gtest/gtest.h>

using namespace std;
using namespace ::testing;

TEST(InitVector, create) {
	vector<bool> v(10, true);
	for (int i = 0; i < 10; ++i)
		EXPECT_TRUE(v[i]);

	vector<bool> v1(10, false);
	for (int i = 0; i < 10; ++i)
		EXPECT_FALSE(v1[i]);
}

TEST(InitVector, resize) {
	vector<bool> v(10, true);
	for (int i = 0; i < 10; ++i)
		EXPECT_TRUE(v[i]);

	v.resize(20, false);
	for (int i = 0; i < 20; ++i)
		EXPECT_FALSE(v[i]) << "i=" << i;
}
