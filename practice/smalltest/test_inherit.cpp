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

template<class T>
class Getter {
	public:
		Getter(T t) {
			_value = new T(t);
		}

		Base& value() {
			return *_value;
		}

	private:
		Base * _value;
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

TEST(TestInherit, Reference) {
	Base base;
	Derived derive;

	Getter<Base> gb(base);
	Getter<Derived> gd(derive);

	Base& refBase = gb.value();  // valid reference
	Base& refDerive = gd.value();  // valid reference
	EXPECT_EQ(1, refBase.getA());
	EXPECT_EQ(2, refDerive.getA());

	// Derived& refB = gb.value();  // invalie reference
	// Derived& refD = gd.value();  // invalie reference

	Base b = gb.value();  // valid statment, but it is not reference, it is assignment
	Base d = gd.value();  // valid statment, but it is not reference, it is assignment
	EXPECT_EQ(1, b.getA());
	EXPECT_EQ(1, d.getA());
}
