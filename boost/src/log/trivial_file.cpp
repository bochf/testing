#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

#include <string>

#include <gtest/gtest.h>

using namespace std;
using namespace boost::log;

void set_log_level(const trivial::severity_level level) {
	core::get()->set_filter(trivial::severity >= level);
}

void set_log_file(const string file_name) {
	add_file_log(
			keywords::file_name = file_name + "%N.log",
			keywords::rotation_size = 10 * 1024,
			keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
			keywords::format = "[%TimeStamp%]: %Message%"
			);
	add_common_attributes();
}

TEST(TestBoostLog, TrivialFilter) {
	set_log_level(trivial::info);
	set_log_file("boost_log");

  BOOST_LOG_TRIVIAL(trace) << "A trace severity message";
  BOOST_LOG_TRIVIAL(debug) << "A debug severity message";
  BOOST_LOG_TRIVIAL(info) << "An informational severity message";
  BOOST_LOG_TRIVIAL(warning) << "A warning severity message";
  BOOST_LOG_TRIVIAL(error) << "An error severity message";
  BOOST_LOG_TRIVIAL(fatal) << "A fatal severity message";
}

GTEST_API_ int main(int argc, char ** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
