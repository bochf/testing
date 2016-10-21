#include <pthread.h>
#include <iostream>
#include <gtest/gtest.h>

using namespace std;

void * work(void * args = NULL)
{
	cout << "thread is running" << endl;

	for (int i = 0; i < 10; ++i)
	{
		usleep(1000000);
		cout << ". ";
		cout.flush();
	}
	cout << endl;
	cout << "thread stopped after " << 10000 << "ms" << endl;
	pthread_exit(NULL);
}

TEST(TestThread, create)
{
	pthread_t tid;
	int rc = pthread_create(&tid, NULL, work, NULL);
	EXPECT_EQ(0, rc);
	EXPECT_NE(0, tid);

	cout << "thread " << tid << " was created in main thread" << endl;

	rc = pthread_join(tid, NULL);
	EXPECT_EQ(0, rc);
}
