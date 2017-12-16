
ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = libpgtcl.la
libpgtcl_la_SOURCES = pgtcl.c pgtclCmds.c pgtclId.c
libpgtcl_la_CFLAGS = -I../../include -I$(top_srcdir)/../../include -I../../backend -I$(top_srcdir)/../../backend -I$(top_srcdir)/../../../../tcl7.3 -I$(srcdir)/../libpq
libpgtcl_la_LDFLAGS = -version.info 1:0:0
include_HEADERS = libpgtcl.h pgtclCmds.h pgtclId.h

