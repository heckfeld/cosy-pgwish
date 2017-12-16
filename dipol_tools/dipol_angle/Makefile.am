ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -I$(top_srcdir)/../calib/h -I$(top_srcdir)/../postgres/src/interfaces/libpq -I$(top_srcdir)/../postgres/src/include

bin_PROGRAMS = dipol_angle
dipol_angle_SOURCES = main.cpp

dipol_angle_LDFLAGS = -rpath '$(libdir)'
dipol_angle_LDADD = $(top_builddir)/../calib/p++.neu/libcosmocalib.la \
		    $(top_builddir)/../calib/p/libcalib.la \
		    $(top_builddir)/../postgres/src/interfaces/libpq/libpq.la
