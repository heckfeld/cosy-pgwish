ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -I$(top_srcdir)/h -I$(top_srcdir)/../postgres/src/interfaces/libpq -I$(top_srcdir)/../postgres/src/include

bin_PROGRAMS = cosmo_calib
cosmo_calib_SOURCES = cosmo_calib.cpp

cosmo_calib_LDFLAGS = -rpath '$(libdir)'
cosmo_calib_LDADD = ../p++.neu/libcosmocalib.la ../p/libcalib.la ../../postgres/src/interfaces/libpq/libpq.la

