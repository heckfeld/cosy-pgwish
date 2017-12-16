proc tixPanedWindow {w args} {
    eval tixInt_CreateWidget $w tixPnw TixPanedWindow $args
    return $w
}

proc tixPnw::CreateClassRec {} {
    global tixPnw

    # OPTIONS
    set tixPnw(rootOptions)   {}
    set tixPnw(options)       {-orientation -sashcolor \
	                       -handlebg -handleactivebg
			       -command}
    set tixPnw(staticOptions) {-orientation}

    # DEFAULT VALUES
    set tixPnw(-command)      {-command command Command {}}
    set tixPnw(-handleactivebg) {-handleactivebg handleActiveBg HandleActiveBg 
                                 bisque2}
    set tixPnw(-handlebg)     {-handlebg handleBg Background #ffe4c4}
    set tixPnw(-orientation)  {-orientation orientation Orientation vertical}
    set tixPnw(-sashcolor)    {-sashcolor sashColor SashColor red}
    # ALIASES

    # METHODS
    set tixPnw(methods)       {add pane panes update}
}

proc tixPnw::InitWidgetRec {w class className args} {
    upvar #0 $w data

    set data(items)       {}
    set data(nItems)      0
    set data(exposed)     0
    set data(totalsize)   0
    set data(pending)     0
}

# Do nothing
proc tixPnw::ConstructWidget {w} {
    upvar #0 $w data
}

proc tixPnw::SetBindings {w} {
    upvar #0 $w data

    bind $w <Configure> "tixPnw::Resize $w"
}

#----------------------------------------------------------------------
#                          CONFIG OPTIONS
#----------------------------------------------------------------------

#----------------------------------------------------------------------
#                         WIDGET COMMANDS
#----------------------------------------------------------------------
#
# options -size -max -min -allowresize
proc tixPnw::add {w name args} {
    upvar #0 $w data


    # Step 1: Parse the options to get the children's size options

    # The default values
    #
    set option(-size)        50
    set option(-min)         30
    set option(-max)         100000
    set option(-allowresize) 1

    tixHandleOptions option {-size -max -min -allowresize} $args

    set data($name,size)        $option(-size)
    set data($name,min)         $option(-min)
    set data($name,max)         $option(-max)
    set data($name,allowresize) $option(-allowresize)

    # Step 2: Add the frame and the sash (if necessary)
    #
    frame $w.$name -bd 1 -relief raised

    lappend data(items) $name
    incr data(nItems)

    if {$data(nItems) > 1} {
	tixPnw::AddSash $w
    }

    # Step 3: Add the new frame and adjust the panedWindow
    #
    if {$data(-orientation) == "vertical"} {
	place $w.$name -x 0 -relwidth 1 \
	    -y $data(totalsize) -height $data($name,size)
    } else {
	place $w.$name -y 0 -relheight 1 \
	    -x $data(totalsize) -width $data($name,size)
    }

    incr data(totalsize) $data($name,size)
}

# return the pathname
proc tixPnw::pane {w name args} {
    upvar #0 $w data

    if {$args != {}} {
	return [eval $w.$name $args]
    } else {
	return $w.name
    }
}

# return the name of all panes
proc tixPnw::panes {w} {
    upvar #0 $w data

    return $data(items)
}

#
# calculate the size of the whole window.
#
proc tixPnw::update {w args} {
    upvar #0 $w data

    if {$data(-orientation) == "vertical"} {
	set width 200
	foreach name $data(items) {
	    set new_w [winfo reqwidth $w.$name]
	    if {$new_w > $width} {
		set width $new_w
	    }
	}
	$data(rootCmd) config -width $width -height $data(totalsize)
    } else {
	set height 200
	foreach name $data(items) {
	    set new_h [winfo reqheight $w.$name]
	    if {$new_h > $height} {
		set height $new_h
	    }
	}
	$data(rootCmd) config -width $data(totalsize) -height $height
    }
}

#----------------------------------------------------------------------
#             INTERNAL COMMANDS
#----------------------------------------------------------------------

proc tixPnw::AddSash {w} {
    upvar #0 $w data

    set n [expr "$data(nItems)-1"]

    set data(btn,$n) [frame $w.btn$n -relief raised \
		      -bd 1 -width 9 -height 9 -cursor crosshair \
		      -bg $data(-handlebg)]
    if {$data(-orientation) == "vertical"} {
	set data(sep,$n) [frame $w.sep$n -relief flat \
			  -bd 1  -height 1 -width 10000 -bg $data(-sashcolor)]
    } else {
	set data(sep,$n) [frame $w.sep$n -relief sunken \
			  -bd 1  -width 1 -height 10000 -bg $data(-sashcolor)]
    }

    bind  $data(btn,$n) \
	<ButtonPress-1>   "tixPnw::BtnDown $w $n"
    bind  $data(btn,$n) \
	<ButtonRelease-1> "tixPnw::BtnUp   $w $n"
    bind  $data(btn,$n) \
	<Any-Enter>       "tixPnw::HighlightBtn $w $n"
    bind  $data(btn,$n) \
	<Any-Leave>       "tixPnw::DeHighlightBtn $w $n"

    if {$data(-orientation) == "vertical"} {
	bind  $data(btn,$n) <B1-Motion> \
	    "tixPnw::BtnMoveCompressed $w $n %Y"
    } else {
	bind  $data(btn,$n) <B1-Motion> \
	    "tixPnw::BtnMoveCompressed $w $n %X"
    }

    if {$data(-orientation) == "vertical"} {
	place $data(btn,$n) -relx 0.90 -y [expr "$data(totalsize)-5"]
    } else {
	place $data(btn,$n) -rely 0.90 -x [expr "$data(totalsize)-5"]
    }
}

proc tixPnw::BtnDown {w item} {
    upvar #0 $w data

    for {set i 1} {$i < $data(nItems)} {incr i} {
	raise $data(sep,$i)
	raise $data(btn,$i)
    }

    $data(btn,$item) config -relief sunken
    tixPnw::GetMotionLimit $w $item
    grab $data(btn,$item)
    set data(pending) 0
}

proc tixPnw::Min2 {a b} {
    if {$a < $b} {
	return $a
    } else {
	return $b
    }
}

proc tixPnw::GetMotionLimit {w item} {
    upvar #0 $w data

    set curBefore 0
    set minBefore 0
    set maxBefore 0

    for {set i 0} {$i < $item} {incr i} {
	set name [lindex $data(items) $i]
	incr curBefore $data($name,size)
	incr minBefore $data($name,min)
	incr maxBefore $data($name,max)
    }

    set curAfter 0
    set minAfter 0
    set maxAfter 0
    while {$i < $data(nItems)} {
	set name [lindex $data(items) $i]
	incr curAfter $data($name,size)
	incr minAfter $data($name,min)
	incr maxAfter $data($name,max)
	incr i
    }

    set beforeToGo  \
       [tixPnw::Min2 [expr "$curBefore-$minBefore"] [expr "$maxAfter-$curAfter"]]
    set afterToGo  \
       [tixPnw::Min2 [expr "$curAfter-$minAfter"] [expr "$maxBefore-$curBefore"]]

    set data(beforeLimit) [expr "$curBefore-$beforeToGo"]
    set data(afterLimit)  [expr "$curBefore+$afterToGo"]
    set data(curSize)     $curBefore

    tixPnw::PlotHandles $w 1
}

# Compress the motion so that update is quick even on slow machines
#
# rootp = root position (either rootx or rooty)
proc tixPnw::BtnMoveCompressed {w item rootp} {
    upvar #0 $w data

    set data(rootp) $rootp
    if {$data(pending) == 0} {
	after 2 tixPnw::BtnMove $w $item
	set data(pending) 1
    }
}

proc tixPnw::BtnMove {w item} {
    upvar #0 $w data

    if {$data(-orientation) == "vertical"} {
	set p [expr $data(rootp)-[winfo rooty $w]]
    } else {
	set p [expr $data(rootp)-[winfo rootx $w]]
    }

    if {$p == $data(curSize)} {
	set data(pending) 0
	return
    }

    if {$p < $data(beforeLimit)} {
	set p $data(beforeLimit)
    }
    if {$p >= $data(afterLimit)} {
	set p $data(afterLimit)
    }
    tixPnw::SetSizes $w $item $p
    update idletasks
    set data(pending) 0
}

proc tixPnw::SetSizes {w item p} {
    upvar #0 $w data

    if {$p < $data(curSize)} {
	tixPnw::MoveBefore $w $item $p
    } elseif {$p > $data(curSize)} {
	tixPnw::MoveAfter $w $item $p
    }

    tixPnw::PlotHandles $w 1
}

proc tixPnw::MoveBefore {w item p} {
    upvar #0 $w data

    set n [expr "$data(curSize)-$p"]

    # Shrink the frames before
    #
    set from [expr $item-1]
    set to   0
    tixPnw::Iterate $w $from $to tixPnw::Shrink $n

    # Adjust the frames after
    #
    set from $item
    set to   [expr "$data(nItems)-1"]
    tixPnw::Iterate $w $from $to tixPnw::Grow $n

    set data(curSize) $p
}

proc tixPnw::MoveAfter {w item p} {
    upvar #0 $w data

    set n    [expr "$p-$data(curSize)"]

    # Shrink the frames after
    #
    set from $item
    set to   [expr "$data(nItems)-1"]
    tixPnw::Iterate $w $from $to tixPnw::Shrink $n

    # Graw the frame before
    #
    set from [expr $item-1]
    set to   0
    tixPnw::Iterate $w $from $to tixPnw::Grow $n

    set data(curSize) $p
}


proc tixPnw::PlotHandles {w showBorders} {
    upvar #0 $w data

    set totalsize 0
    set i 0

    if {$data(-orientation) == "vertical"} {
	set btnp [expr [winfo width $w]-13]
    } else {
	set h [winfo height $w]
	if {$h > 18} {
	    set btnp 9
	} else {
	    set btnp [expr $h-9]
	}
    }

    foreach name $data(items)  {
	incr totalsize $data($name,size)
	incr i

	catch {
	    if {$data(-orientation) == "vertical"} {
		place $data(btn,$i) -x $btnp -y [expr "$totalsize-4"]
		if {$showBorders} {
		    place $data(sep,$i) -x 0 -y $totalsize -relwidth 1
		}
	    } else {
		place $data(btn,$i) -y $btnp -x [expr "$totalsize-5"]
		if {$showBorders} {
		    place $data(sep,$i) -y 0 -x $totalsize -relheight 1
		}
	    }
	}
    }
}

proc tixPnw::BtnUp {w item} {
    upvar #0 $w data

    tixPnw::UpdateSizes $w
    $data(btn,$item) config -relief raised
    grab release $data(btn,$item)
}


proc tixPnw::UpdateSizes {w} {
    upvar #0 $w data

    set data(totalsize) 0

    for {set i 0} {$i < $data(nItems)} {incr i} {
	set name [lindex $data(items) $i]
	if {$data(-orientation) == "vertical"} {
	    place $w.$name -x 0 -relwidth 1 -y $data(totalsize) \
		-height $data($name,size)
	} else {
	    place $w.$name -y 0 -relheight 1 -x $data(totalsize) \
		-width $data($name,size)
	}
	incr data(totalsize) $data($name,size)
    }

    for {set i 1} {$i < $data(nItems)} {incr i} {
	lower $data(sep,$i)
    }

    if {$data(-command) != {}} {
	set sizes {}
	foreach item $data(items) {
	    lappend sizes $data($item,size)
	}
	eval $data(-command) [list $sizes]
    }
}

proc tixPnw::HighlightBtn {w item} {
    upvar #0 $w data

    $data(btn,$item) config -background $data(-handleactivebg)
}

proc tixPnw::DeHighlightBtn {w item} {
    upvar #0 $w data

    $data(btn,$item) config -background $data(-handlebg)
}

#--------------------------------------------------
# Handling resize
#--------------------------------------------------
proc tixPnw::Resize {w} {
    upvar #0 $w data

    if {$data(-orientation) == "vertical"} {
	set newSize [winfo height $w]
    } else {
	set newSize [winfo width $w]
    }

    if {$newSize > $data(totalsize)} {
	# Grow from bottom
	set n    [expr "$newSize-$data(totalsize)"]
	set from [expr "$data(nItems)-1"]
	set to   0
	tixPnw::Iterate $w $from $to tixPnw::Grow $n
	tixPnw::UpdateSizes $w

    } elseif {$newSize < $data(totalsize)} {
	# Shrink from bottom
	set n    [expr "$data(totalsize)-$newSize"]
	set from [expr "$data(nItems)-1"]
	set to   0
	tixPnw::Iterate $w $from $to tixPnw::Shrink $n
	tixPnw::UpdateSizes $w
    }

    tixPnw::PlotHandles $w 0
}

#--------------------------------------------------
# Shrink and grow items
#--------------------------------------------------
proc tixPnw::Shrink {w name n} {
    upvar #0 $w data

    set canShrink [expr "$data($name,size) - $data($name,min)"]

    if {$canShrink > $n} {
	incr data($name,size) -$n
	return 0
    } elseif {$canShrink > 0} {
	set data($name,size) $data($name,min)
	incr n -$canShrink
    }
    return $n
}

proc tixPnw::Grow {w name n} {
    upvar #0 $w data

    set canGrow [expr "$data($name,max) - $data($name,size)"]

    if {$canGrow > $n} {
	incr data($name,size) $n
	return 0
    } elseif {$canGrow > 0} {
	set data($name,size) $data($name,max)
	incr n -$canGrow
    }
    return $n
}

proc tixPnw::Iterate {w from to proc n} {
    upvar #0 $w data

    if {$from <= $to} {
	for {set i $from} {$i <= $to} {incr i} {
	    set n [$proc $w [lindex $data(items) $i] $n]
	    if {$n == 0} {
		break
	    }
	}
    } else {
	for {set i $from} {$i >= $to} {incr i -1} {
	    set n [$proc $w [lindex $data(items) $i] $n]
	    if {$n == 0} {
		break
	    }
	}
    }
}
