GTEST_DIR=/opt/gtest
AM_CXXFLAGS=-DBOOST_ALL_DYN_LINK -I$(GTEST_DIR)/include
LDADD=-lboost_log -lboost_thread -lboost_system -lboost_log_setup -lpthread -L$(GTEST_DIR)/lib -lgtest

bin_PROGRAMS=trivial trivial_filter trivial_file

trivial_SOURCES=trivial.cpp
trivial_filter_SOURCES=trivial_filter.cpp
trivial_file_SOURCES=trivial_file.cpp
