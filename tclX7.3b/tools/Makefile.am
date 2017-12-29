#
# Makefile for building tools used during install
#------------------------------------------------------------------------------
# Copyright 1992-1994 Karl Lehenbauer and Mark Diekhans.
#
# Permission to use, copy, modify, and distribute this software and its
# documentation for any purpose and without fee is hereby granted, provided
# that the above copyright notice appear in all copies.  Karl Lehenbauer and
# Mark Diekhans make no representations about the suitability of this
# software for any purpose.  It is provided "as is" without express or
# implied warranty.
#------------------------------------------------------------------------------
# $Id: Makefile.in,v 4.0 1994/07/16 05:28:50 markd Rel $
#------------------------------------------------------------------------------
#

ACLOCAL_AMFLAGS = -I m4
TCL_DIR = @srcdir@/../../tcl7.3
TK_DIR = @srcdir@/../../tk3.6

#------------------------------------------------------------------------------

# Build but do not distribute (because they are build from source)
bin_PROGRAMS = tclxversion tkxversion
bin_SCRIPTS = instcopy bldmanhelp cpmanpages cphelpdir symlinkext

tclxversion_SOURCES = getversion.c
tclxversion_CFLAGS = -DTCL_GET_VERSION -I$(srcdir)/../src -I$(TCL_DIR) @X_CFLAGS@

tkxversion_SOURCES = getversion.c
tkxversion_CFLAGS = -DTK_GET_VERSION -I$(srcdir)/../src -I$(TCL_DIR) -I$(TK_DIR) @X_CFLAGS@

# distribute source, so scripts can be rebuild
EXTRA_DIST = instcopy.tcl buildutil.tcl bldmanhelp.tcl cpmanpages.tcl cphelpdir.sh symlinkext.sh
CLEAN_FILES = $(bin_SCRIPTS)

#------------------------------------------------------------------------------

instcopy$(EXEEXT): instcopy.tcl buildutil.tcl
	rm -f $@
	cat ${srcdir}/buildutil.tcl ${srcdir}/instcopy.tcl >$@

bldmanhelp$(EXEEXT): bldmanhelp.tcl
	rm -f $@
	cp ${srcdir}/bldmanhelp.tcl $@

cpmanpages$(EXEEXT): cpmanpages.tcl buildutil.tcl
	rm -f $@
	cat ${srcdir}/buildutil.tcl ${srcdir}/cpmanpages.tcl >$@

cphelpdir$(EXEEXT): cphelpdir.sh
	rm -f $@
	echo ":"                           >$@
	echo "TOOLS=${bldbasedir}/tools"  >>$@
	cat ${srcdir}/cphelpdir.sh        >>$@
	chmod a+x $@

symlinkext$(EXEEXT): symlinkext.sh
	rm -f $@
	cat ${srcdir}/symlinkext.sh  >$@
	chmod a+x $@

