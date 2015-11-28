AM_CXXFLAGS=-coverage
AM_LDFLAGS=-lgcov

bin_PROGRAMS=memcheck
memcheck_SOURCES=main.cpp
memcheck_LDADD=-lpthread

memcheck.info: memcheck.gcda
	lcov -c -b . -d . --no-external -o $@

memcheck.gcda: memcheck
	./$< 
	./$< 100 50

report: memcheck.info
	genhtml -o $@ -s --legend -p /home/josh/dev/testing/practice --demangle-cpp $?

clean-local: clean-lcov clean-report

clean-lcov:
	rm -rf *.gcno *.gcda *.info

clean-report:
	rm -rf report
