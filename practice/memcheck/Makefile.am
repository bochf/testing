AM_CXXFLAGS=-coverage
AM_LDFLAGS=-lgcov

bin_PROGRAMS=memcheck
memcheck_SOURCES=main.cpp
memcheck_LDADD=-lpthread

clean-local: clean-lcov clean-report

clean-lcov:
	rm -rf *.gcno *.gcda *.info

clean-report:
	rm -rf report
