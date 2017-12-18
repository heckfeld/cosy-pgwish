ACLOCAL_AMFLAGS = -I m4

itcllwlibdir = $(datadir)/itcl-LW
wigwamlibdir = $(datadir)/wigwam

bin_PROGRAMS = pgwish
pgwish_SOURCES = tkXAppInit.c tcl_binary.c tclsyslog.c
pgwish_CFLAGS = -I$(srcdir)/../tclX7.3b/src -I$(srcdir)/../tcl7.3 -I$(srcdir)/../tk3.6 \
		-DITCL_LW_LIBRARY=\"$(itcllwlibdir)\" \
		-DWIGWAM_LIBRARY=\"$(wigwamlibdir)\"
pgwish_LDFLAGS = -rpath '$(libdir)'
#
# Die Reihenfolge der Libs ist nicht ganz unwichtig.
# Sowohl in der libtcl als auch det libtk steckt die Funktion main.
#
pgwish_LDADD = \
		../blt-1.7/src/libblt.la \
		../itcl-1.5/src/libitcl.la \
		../tcl-dp3.2/libdp.la \
		../spd-3.6/libspd.la \
		../tclX7.3b/tksrc/libtkx.la \
		../tk3.6/libtk.la \
		../tclX7.3b/src/libtclx.la \
		../tcl7.3/libtcl7.3.la \
		../postgres/src/interfaces/libpgtcl/libpgtcl.la \
		../postgres/src/interfaces/libpq/libpq.la \
		-lXpm -lX11 -lm -lcrypt

include_HEADERS = tcl_binary.h
