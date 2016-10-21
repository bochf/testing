#include <iostream>
#include <string>
#include <vector>

#include <pthread.h>
#include <gtest/gtest.h>

using namespace std;
using namespace ::testing;

void * work(void * args = NULL)
{
	cout << "thread start" << endl;

	int wait = 10;
	if (NULL != args)
		wait = *static_cast<int*>(args);

	for (int i = 0; i < wait;  ++i)
	{
		cout << ". ";
		cout.flush();
		usleep(100000);
	}
	cout << endl;
	cout << "thread end" << endl;
}

struct ThreadParam
{
	int n;
	string str;

	ThreadParam() : n(0), str("") 
	{}
	ThreadParam(const int _n, const string _str)
		: n(_n), str(_str)
	{}
	ThreadParam(const ThreadParam& rhs)
	{
		n = rhs.n;
		str = rhs.str;
	}
	ThreadParam& operator=(const ThreadParam& rhs)
	{
		n = rhs.n;
		str = rhs.str;
	}
};

void * workWithParams(void * args = NULL)
{
	cout << "thread start" << endl;

	ThreadParam params(10, "xxx");
	if (NULL != args)
	{
		params = *static_cast<ThreadParam*>(args);
	}

	for (int i = 0; i < params.n;  ++i)
	{
		cout << params.str << " ";
		cout.flush();
		usleep(100000);
	}
	cout << endl;
	cout << "thread end" << endl;
}

TEST(TestThread, create)
{
	pthread_t tid;
	int rc = pthread_create(&tid, NULL, work, NULL);
	cout << "thread " << tid << " is created in main" << endl;
	EXPECT_EQ(0, rc);

	// waiting for thread termination
	rc = pthread_join(tid, NULL);
	cout << "thread " << tid << " is completed in main" << endl;
	EXPECT_EQ(0, rc);
}

TEST(TestThread, passParams)
{
	int * wait = new int(3);
	pthread_t tid;
	int rc = pthread_create(&tid, NULL, work, wait);
	cout << "thread " << tid << " is created in main" << endl;
	EXPECT_EQ(0, rc);

	// waiting for thread termination
	rc = pthread_join(tid, NULL);
	cout << "thread " << tid << " is completed in main" << endl;
	EXPECT_EQ(0, rc);

	delete wait;
}

TEST(TestThread, passComplexParams)
{
	ThreadParam * args = new ThreadParam(3, "abc");
	pthread_t tid;

	int rc = pthread_create(&tid, NULL, workWithParams, NULL);
	cout << "thread " << tid << " is created in main" << endl;
	EXPECT_EQ(0, rc);

	// waiting for thread termination
	rc = pthread_join(tid, NULL);
	cout << "thread " << tid << " is completed in main" << endl;

	rc = pthread_create(&tid, NULL, workWithParams, args);
	cout << "thread " << tid << " is created in main" << endl;
	EXPECT_EQ(0, rc);

	// waiting for thread termination
	rc = pthread_join(tid, NULL);
	cout << "thread " << tid << " is completed in main" << endl;
	EXPECT_EQ(0, rc);

	delete args;
}
