proc tixDlgButtons {w args} {
    eval tixInt_CreateWidget $w tixDlb TixDlgBtns $args
    return $w
}

proc tixDlgBtns {w args} {
    eval tixInt_CreateWidget $w tixDlb TixDlgBtns $args
    return $w
}

proc tixDlb::CreateClassRec {} {
    global tixDlb

    # OPTIONS
    set tixDlb(rootOptions)   {}
    set tixDlb(options)       {-anchor -background -bg -font -state 
                               -foreground -fg -buttonbg -orientation -orient
			       -padx -pady}
    set tixDlb(staticOptions) {}

    # DEFAULT VALUES
    set tixDlb(-anchor)       {-anchor anchor Anchor center}
    set tixDlb(-background)   {-background background Background #ffe4c4}
    set tixDlb(-buttonbg)     {-buttonbg buttonBg Background #ffe4c4}
    set tixDlb(-foreground)   {-foreground foreground Foreground black}
    set tixDlb(-font)         {-font font Font \
	                     "-*-helvetica-medium-r-normal-*-14-*-*-*-*-*-*-*"}
    set tixDlb(-orientation)  {-orientation orientation Orientation horizontal}
    set tixDlb(-padx)         {-padx padx Pad 0}
    set tixDlb(-pady)         {-pady pady Pad 0}
    set tixDlb(-state)        {-state state State normal}

    # ALIASES
    set tixDlb(-bg)           {= -background}
    set tixDlb(-fg)           {= -foreground}
    set tixDlb(-orient)       {= -orientation}
    # METHODS
    set tixDlb(methods)       {add invoke button}
    set tixDlb(staticOptions) {-orientation}
}

proc tixDlb::ConstructWidget {w} {
    # At the beginning we have no contents
}

proc tixDlb::InitWidgetRec {w class className args} {
    upvar #0 $w data

    set data(items)   {}
}

#----------------------------------------------------------------------
#                           CONFIG OPTIONS
#----------------------------------------------------------------------
proc tixDlb::config-anchor {w arg} {
    upvar #0 $w data

    foreach item $data(items) {
	$w.$item config -anchor $arg
    }
}

proc tixDlb::config-background {w arg} {
    upvar #0 $w data

    $data(rootCmd) config -bg $arg
}

proc tixDlb::config-buttonbg {w arg} {
    upvar #0 $w data

    foreach item $data(items) {
	$w.$item config -bg $arg
    }
}

proc tixDlb::config-font {w arg} {
    upvar #0 $w data

    foreach item $data(items) {
	$w.$item config -font $arg
    }
}

proc tixDlb::config-foreground {w arg} {
    upvar #0 $w data

    foreach item $data(items) {
	$w.$item config -fg $arg
    }
}

proc tixDlb::config-state {w arg} {
    upvar #0 $w data

    if {$arg == $data(-state)} {
	return
    } else {
	foreach item $data(items) {
	    $w.$item config -state $arg
	}
    }
}

#----------------------------------------------------------------------
# Methods
#                     WIDGET COMMANDS
#----------------------------------------------------------------------
proc tixDlb::add {w name args} {
    upvar #0 $w data

    eval button $w.$name \
	-fg $data(-foreground) \
	-bg $data(-buttonbg) \
	-font $data(-font) \
	$args
    if {$data(-orientation) == "horizontal"} {
	pack $w.$name -side left -expand yes -fill y\
	    -padx $data(-padx) -pady $data(-pady)
    } else {
	pack $w.$name -side top -expand yes  -fill x\
	    -padx $data(-padx) -pady $data(-pady)
    }

    lappend data(items) $name

    return $w.$name
}

proc tixDlb::button {w name args} {
    upvar #0 $w data

    if {$args != {}} {
	return [eval $w.$name $args]
    } else {
	return $w.$name
    }
}

#call the command
proc tixDlb::invoke {w name} {
    upvar #0 $w data

    $w.$name invoke
}
