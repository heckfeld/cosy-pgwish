set tixBal(active) 1

proc tixEnableBalloon {} {
    global tixBal
    set tixBal(active) 1
}

proc tixDisableBalloon {} {
    global tixBal
    set tixBal(active) 0
}

proc tixAddBalloon {w msg {initWait 200}} { 
    global tixBal

    set class [winfo class $w]

    bind $w <Any-Enter>   +[bind $class <Any-Enter>]
    bind $w <Any-Enter>  "+tixBalStart $w $initWait"
    bind $w <Any-Leave>   +[bind $class <Any-Leave>]
    bind $w <Any-Leave>  "+tixBalEnd $w $initWait"
    bind $w <Any-Motion>  +[bind $class <Motion>]
    bind $w <Any-Motion> "+tixBalMotion $w $initWait"
    bind $w <Destroy>     +[bind $class <Destroy>]
    bind $w <Destroy>     "+unset tixBal($w)"

    set tixBal($w) $msg
}

proc tixBalStart {w initWait} {
    global tixBal

    if {$tixBal(active) == "1"} {
   	set tixBal(count)  0
    	set tixBal(popped) 0
    	after $initWait tixBalActivate $w
    }
}

proc tixBalEnd {w initWait} {
    global tixBal

    set tixBal(count)  -1
    if {$tixBal(active) == "1" && $tixBal(popped) == "1"} {
        tixBalPopdown
    } 
}

proc tixBalMotion {w initWait} {
    global tixBal

    if {$tixBal(active) == "1" && $tixBal(popped) == 0} {
	incr tixBal(count)
        after $initWait tixBalActivate $w
    }
}

proc tixBalActivate {w} {
    global tixBal

    if {$tixBal(count) == "-1"} {
        return
    } elseif {$tixBal(count) == 0} {
        tixBalPopup $w
    } else {
        incr tixBal(count) -1
    }
}

proc tixBalPopup {widget} {
    global tixBal TIX_SOURCE_DIR

    set w .tix_balloon

    if {[winfo exists $w] == 0} {
	toplevel $w -bd 0
	set bg    [tixQueryAppResource tixBalloonBg TixBalloonBg #ffff60]
	set fg    [tixQueryAppResource tixBalloonFg TixBalloonFg black]
	set width [tixQueryAppResource tixBalloonWidth TixBalloonWidth 180]
	wm overrideredirect $w 1
	# Frame 1 : arrow
	frame $w.f1 -bg $bg -bd 0
	label $w.f1.lab -bg $bg -fg $fg -bd 0 -relief flat \
	    -bitmap @$TIX_SOURCE_DIR/bitmaps/balArrow
	pack $w.f1.lab -side left

	# Frame 2 : Message
	frame $w.f2 -bg $bg -bd 0
	message $w.f2.msg -bg $bg -fg $fg -width $width -padx 0 -pady 0 -bd 0
	pack $w.f2.msg -side left -expand yes -fill both -padx 10 -pady 1

	# Pack all
	pack $w.f1 -fill both
	pack $w.f2 -fill both
    }

    $w.f2.msg config -text $tixBal($widget)
    set x [expr [winfo rootx $widget]+[winfo width  $widget]]
    set y [expr [winfo rooty $widget]+[winfo height $widget]]

    wm geometry $w +$x+$y
    wm deiconify $w
    raise $w
    set tixBal(popped) 1
}

proc tixBalPopdown {} {
    global tixBal
    set w .tix_balloon

    wm withdraw $w
    set tixBal(popped) 0
}

proc tixQueryAppResource {name class default} {

    set value [option get . $name $class]
    if {$value == ""} {
	return $default
    } else {
	return $value
    }    
}
