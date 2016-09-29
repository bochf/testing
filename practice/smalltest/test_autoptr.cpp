#include "autoptr.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

class Foo
{
	public:
		Foo()
		{
		}

		virtual ~Foo()
		{
		}

		// void construct() = 0;
		// void destroy() = 0;
};

class MockFoo // : public Foo
{
	public:
		virtual ~MockFoo()
		{
			destroy();
		}

		MOCK_METHOD0(construct, void());
		MOCK_METHOD0(destroy, void());
};

TEST(TestAutoPtr, create)
{
	MockFoo * pMockFoo = new MockFoo();
	AutoPtr<MockFoo> apMockFoo(pMockFoo);
	EXPECT_CALL(*pMockFoo, destroy()).Times(1);
}
