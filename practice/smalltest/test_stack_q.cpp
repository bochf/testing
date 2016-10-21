#include <stack_q.h>

#include <queue>
#include <gtest/gtest.h>

using namespace std;
using namespace ::testing;

TEST(TestQ, create) {
	StackQ<int> q;
}

TEST(TestQ, push_and_pop) {
	StackQ<int> stackQ;
	queue<int> q;
	for (int i = 0; i < 100; ++i) {
		stackQ.push(i);
		q.push(i);
		EXPECT_EQ(q.size(), stackQ.size());
	}

	for (int i = 0; i < 100; ++i) {
		EXPECT_EQ(q.front(), stackQ.front());
		stackQ.pop();
		q.pop();
		EXPECT_EQ(q.size(), stackQ.size());
	}

	EXPECT_TRUE(stackQ.empty());
}
