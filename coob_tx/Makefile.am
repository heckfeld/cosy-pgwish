
ACLOCAL_AMFLAGS = -I m4

AM_LDFLAGS = -lcrypt

bin_PROGRAMS = coob_tx
coob_tx_SOURCES = coob_tx.c
coob_tx_CFLAGS = -I$(top_srcdir)/../postgres/src/interfaces/libpq -I$(top_srcdir)/../postgres/src/include
coob_tx_LDADD = ../postgres/src/interfaces/libpq/libpq.la

include_HEADERS = coobp.h pgdefs.h txp.h
