GTEST_DIR=/opt/gtest
AM_CXXFLAGS=-DBOOST_ALL_DYN_LINK -I$(GTEST_DIR)/include
LDADD=-lboost_system -lpthread -L$(GTEST_DIR)/lib -lgtest

bin_PROGRAMS=timer1 timer2 timer3 sync_server sync_client async_server http_client

timer1_SOURCES=timer1.cpp
timer2_SOURCES=timer2.cpp
timer3_SOURCES=timer3.cpp
sync_server_SOURCES=sync_server.cpp
sync_client_SOURCES=sync_client.cpp
http_client_SOURCES=http_client.cpp
async_server_SOURCES=async_server.cpp
