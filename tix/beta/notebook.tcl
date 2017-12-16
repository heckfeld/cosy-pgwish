# Class     : TixNotebook

proc tixNotebook {w args} {
    upvar #0 $w data

    eval tixInt_CreateWidget $w tixNtBk TixNotebook $args
    return $data(wInframe)
}

proc tixNtBk::CreateClassRec {} {
    global tixNtBk

    set tixNtBk(options)        {-background -bg
				 -foreground -fg
				 -activebackground -inactivebackground
				 -activefont -inactivefont
				 -side
				 -pageheight -pagewidth
				}
    set tixNtBk(rootOptions)    {-bd -borderwidth -cursor
				 -relief
				}

    # DEFAULT VALUES
#    set tixNtBk(-background)    {-background background Background #ffe4c4}
    set tixNtBk(-background)    {-background background Background #ffe4c4}
    set tixNtBk(-foreground)    {-foreground foreground Foreground black}
    set tixNtBk(-activebackground)   {-activebackground activeBackground ActiveBackground \
				 #eed5b7}
    set tixNtBk(-inactivebackground) {-inactivebackground inactiveBackground InactiveBackground \
				 lightgrey}
    set tixNtBk(-activefont)    {-activefont activeFont ActiveFont \
	                         -*-Helvetica-Bold-R-Normal--*-120-*}
    set tixNtBk(-inactivefont)  {-inactivefont inactiveFont InactiveFont \
	                         -*-Helvetica-Medium-R-Normal--*-120-*}

    set tixNtBk(-pagewidth)     {-pagewidth pageWidth PageWidth 0}
    set tixNtBk(-pageheight)    {-pageheight pageHeight PageHeight 0}
    set tixNtBk(-side)          {-side side Side top}
    set tixNtBk(-innerborder)   {-innerborder innerBorder InnerBorder 2}

    # ALIASES
    set tixNtBk(-bg)            {= -background}
    set tixNtBk(-fg)            {= -foreground}
    # METHODS
    set tixNtBk(methods)        {addpage delpage nextpage prevpage setpage}
}

proc tixNtBk::ConstructWidget {w} {
    upvar #0 $w data

    frame $w.f
    place $w.f -in $w -relwidth 1.0 -relheight 1.0

    canvas $w.f.c -height $data(labheight) -bg $data(-background) \
	-borderwidth 0
    # Create a widget we can do tests on to see if colors and fonts are valid
    label $w.f.l

    set data(wFrame) $w.f
    set data(wTabCanvas) $w.f.c
    set data(wTest) $w.f.l


    tixNtBk::SetLabelGeom $w

    pack $w.f.c -fill x -padx 0 -ipadx 0 -pady 0 -ipady 0
    $w.f.c bind $w <1> "tixNtBk::TabPressed $w"

    frame $w.f.f
    pack $w.f.f -expand yes -fill both
    set data(wInframe) $w.f.f

#    pack $w.f -fill both -expand yes
}

proc tixNtBk::SetBindings {w} {

}

proc tixNtBk::InitWidgetRec {w args} {
    upvar #0 $w data

    set data(tablist)		{}
    set data(tablist_items)	{}
    set data(active)		{}
    set data(pageid)		0
    set data(pagelist)		{}
    set data(labellist)		{}
    set data(commandlist)	{}
    set data(indent)		7
    set data(overlap)		6
    set data(labheight)		0
    set data(labwidth)		0
    set data(canvwidth)		0
    set data(maxpageheight)	0
    set data(maxpagewidth)	0
}

# Returns 0 if no redrawing is necessary, 1 if redrawing is necessary
proc tixNtBk::SetLabelGeom {w} {
    upvar #0 $w data

    set len [llength $data(labellist)]
    if {$len == 0} {
	$data(wTest) configure -font $data(-activefont)
	set rh [winfo reqheight $data(wTest)]

	$data(wTest) configure -font $data(-inactivefont)
	set rh2 [winfo reqheight $data(wTest)]

	if {$rh2 > $rh} {
	    set rh $rh2
	}
	set data(labheight) $rh
	$data(wTabCanvas) configure -height $rh
	tixNtBk::SetHeight $w
	return 0
    }

    set height 0
    set width 0
    while {$len > 0} {
	incr len -1
	set label [lindex $data(labellist) $len]

	$data(wTest) configure -text $label -font $data(-activefont)

	set rh [winfo reqheight $data(wTest)]
	set rw [winfo reqwidth  $data(wTest)]
	if {$rh > $height} {
	    set height $rh
	}
	if {$rw > $width} {
	    set width $rw
	}

	$data(wTest) configure -text $label -font $data(-inactivefont)

	set rh [winfo reqheight $data(wTest)]
	set rw [winfo reqwidth  $data(wTest)]

	if {$rh > $height} {
	    set height $rh
	}
	if {$rw > $width} {
	    set width $rw
	}
    }

    if {$data(labheight) != $height || $data(labwidth) != $width} {
	set data(labheight) $height
	set data(labwidth) $width

	$data(wTabCanvas) configure -height $height
	tixNtBk::SetHeight $w
	return 1
    } else {
	return 0
    }
}


#----------------------------------------------------------------------
#                           CONFIG OPTIONS
#----------------------------------------------------------------------
proc tixNtBk::config_begin {w} {
    upvar #0 $w data

    set data(options_changed) {}
}

proc tixNtBk::config_apply {w} {
    upvar #0 $w data

    set reconfig 0
    set noreconfig_actions {}

    # Check for options that cause redrawing of everything
    foreach option {-activefont -inactivefont} {
	if {[lsearch $data(options_changed) "$option *"] != -1} {
	    if [tixNtBk::SetLabelGeom $w] {
		tixNtBk::ReconfigureTabs $w
		return
	    }
	}
    }

    foreach option {-side} {
	if {[lsearch $data(options_changed) "$option *"] != -1} {
	    tixNtBk::ReconfigureTabs $w
	    return
	}
    }

    # No major redrawing was necessary, so do minor redrawing
    foreach pair $data(options_changed) {
	set option [lindex $pair 0]
	set arg    [lindex $pair 1]
	switch -- $option {
	    -foreground {
		foreach tab_items $data(tablist_items) {
		    set line [lindex $tab_items 1]
		    set text [lindex $tab_items 2]

		    $data(wTabCanvas) itemconfigure $line -fill $arg
		    $data(wTabCanvas) itemconfigure $text -fill $arg
		}
	    }
	    -activebackground {
		set index [lsearch $data(tablist) $data(active)]
		set tab_items [lindex $data(tablist_items) $index]
		set poly [lindex $tab_items 0]
		$data(wTabCanvas) itemconfigure $poly -fill $arg
	    }
	    -inactivebackground {
		set len [llength $data(tablist)]
		if {$len == 0} {
		    return
		}

		set index [lsearch $data(tablist) $data(active)]

		while {$len > 0} {
		    incr len -1
		    if {$len == $index} {
			continue
		    }
		    set tab_items [lindex $data(tablist_items) $len]
		    set poly [lindex $tab_items 0]
		    $data(wTabCanvas) itemconfigure $poly -fill $arg
		}
	    }
	}
    }
}

proc tixNtBk::config-background {w arg} {
    upvar #0 $w data

    $data(wTest) config -bg $arg
    lappend data(options_changed) [list -background $arg]
}

proc tixNtBk::config-foreground {w arg} {
    upvar #0 $w data

    $data(wTest) config -fg $arg
    lappend data(options_changed) [list -foreground $arg]
}

proc tixNtBk::config-activebackground {w arg} {
    upvar #0 $w data

    if {$data(active) == ""} {
	return
    }

    $data(wTest) config -bg $arg
    lappend data(options_changed) [list -activebackground $arg]
}
    
proc tixNtBk::config-inactivebackground {w arg} {
    upvar #0 $w data

    $data(wTest) config -bg $arg
    lappend data(options_changed) [list -inactivebackground $arg]
}
    
proc tixNtBk::config-activefont {w arg} {
    upvar #0 $w data

    $data(wTest) config -font $arg
    lappend data(options_changed) [list -activefont $arg]
}

proc tixNtBk::config-inactivefont {w arg} {
    upvar #0 $w data

    $data(wTest) config -font $arg
    lappend data(options_changed) [list -inactivefont $arg]
}

proc tixNtBk::config-side {w arg} {
    upvar #0 $w data

    set data(-side) $arg
    # This procedure does nothing yet: XXXX
    if {$arg == "bottom"} {

    } else {
	# Assume top
    }
}

proc tixNtBk::config-pageheight {w arg} {
    upvar #0 $w data

    set data(-pageheight) $arg
    tixNtBk::SetHeight $w
}

proc tixNtBk::config-pagewidth {w arg} {
    upvar #0 $w data

    tixNtBk::SetWidth $w
}

proc tixNtBk::SetWidth {w} {
    upvar #0 $w data

    set width $data(-pagewidth)
    if {$width <= 0} {
	set width $data(maxpagewidth)
    }
    if {$width < $data(canvwidth)} {
	set width $data(canvwidth)
    }

    $w configure -width $width
}

proc tixNtBk::SetHeight {w} {
    upvar #0 $w data

    set height $data(-pageheight)
    if {$height <= 0} {
	set height $data(maxpageheight)
    }
    set height [expr $height+$data(labheight)]

    $w configure -height $height
}

proc tixNtBk::ReconfigureTabs {w} {
    upvar #0 $w data

    set len [llength $data(tablist)]
    if {$len == 0} {
	return
    }
    set data(canvwidth) \
	[expr $data(overlap)+$len*($data(labwidth)+(2*$data(indent))-$data(overlap))]

    set new_tablist_items {}
    while {$len > 0} {
	incr len -1
	set tab [lindex $data(tablist) $len]
	set tab_items [lindex $data(tablist_items) $len]

	if {$data(active) == $tab} {
	    set bg   $data(-activebackground)
	    set fg   $data(-foreground)
	    set font $data(-activefont)
	} else {
	    set bg   $data(-inactivebackground)
	    set fg   $data(-foreground)
	    set font $data(-inactivefont)
	}

	set taglist1 [lrange [lindex [$data(wTabCanvas) itemconfigure \
			[lindex $tab_items 0] -tags] 4] 0 2]
	set taglist2 [lrange [lindex [$data(wTabCanvas) itemconfigure \
			[lindex $tab_items 1] -tags] 4] 0 2]
	set taglist3 [lrange [lindex [$data(wTabCanvas) itemconfigure \
			[lindex $tab_items 2] -tags] 4] 0 2]
	set label [lindex $data(labellist) $len]

	eval "$data(wTabCanvas) delete $tab_items"

	set x1 [expr $len*($data(labwidth)+(2*$data(indent))-$data(overlap))]
	set x2 [expr $x1+$data(overlap)]
	set x3 [expr $x2+$data(labwidth)]
	set x4 [expr $x3+$data(indent)]

	set y1 1
	set y2 [expr $data(labheight)+$y1]

	set polyid [$data(wTabCanvas) create polygon \
	    $x1 $y2 $x2 $y1 $x3 $y1 $x4 $y2 \
	    -fill $bg -tag $taglist1]
	set lineid [$data(wTabCanvas) create line \
	    $x1 $y2 $x2 $y1 $x3 $y1 $x4 $y2 \
	    -fill $fg -tag $taglist2]

	set x [expr ($x1+$x4)/2]
	set y [expr ($y1+$y2)/2]

	set textid [$data(wTabCanvas) create text $x $y \
	    -anchor c -tag $taglist3 -font $font]
	$data(wTabCanvas) insert $textid 0 $label

	set new_tablist_items \
	    [linsert $new_tablist_items 0 [list $polyid $lineid $textid]]
    }

    set data(tablist_items) $new_tablist_items
    tixNtBk::SetWidth $w

    tixNtBk::FrontTab $w $data(active) $tab 
}

proc tixNtBk::FrontTab {w tab {fronttab {}}} {
    upvar #0 $w data

    if {$fronttab == {}} {
	set fronttab $data(active)
    }

    set l $data(tablist)
    set index [lsearch $l $fronttab]
    if {$index > 0} {
	set prevtab [lindex $l [expr $index-1]]
	set lowerlist [lrange $l $index end]
	# Lowering does not seem to work the way it should.  Lower each
	# item in the list one at a time
	if {0} {
	    $data(wTabCanvas) lower $lowerlist $prevtab
	} else {
	    set len [llength $lowerlist]
	    while {$len > 0} {
		incr len -1
		$data(wTabCanvas) lower [lindex $lowerlist $len]
	    }
	}
    }

    set index [lsearch $l $tab]
    set raiselist [lrange $l $index end]
    # Lowering does not seem to work the way it should.  Lower each
    # item in the list one at a time
    if {0} {
	$data(wTabCanvas)  raise $raiselist $prevtab
    } else {
	set len [llength $raiselist]
	while {$len > 0} {
	    incr len -1
	    $data(wTabCanvas) raise [lindex $raiselist $len]
	}
    }
}

proc tixNtBk::TabPressed {w} {
    upvar #0 $w data

    set c $data(wTabCanvas)

    set currtags [$c gettags current]

    set tab [lindex $currtags 0]

    set active $data(active)
    if {$active == $tab} {
	return
    }

    $c itemconfigure ${active}.poly -fill $data(-inactivebackground)
    $c itemconfigure ${active}.text -font $data(-inactivefont)

    tixNtBk::FrontTab $w $tab $active

    set l $data(tablist)
    set index [lsearch $l $active]
  if {0} {
    if {$index > 0} {
	set prevtab [lindex $l [expr $index-1]]
	set lowerlist [lrange $l $index end]
	# Lowering does not seem to work the way it should.  Lower each
	# item in the list one at a time
	if {0} {
	    $c lower $lowerlist $prevtab
	} else {
	    set len [llength $lowerlist]
	    while {$len > 0} {
		incr len -1
		$c lower [lindex $lowerlist $len]
	    }
	}
    }
  }
    pack forget [lindex $data(pagelist) $index]

    set index [lsearch $l $tab]
  if {0} {
    set raiselist [lrange $l $index end]
    # Lowering does not seem to work the way it should.  Lower each
    # item in the list one at a time
    if {0} {
	$c raise $raiselist $prevtab
    } else {
	set len [llength $raiselist]
	while {$len > 0} {
	    incr len -1
	    $c raise [lindex $raiselist $len]
	}
    }
  }
    set data(active) $tab
    pack [lindex $data(pagelist) $index] -fill both -expand yes

    $c itemconfigure ${tab}.poly -fill $data(-activebackground)
    $c itemconfigure ${tab}.text -font $data(-activefont)

    set command [lindex $data(commandlist) $index]
    uplevel #0 $command
}
    	
#----------------------------------------------------------------------
#                     WIDGET COMMANDS
#----------------------------------------------------------------------
proc tixNtBk::addpage {w page label command} {
    upvar #0 $w data

    $data(wTest) configure -font $data(-activefont) -text $label
    set reqwidth [winfo reqwidth $data(wTest)]
    $data(wTest) configure -font $data(-inactivefont)
    set reqwidth2 [winfo reqwidth $data(wTest)]

    if {$reqwidth2 > $reqwidth} {
	set reqwidth $reqwidth2
    }

    if {$reqwidth > $data(labwidth)} {
	set data(labwidth) $reqwidth
	tixNtBk::ReconfigureTabs $w
    }

    set pageid $data(pageid)
    incr data(pageid)
    
    set tag $w.tab$pageid

    set len [llength $data(tablist)]
    if {$len == 0} {
	set data(active) $tag
    }

    if {$data(active) == $tag} {
	set bg   $data(-activebackground)
	set fg   $data(-foreground)
	set font $data(-activefont)
    } else {
	set bg   $data(-inactivebackground)
	set fg   $data(-foreground)
	set font $data(-inactivefont)
    }

    set taglist1 [list $tag $tag.poly $w]
    set taglist2 [list $tag $tag.line $w]
    set taglist3 [list $tag $tag.text $w]

    set x1 [expr $len*($data(labwidth)+(2*$data(indent))-$data(overlap))]
    set x2 [expr $x1+$data(overlap)]
    set x3 [expr $x2+$data(labwidth)]
    set x4 [expr $x3+$data(indent)]

    set data(canvwidth) $x4

    set y1 1
    set y2 [expr $data(labheight)+$y1]

    set polyid [$data(wTabCanvas) create polygon \
	$x1 $y2 $x2 $y1 $x3 $y1 $x4 $y2 \
	-fill $bg -tag $taglist1]
    set lineid [$data(wTabCanvas) create line \
	$x1 $y2 $x2 $y1 $x3 $y1 $x4 $y2 \
	-fill $fg -tag $taglist2]

    set x [expr ($x1+$x4)/2]
    set y [expr ($y1+$y2)/2]

    set textid [$data(wTabCanvas) create text $x $y \
	-anchor c -tag $taglist3 -font $font]
    $data(wTabCanvas) insert $textid 0 $label

    lappend data(tablist_items) [list $polyid $lineid $textid]
    lappend data(tablist) $tag
    lappend data(pagelist) $page
    lappend data(labellist) $label
    lappend data(commandlist) $command

    if {$len > 0} {
	$data(wTabCanvas) lower $tag
    }

    set rh [winfo reqheight $page]
    set rw [winfo reqwidth $page]

    if {$rh > $data(maxpageheight)} {
	set data(maxpageheight) $rh
	tixNtBk::SetHeight $w
    }

    if {$rw > $data(maxpagewidth)} {
	set data(maxpagewidth) $rw
    }

    bind $page <Configure> "+tixNtBk::PageConfigure $w $page"

    tixNtBk::SetWidth $w

    if {$data(active) == $tag} {
	pack $page -fill both -expand yes -ipadx 0 -ipady 0 -padx 0 -pady 0
	uplevel #0 $command
    }
    return $tag
}

# This procedure makes sure that the frame around the page is at least the
# size of the page being swapped in
proc tixNtBk::PageConfigure {w page} {
    upvar #0 $w data

    set rh [winfo reqheight $page]
    set rw [winfo reqwidth $page]

    if {$rh > $data(maxpageheight)} {
	set data(maxpageheight) $rh
	tixNtBk::SetHeight $w
    }

    if {$rw > $data(maxpagewidth)} {
	set data(maxpagewidth) $rw
	if {$rw > $data(canvwidth)} {
	    tixNtBk::SetWidth $w
	}
    }
}

