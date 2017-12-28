#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.60])
AC_INIT([FULL-PACKAGE-NAME], [VERSION], [BUG-REPORT-ADDRESS])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([src/tclExtdInt.h])
# AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AM_PROG_CC_C_O
AC_PROG_CXX
AC_PROG_YACC
AC_PROG_CC
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL

# Checks for libraries.

# Checks for header files.
AC_PATH_XTRA
AC_CHECK_HEADERS([arpa/inet.h limits.h malloc.h netdb.h netinet/in.h nl_types.h stddef.h stdlib.h string.h sys/time.h sys/times.h sys/param.h sys/socket.h values.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UID_T
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_UID_T
AC_TYPE_MODE_T
AC_TYPE_SIZE_T
AC_STRUCT_TIMEZONE

# Checks for library functions.
AC_FUNC_CHOWN
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STRTOD
AC_CHECK_FUNCS([alarm bcopy bzero catgets endgrent endpwent floor fsync getcwd gethostbyaddr gethostbyname getpriority gettimeofday inet_ntoa isascii memset mkdir rmdir select setitimer setpgid setvbuf sigaction socket strchr strcspn strerror strpbrk strrchr strspn strstr strtol strtoul tzset])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 tclsrc/Makefile
                 tksrc/Makefile
                 tools/Makefile])
AC_OUTPUT
