GTEST_DIR=/opt/gtest
BOOST_DIR=/opt/boost
PROTO_DIR=$(top_srcdir)/proto
GRPC_PLUGIN=`which grpc_cpp_plugin`

AM_CXXFLAGS=-I$(GTEST_DIR)/include -I$(BOOST_DIR) -I$(PROTO_DIR)
AM_CXXFLAGS+=-std=c++11
LDADD=-lprotobuf \
			-L/usr/local/lib -lgrpc++_unsecure -lgrpc -lgpr -ldl \
			-lgtest -lpthread \
			-L$(BOOST_DIR)/libs -lboost_system

bin_PROGRAMS=clio_api

BUILT_SOURCES=clio_api.pb.cc clio_api.pb.h clio_api.grpc.pb.cc clio_api.grpc.pb.h
clio_api.pb.cc: clio_api.pb.h
clio_api.pb.h: Makefile
	protoc --cpp_out=$(top_srcdir)/src $(PROTO_DIR)/clio_api.proto -I$(PROTO_DIR)

clio_api.grpc.pb.cc: clio_api.grpc.pb.h
clio_api.grpc.pb.h: Makefile
	protoc --grpc_out=$(top_srcdir)/src --plugin=protoc-gen-grpc=$(GRPC_PLUGIN) $(PROTO_DIR)/clio_api.proto -I$(PROTO_DIR)


clio_api_SOURCES=clio_api.cpp clio_api.pb.cc clio_api.grpc.pb.cc
