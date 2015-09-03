GTEST_DIR=/opt/gtest
AM_CXXFLAGS=-DBOOST_ALL_DYN_LINK -I$(GTEST_DIR)/include
LDGTEST=-L$(GTEST_DIR)/lib -lgtest -lpthread
LDADD=$(LDGTEST) -lboost_program_options

bin_PROGRAMS=simplepo

simplepo_SOURCES=simplepo.cpp
