#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/noncopyable.hpp>

#include <gtest/gtest.h>

class Worker {
 public:
  Worker() : a(0) {};
	/*
  int operator()(const boost::system::error_code & e, boost::asio::deadline_timer * t) {
    if (a < 5) {
      std::cout << "a=" << a << std::endl;
      t->expires_at(t->expires_at() + boost::posix_time::seconds(1));
      t->async_wait(boost::bind(&Worker::operator(), this, boost::asio::placeholders::error, &t));
    }
    return 0;
  }
	*/

	int wait(const boost::system::error_code& e, boost::asio::deadline_timer* t, const size_t s) {
		if (a < 5) {
			std::cout << "wait " << ++a << "seconds" << std::endl;
			t->expires_at(t->expires_at() + boost::posix_time::seconds(s));
			t->async_wait(boost::bind<int>(&Worker::wait, this, e, t, s));
		}
		else {
			std::cout << "Done" << std::endl;
		}
		return 0;
	}

 private:
  int a;
};

TEST(TestAsio, Timer2) {
  boost::asio::io_service io;
  boost::asio::deadline_timer t(io, boost::posix_time::seconds(1));

  Worker worker;
  t.async_wait(boost::bind<int>(&Worker::wait, boost::ref(worker), boost::asio::placeholders::error, &t, 1));

  std::cout << "Start work" << std::endl;

  io.run();
}

GTEST_API_ int main(int argc, char ** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
