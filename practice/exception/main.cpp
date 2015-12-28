#include <iostream>
#include <string>
#include <gtest/gtest.h>

#include <foo.h>
#include <my_exception.h>

using namespace std;
using namespace ::testing;

// test resource deallocation in exception case
TEST(TestException, ResourceDeallocation) {
	Foo foo(1);

	EXPECT_THROW(foo.throwException(), MyException);
}

int main(int argc, char * argv[]) {
	InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
