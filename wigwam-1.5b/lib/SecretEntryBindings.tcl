# SecretEntryBindings 
# ----------------------------------------------------------------------
# Abstract base class that can be attached under multiple inheritance to
# any subclass of BasicEntry, enabling text that is entered to be secret.
# The real text is secreted in a shadow BasicEntry widget.
#
# SecretEntryBindings overrides BasicEntry's tk_delete, entryBackspace, 
# get, icursor and insert methods, and hence must appear earlier than (the
# subclass of) BasicEntry in inherit statements.
#
#   PUBLIC ATTRIBUTES :
#
#     -echo .......... a boolean specifying whether an asterisk should
#                      be echoed for each character typed
#
#   METHODS :
#
#     None
#
# ----------------------------------------------------------------------
#   AUTHOR:  Jim Wight        <j.k.wight@newcastle.ac.uk>
# ----------------------------------------------------------------------
# Copyright 1994 The University of Newcastle upon Tyne (see COPYRIGHT)
# ======================================================================

itcl_class SecretEntryBindings {
    constructor {args } {
	set class [virtual info class]
	set heritage [virtual info heritage]

	if {[lsearch -exact $heritage BasicEntry] == -1} {
	    error "$class: SecretEntryBindings not attached to a BasicEntry (or subclass)"
	}

	if {[lsearch -exact $heritage BasicEntry] < \
		              [lsearch -exact $heritage SecretEntryBindings]} {
	    error "$class: SecretEntryBindings should occur earlier than (subclass of) BasicEntry in inherit statement"
	}

	if {![winfo exists $this]} {
	    error "No BasicEntry window for $this during SecretEntryBindings construction.\nCall BasicEntry (or subclass) constructor before SecretEntryBinding's"
	}

	set shadow [BasicEntry ${this}-shadow]

	if {![string match "" $args]} {
	    for {set arg 0} {$arg < [llength $args]} {incr arg 2} {
		set option [string range [lindex $args $arg] 1 end]
		if {[$shadow info public $option] != {}} {
		    $shadow configure -$option [lindex $args [expr $arg + 1]]
		}
	    }
	}
    } 

    method tk_delete {first {last ""}} {
        #
        # NB first and last might be sel.first and sel.last, which are
        #    inappropriate for the shadow widget
        #
        set from [$this index $first]
        if {$last != ""} {
            set to [$this index $last]
        } else {
            set to ""
        }

        eval $shadow tk_delete $from $to
	if {$toecho} {
	    eval $this BasicEntry::tk_delete $from $to
	}
    }

    method entryBackspace {} {
	$shadow entryBackspace
	if {$toecho} {
	    $this BasicEntry::entryBackspace
	}
    }

    method get {} {
	return [$shadow get]
    }

    method icursor {index} {
	$shadow icursor $index
	[virtual set tk_widget] icursor $index
    }

    method insert {index string} {
	$shadow insert $index $string
	if {$toecho} {
	    [virtual set tk_widget] insert $index "*"
	}
    }

    method toecho {} {return $toecho}

    public echo {} {
	switch [string tolower $echo] {
	    true    -               tru -   tr -    t -
	    yes     -               ye  -   y  -
	    on      -               o   -
	    1       {set toecho 1}
	    default {set toecho 0}
	}
    }

    public textvariable {} {
	global ${textvariable}-object ${textvariable}-widget

	$shadow configure -textvariable $textvariable
	set ${textvariable}-object $this 
	set ${textvariable}-widget $tk_widget

	# Ensure that tracing is done at global scope
	# Only way I could get it to work
	after 0 trace variable $textvariable w SecretTrace
    } 

    protected shadow
    protected toecho {1}
}

proc SecretTrace {textvar notused op} {
    global $textvar ${textvar}-object ${textvar}-widget

    set objlen [string length [[set ${textvar}-object] get]]
    set varlen [string length [set $textvar]]

    if {$objlen != $varlen} {
	[set ${textvar}-widget] delete 0 end
	if {[[set ${textvar}-object] toecho]} {
	    for {set i 0}  {$i < $varlen} {incr i} {
		[set ${textvar}-widget] insert $i *
	    }
	}
    }
}
