#!/usr/local/bin/itcl_wish -f

# Viewport demo
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

proc Apply {} {
    set bars ""

    set horiz [RadioButton :: chosen horiz]
    if {![string match neither $horiz]} {
	lappend bars $horiz
    }

    set vert [RadioButton :: chosen vert]
    if {![string match neither $vert]} {
	lappend bars $vert
    }

    .v configure -scroll $bars
}

Scanvas .c -width 300 -height 200 -scrollregion {0 0 600 400} -relief ridge
for {set v 20} {$v <= 200} {incr v 20} {
    .c create line 0 $v 300 $v
    set vv [expr $v + 200]
    .c create line 300 $vv 600 $vv
}
for {set h 20} {$h < 300} {incr h 20} {
    .c create line $h 220 $h 400
    set hh [expr $h + 300]
    .c create line $hh 0 $hh 200
}
Viewport .v .c

message .t -aspect 300 -justify center -text "This demonstration involves an\
Scanvas inside a Viewport. In other words the canvas can be scrolled either\
by dragging with middle mouse button 2 held down (scan scrolling) or by\
using scrollbars.

Specify which scrollbars you want to be present through the\
radiobuttons below and then press the apply button to configure the new\
attachments."

frame .f
frame .l
frame .m
frame .r
pack .l .m .r -in .f -side left -padx 5

RadioButton .left -text left -group horiz
RadioButton .top -text top -group vert -anchor w
pack .left .top -in .l -fill x -pady 5
.left select

RadioButton .right -text right -group horiz -anchor w
RadioButton .bottom -text bottom -group vert
pack .right .bottom -in .m -fill x -pady 5

RadioButton .hnone -text neither -group horiz -value neither
RadioButton .vnone -text neither -group vert -value neither
pack .hnone .vnone -in .r -fill x -pady 5

frame .b
button .q -text quit -command exit
button .a -text apply -command Apply
pack .q .a -in .b -side left -padx 5

pack .v -padx 5 -pady 5
pack .t .f .b -pady 5

Window . -title "Viewport demo"
