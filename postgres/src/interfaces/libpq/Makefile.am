
ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = libpq.la
libpq_la_SOURCES = fe-auth.c fe-connect.c fe-exec.c fe-misc.c fe-lobj.c \
		dllist.c pqsignal.c pqcomprim.c
libpq_la_CFLAGS = -I../../include -I$(top_srcdir)/../../include -I$(top_srcdir)/../../backend -I../../../../tcl7.3 -DFRONTEND
libpq_la_LDFLAGS = -version.info 1:0:0
libpq_la_LIBADD = -lcrypt
include_HEADERS = libpq-fe.h fe-auth.h fe-connect.h pqsignal.h

