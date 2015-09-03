#include <iostream>
#include <boost/cstdint.hpp>
#include <boost/asio.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/bind.hpp>

#include <gtest/gtest.h>

using namespace std;
using namespace boost::asio;

class MyAsyncServer {
 public:
  MyAsyncServer(io_service * p_iosvc, const boost::uint16_t port) :
    m_connections(0),
    m_piosvc(p_iosvc),
    m_acceptor(*m_piosvc, ip::tcp::endpoint(ip::tcp::v4(), port)) {
  }

  int on_connect(boost::shared_ptr<ip::tcp::socket> p_socket, boost::system::error_code ec) {
    if (ec) {
      cout << "client connecting failed: " << boost::system::system_error(ec).what() << endl;
      return -1;
    } else {
      cout << "connections: " << ++m_connections << endl;

      if (m_connections <= 10) {
        // continue listen
        start();
      }

      cout << "client connected: " << p_socket->remote_endpoint().address() << endl;

      // send data to client
      p_socket->async_write_some(buffer("hello unblock client"), boost::bind<int>(&MyAsyncServer::on_sent, this, _1, _2));
    }
  }

  int on_sent(boost::system::error_code ec, size_t len) {
    if (ec) {
      cout << "send data failed: " << boost::system::system_error(ec).what() << endl;
      return -1;
    } else {
      cout << "sent " << len << " bytes data to client" << endl;
      return 0;
    }
  }

  void start() {
    // create socket
    boost::shared_ptr<ip::tcp::socket> p_socket(new ip::tcp::socket(*m_piosvc));
    // waiting for connect event
    m_acceptor.async_accept(*p_socket, boost::bind<int>(&MyAsyncServer::on_connect, this, p_socket, _1));
  }

  void on_read(boost::system::error_code ec) {
  }

 private:
  size_t m_connections;
  io_service * m_piosvc;
  ip::tcp::acceptor m_acceptor;
};

TEST(TestSyncServer, Acceptor) {
  io_service iosvc;
  MyAsyncServer my_server(&iosvc, 10000);

  my_server.start();
  iosvc.run();
}

GTEST_API_ int main(int argc, char ** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
