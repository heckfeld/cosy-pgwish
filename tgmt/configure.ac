#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.60])
AC_INIT([tgmt], [1.0], [mi.simon@fz-juelich.de])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([net/if_dl.h net/if_packet.h netpacket/packet.h net/if_ether.h arpa/inet.h netdb.h netinet/in.h stdlib.h string.h sys/socket.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_CHECK_FUNCS([gethostbyaddr inet_ntoa memset putenv socket strdup strpbrk strtol])

# files to be generated
AC_CONFIG_FILES([libtgtmgt/Makefile libtools/Makefile include/Makefile libutils/Makefile download/Makefile tgt_mkcpus/Makefile tgt_mkfc/Makefile tgt_mkparam/Makefile tgt_mkthosts/Makefile tgtmgt/Makefile udpterm/Makefile udpteln/Makefile upload/Makefile utils/Makefile scripts/Makefile ddres/Makefile Makefile])
AC_OUTPUT
