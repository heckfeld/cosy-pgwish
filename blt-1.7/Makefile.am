

ACLOCAL_AMFLAGS = -I m4

AM_CFLAGS = -I$(srcdir)/../tcl7.3 -I$(srcdir)/../tk3.6

bin_PROGRAMS = bltwish
bltwish_SOURCES = tkAppInit.c
bltwish_LDFLAGS = -rpath '$(libdir)'
bltwish_LDADD = src/libblt.la ../tk3.6/libtk.la ../tcl7.3/libtcl.la -lXpm -lX11 -lm


SUBDIRS = src library
