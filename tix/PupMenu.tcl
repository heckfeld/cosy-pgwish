# Local Prefix = _tixPP_ ; Meaning : TIX Popup

#----------------------------------------------------------------------
#                         POPUP MENUS
#----------------------------------------------------------------------
proc tixCreatePopupMenu {w title mkmenu_proc} {
    upvar #0 $w pop
    set pop(widget)  $w
    set pop(parents) {}

    toplevel $w -cursor arrow -class TixPopupMenu
    wm overrideredirect $w 1
    wm withdraw $w
    menubutton $w.mb -text $title -menu $w.mb.m -anchor w
    pack $w.mb -expand yes -fill both

    eval $mkmenu_proc $w.mb.m
    
    bind $w.mb <1>                  "_tixPP_Unpost $w"
    bind $w.mb <3>                  "_tixPP_Unpost $w"
    bind $w.mb.m <Unmap>            "+_tixPP_Unmap $w"
    bind $w.mb.m <ButtonRelease-3>  {tk_invokeMenu %W}
    foreach submenu [tixDescendants $w.mb.m] {
	bind $submenu <ButtonRelease-3> {tk_invokeMenu %W}
    }
}

proc tixDestroyPopupMenu {w} {
    upvar #0 $w pop

    foreach parent $pop(parents) {
	bind $parent <3>         {}
	bind $parent <Control-1> {}
    }

    destroy $pop(widget)
    unset pop
}


proc tixPupBindParents {w args} {
    upvar #0 $w pop

    foreach parent $args {
	bind $parent <3>         "_tixPP_Map $parent $w %x %y"

	if {[lsearch $pop(parents) $parent] == -1} {
	    lappend pop(parents) $parent
	}
    }
}

proc _tixPP_Map {parent w x y} {
    global tk_priv

    set width  [winfo reqwidth  $w]
    set height [winfo reqheight $w]
    wm geometry $w [format "%dx%d" [winfo reqwidth $w.mb.m] $height]    
    set width  [winfo reqwidth  $w]

    set wx [expr [winfo rootx $parent] + $x - $width  / 2]
    set wy [expr [winfo rooty $parent] + $y - $height / 2]

    wm geometry $w [format "+%d+%d" $wx $wy]
    wm deiconify $w
    raise $w

    update idletasks
    grab -global $w
    set tk_priv(inMenuButton) $w.mb 
    tk_mbButtonDown $w.mb
}

proc _tixPP_Unmap {w} {
    grab release $w.mb
    wm withdraw $w
}

proc _tixPP_Unpost {w} {
    grab release $w.mb
    tk_mbUnpost
    wm withdraw $w
}
