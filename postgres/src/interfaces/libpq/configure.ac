#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.60])
AC_INIT([pgwish-autoconf], [1.0], [mi.simon@fz-juelich.de])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CC
AC_PROG_LIBTOOL
AC_PROG_INSTALL

AC_SEARCH_LIBS([crypt], [crypt])

AC_CONFIG_LINKS([pqcomprim.c:../../backend/libpq/pqcomprim.c
                 dllist.c:../../backend/lib/dllist.c])

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
