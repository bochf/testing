#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([test code for study], [1.0], [http://localhost/bug])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([memcheck/main.cpp])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([build-aux/config.h])

LT_INIT

AM_INIT_AUTOMAKE

# Set debug mode for compiler
: ${CXXFLAGS="-g"}

# Checks for programs.
AC_PROG_CXX

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_SIZE_T

# Checks for library functions.

AC_CONFIG_FILES([Makefile
                 memcheck/Makefile
                 gtest/Makefile
                 exception/Makefile
								 ])
AC_OUTPUT
