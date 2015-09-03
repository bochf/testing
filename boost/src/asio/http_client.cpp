#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/cstdint.hpp>

#include <gtest/gtest.h>

using namespace std;
using namespace boost::asio;

TEST(TestSyncClient, Connect) {
	string server = "www.google.com";
	string service = "http";

	// create an io_service object
	io_service iosvc;

	// host name resolve
	ip::tcp::resolver res(iosvc);
	ip::tcp::resolver::query query(server, service);
	ip::tcp::resolver::iterator it_endpoint = res.resolve(query);
	// the default constructed of iterator object is used as the "end" iterator
	ip::tcp::resolver::iterator it_end;
	// initialize the error code to not found
	boost::system::error_code ec = boost::asio::error::host_not_found;
	while (it_endpoint != it_end) {
	}
	// create a socket object
	ip::tcp::socket socket(iosvc);

	// connect to server
	cout << "connecting to " << server << ":" << 80 << endl;
	ip::tcp::endpoint ep(ip::address_v4::from_string(server), 80);
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

