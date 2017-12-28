#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.60])
AC_INIT([FULL-PACKAGE-NAME], [VERSION], [BUG-REPORT-ADDRESS])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([dprpc.c])
# AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL
AC_PATH_XTRA

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([malloc.h errno.h arpa/inet.h fcntl.h limits.h malloc.h netdb.h sys/select.h netinet/in.h stdlib.h string.h sys/ioctl.h sys/param.h sys/socket.h sys/time.h unistd.h sys/un.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([writev gethostbyname gethostname gettimeofday tolower isascii memset select socket strchr strerror strncasecmp strrchr strspn ntohl])

AC_CONFIG_FILES([Makefile
                 library/Makefile])
AC_OUTPUT
