#!/usr/local/bin/itcl_wish -f

# FormEntry demo
# ----------------------------------------------------------------------
# Assumes that the sample classes are in ../lib or that their location
# is communicated through the WIGWAMLIB environment variable.
# ----------------------------------------------------------------------

if {[catch "set env(WIGWAMLIB)"]} {
    set WIGWAMLIB ../lib
} else {
    set WIGWAMLIB [set env(WIGWAMLIB)]
}

lappend auto_path $WIGWAMLIB

itcl_class SpecialFormEntry {
    inherit FormEntry
      
    constructor {args} {
      eval FormEntry::constructor $args
    }
      
    method enter {text} {
	foreach peer [$this peers] {
	    puts -nonewline "$peer: "
	    if {$this == $peer} {
		puts $text
	    } else {
		puts "[$peer get]"
	    }
	}
    }
}
      
message .m -aspect 300 -justify center -text "Tab and Shift/Tab may be used\
to move amongst the top three widgets. Hitting Return in any of them causes\
their current contents to be output.

The bottom entry, because of its pathname, does not belong to the same peer\
group, and is treated as a group of its own"
pack .m
frame .f
SpecialFormEntry .e1 -relief sunken -command {$this enter}
SpecialFormEntry .e2 -relief sunken -command {$this enter}
SpecialFormEntry .e3 -relief sunken -command {$this enter}
pack .e1 .e2 -in .f -side left -padx 5 -pady 5
frame .g
SpecialFormEntry .g.e4 -relief sunken -command {$this enter}
pack .g.e4 -pady 5
pack .f .e3 .g -padx 5 -pady 5
focus .e1

Window . -title "FormEntry demo"
