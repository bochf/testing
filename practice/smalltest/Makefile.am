SUBDIRS=thread
bin_PROGRAMS=test_inherit test_expect_not_call test_delegate_ctor mock11 vec_init \
						 my_memcpy test_autoptr test_stack_q test_template \
						 test_typedef

AM_CXXFLAGS=-std=c++11

test_inherit_SOURCES=test_inherit.cpp
test_inherit_LDADD=-lgtest_main -lgtest -lpthread 

test_expect_not_call_SOURCES=test_expect_not_call.cpp
test_expect_not_call_LDADD=-lgmock_main -lgmock -lpthread

test_delegate_ctor_SOURCES=test_delegate_ctor.cpp
test_delegate_ctor_LDADD=-lgtest_main -lgtest -lpthread

mock11_SOURCES=mock11.cpp
mock11_LDADD=-lgmock_main -lgtest -lpthread

vec_init_SOURCES=vec_init.cpp
vec_init_LDADD=-lgtest_main -lgtest -lpthread

my_memcpy_SOURCES=my_memcpy.cpp
my_memcpy_LDADD=-lgtest_main -lgtest -lpthread

test_autoptr_SOURCES=test_autoptr.cpp
test_autoptr_LDADD=-lgmock_main -lgtest -lpthread

test_stack_q_SOURCES=test_stack_q.cpp
test_stack_q_LDADD=-lgmock_main -lgtest -lpthread

test_template_SOURCES=test_template.cpp
test_template_LDADD=-lgmock_main -lgtest -lpthread

test_typedef_SOURCES=test_typedef.cpp
test_typedef_LDADD=-lgmock_main -lgtest -lpthread
