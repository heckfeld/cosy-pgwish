#!/usr/local/bin/itcl_wish -f

# Paned widget demo
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

Paned .ov -gripIndent .1
Paned .oh -orient horizontal
 
message .m -aspect 350 -justify center -relief ridge -text \
"This demonstration has four Paned instances. Pressing and dragging mouse\
button 1 in a grip (one of the little squares) enables the relative sizes\
of the widgets on either side to be adjusted. This widget, and those labelled\
\"a\" and \"four\" were specified as non-expandable so don't change in size\
when the window is resized."

Paned .h -orient horizontal -gripIndent .5
label .h.a -text a -relief groove -width 5 -height 15
label .h.b -text b -relief groove -width 5 -height 15
label .h.c -text c -relief groove -width 5 -height 15
label .h.d -text d -relief groove -width 5 -height 15
.h insert -noexpand .h.a 
.h insert .h.d
.h insert .h.b .h.c -before .h.d
update

.ov insert .m -noexpand
.ov insert .h
update

Paned .v
label .a -text one -relief ridge -width 20 -height 3
label .b -text two -relief ridge -width 20 -height 3
label .c -text three -relief ridge -width 20 -height 3
label .d -text four -relief ridge -width 20 -height 3
.v insert .a .b .c 
.v insert -noexpand .d
update

.oh insert .ov .v

pack .oh -fill both -expand true

wm minsize . 1 1
