#include <iostream>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

#include "server.pb.h"  // $(top_srcdir)/src/proto
#include "clio_command.pb.h"

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

TEST(ProtoBuf, FindMessage) {
	ClioServerStatus::ServerStatus ss;
	fstream input("clio.status", ios::in | ios::binary);

	// open protobuf description file
	const google::protobuf::FileDescriptor * fd = google::protobuf::DescriptorPool::generated_pool()->FindFileByName("clio_command.proto");
	ASSERT_TRUE(fd != NULL);

	cout << "there are " << fd->message_type_count() << " messages defined in the file" << fd->name() << endl;
	// cout << "detail:" << endl;
	// cout << fd->DebugString() << endl;

	// iterate messages in the file
	for (int i = 0; i < fd->message_type_count(); ++i) {
		cout << "message(" << i << ")\t";
		const google::protobuf::Descriptor * md = fd->message_type(i);
		ASSERT_TRUE(md != NULL);
		cout << md->name() << endl;

		// create message from message descriptor
		const google::protobuf::Message * prototype = google::protobuf::MessageFactory::generated_factory()->GetPrototype(md);
		ASSERT_TRUE(prototype != NULL);
		google::protobuf::Message * message = prototype->New();
		ASSERT_TRUE(message != NULL);
		if(message->ParseFromIstream(&input)) {
			cout << "parse message: " << message->DebugString() << endl;
			break;
		}
	}

	/*
	EXPECT_TRUE(ss.ParseFromIstream(&input));
	input.close();

	cout << ss.DebugString() << endl;

	EXPECT_EQ("clio", ss.proc_name());
	EXPECT_EQ("clio.cisco.com", ss.host_name());
	EXPECT_EQ("127.0.0.1", ss.ip_address());
	EXPECT_EQ(ClioServerStatus::ServerStatus::UP, ss.status());
	*/
}

GTEST_API_ int main(int argc, char ** argv) {
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
