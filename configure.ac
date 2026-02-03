#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.71])
AC_INIT([intmud],[1.16a],[edx2martin@gmail.com])
AC_CONFIG_SRCDIR([src/misc.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])

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
AX_PTHREAD

# Check for Windows
AC_MSG_CHECKING(for Windows)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]], [[
#ifdef _WIN32
#error Windows detected
#endif
]])],[
  AC_MSG_RESULT(no)
  have_socket=0
],[
  AC_MSG_RESULT(yes)
  have_socket=1
  LIBS="$LIBS -lws2_32 -lwsock32 -lwinmm -lpsapi -mwindows -static -static-libgcc -static-libstdc++ -lDbghelp"
  CPPFLAGS="-DWINVER=0x0501"
])

# Non-Windows libs
# Nota: have_socket será usado também para detectar epool/kqueue
if test "x$have_socket" = "x0"; then
  dnl dlopen on libdl
  AC_SEARCH_LIBS([dlopen], [dl dld], [], [
    AC_MSG_ERROR([unable to find the dlopen() function])
  ])

  AC_CHECK_FUNC([backtrace], [have_backtrace=yes], [have_backtrace=no])
  if test "x$have_backtrace" = "xno"; then
      AC_CHECK_LIB([execinfo], [backtrace],
          [LIBS="$LIBS -lexecinfo"
           AC_DEFINE([HAVE_BACKTRACE], [1], [backtrace is available via libexecinfo])],
          [AC_MSG_WARN([backtrace support not found])])
  else
      AC_DEFINE([HAVE_BACKTRACE], [1], [backtrace is available via libc])
  fi
  AC_CHECK_HEADERS([execinfo.h])
fi

AC_PROG_INSTALL
AC_CONFIG_FILES([Makefile])

AC_OUTPUT
