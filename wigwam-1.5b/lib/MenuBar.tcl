# MenuBar
# ----------------------------------------------------------------------
# A MenuBar is a container for menubuttons and MenuButtons that supports
# scanning with the mouse and keyboard traversal. The widget sets the input
# focus to itself in order to enable keyboard traversal, assuming that the
# application does not set the focus elsewhere.
#
#   PUBLIC ATTRIBUTES:
#
#     None
#
#   METHODS:
#
#     pack ........... must be used instead of the pack command to add
#                      children. The arguments are passed on to the pack
#                      command applied to $this.
#
# ----------------------------------------------------------------------
#   AUTHOR:  Jim Wight        <j.k.wight@newcastle.ac.uk>
# ----------------------------------------------------------------------
# Copyright 1994 The University of Newcastle upon Tyne (see COPYRIGHT)
# ======================================================================

itcl_class MenuBar {
    inherit itcl_frame

    constructor {args} {
	eval itcl_frame::constructor $args
	focus $this
    }

    method pack {args} {
	eval ::pack $args
	eval tk_menuBar $this [::pack slaves $this]
    }
}
