#
# genindex.tcl --
#
#  Interface to the buildpackageindex that doesn't go through the auto-load
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
# $Id: genindex.tcl,v 4.0 1994/07/16 05:29:17 markd Rel $
#------------------------------------------------------------------------------
#

source [info library]/buildidx.tcl

buildpackageindex $argv
