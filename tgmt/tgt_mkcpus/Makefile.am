ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -I../include -I$(top_srcdir)/include -I$(top_srcdir)/../postgres/src/interfaces/libpq -I$(top_srcdir)/../postgres/src/include

AM_CFLAGS = -Wall

bin_PROGRAMS = tgt_mkcpus
tgt_mkcpus_SOURCES = tgt_mkcpus.c
tgt_mkcpus_LDADD = ../libutils/libutils.la ../../postgres/src/interfaces/libpq/libpq.la
