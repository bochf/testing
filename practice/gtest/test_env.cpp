#include <gtest/gtest.h>
#include <shard_resource.h>

#include <iostream>
#include <string>

using namespace ::testing;
using namespace std;

void print() {
	cout << "MyEnv::s_count=" << MyEnv::s_count
	     << ", MyTestF::s_count=" << MyTestF::s_count
			 << ", MyTestF::s_fCount=" << MyTestF::s_fCount
			 << ", MyTestP::s_count=" << MyTestP::s_count
			 << ", MyTestP::s_pCount=" << MyTestP::s_pCount
			 << endl;
}

TEST_F(MyTestF, Test) {
	s_count++;
	s_fCount++;

	cout << __FILE__ << ":" << __LINE__ << "::" << __FUNCTION__
	<< " " << s_count << " " << s_fCount << endl;
}

TEST_P(MyTestP, Test) {
	s_count++;
	s_pCount++;

	cout << __FILE__ << ":" << __LINE__ << "::" << __FUNCTION__
	<< " " << s_count << " " << s_pCount << endl;
}

int MyEnv::s_count = 0;
int MyTestF::s_count = 10;
int MyTestF::s_fCount = 20;
int MyTestP::s_count = 100;
int MyTestP::s_pCount = 200;

INSTANTIATE_TEST_CASE_P(MyEnv, MyTestP, Range(1, 10));

int main(int argc, char * argv[]) {
	AddGlobalTestEnvironment(new MyEnv);
	InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
