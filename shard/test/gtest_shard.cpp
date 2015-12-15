#include <iostream>
#include <string>

#include <gtest/gtest.h>

using namespace std;

class TestShard : public ::testing::Test {
	protected:
	static void SetUpTestCase() {
	}

	static void TearDownTestCase() {
	}
};

TEST_F(TestShard, Mod) {
}

TEST_F(TestShard, JumpConsistent) {
}

int main(int argc, char * argv[]) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
