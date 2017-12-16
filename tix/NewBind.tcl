proc tixClassBindAutoScroll {w time unit limit func} {
  global tix_priv
  set tix_priv(as_ignore) 0 
  set tix_priv(as_state)  STOPPED
  set tix_priv(as_loaded) yes

  bind $w <B1-Leave>     "+tixAutoS_Start  %W $time $unit $limit $func %x %y"
  bind $w <B1-Motion>    "+tixAutoS_Motion %W $time $func %x %y"
  bind $w <B1-Enter>         "+tixAutoS_Stop %W"
  bind $w <ButtonRelease-1>  "+tixAutoS_Stop %W"
}

proc tixWidgetBindAutoScroll {w class} {
    global tix_priv
    set tix_priv(as_ignore) 0 
    set tix_priv(as_state)  STOPPED

    bind $w <B1-Leave>         "+[bind $class <B1-Leave>]"
    bind $w <B1-Enter>         "+[bind $class <B1-Enter>]"
    bind $w <B1-Motion>        "+[bind $class <B1-Motion>]"
    bind $w <ButtonRelease-1>  "+[bind $class <ButtonRelease-1>]"
}

proc tixAutoS_Start {w time unit limit func x y} {
    global tix_priv

    set tix_priv(as_xcount) 0
    set tix_priv(as_ycount) 0
    set tix_priv(as_state)  STARTED
    set tix_priv(as_width)  [winfo width  $w]
    set tix_priv(as_height) [winfo height $w]
    set tix_priv(as_unit)   $unit
    set tix_priv(as_limit)  $limit

    if {$tix_priv(as_ignore) == 0} {
	tixAutoS_Calc $w $x $y
	tixAutoS_ReSend $w $time $func
    } else {
	puts conf
    }
}

proc tixAutoS_Stop {w} {
    global tix_priv

    set tix_priv(as_state) STOPPED
}

proc tixAutoS_Motion {w time func x y} {
    global tix_priv

    if {$tix_priv(as_state) != "STOPPED"} {
       tixAutoS_Calc $w $x $y
       tixAutoS_Send $w $func
    }
}

proc tixAutoS_Send {w func} {
    global tix_priv

    if {$tix_priv(as_dx) < $tix_priv(as_unit)} {
	incr tix_priv(as_xcount) $tix_priv(as_dx)
	if {$tix_priv(as_xcount) >= $tix_priv(as_unit)} {
	    set dx 1
	    set tix_priv(as_xcount) 0
	} else {
	    set dx 0
	}
    } else {
	set dx [expr "$tix_priv(as_dx) / $tix_priv(as_unit)"]
    }

    if {$tix_priv(as_dy) < $tix_priv(as_unit)} {
	incr tix_priv(as_ycount) $tix_priv(as_dy)
	if {$tix_priv(as_ycount) >= $tix_priv(as_unit)} {
	    set dy 1
	    set tix_priv(as_ycount) 0
	} else {
	    set dy 0
	}
    } else {
	set dy [expr "$tix_priv(as_dy) / $tix_priv(as_unit)"]
    }

    if {$dx > $tix_priv(as_limit)} {
	set dx $tix_priv(as_limit)
    }
    if {$dy > $tix_priv(as_limit)} {
	set dy $tix_priv(as_limit)
    }

    eval $func $w $tix_priv(as_dir) $dx $dy

}

proc tixAutoS_ReSend {w time func} {
    global tix_priv

    if {$tix_priv(as_state) != "STOPPED"} {
	tixAutoS_Send $w $func
	after $time tixAutoS_ReSend $w $time $func
    }
}

proc tixAutoS_Calc {w x y} {
    global tix_priv

    if {$x <= 0} {
	if {$y <= 0} {
	    set tix_priv(as_dir) nw
	} elseif {$y >= $tix_priv(as_height)} {
	    set tix_priv(as_dir) sw
	} else {
	    set tix_priv(as_dir) w
	}
    } elseif {$x >= $tix_priv(as_width)} {
	if {$y <= 0} {
	    set tix_priv(as_dir) ne
	} elseif {$y >= $tix_priv(as_height)} {
	    set tix_priv(as_dir) se
	} else {
	    set tix_priv(as_dir) e
	}
    } elseif {$y <= 0} {
	set tix_priv(as_dir) n
    } elseif {$y >= $tix_priv(as_height)} {
	set tix_priv(as_dir) s
    } else {
	set tix_priv(as_dir) c
    }

    if {$x <= 0} {
	set dx [expr "0 - $x"]
    } elseif {$x >= $tix_priv(as_width)} {
	set dx [expr "$x - $tix_priv(as_width)"]
    } else {
	set dx 0
    }

    if [catch {set tix_priv(as_dx) [expr "round($dx * log($dx))"]}] {
	set tix_priv(as_dx) 0
    }

    if {$y <= 0} {
	set dy [expr "0 - $y"]
    } elseif {$y >= $tix_priv(as_height)} {
	set dy [expr "$y - $tix_priv(as_height)"]
    } else {
	set dy 0
    }

    if [catch {set tix_priv(as_dy) [expr "round($dy * log($dy))"]}] {
	set tix_priv(as_dy) 0
    }
}

proc tixAutoS_Update {} {
    global tix_priv

    set tix_priv(as_ignore) 1
    update idletasks
    set tix_priv(as_ignore) 0
}

#----------------------------------------------------------------------
# Scroll for text
#----------------------------------------------------------------------
proc tixAutoS_Text {w dir dx dy} {
    global tix_priv

    if {$dy == 0} {
	return
    }

    case $dir {
	{n nw ne} {
	    tk_textSelectTo $w "@0,0 -$dy line"
	    $w yview [$w index "@0,0 -$dy line"]
	    tixAutoS_Update
	}
	{s sw se} {
	    tk_textSelectTo $w \
		"@$tix_priv(as_width),$tix_priv(as_height) +$dy line"
	    $w yview [$w index "@0,0 +$dy line"]
	    tixAutoS_Update
	}
    }
}

proc tixAutoS_Entry {w dir dx dy} {
    global tix_priv

    if {$dx == 0} {
	return
    }

    case $dir {
	{w nw sw} {
	    $w icursor [expr "[$w index insert] - $dx"]
	    $w select to insert
	    tk_entrySeeCaret $w
	    tixAutoS_Update
	}
	{e ne se} {
	    $w icursor [expr "[$w index insert] + $dx"]
	    $w select to insert
	    tk_entrySeeCaret $w
	    tixAutoS_Update
	}
    }
}

proc tixAutoS_ListboxSingle {w dir dx dy} {
    global tix_priv


    case $dir {
	{n} {
	    if {$dy == 0} {
		return
	    }
	    set sel [expr "[$w nearest 0]-$dy"]
	    $w yview $sel
	    $w select from $sel
	    $w select to $sel
	    tixAutoS_Update	
	}
	{s} {
	    if {$dy == 0} {
		return
	    }
	    set height [winfo height $w]
	    set sel [expr "[$w nearest $height]+$dy"]
	    $w yview [expr "[$w nearest 0]+$dy"]
	    $w select from $sel
	    $w select to $sel
	    tixAutoS_Update
	}
	{e} {

	}
	{w} {

	}
    }
}

proc tixAutoS_Listbox {w dir dx dy} {
    global tix_priv

    case $dir {
	{n} {
	    if {$dy == 0} {
		return
	    }
	    set sel [expr "[$w nearest 0]-$dy"]
	    $w yview $sel
	    $w select to $sel
	    tixAutoS_Update	
	}
	{s} {
	    if {$dy == 0} {
		return
	    }
	    set height [winfo height $w]
	    set sel [expr "[$w nearest $height]+$dy"]
	    $w yview [expr "[$w nearest 0]+$dy"]
	    $w select to $sel
	    tixAutoS_Update
	}
	{e} {

	}
	{w} {

	}
    }
}


# New bindings
proc tixDefaultAutoBind {} {
    global tix_priv

    tixClassBindAutoScroll Entry \
	$tix_priv(as_entRate) \
	$tix_priv(as_entUnit) \
	$tix_priv(as_entLimit)\
	tixAutoS_Entry
    tixClassBindAutoScroll Listbox \
	$tix_priv(as_listRate) \
	$tix_priv(as_listUnit) \
	$tix_priv(as_listLimit)\
	tixAutoS_Listbox
    tixClassBindAutoScroll Text \
	$tix_priv(as_textRate) \
	$tix_priv(as_textUnit) \
	$tix_priv(as_textLimit)\
	tixAutoS_Text
}

proc tixAutoS_BindListbox {w} {
    tixWidgetBindAutoScroll $w Listbox
}

proc tixAutoS_BindListboxSingle {w} {
  global tix_priv

  set time $tix_priv(as_listRate)
  set unit $tix_priv(as_listUnit)
  set limit $tix_priv(as_listLimit)
  set func tixAutoS_ListboxSingle

  set tix_priv(as_ignore) 0 
  set tix_priv(as_state)  STOPPED
  set tix_priv(as_loaded) yes

  bind $w <B1-Leave>     "+tixAutoS_Start  %W $time $unit $limit $func %x %y"
  bind $w <B1-Motion>    "+tixAutoS_Motion %W $time $func %x %y"
  bind $w <B1-Enter>         "+tixAutoS_Stop %W"
  bind $w <ButtonRelease-1>  "+tixAutoS_Stop %W"
}

proc tixAutoS_BindEntry {w} {
    tixWidgetBindAutoScroll $w Entry
}

proc tixAutoS_BindText {w} {
    tixWidgetBindAutoScroll $w Text 30 tixAutoS_Text
}

set tix_priv(as_listRate)  30
set tix_priv(as_listUnit)  130
set tix_priv(as_listLimit) 10

set tix_priv(as_textRate)  30
set tix_priv(as_textUnit)  130
set tix_priv(as_textLimit) 45

set tix_priv(as_entRate)   30
set tix_priv(as_entUnit)   130
set tix_priv(as_entLimit)  100
