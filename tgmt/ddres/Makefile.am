
ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -I../include -I$(top_srcdir)/include

bin_PROGRAMS = ddres
ddres_SOURCES = ddres.c
init_ddir = $(sysconfdir)/init.d
defaultdir = $(sysconfdir)/default

dist_init_d_DATA = init.d/ddres
dist_default_DATA = default/ddres

ddres_LDADD = ../libtgtmgt/libtgtmgt.la ../../postgres/src/interfaces/libpq/libpq.la

