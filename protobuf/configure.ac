#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([test], [1.0], [bug@webex.com])
AC_CONFIG_SRCDIR([src/example.cpp])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([build-aux/config.h])

LT_INIT
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([1.13])
# Checks for programs.
: ${CXXFLAGS="-g"}
AC_PROG_CXX

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.


PKG_CHECK_MODULES([protobuf], [protobuf])

AC_CONFIG_FILES([Makefile
								 src/Makefile
								 ])
AC_OUTPUT
