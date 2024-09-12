#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.71])
AC_INIT([intmud],[1.15b],[edx2martin@gmail.com])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([misc.cpp])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

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

AC_TYPE_SIZE_T

# Checks for libraries.
AC_CHECK_LIB(socket, getsockopt)

dnl Test for mingw32

# There might be a better way to do this.
AC_MSG_CHECKING(for mingw32)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]], [[
#ifdef __MINGW32__
#error MingW32 detected, adding -lws2_32 and -lwsock32 to LIBS
#endif
]])],[
AC_MSG_RESULT(no)
dnl dlopen on libdl
AC_SEARCH_LIBS([dlopen], [dl dld], [], [
  AC_MSG_ERROR([unable to find the dlopen() function])
])
],[
AC_MSG_RESULT(yes)
LIBS="$LIBS -lws2_32 -lwsock32 -lwinmm -lpsapi -mms-bitfields -mwindows -static -static-libgcc -static-libstdc++"
CPPFLAGS="-DWINVER=0x0501"
])

AC_PROG_INSTALL
AC_CONFIG_FILES([Makefile])

AC_OUTPUT
