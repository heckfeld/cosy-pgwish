proc tixControl {w args} {
    eval tixInt_CreateWidget $w tixCtl TixControl $args
    return $w
}

proc tixCtl::CreateClassRec {} {
    global tixCtl

    # OPTIONS
    set tixCtl(rootOptions)   {}
    set tixCtl(options)       {-foreground -fg -font
                               -state
	                       -value -variable -command
			       -ulimit -llimit -step -repeatrate
			       -initwait -autorepeat
			       -disabledforeground
			       -label -labelwidth -labelfont
			       -entrywidth -entrybg -buttonbg -buttonfg}
    set tixCtl(staticOptions) {}

    # DEFAULT VALUES
    set tixCtl(-label)        {-label label Label ""}
    set tixCtl(-autorepeat)   {-autorepeat autoRepeat AutoRepeat true}
    set tixCtl(-buttonbg)     {-buttonbg buttonBg ButtonBg #ffe4c4}
    set tixCtl(-foreground)   {-foreground foreground Foreground black}
    set tixCtl(-buttonfg)     {-buttonfg buttonFg Foreground black}
    set tixCtl(-disabledforeground) {\
	                       -disabledforeground\
	                       disabledForeground\
	                       DisabledForeground #606060}
    set tixCtl(-entrybg)      {-entrybg entryBg Background #ffe4c4}
    set tixCtl(-font)         {-font font Font \
	                     "-*-helvetica-medium-r-normal-*-14-*-*-*-*-*-*-*"}
    set tixCtl(-value)        {-value value Value 0}
    set tixCtl(-variable)     {-variable variable Variable {}}
    set tixCtl(-step)         {-step step Step 1}
    set tixCtl(-state)        {-state state State normal}
    set tixCtl(-ulimit)       {-ulimit ulimit Ulimit {}}
    set tixCtl(-llimit)       {-llimit llimit Llimit {}}
    set tixCtl(-repeatrate)   {-repeatrate repeatRate RepeatRate 50}
    set tixCtl(-initwait)     {-initwait initWait InitWait 300}
    set tixCtl(-entrywidth)   {-entrywidth entryWidth EntryWidth 5}
    set tixCtl(-labelwidth)   {-labelwidth labelWidth LabelWidth {}}
    set tixCtl(-labelfont)    {-labelfont labelFont Font 
	                     "-*-helvetica-medium-r-normal-*-14-*-*-*-*-*-*-*"}

    set tixCtl(-command)      {-command command Command {}}

 #obsolete
    set tixCtl(-orient)       {-orient orient Orient horizontal}

    # ALIASES
    set tixCtl(-fg)           {= -foreground}
    # METHODS
    set tixCtl(methods)       {get incr decr invoke}
}

proc tixCtl::ConstructWidget {w} {
    upvar #0 $w data
    global TIX_SOURCE_DIR

    set data(ent)  [entry $w.ent \
		    -width $data(-entrywidth) \
		    -fg $data(-foreground) \
		    -bg $data(-entrybg) \
		    -font $data(-font)]
    set data(lab)  [label $w.lab \
		    -fg $data(-foreground) \
		    -text $data(-label) \
		    -font $data(-labelfont)]
    if {$data(-labelwidth) != ""} {
	$data(lab) config -width $data(-labelwidth)
    }

    frame $w.f1

    set data(incr) [button $w.f1.incr -bitmap @$TIX_SOURCE_DIR/bitmaps/incr \
		   -bg $data(-buttonbg)\
		   -fg $data(-buttonfg)]
    set data(decr) [button $w.f1.decr -bitmap @$TIX_SOURCE_DIR/bitmaps/decr \
		   -bg $data(-buttonbg)\
		   -fg $data(-buttonfg)]

    pack $data(decr) -side bottom -fill both -expand yes
    pack $data(incr) -side top    -fill both -expand yes

    pack $w.lab -side left  -fill both -padx 2
    pack $w.ent -side left  -expand yes -fill both
    pack $w.f1  -side right -fill y

    $w.ent delete 0 end
    $w.ent insert 0 $data(-value)

    bind $data(incr) <1> \
	"tk_butDown %W; set [set w](flag) 1; tixCtl::IncrCmd $w"
    bind $data(decr) <1> \
	"tk_butDown %W; set [set w](flag) 1; tixCtl::DecrCmd $w"

    bind $data(incr) <ButtonRelease-1> "tk_butUp %W; set [set w](flag) 0"
    bind $data(decr) <ButtonRelease-1> "tk_butUp %W; set [set w](flag) 0"
    bind $data(incr) <B1-Leave> "tk_butUp %W; set [set w](flag) 0"
    bind $data(decr) <B1-Leave> "tk_butUp %W; set [set w](flag) 0"
    bind $data(incr) <B1-Motion> "tk_butUp %W; set [set w](flag) 0"
    bind $data(decr) <B1-Motion> "tk_butUp %W; set [set w](flag) 0"
    bind $w.ent  <Return> "tixCtl::invoke $w"

    if {$data(-variable) != {}} {
	tixCtl::config-variable $w $data(-variable)
    }
}

#----------------------------------------------------------------------
#                           CONFIG OPTIONS
#----------------------------------------------------------------------
proc tixCtl::config-buttonbg {w arg} {
    upvar #0 $w data

    $data(incr) config -bg $arg
    $data(decr) config -bg $arg
}

proc tixCtl::config-foreground {w arg} {
    upvar #0 $w data

    $w.ent  config -fg $arg
    $data(incr) config -fg $arg
    $data(decr) config -fg $arg
}

proc tixCtl::config-state {w arg} {
    upvar #0 $w data

    if {$arg == "normal"} {
	$data(incr) config -state $arg
	$data(decr) config -state $arg
	$w.ent  config -state $arg -fg $data(-foreground)
    } else {
	$data(incr) config -state $arg
	$data(decr) config -state $arg
	$w.ent  config -state $arg -fg $data(-disabledforeground)
    }
}

proc tixCtl::config-entrybg {w arg} {
    upvar #0 $w data

    $w.ent config -bg $arg
}

proc tixCtl::config-font {w arg} {
    upvar #0 $w data

    $w.ent config -font $arg
}

proc tixCtl::config-entrywidth {w arg} {
    upvar #0 $w data

    $data(ent) config -width $arg
}

proc tixCtl::config-command {w arg} {
    # nothing needed here
}

proc tixCtl::config-value {w arg} {
    upvar #0 $w data

    if {$data(-llimit) != {} && $arg < $data(-llimit)} {
	puts error
	return
    }
    if {$data(-ulimit) != {} && $arg > $data(-llimit)} {
	puts error
	return
    }
    $data(ent) delete 0 end
    $data(ent) insert 0 $arg
}

proc tixCtl::config-variable {w arg} {
    upvar #0 $w data

    if {$data(-variable) != {}} {
	upvar #0 $data(-variable) oldVar
	trace vdelete oldVar w "tixCtl::TraceProc $w"
    }

    if {$arg != {}} {
	upvar #0 $arg variable
	if [info exists $arg] {
	    $w config -value $variable
	} else {
	    set variable $data(-value)
	}
	upvar #0 $arg newVar
	trace variable newVar w "tixCtl::TraceProc $w"
    }
}

#----------------------------------------------------------------------
#                         User Commands
#----------------------------------------------------------------------
proc tixCtl::get {w} {
    upvar #0 $w data

    return $data(-value)
}

proc tixCtl::incr {w} {
    tixCtl::IncrCmd $w
}

proc tixCtl::decr {w} {
    tixCtl::DecrCmd $w
}

proc tixCtl::invoke {w} {
    upvar #0 $w data

    if {[catch {$data(ent) index sel.first}] == 0} {
	# OWNS SELECTION --> TURN IT OFF
	$data(ent) select from end
	$data(ent) select to   end
    }
    set oldvalue $data(-value)
    set newvalue [$data(ent) get]
    if [catch {set data(-value) [format "%d" $newvalue]}] {
	if [catch {set data(-value) [format "%f" $newvalue]}] {
	    return -code error\
		"expected integer or float but got \"$newvalue\""
	}
    }

    # Check the ulimit and llimit.
    if [catch {
	if {$data(-ulimit) != {} && $data(-value) > $data(-ulimit)} {
	    set data(-value) $data(-ulimit)
	    $data(ent) delete 0 end
	    $data(ent) insert 0 $data(-value)
	}
 	if {$data(-llimit) != {} && $data(-value) < $data(-llimit)} {
	    set data(-value) $data(-llimit)
	    $data(ent) delete 0 end
	    $data(ent) insert 0 $data(-value)
	}
    }] {
	set data(-value) $oldvalue
	$data(ent) delete 0 end
	$data(ent) insert 0 $data(-value)
    }

    tixCtl::UpdateVariable $w

    if {$data(-command) != {}} {
	eval $data(-command) $data(-value)
    }
}


#----------------------------------------------------------------------
#                       Internal Commands
#----------------------------------------------------------------------
proc tixCtl::IncrCmd {w} {
    upvar #0 $w data

    if {$data(flag) != "0"} {
	set data(-value) [expr "$data(-value) + $data(-step)"]
	if {$data(-ulimit) != {} && $data(-value) > $data(-ulimit)} {
	    set data(-value) $data(-ulimit)
	}
	$data(ent) delete 0 end
	$data(ent) insert 0 $data(-value)

	tixCtl::UpdateVariable $w

	if {$data(-command) != {}} {
	    eval $data(-command) $data(-value)
	}

	if {$data(-autorepeat) == "true"} {
	    if {$data(flag) == "1"} {
		after $data(-initwait) tixCtl::IncrCmd $w
		set data(flag) 2
	    } else {
		after $data(-repeatrate) tixCtl::IncrCmd $w
	    }
	}
    }
}

proc tixCtl::DecrCmd {w} {
    upvar #0 $w data

    if {$data(flag) != "0"} {
	set data(-value) [expr "$data(-value) - $data(-step)"]
	if {$data(-llimit) != {} && $data(-value) < $data(-llimit)} {
	    set data(-value) $data(-llimit)
	}
	$data(ent) delete 0 end
	$data(ent) insert 0 $data(-value)

	tixCtl::UpdateVariable $w

	if {$data(-command) != {}} {
	    eval $data(-command) $data(-value)
	}
	if {$data(-autorepeat) == "true"} {
	    if {$data(flag) == "1"} {
		after $data(-initwait) tixCtl::DecrCmd $w
		set data(flag) 2
	    } else {
		after $data(-repeatrate) tixCtl::DecrCmd $w
	    }
	}
    }
}

proc tixCtl::UpdateVariable {w} {
    upvar #0 $w data

    if {$data(-variable) != {}} {
	upvar #0 $data(-variable) variable

	trace vdelete  variable w "tixCtl::TraceProc $w"
	set variable $data(-value)
	trace variable variable w "tixCtl::TraceProc $w"
    }
}

proc tixCtl::TraceProc {w name1 name2 op} {
    upvar #0 $name1 var

    if {$name2 == {}} {
	$w config -value $var
    } else {
	$w config -value $var($name2)
    }
}
	
