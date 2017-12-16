# CheckButton
# ----------------------------------------------------------------------
# A checkbutton variant that permits interrogation of its state without
# the need for reference to its global variable.
#
#   PUBLIC ATTRIBUTES:
#
#     None
#
#   METHODS:
#
#     isSelected ..... returns selected state of caller
#
# ----------------------------------------------------------------------
#   AUTHOR:  Jim Wight        <j.k.wight@newcastle.ac.uk>
# ----------------------------------------------------------------------
# Copyright 1994 The University of Newcastle upon Tyne (see COPYRIGHT)
# ======================================================================

itcl_class CheckButton {
    inherit itcl_checkbutton

    constructor {args} {
	eval itcl_checkbutton::constructor $args
    }

    # ------------------------------------------------------------------
    #  METHOD isSelected - return state of caller
    # ------------------------------------------------------------------
    method isSelected {} {
	global [set globvar [lindex [configure -variable] 4]]

	return [string match [set $globvar] [lindex [configure -onvalue] 4]]
    }
}
