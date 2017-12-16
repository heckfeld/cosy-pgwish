#----------------------------------------------------------------------
#
#
#           Package loading
#
#
#----------------------------------------------------------------------
proc setenv {name args} {
    global env

    if {[llength $args] == 1} {
        return [set env($name) [lindex $args 0]]
    } else {
        if {[info exists env($ename)] == 0} {
            tkerror "Error in setenv: "
                    "environment variable \"$name\" does not exist"
        } else {
            return $env($name)
        }
    }
}
#----------------------------------------------------------------------
#
#
#           Handling options in a command
#
#
#----------------------------------------------------------------------
proc tixHandleOptions {record validOptions arglist} {
    upvar $record data

    set len [expr [llength $arglist]-2 ]
    set i 0

    while {$i <= $len} {
	set option [lindex $arglist $i]
	incr i
	set arg [lindex $arglist $i]
	incr i

	if {[lsearch $validOptions $option] == "-1"} {
	    return -code error \
		"unknown option $option; must be one of $validOptions"
	}
	set data($option) $arg
    }
}


#----------------------------------------------------------------------
#
#
#           U T I L I T Y   F U N C T I O N S  F O R   T I X 
#
#
#----------------------------------------------------------------------

# RESET THE STRING IN THE ENTRY
proc tixSetEntry {entry string} {
    set oldstate [lindex [$entry config -state] 4]
    $entry config -state normal
    $entry delete 0 end
    $entry insert 0 $string
    $entry config -state $oldstate
}

# GET THE FIRST SELECTED ITEM IN A LIST
proc tixListGetSingle {lst} {
    set indices [$lst curselection]
    if {$indices != "" } {
	return [$lst get [lindex $indices 0]]
    } else {
	return ""
    }
}
#-------------------------------
# DISABLE EVERYTHING IN A WINDOW
#-------------------------------
proc tixDisableAll {w} {
    foreach x [tixDescendants $w] {
	catch {$x config -state disabled}
    }
}
#------------------------------
# ENSABLE EVERYTHING IN A WINDOW
#------------------------------
proc tixEnableAll {w} {
    foreach x [tixDescendants $w] {
	catch {$x config -state normal}
    }
}
#------------------------------
# RETURN EVERYTHING IN A WINDOW
#------------------------------
proc tixDescendants {parent} {
    set des {}
    lappend des $parent

    foreach w [winfo children $parent] {
	foreach x [tixDescendants $w] {
	    lappend des $x
	}
    }
    return $des
}
#----------------------------------------------------------------------
# RECORD A DIALOG'S POSITION AND RESTORE IT THE NEXT TIME IT IS OPENED
#----------------------------------------------------------------------
proc tixDialogRestore {w {flag -geometry}} {
    global tixDPos

    if [info exists tixDPos($w)] {
	if ![winfo ismapped $w] {
	    wm geometry $w $tixDPos($w)
	    wm deiconify $w
	}
    } elseif {$flag == "-geometry"} {
	update
	set tixDPos($w) [winfo geometry $w]
    } else {
	update
	set tixDPos($w) +[winfo rootx $w]+[winfo rooty $w]
    }
}
#----------------------------------------------------------------------
# RECORD A DIALOG'S POSITION AND RESTORE IT THE NEXT TIME IT IS OPENED
#----------------------------------------------------------------------
proc tixDialogWithdraw {w {flag -geometry}} {
    global tixDPos

    if [winfo ismapped $w] {
	if {$flag == "-geometry"} {
	    set tixDPos($w) [winfo geometry $w]
	} else {
	    set tixDPos($w) +[winfo rootx $w]+[winfo rooty $w]
	}
	wm withdraw $w
    }
}
#----------------------------------------------------------------------
# RECORD A DIALOG'S POSITION AND RESTORE IT THE NEXT TIME IT IS OPENED
#----------------------------------------------------------------------
proc tixDialogDestroy {w {flag -geometry}} {
    global tixDPos

    if [winfo ismapped $w] {
	if {$flag == "-geometry"} {
	    set tixDPos($w) [winfo geometry $w]
	} else {
	    set tixDPos($w) +[winfo rootx $w]+[winfo rooty $w]
	}
    }
    destroy $w
}
