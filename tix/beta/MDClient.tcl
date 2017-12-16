proc tixMwmClient {w args} {
    eval tixInt_CreateWidget $w tixMwmCn TixMwmClient $args
    return $w
}

proc tixMwmCn::CreateClassRec {} {
  global tixMwmCn

  # OPTIONS
  set tixMwmCn(rootOptions)   {}
  set tixMwmCn(options)       {-background -foreground -bg -fg
			       -borderbg -borderwidth -bd
			       -cursorfg -cursorbg
			       -framewidth -handlesize
			       -titleheight -title -titlebd
			       -hintcolor -hintwidth
			       -parent
			       -minwidth -minheight
			      }
  set tixMwmCn(staticOptions) {}

  # DEFAULT VALUES
  set tixMwmCn(-background)   {-background background Background lightgray}
  set tixMwmCn(-borderbg)     {-borderbg borderBg Background lightgray}
  set tixMwmCn(-borderwidth)  {-borderwidth borderWidth BorderWidth 1}
  set tixMwmCn(-cursorfg)     {-cursorfg cursorFg CursorColor white}
  set tixMwmCn(-cursorbg)     {-cursorbg cursorBg CursorColor red}
  set tixMwmCn(-foreground)   {-foreground foreground Foreground lightgray}
  set tixMwmCn(-framewidth)   {-framewidth frameWidth FrameWidth 3}
  set tixMwmCn(-handlesize)   {-handlesize handleSize HandleSize 30}
  set tixMwmCn(-hintcolor)    {-hintcolor hintColor HintColor red}
  set tixMwmCn(-hintwidth)    {-hintwidth hintWidth HintWidth 1}
  set tixMwmCn(-minwidth)     {minwidth minWidth MinWidth 100}
  set tixMwmCn(-minheight)    {minwidth minHeight MinHeight 100}
  set tixMwmCn(-parent)       {-parent parent Parent {}}
  set tixMwmCn(-title)        {-title title Title {}}
  set tixMwmCn(-titlebd)      {-titlebd titleBd BorderWidth 1}
  set tixMwmCn(-titleheight)  {-titleheight titleHeight titleHeight {}}

  # ALIASES
  set tixMwmCn(-bg)           {= -background}
  set tixMwmCn(-fg)           {= -foreground}
  set tixMwmCn(-bd)           {= -borderwidth}

  # METHODS
  set tixMwmCn(methods)       {maximize}
}

proc tixMwmCn::InitWidgetRec {w class className args} {
    upvar #0 $w data

    set data(movePending) 0
}

proc tixMwmCn::ConstructWidget {w} {
    upvar #0 $w data

    $data(rootCmd) config \
	-bd 0 \
	-bg $data(-background) \
	-width 200 \
	-height 200

    # the four corner handles
    set r00 [frame $w.r00 -relief raised -bd $data(-borderwidth) \
	     -cursor "top_left_corner $data(-cursorbg) $data(-cursorfg)"]
    set r01 [frame $w.r01 -relief raised -bd $data(-borderwidth)\
	     -cursor "bottom_left_corner $data(-cursorbg) $data(-cursorfg)"]
    set r10 [frame $w.r10 -relief raised -bd $data(-borderwidth)\
	     -cursor "top_right_corner $data(-cursorbg) $data(-cursorfg)"]
    set r11 [frame $w.r11 -relief raised -bd $data(-borderwidth)\
	     -cursor "bottom_right_corner $data(-cursorbg) $data(-cursorfg)"]

    # the four border handles
    set bt [frame $w.bt -relief raised -bd $data(-borderwidth) \
	     -cursor "top_side $data(-cursorbg) $data(-cursorfg)"]
    set bb [frame $w.bb -relief raised -bd $data(-borderwidth)\
	     -cursor "bottom_side $data(-cursorbg) $data(-cursorfg)"]
    set br [frame $w.br -relief raised -bd $data(-borderwidth)\
	     -cursor "right_side $data(-cursorbg) $data(-cursorfg)"]
    set bl [frame $w.bl -relief raised -bd $data(-borderwidth)\
	     -cursor "left_side $data(-cursorbg) $data(-cursorfg)"]

    # the inner sunken frame
    set sunken [frame $w.sunken -relief sunken -bd $data(-borderwidth)]

    # the top bar
    set bar [frame $sunken.bar]

    # Do the buttons inside the title. they are packed, not placed
    set title [button $bar.title -anchor sw\
	       -text $data(-title) -relief raised -bd $data(-titlebd)]
    set sys [button $bar.sys -text x -relief raised -bd $data(-titlebd)]
    set min [button $bar.min -text x -relief raised -bd $data(-titlebd)]
    set max [button $bar.max -text x -relief raised -bd $data(-titlebd)]

    pack $sys -side left
    pack $title -side left -fill x -expand yes
    pack $max $min -side left

    # The frames to use for resizing and dragging
    #
    set data(w:ht) $w:tix_priv_ht
    set data(w:hb) $w:tix_priv_hb
    set data(w:hx) $w:tix_priv_hx
    set data(w:hl) $w:tix_priv_hl
    set data(w:hr) $w:tix_priv_hr

    frame $data(w:ht) -bg $data(-hintcolor) -height $data(-hintwidth)
    frame $data(w:hb) -bg $data(-hintcolor) -height $data(-hintwidth)
    frame $data(w:hx) -bg $data(-hintcolor) -height $data(-hintwidth)
    frame $data(w:hl) -bg $data(-hintcolor) -width  $data(-hintwidth)
    frame $data(w:hr) -bg $data(-hintcolor) -width  $data(-hintwidth)

    # Record the internal widget pathnames
    set data(w:title) $title
    set data(w:r00)   $w.r00
    set data(w:r01)   $w.r01
    set data(w:r10)   $w.r10
    set data(w:r11)   $w.r11
    set data(w:bt)    $w.bt
    set data(w:bb)    $w.bb
    set data(w:bl)    $w.bl
    set data(w:br)    $w.br
} 

proc tixMwmCn::SetBindings {w} {
    upvar #0 $w data

    bind $w <Configure> "tixMwmCn::ComposeWindow $w"

    bind $data(w:title) <1> \
	"tixMwmCn::DragStart $w $data(w:title) 1 %X %Y  1  1  0  0"
    bind $data(w:r00)   <1> \
	"tixMwmCn::DragStart $w $data(w:r00)   0 %X %Y  1  1 -1 -1"
    bind $data(w:r01)   <1> \
	"tixMwmCn::DragStart $w $data(w:r01)   0 %X %Y  1  0 -1  1"
    bind $data(w:r10)   <1> \
	"tixMwmCn::DragStart $w $data(w:r10)   0 %X %Y  0  1  1 -1"
    bind $data(w:r11)   <1> \
	"tixMwmCn::DragStart $w $data(w:r11)   0 %X %Y  0  0  1  1"
    bind $data(w:bt)    <1> \
	"tixMwmCn::DragStart $w $data(w:bt)    0 %X %Y  0  1  0 -1"
    bind $data(w:bb)    <1> \
	"tixMwmCn::DragStart $w $data(w:bb)    0 %X %Y  0  0  0  1"
    bind $data(w:bl)    <1> \
	"tixMwmCn::DragStart $w $data(w:bl)    0 %X %Y  1  0 -1  0"
    bind $data(w:br)    <1> \
	"tixMwmCn::DragStart $w $data(w:br)    0 %X %Y  0  0  1  0"

    foreach win [list \
		 $data(w:title)\
		 $data(w:r00)\
		 $data(w:r01)\
		 $data(w:r10)\
		 $data(w:r11)\
		 $data(w:bt)\
		 $data(w:bb)\
		 $data(w:bl)\
		 $data(w:br)\
		 ] {
	bind $win <B1-Motion>       "tixMwmCn::Drag    $w %X %Y"
	bind $win <ButtonRelease-1> "tixMwmCn::DragEnd $w $win 0 %X %Y"
	bind $win <Any-Escape>      "tixMwmCn::DragEnd $w $win 1  0  0"
    }

    # special attention
    bind $data(w:title) <1> [list +$data(w:title) config \
			     -cursor "fleur $data(-cursorbg) $data(-cursorfg)"]
    bind $data(w:title) <ButtonRelease-1> \
	"+$data(w:title) config -cursor {}"

}

#----------------------------------------------------------------------
#                    Internal methods
#----------------------------------------------------------------------
# Start dragging a window
#
proc tixMwmCn::DragStart {w win depress rootx rooty mx my mw mh} {
    upvar #0 $w data

    set data(rootx) $rootx
    set data(rooty) $rooty

    set data(mx) $mx
    set data(my) $my
    set data(mw) $mw
    set data(mh) $mh

    set data(fx) [winfo x      $w]
    set data(fy) [winfo y      $w]
    set data(fw) [winfo width  $w]
    set data(fh) [winfo height $w]

    set data(old_x) [winfo x      $w]
    set data(old_y) [winfo y      $w]
    set data(old_w) [winfo width  $w]
    set data(old_h) [winfo height $w]

    set data(aborted) 0
    tixMwmCn::PlaceHintFrame $w

    raise $w
    raise $data(w:ht)
    raise $data(w:hb)
    raise $data(w:hx)
    raise $data(w:hl)
    raise $data(w:hr)

    if {$depress} {
	$win config -relief sunken
    }
    # Grab so that all button events are captured
    #
    grab $win
    focus $win
}

proc tixMwmCn::Drag {w rootx rooty} {
    upvar #0 $w data

    if {$data(aborted) == 0} {
	set data(newrootx) $rootx
	set data(newrooty) $rooty

	if {$data(movePending) == 0} {
	    set data(movePending) 1
	    after 2 tixMwmCn::DragCompressed $w
	}
    }
}

proc tixMwmCn::DragCompressed {w} {
    upvar #0 $w data

    if {$data(aborted) == 1 || $data(movePending) == 0} {
	return
    }

    set dx [expr "$data(newrootx) - $data(rootx)"]
    set dy [expr "$data(newrooty) - $data(rooty)"]

    set data(fx) [expr "$data(old_x) + ($dx * $data(mx))"]
    set data(fy) [expr "$data(old_y) + ($dy * $data(my))"]
    set data(fw) [expr "$data(old_w) + ($dx * $data(mw))"]
    set data(fh) [expr "$data(old_h) + ($dy * $data(mh))"]

    if {$data(fw) < $data(-minwidth)} {
	set data(fw) $data(-minwidth)
    }
    if {$data(fh) < $data(-minheight)} {
	set data(fh) $data(-minheight)
    }

    tixMwmCn::PlaceHintFrame $w
    update idletasks

    set data(movePending) 0
}

proc tixMwmCn::DragEnd {w win isAbort rootx rooty} {
    upvar #0 $w data

    if {$data(aborted)} {
	if {$isAbort == 0} {
	    grab release $win
	}
	return
    }

    # Just in case some draggings are not applied.
    #
    update

    # Hide the hint frame
    #
    place forget $data(w:ht)
    place forget $data(w:hb)
    place forget $data(w:hl)
    place forget $data(w:hr)
    place forget $data(w:hx)

    if {$isAbort} {
	set data(aborted) 1
    } else {
	# Apply the changes
	#
	place $w -x $data(fx) -y $data(fy) -width $data(fw) -height $data(fh)
	# Release the grab
	#
	grab release $win
    }

    $win config -relief raised
}

# Place the hint frame to indicate the changes
#
proc tixMwmCn::PlaceHintFrame {w} {
    upvar #0 $w data

    set fx2 [expr "$data(fx)+$data(fw)"]
    set fy2 [expr "$data(fy)+$data(fh)"]
    set fyx [expr "$data(fy)+$data(-handlesize)"]

    place $data(w:ht) -x $data(fx) -y $data(fy) -width  $data(fw)
    place $data(w:hb) -x $data(fx) -y $fy2      -width  $data(fw)
    place $data(w:hx) -x $data(fx) -y $fyx      -width  $data(fw)
    place $data(w:hl) -x $data(fx) -y $data(fy) -height $data(fh)
    place $data(w:hr) -x $fx2      -y $data(fy) -height $data(fh)
}

# Call this after each resize
#
proc tixMwmCn::ComposeWindow {w} {
    upvar #0 $w data

    set width  [expr [winfo width $w]]
    set height [expr [winfo height $w]]

    # place the corner handlers
    set size $data(-handlesize)

    set rx [expr "$width-$size"]
    set ry [expr "$height-$size"]

    place $w.r00 -x 0   -y 0   -width $size -height $size
    place $w.r01 -x 0   -y $ry -width $size -height $size
    place $w.r10 -x $rx -y 0   -width $size -height $size
    place $w.r11 -x $rx -y $ry -width $size -height $size

    # Place the border handlers
    set size2 [expr "2 * $size"]
    set bw [expr "$width-$size2"]
    set bh [expr "$height-$size2"]

    place $w.bt  -x $size -y 0      -width $bw   -height $size
    place $w.bb  -x $size -y $ry    -width $bw   -height $size
    place $w.bl  -x 0     -y $size  -width $size -height $bh
    place $w.br  -x $rx   -y $size  -width $size -height $bh

    # Place the sunken frame
    set fw [expr "$data(-framewidth) + $data(-borderwidth)"]
    set sw [expr "$width  - (2 * $fw)"]
    set sh [expr "$height - (2 * $fw)"]
    place $w.sunken -x $fw -y $fw -width $sw  -height $sh

    # Place the title bar
    set tw [expr "$data(-framewidth) + (2 * $data(-borderwidth) )"]
    set aw [expr "$width - (2 * $tw)"]

    if {$data(-titleheight) == {}} {
      set data(-titleheight) \
	[expr "$data(-handlesize)-$data(-framewidth)-(2*$data(-borderwidth))"]
    }
    place $w.sunken.bar -x 0 -y 0 -width $aw -height $data(-titleheight)
}


#----------------------------------------------------------------------
#                  public methods
#----------------------------------------------------------------------
proc tixMwmCn::maximize {w} {
    upvar #0 $w data

    place $w -x 0 -y 0 -relwidth 1 -relheight 1
}
