// learn how to use gtest Environment

#ifndef __SHARD_RESOURCE_H__
#define __SHARD_RESOURCE_H__

#include <gtest/gtest.h>
#include <iostream>

using namespace std;
using namespace ::testing;

class MyEnv : public Environment
{
  protected:
    virtual void SetUp()
    {
      cout << "MyEnv::" << __FUNCTION__ << " run" << endl;
    }

    virtual void TearDown()
    {
      cout << "MyEnv::" << __FUNCTION__ << " run" << endl;
    }

	public:
    static int s_count;
};

class MyTestF : public Test
{
  public:
    static void SetUpTestCase()
    {
      cout << "MyTestF::" << __FUNCTION__ << " run" << endl;
    }

    static void TearDownTestCase()
    {
      cout << "MyTestF::" << __FUNCTION__ << " run" << endl;
    }

  protected:
    virtual void SetUp()
    {
      cout << "MyTestF::" << __FUNCTION__ << " run" << endl;
    }

    virtual void TearDown()
    {
      cout << "MyTestF::" << __FUNCTION__ << " run" << endl;
    }

  public:
    static int s_count;
    static int s_fCount;
};

class MyTestP : public TestWithParam<int>
{
  public:
    static void SetUpTestCase()
    {
      cout << "MyTestP::" << __FUNCTION__ << " run" << endl;
    }

    static void TearDownTestCase()
    {
      cout << "MyTestP::" << __FUNCTION__ << " run" << endl;
    }

    virtual void SetUp()
    {
      cout << "MyTestP::" << __FUNCTION__ << " run" << endl;
    }

    virtual void TearDown()
    {
      cout << "MyTestP::" << __FUNCTION__ << " run" << endl;
    }

  public:
    static int s_count;
    static int s_pCount;
};

class Foo {
	public:
		Foo() : n(0) {
			cout << "create Foo" << endl;
		}
		virtual ~Foo() {
			cout << "destroy Foo" << endl;
		}

		int& value() {
			return n;
		}

	private:
		int n;
};

class MyTestP2 : public MyTestF,
  public WithParamInterface<int>
{
  protected:
		virtual void SetUp() {
			cout << "MyTestP2::" << __FUNCTION__ << " run" << endl;
		}

		virtual void TearDonw() {
			cout << "MyTestP2::" << __FUNCTION__ << " run" << endl;
		}

	protected:
		Foo foo;
};

#endif
