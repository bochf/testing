#include <iostream>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/noncopyable.hpp>

#include <gtest/gtest.h>

class Worker {
 public:
  Worker() : a(0) {};
  void operator()(const boost::system::error_code & e) {
    std::cout << "a=" << a << std::endl;
  }

 private:
  int a;
};

TEST(TestAsio, Timer2) {
  boost::asio::io_service io;
  boost::asio::deadline_timer t(io, boost::posix_time::seconds(5));

  Worker worker;
  t.async_wait(worker);

  std::cout << "Start work" << std::endl;

  io.run();
}

GTEST_API_ int main(int argc, char ** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
