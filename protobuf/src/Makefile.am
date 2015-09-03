BOOST_INC=/opt/boost_1_59_0
LDADD=-lprotobuf
bin_PROGRAMS=example

AM_CXXFLAGS=-I$(BOOST_INC)
example_SOURCES=example.cpp $(top_srcdir)/proto/server.pb.cc
