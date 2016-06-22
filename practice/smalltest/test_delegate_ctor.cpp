#include <gtest/gtest.h>

using namespace ::testing;

class Foo {
	public:
	int _x;
	int _y;

	Foo() : _x(0), _y(0) {};
	Foo(int x) : _x(x), _y(0) {};
	Foo(int x, int y) : Foo(x) {
		 _y = y;
	};
};

TEST(TestDelegate, Basic) {
	Foo a;
	Foo b(100);
	Foo c(200, 300);

	EXPECT_EQ(0, a._x);
	EXPECT_EQ(0, a._y);
	EXPECT_EQ(100, b._x);
	EXPECT_EQ(200, c._x);
	EXPECT_EQ(300, c._y);
}
