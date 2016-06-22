/**
 * how to expect a method not to be called
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace ::testing;

class Foo {
	public:
	virtual int get() const {
		return 0;
	}
};

class MockFoo : public Foo {
 public:
  MOCK_CONST_METHOD0(get, int());
};

class Caller {
	public:
	void callFoo(Foo * foo, bool flag) {
		if (flag)
			foo->get();
	}
};

TEST(TestMockMethod, ExpectCall) {
	MockFoo foo;
	EXPECT_CALL(foo, get()).Times(1);

	Caller caller;
	caller.callFoo(&foo, true);
}

TEST(TestMockMethod, ExpectNotCall) {
	MockFoo foo;
	EXPECT_CALL(foo, get()).Times(0);

	Caller caller;
	caller.callFoo(&foo, true);
}
