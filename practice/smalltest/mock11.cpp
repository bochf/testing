#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace ::testing;

class Foo
{
  public:
	virtual ~Foo() {}

    virtual int add(const int x0, const int x1)
    {
      return x0 + x1;
    }
		// more than 10 parameters function is not supported by gmock
		/*
    virtual int add11(const int x0,
                      const int x1,
                      const int x2,
                      const int x3,
                      const int x4,
                      const int x5,
                      const int x6,
                      const int x7,
                      const int x8,
                      const int x9,
                      const int x10)
    {
      return x0 + x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9 + x10;
    }
		*/
};

class Bar
{
  public:
    Bar(Foo * foo) : d_foo(foo) {}

    virtual int accu(const int x)
    {
      return d_foo->add(x, x+1);
    }

    Foo * d_foo;
};

class MockFoo : public Foo
{
  public:
    virtual ~MockFoo() {};

    MOCK_METHOD2(add, int(const int x0, const int x1));
		// more than 10 parameters function is not supported by gmock
		/*
    virtual int gmock_add11(const int x0, const int x1, const int x2,
              const int x3, const int x4, const int x5,
              const int x6, const int x7, const int x8,
              const int x9, const int x10)
		{}
		*/
};


TEST(TestMock, more_than_11)
{
  MockFoo mockFoo;
  Bar     bar(&mockFoo);

  // EXPECT_CALL(mockFoo, add11(_, _, _, _, _, _, _, _, _, _, _)).Times(1).WillOnce(Return(0));
  EXPECT_EQ(0, bar.accu(0));
}
