ACLOCAL_AMFLAGS = -I m4

itcllibdir = $(datadir)/itcl

AM_CFLAGS = -I$(srcdir)/../../tk3.6 -I$(srcdir)/../../tcl7.3 -DITCL_LIBRARY=\"$(itcllibdir)\" @X_CFLAGS@

bin_PROGRAMS = itcl_sh itcl_wish
itcl_wish_SOURCES = tkAppInit.c
itcl_wish_LDFLAGS = -rpath '$(libdir)' -version-info 1:0:0 @X_LIBS@
itcl_wish_LDADD = libitcl.la ../../tk3.6/libtk.la ../../tcl7.3/libtcl7.3.la -lXpm -lX11 -lm
itcl_sh_SOURCES = tclAppInit.c
itcl_sh_LDFLAGS = -rpath '$(libdir)' @X_LIBS@
itcl_sh_LDADD = libitcl.la ../../tcl7.3/libtcl7.3.la -lm

lib_LTLIBRARIES = libitcl.la
libitcl_la_SOURCES = itcl_class.c itcl_core.c itcl_methods.c itcl_objects.c \
	             itcl_util.c itcl_vars.c
libitcl_la_LDFLAGS = -version-info 1:0:0 @X_LIBS@
include_HEADERS = itcl_class.h itcl_core.h itcl.h itcl_methods.h itcl_objects.h itcl_util.h itcl_vars.h
