#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([TMPlayer], [1.0.0], [limdh3@telechips.com])
AM_INIT_AUTOMAKE
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP

AC_ARG_ENABLE([iap2],
		AC_HELP_STRING([--enable-iap2], [Enable iAP2Player support @<:@default=no@:>@]),
		[case "${enableval}" in
			yes) enable_iap2=true ;;
			no) enable_iap2=false ;;
			*) AC_MSG_ERROR([bad value ${enableval} for --enable-iap2]) ;;
		esac],[enable_iap2=false])

AM_CONDITIONAL([IAP2], [test x$enable_iap2 = xtrue])

AC_ARG_ENABLE([a2dp],
		AC_HELP_STRING([--enable-a2dp], [Enable A2DP Player support @<:@default=no@:>@]),
		[case "${enableval}" in
			yes) enable_a2dp=true ;;
			no) enable_a2dp=false ;;
			*) AC_MSG_ERROR([bad value ${enableval} for --enable-a2dp]) ;;
		esac],[enable_a2dp=false])

AM_CONDITIONAL([A2DP], [test x$enable_a2dp = xtrue])

# Checks PKG-CONFIGn
PKG_CHECK_MODULES([TMP],[glib-2.0 dbus-1 libxml-2.0 TcUtils alsa])
# Checks for libraries.
# FIXME: Replace `main' with a function in `-lTcConnect':
AC_CHECK_LIB([TcConnect], [main])
# FIXME: Replace `main' with a function in `-lTcDBGen':
AC_CHECK_LIB([TcDBGen], [main])
# FIXME: Replace `main' with a function in `-lTcMultiMedia':
AC_CHECK_LIB([TcMultiMedia], [main])
# FIXME: Replace `main' with a function in `-lpthread':
AC_CHECK_LIB([pthread], [main])
# FIXME: Replace `main' with a function in `-lrt':
AC_CHECK_LIB([rt], [main])
# FIXME: Replace `main' with a function in `-lsqlite3':
AC_CHECK_LIB([sqlite3], [main])

AC_CHECK_LIB([pulse], [pa_mainloop_new], , )

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h sys/vfs.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_CHECK_FUNCS([memset mkdir munmap strncasecmp strrchr strstr])

AC_SUBST(AUDIOMANAGER)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
