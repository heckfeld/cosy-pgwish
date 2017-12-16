# MenuButton
# ----------------------------------------------------------------------
# A MenuButton is a menubutton that creates and overlays a second button
# on top of itself. The menubutton displays a textual label, the child a
# bitmap (default X menu10 bitmap). Space is made for the bitmap at the
# left hand side by setting the padX option to the width of the bitmap.
# If the height of the bitmap is greater than the natural height of
# $this then $this's padY option is set so that the two heights become
# the same. User settings of padX and padY are ignored.
#
#   PUBLIC ATTRIBUTES:
#
#     -bitmap ........ pathname of the file containing the bitmap to be
#                      displayed. N.B. A leading @ is not required.
#
#   METHODS:
#
#     None
#
#   X11 OPTION DATABASE ATTRIBUTES
#
#     bitmapFilePath .. list of paths to search for bitmap file, i.e
#                       -bitmap value, if it is not an absolute pathname.
#                       The default is at the bottom of this file.
# 
# ----------------------------------------------------------------------
#   AUTHOR:  Jim Wight        <j.k.wight@newcastle.ac.uk>
# ----------------------------------------------------------------------
# Copyright 1994 The University of Newcastle upon Tyne (see COPYRIGHT)
# ======================================================================

itcl_class MenuButton {
    inherit itcl_menubutton

    constructor {args} {
        eval itcl_menubutton::constructor $args

	button $this.bitmap -bitmap [_bitmap] -bd 0
	place $this.bitmap -x 0 -rely 0.5 -anchor w

	_padxy

	bind $this <Any-Enter> "
            [bind Menubutton <Any-Enter>]; $this.bitmap configure -state active
        "
        bind $this <B1-Enter> "
            [bind Menubutton <B1-Enter>]; $this.bitmap configure -state active
        "
        bind $this.bitmap <Button-1> "
            set tk_priv(inMenuButton) $this
            tk_mbButtonDown $this
        "
	bind $this <Any-Leave> "
            [bind Menubutton <Any-Leave>]; $this.bitmap configure -state normal
        "
	bind $this.bitmap <Any-Enter> "
            [bind Button <Any-Enter>]; $this configure -state active
        "
	bind $this.bitmap <Any-Leave> "
            [bind Button <Any-Leave>]; $this configure -state normal
        "
    }

    public bitmap {menu10} {
	if {[winfo exists $this.bitmap]} {
	    $this.bitmap configure -bitmap [_bitmap]

	    _padxy
	}
    }

    # Private from here on down
    #

    method _bitmap {} {
	global _menubutton_priv

	if {$bitmap == ""} {
	    return $bitmap
	}

	if {[string range $bitmap 0 0] == "/"} {
	    if {[file readable $bitmap]} {
 		return @$bitmap
	    } else {
		error "MenuButton $this: $bitmap is not readable"
	    }
	}

	if {![catch {set _menubutton_priv($bitmap)}]} {
	    return [set _menubutton_priv($bitmap)]
	}

	if {[set path [option get $this bitmapFilePath ""]] == ""} {
	    return ""
	}

	set dirs [split $path :]
	foreach dir $dirs {
	    if {[file readable [set xbm $dir/$bitmap]]} {
		set _menubutton_priv($bitmap) @$xbm
		return @$xbm
	    }
	}

	error "MenuButton $this: $bitmap not found in $dirs"
    }

    method _padxy {} {
	$this configure -padx [winfo reqwidth $this.bitmap]

	set ht [winfo reqheight $this]
	set bht [winfo reqheight $this.bitmap]
	set pady [lindex [$this configure -pady] 4]
        incr pady [lindex [$this configure -bd] 4]

	$this configure -pady [expr ($bht - $ht + 1) / 2 + $pady]
    }
}

option add *bitmapFilePath \
      /usr/include/X11/bitmaps:/usr/local/X11/include/X11/bitmaps widgetDefault
