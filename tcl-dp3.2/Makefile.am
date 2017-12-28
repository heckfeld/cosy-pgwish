ACLOCAL_AMFLAGS = -I m4
TCL_DIR = @srcdir@/../tcl7.3
TK_DIR = @srcdir@/../tk3.6

dplibdir = $(datadir)/dp

bin_PROGRAMS = dpwish
dpwish_SOURCES = tkAppInit.c tkMain.c
dpwish_CFLAGS = -I$(TCL_DIR) -I$(TK_DIR) -DUNIX_SOCKET @X_CFLAGS@
dpwish_LDFLAGS = -rpath '$(libdir)' @X_LIBS@
dpwish_LDADD = libdp.la ../tk3.6/libtk.la ../tcl7.3/libtcl7.3.la -lXpm -lX11 -lm

lib_LTLIBRARIES = libdp.la
libdp_la_SOURCES = dpaddress.c dpnetInit.c dpnetwork.c dprpc.c
libdp_la_LDFLAGS = -version-info 1:0:0 @X_LIBS@
libdp_la_CFLAGS = -DDP_LIBRARY=\"$(dplibdir)\" -I$(TK_DIR) -I$(TCL_DIR) @X_CFLAGS@

include_HEADERS = dp.h dpInt.h

SUBDIRS = library
