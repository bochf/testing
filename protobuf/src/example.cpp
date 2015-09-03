#include <iostream>
#include <fstream>
#include <string>

#include <boost/asio/ip/host_name.hpp>

#include "server.pb.h"   // $(top_srcdir)/src/proto

using namespace std;

int main() {
	ClioServerStatus::ServerStatus ss;

	ss.set_proc_name("clio");
	ss.set_host_name(boost::asio::ip::host_name());
	ss.set_ip_address("127.0.0.1");
	ss.set_status(ClioServerStatus::ServerStatus::UP);

	cout << ss.DebugString() << endl;
}
