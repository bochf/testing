// learn how to use gtest Environment

#ifndef __SHARD_RESOURCE_H__
#define __SHARD_RESOURCE_H__

#include <gtest/gtest.h>
#include <iostream>

using namespace std;
using namespace ::testing;

class MyEnv : public Environment {
	public:
	virtual void SetUp() {
		cout << __FILE__ << ":" << __LINE__ << __FUNCTION__ << " run" << endl;
	}

	virtual void TearDown() {
		cout << __FILE__ << ":" << __LINE__ << __FUNCTION__ << " run" << endl;
	}

	static int s_count;
};

class MyTestF : public Test {
	protected:
	static void SetUpTestCase() {
		cout << __FILE__ << ":" << __LINE__ << __FUNCTION__ << " run" << endl;
	}

	static void TearDownTestCase() {
		cout << __FILE__ << ":" << __LINE__ << __FUNCTION__ << " run" << endl;
	}

	virtual void SetUp() {
		cout << __FILE__ << ":" << __LINE__ << __FUNCTION__ << " run" << endl;
	}

	virtual void TearDown() {
		cout << __FILE__ << ":" << __LINE__ << __FUNCTION__ << " run" << endl;
	}

	public:
	static int s_count;
	static int s_fCount;
};

class MyTestP : public TestWithParam<int> {
	public:
	static void SetUpTestCase() {
		cout << __FILE__ << ":" << __LINE__ << __FUNCTION__ << " run" << endl;
	}

	static void TearDownTestCase() {
		cout << __FILE__ << ":" << __LINE__ << __FUNCTION__ << " run" << endl;
	}

	virtual void SetUp() {
		cout << __FILE__ << ":" << __LINE__ << __FUNCTION__ << " run" << endl;
	}

	virtual void TearDown() {
		cout << __FILE__ << ":" << __LINE__ << __FUNCTION__ << " run" << endl;
	}

	public:
	static int s_count;
	static int s_pCount;
};

#endif
