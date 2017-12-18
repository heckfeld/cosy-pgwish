AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4
TCL_DIR = @srcdir@/../../tcl7.3

tclsrcdir = $(datadir)/tclx


bin_PROGRAMS = tclx
tclx_SOURCES = tclXAppInit.c
tclx_CFLAGS = -I$(TCL_DIR)
tclx_LDFLAGS = -rpath '$(libdir)'
tclx_LDADD = libtclx.la ../../tcl7.3/libtcl7.3.la -lm


lib_LTLIBRARIES = libtclx.la
libtclx_la_SOURCES = \
	tclXbsearch.c tclXchmod.c      tclXclock.c      tclXcmdInit.c \
	tclXcmdloop.c    tclXcnvclock.c   tclXdebug.c      tclXdup.c     \
	tclXfcntl.c      tclXfilecmds.c   tclXfilescan.c   tclXflock.c   \
	tclXfstat.c      tclXgeneral.c    tclXgetdate.y    tclXhandles.c \
	tclXid.c         tclXinit.c       tclXkeylist.c    tclXlib.c     \
	tclXlist.c       tclXmain.c       tclXmath.c       tclXmsgcat.c  \
	tclXprocess.c    tclXprofile.c    tclXregexp.c     tclXselect.c  \
	tclXserver.c     tclXsignal.c     tclXshell.c      tclXstring.c  \
	tclXunixcmds.c   tclXutil.c       tclXdata.c \
	tclCmdIL.c
libtclx_la_LDFLAGS = -version-info 1:0:0
libtclx_la_CFLAGS = -I$(TCL_DIR) -DTCL_MASTERDIR=\"$(tclsrcdir)\" -DTCL_LIBRARY=\"$(tclsrcdir)\"

include_HEADERS = tclExtdInt.h tclExtend.h tclXconfig.h tclXpatchl.h

tclCmdIL.c:
	rm -f $@
	ln -s $(srcdir)/../../tcl7.3/tclCmdIL.c $@
