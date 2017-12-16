# Paned
# ----------------------------------------------------------------------
# Implements a paned widget, similar to the Paned widget of the Athena 
# Widget Set. 
#
#   PUBLIC ATTRIBUTES:
#                                                                       Default
#     -orient ........ orientation of widget ....................... [vertical]
#
#     -gripIndent..... position of grips from left (vertical orientation)
#                      or top (horizontal). A value > 1 means an absolute
#                      number of pixels; a positive value <= 1.0 means
#                      proportion of distance. 
#                          vertical ..................................... [0.9]
#                          horizontal ................................... [0.1]
#
#   METHODS:
#
#     insert ......... add one or more children to the widget. The children
#                      may be at the same level as the Paned in the widget
#                      hierarchy, or direct descendants of it.
#
#                      Options:
#                          -before child   add before the named child
#                          -after  child   add after the named child
#                          -noexpand       don't stretch the child/ren
#                                          when the widget is resized.
#
#                      N.B. The width and the height of the children must
#                           be available at the time of insertion, i.e. it
#                           may be necessary to call update beforehand.
#
#     forget ......... delete one or more children
#
#     children ....... return list of children
#
#   BUGS:
#
#     If the behaviour is not as you would wish it then see the comments
#     in method drop-grip.
#
# ----------------------------------------------------------------------
#   AUTHOR:  Jim Wight <j.k.wight@newcastle.ac.uk>
# ----------------------------------------------------------------------
# Copyright 1994 The University of Newcastle upon Tyne (see COPYRIGHT)
# ======================================================================

itcl_class Paned {
    inherit itcl_frame

    constructor {args} {
	eval itcl_frame::constructor $args

        # It would be preferable if this - and the grips - were children
        # of the Paned, but the user's children may be equals, not
        # descendants, and then raising and lowering don't work.
        #
	frame ${this}sash -$horw 2 -borderwidth 1 -relief raised

	bind $this <Map> "$this map"

	set configured 1
    }

    destructor {
        foreach slave [pack slaves $this] {
            destroy [pack slaves $slave] $slave
        }
	for {set child 1} {$child < $numChildren} {incr child} {
	    destroy ${this}grip$child
	}
	destroy ${this}sash
    }

    # ------------------------------------------------------------------
    #  METHOD insert - Add children
    # ------------------------------------------------------------------
    method insert {args} {
	if {[set rigid [lsearch $args "-noexpand"]] != -1} {
	    set args [lreplace $args $rigid $rigid]
	    set expand false
	} else {
	    set expand true
	}

	set packopt {}
	if {[set before [lsearch $args "-before"]] != -1 || \
	                          [set after [lsearch $args "-after"]] != -1} {
	    if {$before != -1} {
		set location -before
		set index $before
	    } else {
		set location -after
		set index $after
	    }
	    set arg [expr $index + 1]
	    if {[set frame [frame-of [lindex $args $arg]]] != ""} {
		set packopt "$location $frame"
		set args [lreplace $args $index $arg]
		if {[string compare $location "-after"] == 0 && \
		                                         [llength $args] > 1} {
		    set rargs {}
		    for {set arg [expr [llength $args] - 1]} {$arg >= 0} \
			                                        {incr arg -1} {
			lappend rargs [lindex $args $arg]
		    }
		    set args $rargs
 	        }
	    }
	}

	foreach child $args {
	    frame $this._f_$frameId
            pack propagate $this._f_$frameId false

	    pack $child \
	        -in $this._f_$frameId \
		-expand true -fill both

	    eval pack $this._f_$frameId \
                -in $this \
		-side $side \
		-fill both -expand $expand \
		$packopt
	    lower $this._f_$frameId

	    if {$numChildren > 0} {
		frame ${this}grip$numChildren \
		    -geometry 10x10 \
		    -borderwidth 2 \
		    -relief raised -cursor crosshair

                bind ${this}grip$numChildren <Button-1> \
		    "$this grab-grip $numChildren %$yorx"
                bind ${this}grip$numChildren <B1-Motion> \
		    "$this drag-grip $numChildren %$yorx"
                bind ${this}grip$numChildren <B1-ButtonRelease-1> \
		    "$this drop-grip $numChildren %$yorx"
	    }

	    $this._f_$frameId configure \
	        -width [winfo reqwidth $child] \
		-height [winfo reqheight $child]

            re-grip

	    incr numChildren  
            incr frameId
	}
    }

    # ------------------------------------------------------------------
    #  METHOD forget - delete one or more children
    # ------------------------------------------------------------------
    method forget {args} {
        foreach child $args {
            if {![string match [set frame [frame-of $child]] ""]} {
                incr numChildren -1
                destroy [pack slaves $frame] $frame ${this}grip$numChildren
            }
        }
        re-grip
    }

    # ------------------------------------------------------------------
    #  METHOD children - return a list of children
    # ------------------------------------------------------------------
    method children {} {
	set children {}
	foreach slave [pack slaves $this] {
	    lappend children [pack slaves $slave]
	}
	return $children
    }

    # ------------------------------------------------------------------
    #  PUBLIC variables
    # ------------------------------------------------------------------
    public gripIndent {} {
	set indent $gripIndent
	if {$indent >= 0  && $indent <= 1.0} {
	    set absrel rel
	} else {
	    set absrel {}
	}
	re-grip
    }
 
    public orient vertical {
	if {!$configured && $orient == "horizontal"} {
	    set side   left
	    set anchor n
	    set xory   y
	    set yorx   x
	    set horw   width
	    set worh   height
	    if {[string compare $gripIndent ""] == 0} {
		set indent 0.1
	    }
	}
    }


    # ------------------------------------------------------------------
    #  Everything is for private use from here on down


    # ------------------------------------------------------------------
    #  METHOD grab-grip - Starts the sash drag and drop operation
    # ------------------------------------------------------------------
    method grab-grip {child where} {
        set offset [expr [winfo $yorx $this]]
	if {$child == 1} {
	    set mindrag 0
	} else {
	    set mindrag [expr [winfo $yorx ${this}grip[expr $child - 1]] \
                             - $offset]
	}

	if {$child == [expr $numChildren - 1]} {
	    set maxdrag [winfo $horw $this]
	} else {
	    set maxdrag [expr [winfo $yorx ${this}grip[expr $child + 1]] \
                             - $offset]
	}

	set position [expr [expr [winfo $yorx ${this}grip$child] + $where] \
		          - $offset]
	${this}sash configure -$worh [winfo $worh $this]

	place ${this}sash -in $this -$xory 0 -$yorx $position -anchor $anchor
	raise ${this}sash
	raise ${this}grip$child
	grab ${this}grip$child
	${this}grip$child configure -relief sunken
    }

    # ------------------------------------------------------------------
    #  METHOD drag-grip - Motion action for sash
    # ------------------------------------------------------------------
    method drag-grip {child where} {
	set position [expr [expr [winfo $yorx ${this}grip$child] + $where] \
		          - $offset]
	if {$position > $mindrag && $position < $maxdrag} {
	    place ${this}sash \
	        -in $this \
		-$xory 0 \
		-$yorx $position \
		-anchor $anchor

	    place ${this}grip$child \
	        -in $this \
		-$absrel$xory $indent \
		-$yorx $position \
		-anchor center
	}
    }

    # ------------------------------------------------------------------
    #  METHOD drop-grip - Ends the sash drag and drop operation
    # ------------------------------------------------------------------
     method drop-grip {child where} {
 	grab release ${this}grip$child
 	${this}grip$child configure -relief raised
 	lower ${this}sash
 
	set position [expr [expr [winfo $yorx ${this}grip$child] + $where] \
		          - $offset]
	set slaves [pack slaves $this]
	set lower [lindex $slaves [expr $child - 1]]
	set higher [lindex $slaves $child]
	set adjustment [expr $position - [winfo $yorx $lower] \
			               - [winfo $horw $lower]]

	# The approved method of adjusting the relative widths/heights of
        # adjacent panes is to adjust their requested widths/heights and
        # to leave the packer to redistribute the remaining space, if any,
        # exactly as before. However, this leads to undesirable behaviour
        # if a requested width/height becomes negative.
        #
	# An alternative is to set the widths/heights of all panes to their
        # actual sizes, and so there is no space left over for the packer
        # to distribute. A consequence is that if the window is made
        # smaller the packer has no slack that can be used to absorb the
        # reduction and embarks on taking away from the panes, apparently
        # in the reverse order of packing.
        #
        # The adopted solution is a compromise: the first method is used
        # if it can, and the second otherwise. Once the 2nd has been used
        # though the 1st can never be used again until the Paned is resized
        # bigger and the packer once more introduces some slack.

	set reqlow [expr [winfo req$horw $lower] + $adjustment]
	set reqhigh [expr [winfo req$horw $higher] - $adjustment]
	if {$reqlow < 0 || $reqhigh < 0} {
	    foreach slave $slaves {
		if {[string compare $slave $lower] == 0} {
		    $lower configure \
		        -$horw [expr [winfo $horw $lower] + $adjustment]
		} elseif {[string compare $slave $higher] == 0} {
		    $higher configure \
		        -$horw [expr [winfo $horw $higher] - $adjustment]
		} else {	
		    $slave configure -$horw [winfo $horw $slave]
		}
	}
	} else {
	    $lower configure -$horw $reqlow
	    $higher configure -$horw $reqhigh
	}

	place ${this}grip$child \
	    -in $this \
	    -$absrel$xory $indent \
	    -$yorx $position \
	    -anchor center
     }

    # ------------------------------------------------------------------
    #  METHOD re-grip - Reposition the grips after an adjustment
    # ------------------------------------------------------------------
    method re-grip {} {
        if {!$mapped} {
            return
        }
	update
	set child 0
	foreach slave [lrange [pack slaves $this] 1 end] {
	    incr child
	    place ${this}grip$child \
	        -in $this \
		-$absrel$xory $indent \
		-$yorx [winfo $yorx $slave] \
		-anchor center
            raise ${this}grip$child
        }
    }

    # ------------------------------------------------------------------
    #  METHOD map - Initial positioning of the grips and setup for resizes
    # ------------------------------------------------------------------
    method map {} {
        set mapped 1
	re-grip
	bind $this <Map> {#}
	bind $this <Configure> "$this re-grip"
    }

    # ------------------------------------------------------------------
    #  METHOD frame-of - find the frame containing the specified child
    # ------------------------------------------------------------------
    method frame-of {child} {
	set result {}
        foreach slave [pack slaves $this] {
            if {[string compare [pack slaves $slave] $child] == 0} {
		set result $slave
		break
	    }
        }
	return $result
    }

    # ------------------------------------------------------------------
    #  PROTECTED variables
    # ------------------------------------------------------------------
    protected side        top
    protected anchor      w
    protected xory        x
    protected yorx        y
    protected horw        height
    protected worh        width
    protected absrel      rel
    protected indent      .9
    protected numChildren 0
    protected frameId     0
    protected configured  0
    protected mapped      0
    protected mindrag
    protected maxdrag
    protected offset
}
