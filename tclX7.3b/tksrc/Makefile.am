
ACLOCAL_AMFLAGS = -I m4
TCLX_DIR = @srcdir@/../src
TCL_DIR = @srcdir@/../../tcl7.3
TK_DIR = @srcdir@/../../tk3.6

bin_PROGRAMS = wishx
wishx_SOURCES = tkXAppInit.c
wishx_CFLAGS = -I$(TK_DIR) -I$(TCL_DIR) -I$(TCLX_DIR) @X_CFLAGS@
wishx_LDFLAGS = -rpath '$(libdir)' @X_LIBS@
wishx_LDADD = libtkx.la ../../tk3.6/libtk.la ../src/libtclx.la ../../tcl7.3/libtcl7.3.la -lXpm -lX11 -lm


lib_LTLIBRARIES = libtkx.la
libtkx_la_SOURCES = tkXinit.c tkXmain.c tkXshell.c tkXdata.c tclXgettimeofday.c tkWm.c
libtkx_la_LDFLAGS = -version-info 1:0:0 @X_LIBS@
libtkx_la_CFLAGS = -I$(TK_DIR) -I$(TCL_DIR) -I$(TCLX_DIR) @X_CFLAGS@

include_HEADERS = tkX.h
