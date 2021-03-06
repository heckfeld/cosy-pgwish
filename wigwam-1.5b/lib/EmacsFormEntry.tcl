# EmacsFormEntry
# ----------------------------------------------------------------------
# Implements an entry widget with Emacs bindings in which <Tab> causes
# the focus to transfer to the next such widget with the same parent.
#
#   PUBLIC ATTRIBUTES (inherited):
#
#     -command ....... a command to execute when <Return> is entered.
#                      The text in the widget is passed as a argument.
#
#   METHODS (inherited):
#
#     invoke ......... invoke the Tcl command associated with <Return>
#
# ----------------------------------------------------------------------
#   AUTHOR:  Jim Wight        <j.k.wight@newcastle.ac.uk>
# ----------------------------------------------------------------------
# Copyright 1994 The University of Newcastle upon Tyne (see COPYRIGHT)
# ======================================================================

itcl_class EmacsFormEntry {
    inherit EmacsEntryBindings FormEntry

    constructor {args} {
	eval FormEntry::constructor $args
        EmacsEntryBindings::constructor
    }
}
