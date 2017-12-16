# Scanvas
# ----------------------------------------------------------------------
# A canvas with scan scrolling enabled
#
#   PUBLIC ATTRIBUTES:
#
#     None
#
#   METHODS:
#
#     None
#
# ----------------------------------------------------------------------
#   AUTHOR:  Jim Wight        <j.k.wight@newcastle.ac.uk>
# ----------------------------------------------------------------------
# Copyright 1994 The University of Newcastle upon Tyne (see COPYRIGHT)
# ======================================================================

itcl_class Scanvas {
    inherit itcl_canvas

    constructor {args} {
        eval itcl_canvas::constructor $args
      
	::bind $this <Button-2>  "$this scan mark %X %Y"
	::bind $this <B2-Motion> "$this scan dragto %X %Y"
    }
}
