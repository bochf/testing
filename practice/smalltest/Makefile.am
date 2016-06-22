bin_PROGRAMS=test_inherit test_expect_not_call test_delegate_ctor mock11

AM_CXXFLAGS=-std=c++11

test_inherit_SOURCES=test_inherit.cpp
test_inherit_LDADD=-lgtest_main -lgtest -lpthread 

test_expect_not_call_SOURCES=test_expect_not_call.cpp
test_expect_not_call_LDADD=-lgmock_main -lgmock -lpthread

test_delegate_ctor_SOURCES=test_delegate_ctor.cpp
test_delegate_ctor_LDADD=-lgtest_main -lgtest -lpthread

mock11_SOURCES=mock11.cpp
mock11_LDADD=-lgmock_main -lgtest -lpthread
