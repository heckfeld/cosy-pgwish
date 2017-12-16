# -*-TCL-*-  TCL mode for Emacs
#
# tcl_file: dd-prod
#---------------------------------------------------------------
# Description : A few procedures for support drag&drop
#
# TCL/TK extensions required: blt
#
#
# PROCEDURES:
#	dd_send_by_handler ...... handler drop basing on passed handle
#	package_text ............ package d&d token for text passing
#	site_text ............... change d&d token appearance on valid sites
#	set_target_message ...... accepts text drop
#


proc dd_send_by_handler {handler interp ddwin data} {
    send $interp "
		global DragDrop
		set DragDrop($handler) [list $data]
	"
    send $interp "blt_drag&drop target $ddwin handle $handler"
}


# -----------------------------------------------------------------------
#  USAGE:  package_text <entry-widget> <token-win>
#
#  Performs the task of "packaging" for drag&drop operations.  Figures
#  out what data is being dragged/dropped and updates the appearance
#  of the token window to reflect this.  This simple routine merely
#  displays the string from an entry widget as a label in the token
#  window.
# -----------------------------------------------------------------------
proc package_text {src win} {
    if {[winfo children $win] == ""} {
        label $win.label
        pack $win.label
    }
    set text [$src get]
    if { [llength $text] > 3 } {
	$win.label config -text "[lrange $text 0 1] ..." -bitmap {}
    } else {
	$win.label config -text $text -bitmap {}
    }
    return $text
}

# -----------------------------------------------------------------------
#  USAGE:  site_text <state> <token-win>
#
#  Acts as the "sitecmd" for drag&drop operations involving windows
#  configured via package_text.  When <state> is non-zero, the color
#  of the label in the token window is changed to indicate that the
#  token is over a valid target.
# -----------------------------------------------------------------------
proc site_text {state win} {
    set normbg [lindex [$win.label config -background] 4]
    set normfg [lindex [$win.label config -foreground] 4]
    $win.label config -background $normfg -foreground $normbg
}

# -----------------------------------------------------------------------
#  USAGE:  set_target_mesg <target>
#
#  Invoked as the handler for drag&drop targets.  Takes the incoming
#  drag&drop text data and sets target to be it
# -----------------------------------------------------------------------
proc set_target_mesg {target} {
    global DragDrop
    global $target; set $target $DragDrop(text)
}

