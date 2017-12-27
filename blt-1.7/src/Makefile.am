
ACLOCAL_AMFLAGS = -I m4

TCL_DIR = $(srcdir)/../../tcl7.3
TK_DIR = $(srcdir)/../../tk3.6

bltlibdir = $(datadir)/blt

lib_LTLIBRARIES = libblt.la
libblt_la_SOURCES = bltBell.c bltBgexec.c bltBitmap.c bltBusy.c bltCutbuffer.c \
		bltDebug.c bltDragDrop.c bltGraph.c bltGrAxis.c bltGrBar.c bltGrElem.c \
		bltGrHairs.c bltGrLegd.c bltGrLine.c bltGrMisc.c bltGrPS.c bltGrTag.c \
		bltHtext.c bltInit.c bltList.c bltTable.c bltUtil.c bltWatch.c bltWin.c
libblt_la_CFLAGS = -DBLT_LIBRARY=\"$(bltlibdir)\" -I$(TK_DIR) -I$(TCL_DIR) @X_CFLAGS@

include_HEADERS = bltConfig.h bltGraph.h bltGrElem.h bltGrPS.h bltGrTag.h blt.h bltList.h
