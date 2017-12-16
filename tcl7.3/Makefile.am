
ACLOCAL_AMFLAGS = -I m4

tcllibdir = $(datadir)/tcl

bin_PROGRAMS = tclsh7.3
tclsh7_3_SOURCES = tclAppInit.c
tclsh7_3_LDFLAGS = -rpath '$(libdir)'
tclsh7_3_LDADD = -ltcl7.3 -lm

lib_LTLIBRARIES = libtcl7.3.la
libtcl7_3_la_SOURCES = regexp.c tclAsync.c tclBasic.c tclCkalloc.c \
	tclCmdAH.c tclCmdIL.c tclCmdMZ.c tclExpr.c tclGet.c \
	tclHash.c tclHistory.c tclLink.c tclParse.c tclProc.c \
	tclUtil.c tclVar.c \
	panic.c tclEnv.c tclGlob.c tclMain.c tclUnixAZ.c \
        tclUnixStr.c tclUnixUtil.c
libtcl7_3_la_LDFLAGS = -version-info 1:0:0
libtcl7_3_la_CFLAGS = -DTCL_LIBRARY=\"$(tcllibdir)\"

dist_tcllib_DATA = library/init.tcl library/parray.tcl library/tclIndex

include_HEADERS = patchlevel.h tcl.h tclInt.h tclRegexp.h tclUnix.h

install-exec-hook:
#	-rm -f $(DESTDIR)$(bindir)/tclsh
#	-ln -s $(DESTDIR)$(bindir)/tclsh7.3 $(DESTDIR)$(bindir)/tclsh

uninstall-hook:
#	rm -f $(DESTDIR)$(bindir)/tclsh

