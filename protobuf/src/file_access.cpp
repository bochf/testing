#include <iostream>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

#include "server.pb.h"  // $(top_srcdir)/src/proto

using namespace std;

TEST(ProtoBuf, SaveMessageToFile) {
	ClioServerStatus::ServerStatus ss;
	ss.set_proc_name("clio");
	ss.set_host_name("clio.cisco.com");
	ss.set_ip_address("127.0.0.1");
	ss.set_status(ClioServerStatus::ServerStatus::UP);

	fstream output("clio.status", ios::out | ios::trunc | ios::binary);
	EXPECT_TRUE(ss.SerializeToOstream(&output));
	output.close();
}

TEST(ProtoBuf, ReadMessageFromFile) {
	ClioServerStatus::ServerStatus ss;
	fstream input("clio.status", ios::in | ios::binary);

	EXPECT_TRUE(ss.ParseFromIstream(&input));
	input.close();

	cout << ss.DebugString() << endl;

	EXPECT_EQ("clio", ss.proc_name());
	EXPECT_EQ("clio.cisco.com", ss.host_name());
	EXPECT_EQ("127.0.0.1", ss.ip_address());
	EXPECT_EQ(ClioServerStatus::ServerStatus::UP, ss.status());
}

GTEST_API_ int main(int argc, char ** argv) {
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
