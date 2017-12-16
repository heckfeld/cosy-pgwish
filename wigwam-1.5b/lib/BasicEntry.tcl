# BasicEntry
# ----------------------------------------------------------------------
# Implements a base class for entry classes
#
#   PUBLIC ATTRIBUTES:
#
#     -command ....... a command to execute when <Return> is entered.
#                      The text in the widget is passed as argument.
#
#   METHODS:
#
#     invoke ......... invoke the Tcl command associated with <Return>.
#
# ----------------------------------------------------------------------
#   AUTHOR:  Jim Wight <j.k.wight@newcastle.ac.uk>
# ----------------------------------------------------------------------
# Copyright 1994 The University of Newcastle upon Tyne (see COPYRIGHT)
# ======================================================================

itcl_class BasicEntry {
    inherit itcl_entry

    constructor {args} {
        eval itcl_entry::constructor $args

	::bind $this <Return>    "$this invoke"
	::bind $this <KP_Enter>  "$this invoke"
	::bind $this <Delete>    "$this entryBackspace ; tk_entrySeeCaret %W"
	::bind $this <BackSpace> "$this entryBackspace ; tk_entrySeeCaret %W"
	::bind $this <Control-h> "$this entryBackspace ; tk_entrySeeCaret %W"
    }

    method invoke {} {
	if {![string match "" $command]} {eval $command \{[$this get]\}}
    }

    method entryBackspace {} {
        set x [expr {[$this index insert] - 1}]
        if {$x != -1} { $this tk_delete $x }
    }

    public command {}
}
