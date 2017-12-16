# EmacsEntry
# ----------------------------------------------------------------------
# Implements Emacs bindings for entry widgets
#
#       C-f       forward-char
#       Right     forward-char
#       C-b       backward-char
#       Left      backward-char
#       C-a       beginning-of-line
#       C-e       end-of-line
#       C-d       delete-char
#       C-k       kill-line
#       C-u       beginning-of-line, kill-line
#       C-w       delete-selection
#       C-h       delete-selection or backward-delete-char 
#       Delete    delete-selection or backward-delete-char
#       BackSpace delete-selection or backward-delete-char
#       Any       [delete-selection and] insert-char
# 
# ----------------------------------------------------------------------
#   AUTHOR:  Lindsay Marshall <lindsay.marshall@newcastle.ac.uk>
#            Jim Wight        <j.k.wight@newcastle.ac.uk>
# ----------------------------------------------------------------------
# Copyright 1994 The University of Newcastle upon Tyne (see COPYRIGHT)
# ======================================================================

itcl_class EmacsEntryBindings {
    constructor {} {
	set class [virtual info class]
	set heritage [virtual info heritage]

	if {[lsearch -exact $heritage BasicEntry] == -1} {
	    error "$class: EmacsEntryBindings not attached to a BasicEntry (or subclass)"
	}

        ::bind $this <Control-a> { %W icursor 0 ; tk_entrySeeCaret %W }
        ::bind $this <Control-b> { %W icursor [expr {[%W index insert] - 1}]; \
                                   tk_entrySeeCaret %W }
        ::bind $this <Left>      { %W icursor [expr {[%W index insert] - 1}]; \
                                   tk_entrySeeCaret %W }
        ::bind $this <Control-e> { %W icursor end ; tk_entrySeeCaret %W }
        ::bind $this <Control-f> { %W icursor [expr {[%W index insert] + 1}]; \
                                   tk_entrySeeCaret %W }
        ::bind $this <Right>     { %W icursor [expr {[%W index insert] + 1}]; \
                                   tk_entrySeeCaret %W }
        ::bind $this <Control-u> { %W tk_delete 0 end }
        ::bind $this <Control-d> { %W tk_delete insert }
        ::bind $this <Control-k> { %W tk_delete insert end }
        ::bind $this <Control-w> "catch \"$this delete-selection %W\""
        ::bind $this <Delete>    "$this delete-selection-or-char %W"
        ::bind $this <BackSpace> "$this delete-selection-or-char %W"
        ::bind $this <Control-h> "$this delete-selection-or-char %W"
        ::bind $this <Any-Key>   "$this self-insert %A %W"
	::bind $this <Control-Any-Key> { }
	::bind $this <Meta-Any-Key> { }
    } 

    method delete-selection {w} {
	$w tk_delete sel.first sel.last
	tk_entrySeeCaret $w
    }

    method delete-selection-or-char {w} {
	if {[catch "delete-selection $w"]} {
	    $this entryBackspace
	}
	tk_entrySeeCaret $w
    }

    method self-insert {a w} {
        if {"$a" != ""} {
            catch "delete-selection $w"
            $w insert insert $a
            tk_entrySeeCaret $w
        }
    }
}
