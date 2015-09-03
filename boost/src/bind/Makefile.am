GTEST_DIR=/opt/gtest
AM_CXXFLAGS=-DBOOST_ALL_DYN_LINK -I$(GTEST_DIR)/include
LDADD=-lpthread -L$(GTEST_DIR)/lib -lgtest

bin_PROGRAMS=bind

bind_SOURCES=bind.cpp
