ACLOCAL_AMFLAGS = -I m4

SUBDIRS = library bitmaps pixmaps

USE_DEFS = -DUSE_PINFO -DUSE_XPM3 -DUSE_GIF -DUSE_UXPM -DLOAD_PIXMAPS -DTK_NO_SECURITY -DVUW

TCL_DIR = @srcdir@/../tcl7.3
tklibdir = $(datadir)/tk

bin_PROGRAMS = wish
wish_SOURCES = tkAppInit.c
wish_CFLAGS = -I$(TCL_DIR) $(USE_DEFS) @X_CFLAGS@
wish_LDFLAGS = -rpath '$(libdir)' @X_LIBS@
wish_LDADD = libtk.la $(top_builddir)/../tcl7.3/libtcl7.3.la -lXpm -lX11 -lm

lib_LTLIBRARIES = libtk.la
libtk_la_SOURCES = tk3d.c tkArgv.c tkAtom.c tkBind.c tkPixmap.c  tkCmds.c \
	tkColor.c tkConfig.c tkCursor.c tkError.c tkEvent.c \
	tkFocus.c tkFont.c tkGet.c tkGC.c tkGeometry.c tkGrab.c \
	tkMain.c tkOption.c tkPack.c tkPlace.c tkPreserve.c tkSelect.c \
	tkSend.c tkWindow.c tkWm.c \
	tkButton.c tkEntry.c tkFrame.c tkListbox.c \
	tkMenu.c tkMenubutton.c tkMessage.c tkScale.c \
	tkScrollbar.c tkFScale.c \
	tkCanvas.c tkCanvArc.c tkCanvBmap.c tkCanvLine.c \
	tkCanvPoly.c tkCanvPs.c tkCanvText.c tkCanvWind.c \
	tkRectOval.c tkTrig.c \
	tkText.c tkTextBTree.c tkTextDisp.c tkTextIndex.c tkTextTag.c \
	bitmap.c gif.c xpm3.c raw256.c dpsSupport.c uxpm.c \
	tkDial.c tkBargraph.c tkStripchart.c tkPie.c
libtk_la_LDFLAGS = -version-info 1:0:0
libtk_la_CFLAGS = -DTK_LIBRARY=\"$(tklibdir)\" -I$(TCL_DIR) $(USE_DEFS)

include_HEADERS = default.h ks_names.h patchlevel.h tclRawTCP.h tkCanvas.h tkConfig.h tk.h tkInt.h tkPhoto.h tkText.h tkXAccess.h
