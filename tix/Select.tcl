proc tixSelect {w args} {
    eval tixInt_CreateWidget $w tixSel TixSelect $args
    return $w
}

proc tixSel::CreateClassRec {} {
    global tixSel

    # OPTIONS
    set tixSel(rootOptions) {}
    set tixSel(options)     {-anchor -background -bg -command -font \
	                     -state -variable -value -buttonbg -selectedbg 
			     -allowzero -radio -orientation -orient
			     -padx -pady}
    set tixSel(staticOptions) {-orientation -radio -allowzero}

    # Intrinsics must config these options after widget creation. unimplemented
    set tixSel(delayOptions) {-variable}

    # DEFAULT VALUES
    set tixSel(-anchor)     {-anchor anchor Anchor center}
    set tixSel(-background) {-background background Background #ffe4c4}
    set tixSel(-buttonbg)   {-buttonbg buttonBg Background #ffe4c4}
    set tixSel(-selectedbg) {-selectedbg selectedBg SelectedBg #e4c4ff}
    set tixSel(-command)    {-command command Command {}}
    set tixSel(-font)       {-font font Font \
	                     "-*-helvetica-medium-r-normal-*-14-*-*-*-*-*-*-*"}
    set tixSel(-state)      {-state state State normal}
    set tixSel(-variable)   {-variable variable Variable {}}
    set tixSel(-value)      {-value value Value {}}
    set tixSel(-radio)      {-radio radio Radio true }
    set tixSel(-allowzero)  {-allowzero allowZero AllowZero false}
    set tixSel(-orientation) {-orientation orientation Orientation horizontal}
    set tixSel(-padx)       {-padx padx Pad 0}
    set tixSel(-pady)       {-pady pady Pad 0}

    # ALIASES
    set tixSel(-bg)           {= -background}
    set tixSel(-orient)       {= -orientation}
    # METHODS
    set tixSel(methods)       {add button invoke}
}


proc tixSel::ConstructWidget {w} {
    upvar #0 $w data

    if {$data(-variable) != {}} {
	tixSel::config-variable $w $data(-variable)
    }
}

proc tixSel::InitWidgetRec {w class className args} {
    upvar #0 $w data

    set data(items)   {}
}

#----------------------------------------------------------------------
#                           CONFIG OPTIONS
#----------------------------------------------------------------------
#
proc tixSel::config-anchor {w arg} {
    upvar #0 $w data

    foreach item $data(items) {
	$w.$item config -anchor $arg
    }
}

proc tixSel::config-background {w arg} {
    upvar #0 $w data

    $data(rootCmd) config -bg $arg
}

proc tixSel::config-buttonbg {w arg} {
    upvar #0 $w data

    foreach item $data(items) {
	$w.$item config -bg $arg
    }
}

proc tixSel::config-selectedbg {w arg} {
    upvar #0 $w data

    if {$data(-state) == "normal"} {
	foreach item $data(-value) {
	    $w.item config -bg $arg
	}
    }
}

proc tixSel::config-font {w arg} {
    upvar #0 $w data

    foreach item $data(items) {
	$w.$item config -font $arg
    }
}

proc tixSel::config-state {w arg} {
    upvar #0 $w data

    if {$arg == "disabled"} {
	foreach item $data(items) {
	    $w.$item config -state disabled -relief raised \
		-bg $data(-buttonbg)
	}
    } else {
	foreach item $data(items) {
	    if {$data(w:$item) == "selected"} {
		$w.$item config -relief sunken -bg $data(-selectedbg) \
		    -state normal
	    } else {
		$w.$item config -relief raised -bg $data(-buttonbg) \
		    -command "$w invoke $item" -state normal
	    }
	}
    }
}

proc tixSel::config-variable {w arg} {
    upvar #0 $w data

    if {$data(-variable) != {}} {
	trace vdelete $data(-variable) w "tixSel::TraceProc $w"
    }

    if {$arg != {}} {
	upvar #0 $arg variable
	if [info exists $arg] {
	    $w config -value $variable
	} else {
	    set variable $data(-value)
	}
	global $arg
	trace variable $arg w "tixSel::TraceProc $w"
    }
}


proc tixSel::config-variable {w arg} {
    upvar #0 $w data

    if {$data(-variable) != {}} {
	upvar #0 $data(-variable) oldVar
	trace vdelete oldVar w "tixSel::TraceProc $w"
    }

    if {$arg != {}} {
	upvar #0 $arg variable
	if [info exists $arg] {
	    $w config -value $variable
	} else {
	    set variable $data(-value)
	}
	upvar #0 $arg newVar
	trace variable newVar w "tixSel::TraceProc $w"
    }
}

proc tixSel::config-value {w arg} {
    upvar #0 $w data

    if {[lindex $arg 0] == "-nocallback"} {
	set docallback 0
	set arg [lrange $arg 1 end]
    } else {
	set docallback 1
    }

    # sanity checking
    foreach item $arg {
	if {[lsearch $data(items) $item] == "-1"} {
	    error "button subwidget \"$item\" does not exist"
	}
    }

    if {$data(-radio) == "true" && [llength $arg] > 1} {
	error "cannot choose more than one items in a radio box"
    }

    if {$data(-allowzero) == "false" && [llength $arg] == 0} {
	error "empty selection not allowed"
    }

    set data(-value) $arg
    tixSel::UpdateVariable $w

    # Reset all to be unselected
    foreach item $data(items) {
	if {[lsearch $arg $item] == "-1"} {
	    if {$data(w:$item) == "selected"} {
		# was selected but now unselected
		# -> popup the button, call command
		$w.$item config -relief raised -bg $data(-buttonbg)
		set data(w:$item) unselected
		incr data(nSelected) -1

		if {$docallback && $data(-command) != {}} {
		    eval $data(-command) $item 0
		}
	    }
	} else {
	    if {$data(w:$item) == "unselected"} {
		# was unselected but now selected
		# -> push down the button, call command
		$w.$item config -relief sunken -bg $data(-selectedbg)
		set data(w:$item) selected
		incr data(nSelected)

		if {$docallback && $data(-command) != {}} {
		    eval $data(-command) $item 1
		}
	    }
	}
    }
}

#----------------------------------------------------------------------
#                     WIDGET COMMANDS
#----------------------------------------------------------------------
proc tixSel::add {w name args} {
    upvar #0 $w data

    eval [list button $w.$name -command "$w invoke $name"\
	 -bg $data(-buttonbg) -font $data(-font)] $args

    if {$data(-orientation) == "horizontal"} {
	pack $w.$name -side left -expand yes -fill y\
	    -padx $data(-padx) -pady $data(-pady)
    } else {
	pack $w.$name -side top -expand yes  -fill x\
	    -padx $data(-padx) -pady $data(-pady)
    }

    if {$data(-state) == "disabled"} {
	$name config -relief raised -state disabled
    }

    set data(w:$name) unselected
    set data(nSelected) 0

    lappend data(items) $name
}

proc tixSel::button {w name args} {
    upvar #0 $w data

    if {$args != {}} {
	return [eval $w.$name $args]
    } else {
	return $w.name
    }
}

#call the command
proc tixSel::invoke {w name} {
    upvar #0 $w data

    if {$data(-state) != "normal"} {
	return
    }

    if {$data(w:$name) == "selected"} {
	if {$data(nSelected) > 1 || $data(-allowzero) == "true"} {
	    $w.$name config -relief raised -bg $data(-buttonbg)
	    set data(w:$name) unselected
	    incr data(nSelected) -1
	    tixSel::UpdateValue $w 

	    if {$data(-command) != {}} {
		eval $data(-command) $name 0
	    }
	}
    } else {
	if {$data(-radio) == "true"} {
	    set data(nSelected) 1
	    # cannot call Updatevalue because data(w:??) not set yet.
	    set data(-value) $name
	    tixSel::UpdateVariable $w

	    foreach item $data(items) {
		if {$item == $name} {
		    $w.$item config -relief sunken -bg $data(-selectedbg)
		    set data(w:$item) selected

		    if {$data(-command) != {}} {
			eval $data(-command) $item 1
		    }
		} elseif {$data(w:$item) == "selected"} {
		    $w.$item config -relief raised -bg $data(-buttonbg)
		    set data(w:$item) unselected

		    if {$data(-command) != {}} {
			eval $data(-command) $item 0
		    }
		}
	    }
	} else {
	    $w.$name config -relief sunken -bg $data(-selectedbg)
	    set data(w:$name) selected
	    incr data(nSelected)
	    tixSel::UpdateValue $w 

	    if {$data(-command) != {}} {
		eval $data(-command) $name 1
	    }
	}
    }
}

#----------------------------------------------------------------------
#                Private functions
#----------------------------------------------------------------------
proc tixSel::UpdateValue {w} {
    upvar #0 $w data

    set data(-value) {}

    foreach item $data(items) {
	if {$data(w:$item) == "selected"} {
	    lappend data(-value) $item
	}
    }

    tixSel::UpdateVariable $w
}


proc tixSel::UpdateVariable {w} {
    upvar #0 $w data

    if {$data(-variable) != {}} {
	upvar #0 $data(-variable) variable

	trace vdelete  variable w "tixSel::TraceProc $w"
	set variable $data(-value)
	trace variable variable w "tixSel::TraceProc $w"
    }
}

proc tixSel::TraceProc {w name1 name2 op} {
    upvar #0 $name1 var

    if {$name2 == {}} {
	$w config -value $var
    } else {
	$w config -value $var($name2)
    }
}
	
# requirement of -valiable
# 1) Always updated before -command is called
