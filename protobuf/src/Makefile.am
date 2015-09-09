GTEST_DIR=/opt/gtest
BOOST_DIR=/opt/boost
PROTO_AUTO_GEN_DIR=$(top_srcdir)/proto

AM_CXXFLAGS=-I$(GTEST_DIR)/include -I$(BOOST_DIR) -I$(PROTO_AUTO_GEN_DIR)
LDADD=-lprotobuf \
			-lgtest -lpthread \
			-L$(BOOST_DIR)/libs -lboost_system

bin_PROGRAMS=example file_access clio_api

example_SOURCES=example.cpp $(PROTO_AUTO_GEN_DIR)/server.pb.cc
file_access_SOURCES=file_access.cpp $(PROTO_AUTO_GEN_DIR)/server.pb.cc $(PROTO_AUTO_GEN_DIR)/clio_command.pb.cc
clio_api_SOURCES=clio_api.cpp $(PROTO_AUTO_GEN_DIR)/clio_api.pb.cc
