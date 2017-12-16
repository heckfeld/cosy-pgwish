proc tixScrolledListBox {w args} {
    eval tixInt_CreateWidget $w tixSlb TixScrolledListbox $args
    return $w
}

proc tixScrolledListbox {w args} {
    eval tixInt_CreateWidget $w tixSlb TixScrolledListbox $args
    return $w
}

proc tixSlb::CreateClassRec {} {
    global tixSlb

    # OPTIONS
    set tixSlb(rootOptions)   {}
    set tixSlb(options)       {-scrollbar -anchor
			       -foreground -fg
			       -background -bg
			       -font
			       -geometry -geom -listboxbg
			       -scrollbarbg -scrollbarfg
                               -scrollbarwidth -scbwidth
			      }
    set tixSlb(staticOptions) {}


    # DEFAULT VALUES
    set tixSlb(-anchor)         {-anchor anchor Anchor w}
    set tixSlb(-background)     {-background background Background #ffe4c4}
    set tixSlb(-foreground)     {-foreground foreground Foreground black}
    set tixSlb(-scrollbarwidth) {-scrollbarwidth scrollbarWidth
	                          ScrollbarWidth 15}
    set tixSlb(-geometry)       {-geometry geometry Geometry 10x10}
    set tixSlb(-listboxbg)      {-listboxbg listboxBg Background #ffe4c4}
    set tixSlb(-scrollbarbg)    {-scrollbarbg scrollbarBg ScrollbarBg #dfc4a4}
    set tixSlb(-scrollbarfg)    {-scrollbarfg scrollbarFg ScrollbarFg #ffe4c4}
    set tixSlb(-scrollbar)      {-scrollbar scrollbar Scrollbar both}
    set tixSlb(-font)           {-font font Font \
	                     "-*-helvetica-medium-r-normal-*-14-*-*-*-*-*-*-*"}

    # ALIASES
    set tixSlb(-geom)           {= -geometry}
    set tixSlb(-bg)             {= -background}
    set tixSlb(-fg)             {= -foreground}
    set tixSlb(-scbwidth)       {= -scrollbarwidth}

    # METHODS
    set tixSlb(methods)       {listbox info}
}

proc tixSlb::InitWidgetRec {w class className args} {
    upvar #0 $w data

    set data(width)          0
    set data(height)         0
    set data(xview)          0
    set data(yview)          0
    set data(totalwidth)     0
    set data(totalheight)    0
    set data(type)           nil
    set data(flag)           0
}

proc tixSlb::ConstructWidget {w} {
    upvar #0 $w data

    frame $w.f1 -border 0
    frame $w.f2 -border 0

    scrollbar $w.sb1 \
	-width $data(-scrollbarwidth) \
	-bg $data(-scrollbarbg) \
	-fg $data(-scrollbarfg) \
	-orient horiz \
	-command "$w.lb1 xview"

    scrollbar $w.sb2 \
	-width $data(-scrollbarwidth) \
	-bg $data(-scrollbarbg) \
	-fg $data(-scrollbarfg) \
	-command "$w.lb1 yview"

    listbox $w.lb1 \
	-exportselection false \
	-xscrollcommand "tixSlb::MonitorSize $w width $w.sb1"  \
	-yscrollcommand "tixSlb::MonitorSize $w height $w.sb2" \
	-font $data(-font) \
	-bg $data(-listboxbg) \
	-fg $data(-foreground) \
	-geom $data(-geometry)

    frame $w.fx -height $data(-scrollbarwidth) -relief flat \
	-width [expr \
	  "$data(-scrollbarwidth) + 2 * [lindex [$w.sb2 config -bd] 4]"] \
	-bg $data(-background)

    set data(listbox) $w.lb1
    set data(hsb)     $w.sb1
    set data(vsb)     $w.sb2
    set data(xframe)  $w.fx

    tixSlb::Repack $w $data(-scrollbar)

    bind $w.sb1 <1>                "+set [format "%s(flag)" $w] 1"
    bind $w.sb1 <ButtonRelease-1>  "+set [format "%s(flag)" $w] 0"
}

proc tixSlb::ScrollBarsNeeded {w} {
    upvar #0 $w data

    if {$data(totalwidth) > $data(width)} {
	if {$data(totalheight) > $data(height) || $data(yview) > 0 } {
	    return both
	} else {
	    return x
	}
    } elseif {$data(totalheight) > $data(height) || $data(yview) > 0 } {
	return y
    } else {
	return none
    }
}

proc tixSlb::Repack {w type} {
    upvar #0 $w data

    if {$type == "auto"} {
	set type [tixSlb::ScrollBarsNeeded $w]
    }

    if {$type != $data(type)} {
	pack forget $data(listbox)
	pack forget $data(hsb)
	pack forget $data(vsb)
	pack forget $data(xframe)
	pack forget $w.f1
	pack forget $w.f2

	tixSlb::Pack $w $type
	set data(type) $type
    }
}

proc tixSlb::Pack {w type} {
    upvar #0 $w data

    if {$type == "both"} {
	pack $data(listbox) -in $w.f1 -side left -fill both -expand yes
	pack $data(vsb)     -in $w.f1 -side left -fill y

	pack $data(hsb)     -in $w.f2 -side left -fill x -expand yes
	pack $data(xframe)  -in $w.f2 -side left -fill y

	pack $w.f2 -side bottom -fill x
	pack $w.f1 -side top -fill both -expand yes
    } elseif {$type == "x"} {
	pack $data(hsb)     -side bottom -fill x
	pack $data(listbox) -side top -fill both -expand yes
    } elseif {$type == "y"} {
	pack $data(vsb)     -side right -fill y
	pack $data(listbox) -side left -fill both -expand yes
    } else {
	pack $data(listbox) -fill both -expand yes
    }
}

#----------------------------------------------------------------------
#                           CONFIG OPTIONS
#----------------------------------------------------------------------
proc tixSlb::config-anchor {w arg} {
    puts stderr "TixScrolledListbox: config anchor not implemented"
}

proc tixSlb::config-background {w arg} {
    upvar #0 $w data

    $data(rootCmd) config -bg $arg
    $data(xframe)  config -bg $arg
}

proc tixSlb::config-foreground {w arg} {
    upvar #0 $w data

    $data(listbox) config -fg $arg
}

proc tixSlb::config-scrollbarfg {w arg} {
    upvar #0 $w data

    $data(hsb) config -fg $arg
    $data(vsb) config -fg $arg
}

proc tixSlb::config-scrollbarbg {w arg} {
    upvar #0 $w data

    $data(hsb) config -bg $arg
    $data(vsb) config -bg $arg
}

proc tixSlb::config-listboxbg {w arg} {
    upvar #0 $w data

    $data(listbox) config -bg $arg
}

proc tixSlb::config-font {w arg} {
    upvar #0 $w data

    $data(listbox) config -font $arg
}

proc tixSlb::config-scrollbar {w arg} {
    upvar #0 $w data
}

proc tixSlb::config-scrollbarwidth {w arg} {
    upvar #0 $w data

    $data(hsb) config -width $arg
    $data(vsb) config -width $arg

    $data(xframe) config -height $arg \
      -width [expr "$arg + 2 * [lindex [$data(vsb) config -bd] 4]"]
}

proc tixSlb::config-geometry {w arg} {
    upvar #0 $w data

    $data(listbox) config -geometry $arg
}
#----------------------------------------------------------------------
#                     WIDGET COMMANDS
#----------------------------------------------------------------------
proc tixSlb::listbox {w args} {
    upvar #0 $w data

    if {$args != {}} {
	return [eval $data(listbox) $args]
    } else {
	return $data(listbox)
    }
}

proc tixSlb::info {w spec} {
    upvar #0 $w data

    if {$spec == "width" || $spec == "height"} {
	return $data($spec)
    }
    if {$spec == "totalwidth" || $spec == "totalheight"} {
	return $data($spec)
    }
    if {$spec == "xview" || $spec == "yview"} {
	return $data($spec)
    }
}

#----------------------------------------------------------------------
#                     Internal commands
#----------------------------------------------------------------------
proc tixSlb::MonitorSize {w spec scb total max begin end} {
    upvar #0 $w data

    set data($spec) $max
    set data(total$spec) $total
    if {$spec == "width"} {
	set data(xview) $begin
    } else {
	set data(yview) $begin
    }
    $scb set $total $max $begin $end

    if {$data(-scrollbar) == "auto"} {
	tixSlb::Repack $w auto
    }

    if {$data(flag) == 0} {
	if {$data(-anchor) == "e"} {
	    set data(flag) 1
	    if {$total > $max && $total > [expr $end +1]} {
		$data(listbox) xview [expr "$total - $max + 1"]
	    }
	    set data(flag) 0
	}
    }
}
