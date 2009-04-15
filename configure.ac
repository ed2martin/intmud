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
AC_CHECK_LIB(socket, getsockopt)

dnl Test for mingw32

# There might be a better way to do this.
AC_MSG_CHECKING(for mingw32)
AC_TRY_COMPILE(,[
#ifdef __MINGW32__
#error MingW32 detected, adding -lws2_32 and -lwsock32 to LIBS
#endif
],
AC_MSG_RESULT(no),[
AC_MSG_RESULT(yes)
LIBS="$LIBS -lws2_32 -lwsock32 -lwinmm -mms-bitfields -mwindows"
SKSTREAM_EXTRA_LIBS="$SKSTREAM_EXTRA_LIBS -lws2_32 -lwsock32"
])

dnl from: http://fisheye.globus.org/browse/~raw,r=1.5/GlobusToolkit/ogsa-c/tools/libxml25/configure.in

dnl Determine what socket length (socklen_t) data type is
AC_MSG_CHECKING([for type of socket length (socklen_t)])
AC_TRY_COMPILE2([
#include <stddef.h>
#include <sys/types.h>
#include <sys/socket.h>],[
(void)getsockopt (1, 1, 1, NULL, (socklen_t *)NULL)],[
  AC_MSG_RESULT(socklen_t *)
  SOCK_SOCKLEN_T=socklen_t],[
  AC_TRY_COMPILE2([
#include <stddef.h>
#include <sys/types.h>
#include <sys/socket.h>],[
(void)getsockopt (1, 1, 1, NULL, (size_t *)NULL)],[
    AC_MSG_RESULT(size_t *)
    SOCK_SOCKLEN_T=size_t],[
    AC_TRY_COMPILE2([
#include <stddef.h>
#include <sys/types.h>
#include <sys/socket.h>],[
(void)getsockopt (1, 1, 1, NULL, (int *)NULL)],[
      AC_MSG_RESULT(int *)
      SOCK_SOCKLEN_T=int],[
      AC_MSG_WARN(could not determine)
      SOCK_SOCKLEN_T="int"])])])
AC_DEFINE_UNQUOTED(SOCK_SOCKLEN_T, $SOCK_SOCKLEN_T, [Determine what socket length
(socklen_t) data type is])


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
