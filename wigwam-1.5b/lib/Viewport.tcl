# Viewport
# ----------------------------------------------------------------------
# Implements a viewport class
#
#   PUBLIC ATTRIBUTES:
#
#     -scroll ........ specifies which scrollbars are to be attached.
#                      Value is a list consisting of some of the words
#                      left, right, top and bottom. The default is "left".
#                      Only one vertical and one horizontal scrollbar
#                      are permitted. If both of left/right or top/bottom
#                      are specified then left and bottom take precedence.
#
#                      A scrollbar is added even if the child widget does
#                      not support scrolling in a requested direction. It
#                      just won't do anything.
#
#   METHODS:
#
#     None.
#
#   USAGE:
#
#     Viewport pathName child ?options?
#
#        child may either be the pathname of an object that has already
#        been created or a command to create it, e.g.
#
#            listbox .l -geometry 20x10
#            Viewport .v .l -scroll right
#
#        or
#
#            Viewport .v {listbox .l -geometry 20x10} -scroll right
#
# ----------------------------------------------------------------------
#   AUTHOR:  Jim Wight <j.k.wight@newcastle.ac.uk>
# ----------------------------------------------------------------------
# Copyright 1994 The University of Newcastle upon Tyne (see COPYRIGHT)
# ======================================================================

itcl_class Viewport {
    inherit itcl_frame

    constructor {widget args} {
        eval itcl_frame::constructor $args

	frame $this.up
	frame $this.across
	frame $this.corner
	if {[llength $widget] == 1} {
	    set child $widget
	} else {
	    set child [eval $widget]
	}

	set dox 1; set doy 1
	if {[itcl_info objects $child] != ""} {
	    if {[$child info public xscrollcommand] == ""} {
		set dox 0
	    }
	    if {[$child info public yscrollcommand] == ""} {
		set doy 0
	    }
	} else {
	    if {[catch "$child configure -xscrollcommand"]} {
		set dox 0
	    }
	    if {[catch "$child configure -yscrollcommand"]} {
		set doy 0
	    }
	}

	scrollbar $this.vert 
	if {$doy} {
	    $this.vert configure -command "$child yview"
	    $child configure -yscrollcommand "$this vscroll"
	}

	scrollbar $this.horiz
	if {$dox} {
	    $this.horiz configure -orient horizontal -command "$child xview"
	    $child configure -xscrollcommand "$this hscroll"
	}

	repack
    }
    
    public scroll {left} {
	set left 0; set right 0; set top 0; set bottom 0
	set varh 0; set varv 0
	foreach scrollbar $scroll {
            switch -glob $scrollbar {
		var* {regsub "var(.*)" $scrollbar {\1} scroller
		    switch $scroller {
			left -
			right  {set varv 1}
			top -
			bottom {set varh 1}
		    }
		}
		*    {set scroller $scrollbar}
	    }
	    set $scroller 1
	}
	if {[winfo exists $this.up]} {
	    repack
	}
    }


    # Private methods and variables beyond here
    #
    method repack {} {
	if {$left && $right} {
	    set left 1; set right 0
	}
	if {$top && $bottom} {
	    set top 0; set bottom 1
	}  

	pack forget $this.vert $child
	if {$right && (!$varv || ($varv && $von))} {
	    pack $this.vert -in $this.up -side right -fill y
	}

	pack $child -in $this.up -side right -fill both -expand true
	if {$left && (!$varv || ($varv && $von))} {
	    pack $this.vert -in $this.up -side left -fill y
	}

	pack forget $this.horiz $this.corner
	set hon 0
	if {$top || $bottom} {
	    if {$left || $right} {
		update
		regsub {.*x([0-9]+).*} [winfo reqwidth $this.vert] {\1} pad
		set pad [expr ($pad + 1) / 2]
	    }
	    if {$left} {
		pack $this.corner -in $this.across -side left -ipadx $pad
	    }
	    pack $this.horiz -in $this.across -side left -fill x -expand true
	    set hon 1
	    if {$right} {
		pack $this.corner -in $this.across -side right -ipadx $pad
	    }
	}

	pack forget $this.across $this.up
	if {$bottom && (!$varh || ($varh && $hon))} {
	    pack $this.across -side bottom -fill x
	} 
	pack $this.up -side bottom -fill both -expand true
	if {$top && (!$varh || ($varh && $hon))} {
	    pack $this.across -fill x
	}

	raise $child
    }

    protected child {}

    protected left   1 
    protected right  0 
    protected top    0 
    protected bottom 0 

    protected varh   0
    protected varv   0

    protected hon    1
    protected von    1

    method hscroll {max size args} {
	if {$varh && $max <= $size && $hon } {set hon 0; repack}
	if {$varh && $max > $size && !$hon}  {set hon 1; repack}
	eval $this.horiz set $max $size $args
    }

    method vscroll {max size args} {
	if {$varv && $max <= $size && $von } {set von 0; repack}
	if {$varv && $max > $size && !$von}  {set von 1; repack}
	eval $this.vert set $max $size $args
    }
}

