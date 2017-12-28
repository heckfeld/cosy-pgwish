#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.60])
AC_INIT([FULL-PACKAGE-NAME], [VERSION], [BUG-REPORT-ADDRESS])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([tkAppInit.c])
# AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL

AC_PATH_XTRA

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([errno.h fcntl.h float.h limits.h malloc.h memory.h stdlib.h string.h sys/param.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([floor isascii sqrt strcasecmp strchr strdup strerror strncasecmp])

AC_CONFIG_FILES([library/Makefile
                 library/dd_protocols/Makefile
                 src/Makefile
                 Makefile])
AC_OUTPUT
