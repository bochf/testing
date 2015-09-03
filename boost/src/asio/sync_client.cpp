#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/cstdint.hpp>

#include <gtest/gtest.h>

using namespace std;
using namespace boost::asio;

TEST(TestSyncClient, Connect) {
	string server = "127.0.0.1";
	boost::uint16_t port = 10000;

	// create an io_service object
	io_service iosvc;

	// create a socket object
	ip::tcp::socket socket(iosvc);

	// connect to server
	cout << "connecting to " << server << ":" << port << endl;
	ip::tcp::endpoint ep(ip::address_v4::from_string(server), port);
	boost::system::error_code ec;
	socket.connect(ep, ec);

	// quit on error
	if (ec) {
		cout << boost::system::system_error(ec).what() << endl;
		return;
	}

	cout << "connected" << endl;

	// receive message from server
	cout << "reading from server" << endl;
	char buf[1024];
	size_t len = socket.read_some(buffer(buf), ec);

	// quit on error
	if (ec) {
		cout << boost::system::system_error(ec).what() << endl;
		return;
	}

	cout << buf << endl;
}

GTEST_API_ int main(int argc, char ** argv) {
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

