ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -I$(top_srcdir)/h -I$(top_srcdir)/../postgres/src/interfaces/libpq -I$(top_srcdir)/../postgres/src/include

bin_PROGRAMS = cosmo_calib_get
cosmo_calib_get_SOURCES = cosmo_calib_get.cpp

cosmo_calib_get_LDFLAGS = -rpath '$(libdir)'
cosmo_calib_get_LDADD = ../p++.neu/libcosmocalib.la ../p/libcalib.la ../../postgres/src/interfaces/libpq/libpq.la

