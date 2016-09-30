#include "autoptr.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

class MockFoo
{
  public:
    virtual ~MockFoo() {
      destroy();
    }

    MOCK_METHOD0(destroy, void());
    MOCK_METHOD0(test, void());
};

TEST(TestAutoPtr, create)
{
  MockFoo * pMockFoo = new MockFoo();
  AutoPtr<MockFoo> apMockFoo(pMockFoo);
  EXPECT_CALL(*pMockFoo, destroy()).Times(1);
}

TEST(TestAutoPtr, set)
{
  AutoPtr<MockFoo> apMockFoo;
  MockFoo * pMockFoo = new MockFoo();
  apMockFoo.set(pMockFoo);
  EXPECT_CALL(*pMockFoo, destroy()).Times(1);
}

TEST(TestAutoPtr, release)
{
  MockFoo * pMockFoo = NULL;
  {
    AutoPtr<MockFoo> apMockFoo(new MockFoo());
    pMockFoo = apMockFoo.release();
    EXPECT_CALL(*pMockFoo, destroy()).Times(0);
  }
  EXPECT_CALL(*pMockFoo, destroy()).Times(1);
  delete pMockFoo;
}
