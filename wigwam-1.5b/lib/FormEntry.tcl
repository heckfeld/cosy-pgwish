# FormEntry
# ----------------------------------------------------------------------
# Implements an entry widget in which <Tab>/Shift+Tab causes the focus to
# be transferred to the next/previous such widget with the same parent.
#
#   PUBLIC ATTRIBUTES (inherited):
#
#     -command ....... a command to execute when <Return> is entered.
#                      The text in the widget is passed as argument.
#
#   METHODS:
#
#     invoke ......... invoke the Tcl command associated with <Return>
#
#     next ........... steps to the next or...
#     previous .......           ...previous FormEntry in the peer group
#
#     peers .......... returns a list of FormEntry objects belonging to
#                      the peer group
#
# ----------------------------------------------------------------------
#   AUTHOR:  Jim Wight        <j.k.wight@newcastle.ac.uk>
# ----------------------------------------------------------------------
# Copyright 1994 The University of Newcastle upon Tyne (see COPYRIGHT)
# ======================================================================

itcl_class FormEntry {
    inherit BasicEntry

    constructor {args} {
        eval BasicEntry::constructor $args

	::bind $this <Tab> "$this next"
	::bind $this <Shift-Tab> "$this previous"
    }

    method peers {} {
	set components [llength [split $this .]]
	set parent [file rootname $this]

	set forms ""
	foreach form [itcl_info objects $parent.* -isa FormEntry] {
	    if {[llength [split $form .]] == $components} {
		lappend forms $form
	    }
	}
	return $forms
    }

    method next {} {
	set entries [peers]

	set elen [expr [llength $entries] - 1]
	set where [lsearch -exact $entries $this]
	if {$where == $elen} {
	    focus [lindex $entries 0]
	} else {
	    focus [lindex $entries [expr $where + 1]]
	}
    }

    method previous {} {
	set entries [peers]

	set elen [expr [llength $entries] - 1]
	set where [lsearch -exact $entries $this]
	if {$where == 0} {
	    focus [lindex $entries $elen]
	} else {
	    focus [lindex $entries [expr $where - 1]]
	}
    }
}
