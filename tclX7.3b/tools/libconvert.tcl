#
# libconvert.tcl --
#
#  Interface to the convert_lib that doesn't go through the auto-load
# mechanism.  This helps if something is broken with auto-load so the
# build at least completes.
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
# $Id: libconvert.tcl,v 4.0 1994/07/16 05:29:21 markd Rel $
#------------------------------------------------------------------------------
#

if ![info exists env(TCL_LIBRARY)] {
    puts stderr "This script is to only be used during the build run by"
    puts stderr "the `runtcl' script."
    exit 1
}

source $env(TCL_LIBRARY)/tcl.tlib

eval convert_lib $argv
