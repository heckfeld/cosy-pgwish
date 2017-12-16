#!/usr/local/bin/itcl_wish -f

# Login window demo
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

itcl_class PasswordEntry {
    inherit SecretEntryBindings EmacsFormEntry

    constructor {args} {
	eval EmacsFormEntry::constructor $args
	eval SecretEntryBindings::constructor $args
    }
}

proc Login {entry text} {
    set peers [$entry peers]
    set uname [[lindex $peers 0] get]
    set pwd [[lindex $peers 1] get]
    if {$uname != "" && $pwd != ""} {
	puts "Your name is \"$uname\""
	puts "Your password is \"$pwd\""
	destroy .
    }
}

frame .l
label .llogin -text "   Login:"
EmacsFormEntry .elogin -command {Login .elogin}
pack .llogin .elogin -in .l -side left

frame .p
label .lpwd -text Password:
PasswordEntry .epwd -command {Login .epwd}
pack .lpwd .epwd -in .p -side left

pack .l .p -padx 5 -pady 5
focus .elogin
