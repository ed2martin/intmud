#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(intmud, 0.01, ed2martin@yahoo.com.br)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([misc.cpp])
AM_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h sys/time.h unistd.h])
AC_CHECK_HEADERS([sys/types.h sys/socket.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MEMCMP
AC_CHECK_FUNCS([dup2 memset strerror getcwd])
AC_FUNC_CLOSEDIR_VOID
AC_HEADER_DIRENT
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_ACCEPT_ARGTYPES
AC_TYPE_SIGNAL
AC_TYPE_SIZE_T

AC_PROG_INSTALL
AC_CONFIG_FILES([Makefile])

AC_OUTPUT

