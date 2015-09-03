#include <iostream>
#include <boost/cstdint.hpp>
#include <boost/asio.hpp>

#include <gtest/gtest.h>

using namespace std;

TEST(TestSyncServer, Acceptor) {
	using namespace boost::asio;

	// create an io_service object
	io_service iosvc;
	boost::uint16_t port = 10000;

	// create a socket acceptor
	ip::tcp::acceptor acceptor(iosvc, ip::tcp::endpoint(ip::tcp::v4(), port));

	bool stop = false;
	size_t connections = 0;
	while(!stop) {
		// create a socket object
		ip::tcp::socket socket(iosvc);
		// waiting for connection from client
		acceptor.accept(socket);
		cout << "number of connections=" << ++connections << endl;
		cout << socket.remote_endpoint().address() << endl;

		// send message to client
		boost::system::error_code ec;
		socket.write_some(buffer("hello block client"), ec);

		// quit on error
		if (ec) {
			cout << boost::system::system_error(ec).what() << endl;
			stop = true;
		}

		if (connections >= 10) {
			cout << "reach the max capacity" << endl;
			stop = true;
		}
	} // while(!stop)
}

GTEST_API_ int main(int argc, char ** argv) {
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
