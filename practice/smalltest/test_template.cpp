#include <gtest/gtest.h>

using namespace ::testing;

template<class T>
class Nullable
{
	public:
		Nullable() : _isNull(true) {};
		Nullable(const T& t) : _isNull(false), _value(t) {};
		virtual ~Nullable() {};

		virtual const T& value() const {
			return _value;
		}
		virtual void set(const T t) {
			_isNull = false;
			_value = t;
		}

		bool isNull() const {
			return _isNull;
		}

		virtual bool process() { return false; };
	private:
		bool _isNull;
		T _value;
};

class NullableInt : public Nullable<int>
{
	public:
		NullableInt() : Nullable<int>() {};
		NullableInt(const int& i) : Nullable<int>(i) {};

		virtual bool process() { return isNull(); };
};

template<class T>
class NullableEx : public Nullable<T>
{
	public:
		NullableEx() : Nullable<T>() {};
		NullableEx(const T& t) : Nullable<T>(t) {};

		virtual bool process() { return this->isNull(); };
};

TEST(TestTemplate, base) {
	Nullable<int> a;
	Nullable<int> b(100);

	EXPECT_TRUE(a.isNull());
	EXPECT_FALSE(b.isNull());

	EXPECT_EQ(100, b.value());

	a.set(3);
	EXPECT_FALSE(a.isNull());
	EXPECT_EQ(3, a.value());

	EXPECT_FALSE(a.process());
	EXPECT_FALSE(b.process());
}

TEST(TestTemplate, derive) {
	NullableInt a;
	NullableInt b(100);

	EXPECT_TRUE(a.isNull());
	EXPECT_FALSE(b.isNull());

	EXPECT_EQ(100, b.value());

	a.set(3);
	EXPECT_FALSE(a.isNull());
	EXPECT_EQ(3, a.value());

	EXPECT_FALSE(a.process());
	EXPECT_FALSE(b.process());
}

TEST(TestTemplate, deriveTemplate) {
	NullableEx<int> a;
	NullableEx<int> b(100);

	EXPECT_TRUE(a.isNull());
	EXPECT_FALSE(b.isNull());

	EXPECT_EQ(100, b.value());

	a.set(3);
	EXPECT_FALSE(a.isNull());
	EXPECT_EQ(3, a.value());

	EXPECT_FALSE(a.process());
	EXPECT_FALSE(b.process());
}
