#!/usr/local/bin/itcl_wish -f

# Class viewer
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

option add *bitmapFilePath .

set here [pwd]
set demos [glob {*tcl}]

if {![catch "cd $WIGWAMLIB"]} {
    set classes [glob {[A-Z]*tcl}]
    cd $here
} else {
    puts "Unable to access $WIGWAMLIB"
    exit
}

itcl_class FileDisplayer {
    inherit itcl_text

    constructor {args} {
	eval itcl_text::constructor $args
    }

    public file {} {
	if {$file != "" && ![catch {open $file r} fd]} {
	    tk_delete 1.0 end
	    insert end [read $fd]
	    close $fd
	}  
    }
}

proc Select {} {
    global WIGWAMLIB

    if {![winfo exists .fnw]} {
	FileNominatorWindow .fnw \
	    -title FileNominator \
	    -directory $WIGWAMLIB
	.fnw.nominator configure \
	    -select {.display configure -file} \
	    -cancel {destroy .fnw}
    } else {
	raise .fnw
    }
}

button .quit \
    -text Quit \
    -command exit \

MenuBar .menus -relief raised -bd 2

MenuButton .menus.class \
    -text Classes \
    -menu .menus.class.menu \
    -underline 0 \
    -bd 0

menu .menus.class.menu
foreach class [lsort $classes] {
    regexp {(.*)\.tcl} $class {\1} item
    .menus.class.menu add command \
        -label $item \
	-command ".display configure -file  $WIGWAMLIB/$class"
}

menubutton .menus.demo \
    -text Demos \
    -menu .menus.demo.menu \
    -underline 0 \
    -bd 0

menu .menus.demo.menu
foreach demo [lsort $demos] {
    regexp {(.*)\.tcl} $demo {\1} item
    .menus.demo.menu add command \
        -label $item \
	-command ".display configure -file $demo"
}

.menus pack .menus.class .menus.demo -side left -fill y

button .fnom \
    -text "File selector" \
    -command Select 

label .which \
    -text "Version [wigwam_version]" -relief ridge

Viewport .view {FileDisplayer .display \
    -relief ridge \
    -bd 2 -setgrid true}

pack .view -side bottom -fill both -expand true -padx 2 -pady 2
pack .quit .fnom .menus -side left -fill y -padx 2 -pady 2
pack .which -side left -expand true -fill both -padx 2 -pady 2
.menus.class.menu invoke 0

Window . -title "wigwam" -minsize {1 1}
