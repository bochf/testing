TARGET_DIR=../src
GTEST_DIR=
AM_CXXFLAGS=-I. -I$(TARGET_DIR) -std=c++11
AM_LDFLAGS=-lgtest -lpthread

bin_PROGRAMS=test_util test_rep
test_util_SOURCES=test_util.cpp
test_util_LDADD=$(AM_LDFLAGS)
test_util_LDFLAGS=-L$(TARGET_DIR) -lutil

test_rep_SOURCES=test_rep.cpp
test_rep_LDADD=$(AM_LDFLAGS)
test_rep_LDFLAGS=-L$(TARGET_DIR) -lrep -lutil
