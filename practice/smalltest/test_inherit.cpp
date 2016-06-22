/**
 * how does override function work
 */

#include <gtest/gtest.h>

using namespace testing;

class Base {
	public:
	virtual int getA() { return 1; };

	int callA() { return getA(); };
	int callAThis() { return this->getA(); };
	virtual int callAVirtual() { return getA(); };
	virtual int callAVirtualThis() { return this->getA(); };

	protected:
	int a;
};

class Derived : public Base {
	public:
	Derived() {
		a = 0;
	}
	int getA() override { return 2; };
};

TEST(TestInherit, BaseCall) {
	Base base;
	Derived derive;
	Base * pDerive = new Derived();

	EXPECT_EQ(1, base.callA());
	EXPECT_EQ(1, base.callAThis());
	EXPECT_EQ(1, base.callAVirtual());
	EXPECT_EQ(1, base.callAVirtualThis());

	EXPECT_EQ(2, derive.callA());
	EXPECT_EQ(2, derive.callAThis());
	EXPECT_EQ(2, derive.callAVirtual());
	EXPECT_EQ(2, derive.callAVirtualThis());

	EXPECT_EQ(2, pDerive->callA());
	EXPECT_EQ(2, pDerive->callAThis());
	EXPECT_EQ(2, pDerive->callAVirtual());
	EXPECT_EQ(2, pDerive->callAVirtualThis());
}
