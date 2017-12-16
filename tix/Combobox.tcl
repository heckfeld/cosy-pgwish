proc tixCombobox {w args} {
    eval tixInt_CreateWidget $w tixCbx TixCombobox $args
    return $w
}

# Obsolete
proc tixComboBox {w args} {
    eval tixInt_CreateWidget $w tixCbx TixCombobox $args
    return $w
}

proc tixCbx::CreateClassRec {} {
    global tixCbx

    # OPTIONS
    set tixCbx(rootOptions)   {}
    set tixCbx(options)       {-command -fancy
	                       -type -state -anchor
                               -scrollbar -histlimit
			       -background -foreground -bg -fg
			       -disabledforeground
			       -entrywidth -entrybg -entryfont
			       -listboxbg -listboxfont -listboxheight
			       -width -height
			       }
    set tixCbx(staticOptions) {}

    # DEFAULT VALUES
    set tixCbx(-anchor)       {-anchor anchor Anchor w}
    set tixCbx(-command)      {-command command Command {}}
    set tixCbx(-state)        {-state state State normal}
    set tixCbx(-type)         {-type type Type static}
    set tixCbx(-histlimit)    {-histlimit histLimit HistLimit 20}
    set tixCbx(-fancy)        {-fancy fancy Fancy "false"}
    set tixCbx(-scrollbar)    {-scrollbar scrollbar Scrollbar y}

    # Sub widget default
    set tixCbx(-background)   {-background background Background #ffe4c4}
    set tixCbx(-foreground)   {-foreground foreground Foreground black}
    set tixCbx(-disabledforeground) {\
	                       -disabledforeground\
	                       -disabledForeground\
	                       -DisabledForeground #606060}
    set tixCbx(-entrywidth)   {-entrywidth entryWidth EntryWidth 10}
    set tixCbx(-entryfont)    {-entryfont entryFont Font \
                             "-*-helvetica-medium-r-normal-*-14-*-*-*-*-*-*-*"}
    set tixCbx(-entrybg)      {-entrybg entryBg EntryBg #ffe4c4}
    set tixCbx(-listboxbg)    {-listboxbg listboxBg ListbixBg #ffe4c4}
    set tixCbx(-listboxfont)  {-listboxfont listboxFont Font \
	                     "-*-helvetica-medium-r-normal-*-14-*-*-*-*-*-*-*"}
    set tixCbx(-listboxheight) {-listboxheight listboxHeight ListboxHeight 6}

    # ALIASES
    set tixCbx(-width)        {= -entrywidth}
    set tixCbx(-height)       {= -listboxheight}
    set tixCbx(-bg)           {= -background}
    set tixCbx(-fg)           {= -foreground}

    # METHODS
    set tixCbx(methods)       {listbox entry get pick addhistory appendhistory\
                               align}
}

proc tixCbx::ConstructWidget {w} {
    upvar #0 $w data
    global TIX_SOURCE_DIR

    $data(rootCmd) config -bg $data(-background) 

    # Create the static part of the combo box
    #----------------------------------------
    set data(entry) [entry $w.ent \
		     -width $data(-entrywidth) -font $data(-entryfont) \
		     -bg $data(-entrybg) -fg $data(-foreground)]
    set data(cross) [button $w.cross -bitmap @$TIX_SOURCE_DIR/bitmaps/cross\
		     -bg $data(-background) \
		     -fg $data(-foreground)]
    set data(tick)  [button $w.tick  -bitmap @$TIX_SOURCE_DIR/bitmaps/tick\
		     -bg $data(-background) \
		     -fg $data(-foreground)]
    set data(btn)   [button $w.btn -bitmap @$TIX_SOURCE_DIR/bitmaps/cbxarrow\
		     -bg $data(-background) \
		     -fg $data(-foreground)]

    if {$data(-fancy) == "true" || $data(-fancy) == "yes"} {
	pack $w.cross -side left -padx 1
	pack $w.tick  -side left -padx 1
    }
    pack $w.btn -side right -padx 1
    pack $w.ent -side left -fill x -expand yes -padx 1

    # Create the shell and the list
    #------------------------------

    set data(shell) [toplevel $w.shell -border 2 -relief raised\
		     -bg $data(-background) -cursor arrow]
    wm overrideredirect $w.shell 1
    wm withdraw $w.shell

    set data(slb) \
	[tixScrolledListbox $w.shell.slb \
	 -scrollbar $data(-scrollbar) \
	 -anchor $data(-anchor) \
	 -listboxbg $data(-listboxbg)\
	 -bg $data(-background)\
	 -fg $data(-foreground)\
	 -font $data(-listboxfont)]

    pack $data(slb) -expand yes -fill both

    # Set the behavior of the combobox
    #---------------------------------

    bind $data(entry) <Down>     "tixCbx::EntArrowKeyDown $w"
    bind $data(entry) <Up>       "tixCbx::EntArrowKeyUp   $w"
    bind $data(entry) <Return>   "tixCbx::EntReturn $w"
    bind $data(entry) <Double-ButtonPress-1> "tixCbx::EntReturn $w"
    bind $data(entry) <1>        "+focus %W"
    bind $data(entry) <1>        "+[bind Entry <1>]"
    global tix_priv
    if [info exists tix_priv(as_loaded)] {
	tixAutoS_BindEntry $data(entry)
    }

    bind [$data(slb) listbox] <Down>      "tixCbx::LbArrowKeyDown $w"
    bind [$data(slb) listbox] <Up>        "tixCbx::LbArrowKeyUp   $w"
    bind [$data(slb) listbox] <Return>    "tixCbx::LbReturn $w"

    bind [$data(slb) listbox] <B1-Motion> "tixCbx::LbButton $w %y"
    bind [$data(slb) listbox] <1>         "tixCbx::LbButton $w %y"
    bind [$data(slb) listbox] <ButtonRelease-1> "tixCbx::LbRelease $w %x %y"
    if [info exists tix_priv(as_loaded)] {
	tixAutoS_BindListboxSingle [$data(slb) listbox]
    }

    # Note that <1> is treated the same as escape if it happens
    # outsides of the combobox
    bind [$data(slb) listbox] <Escape> "tixCbx::EscKey $w"
    bind $data(entry)   <Escape> "tixCbx::EscKey $w"
    bind $data(btn)     <Escape> "tixCbx::EscKey $w"
    bind $w             <Escape> "tixCbx::EscKey $w"
    bind $w             <1>      "tixCbx::EscKey $w"


    $data(btn)   config -command "tixCbx::BtnUp $w"
    $data(tick)  config -command "tixCbx::EntReturn $w"
    $data(cross) config -command "tixCbx::ClearEntry $w"

    if {$data(editable) != "1"} {
	global tixOption
	$data(entry) config -bg $tixOption(bg)
    }
}

proc tixCbx::InitWidgetRec {w class className args} {
    upvar #0 $w data

    set data(grabbed)      0
    set data(first_expose) 0
    set data(flag)         0
    set data(histCount)    0

    if {$data(-type) == "history" || $data(-type) == "editable"} {
	set data(editable) 1
    } else {
	set data(editable) 0
    }
}

proc tixCbx::SetBindings {w} {
    upvar #0 $w data

    bind $w <Expose> "tixCbx::Expose $w"
}

#----------------------------------------------------------------------
#                           CONFIG OPTIONS
#----------------------------------------------------------------------
proc tixCbx::config-anchor {w arg} {
    # $$temp not implemented
}

proc tixCbx::config-background {w arg} {
    upvar #0 $w data

    $data(rootCmd) config -bg $arg
    $data(cross)   config -bg $arg
    $data(tick)    config -bg $arg
    $data(btn)     config -bg $arg
    $data(shell)   config -bg $arg
    $data(slb)     config -bg $arg
}

proc tixCbx::config-foreground {w arg} {
    upvar #0 $w data

    $data(entry) config -fg $arg
    $data(cross) config -fg $arg
    $data(tick)  config -fg $arg
    $data(btn)   config -fg $arg
    $data(slb)   config -fg $arg
}

proc tixCbx::config-entrybg {w arg} {
    upvar #0 $w data

    $data(entry) config -bg $arg
}

proc tixCbx::config-entryfont {w arg} {
    upvar #0 $w data

    $data(entry) config -font $arg
}

proc tixCbx::config-entrywidth {w arg} {
    upvar #0 $w data

    $data(entry) config -width $arg
}

proc tixCbx::config-listboxbg {w arg} {
    upvar #0 $w data

    $data(slb) config -listboxbg $arg
}

proc tixCbx::config-listboxfont {w arg} {
    upvar #0 $w data

    $data(slb) config -font $arg
}

proc tixCbx::config-state {w arg} {
    upvar #0 $w data

    if {$arg == "normal"} {
	$data(entry) config -fg $data(-foreground)
	$data(btn)   config -state normal
	$data(tick)  config -state normal
	$data(cross) config -state normal
    } else {
	$data(entry) config -fg $data(-disabledforeground)
	$data(btn)   config -state disabled
	$data(tick)  config -state disabled
	$data(cross) config -state disabled
    }
}

#proc tixCbx::config-type {w arg} {
#    error "Cannot reconfigure -type after widget creation"
#}

proc tixCbx::config-listboxheight {w arg} {
    upvar #0 $w data

    $data(slb) config -geom $data(-entrywidth)x$data(-listboxheight)
}

proc tixCbx::config-entrywidth {w arg} {
    upvar #0 $w data
    $data(entry) config -width $data(-entrywidth)
}

#----------------------------------------------------------------------
#                     WIDGET COMMANDS
#----------------------------------------------------------------------
proc tixCbx::listbox {w args} {
    upvar #0 $w data

    if {$args != {}} {
	return [eval $data(slb) listbox $args]
    } else {
	return [$data(slb) listbox]
    }
}

proc tixCbx::entry {w args} {
    upvar #0 $w data

    if {$args == {}} {
	return $data(entry)
    } elseif {[lindex $args 0] == "set"} {
	if {$data(editable) == "0"} {
	    $data(entry) config -state normal
	}
	$data(entry) delete 0 end
	$data(entry) insert 0 [lindex $args 1]
	if {$data(editable) == "0"} {
	    $data(entry) config -state disabled
	}
	if {$data(-anchor) == "e"} {
	    tixCbx::EntryAlignEnd $w
	}
    } else {
	return [eval $data(entry) $args]
    }
}

proc tixCbx::pick {w args} {
    upvar #0 $w data

    if {$data(editable) == "0"} {
	$data(entry) config -state normal
    }
    if {[llength $args] == "2"} {
	set index [lindex $args 1]
	$data(slb) listbox select from $index
	$data(slb) listbox select to   $index
	set text [$data(slb) listbox get $index]
    } else {
	set text [lindex $args 0]
    }
    $data(entry) delete 0 end
    $data(entry) insert 0 $text

    #kludge : if pick before exposing, picked item will be lost

    if {$data(first_expose) == "1"} {
	set data(first_expose) 0
    }
    if {$data(editable) == "0"} {
	$data(entry) config -state disabled
    }
}

proc tixCbx::addhistory {w newtext} {
    upvar #0 $w data

    set indices [$data(slb) listbox curselection]
    if {$indices == "" } {
	set i 0
    } else {
	set i [lindex $indices 0]
    }
    set newtext [string trim $newtext]
    if {$newtext != ""} {
	$data(slb) listbox insert $i $newtext
	$data(slb) listbox select from $i
	$data(slb) listbox select to   $i
	$data(slb) listbox yview $i
    }
    if {$data(histCount) == $data(-histlimit)} {
	$data(slb) listbox delete end
    } else {
	incr data(histCount)
    }
}

proc tixCbx::appendhistory {w newtext} {
    upvar #0 $w data

    $data(slb) listbox insert end $newtext
    if {$data(histCount) == $data(-histlimit)} {
	$data(slb) listbox delete 0
    } else {
	incr data(histCount)
    }
}

proc tixCbx::align {w} {
    upvar #0 $w data

    if {$data(-anchor) == "e"} {
	tixCbx::EntryAlignEnd $w
    }
}

proc tixCbx::get {w} {
    upvar #0 $w data

    return [$data(entry) get]
}

#----------------------------------------------------------------------
#                   E V E N T   B I N D I N G S
#----------------------------------------------------------------------
proc tixCbx::BtnUp {w} {
    upvar #0 $w data

    if {$data(grabbed) == 0} {
	tixCbx::PopupShell $w
    } else {
	tixCbx::RestoreValue $w
	tixCbx::PopdownShell $w
    }
}

proc tixCbx::EntArrowKeyUp {w} {
    upvar #0 $w data

    if {$data(grabbed) == 0} {
	tixCbx::PopupShell $w
	tixCbx::SetEntry $w
    } else {
	tixCbx::LbArrowKeyUp $w
    }
}

proc tixCbx::EntArrowKeyDown {w} {
    upvar #0 $w data

    if {$data(grabbed) == 0} {
	tixCbx::PopupShell $w
	tixCbx::SetEntry $w
    } else {
	tixCbx::LbArrowKeyDown $w
    }
}

proc tixCbx::EntReturn {w} {
    upvar #0 $w data

    if {$data(grabbed) != 0} {
	tixCbx::PopdownShell $w
    }

    set old_bg [lindex [$data(entry) config -bg] 4]
    set old_fg [lindex [$data(entry) config -fg] 4]
    $data(entry) config -fg $old_bg
    $data(entry) config -bg $old_fg
    update idletasks
    $data(entry) config -fg $old_fg
    $data(entry) config -bg $old_bg

    if {$data(-command) != {} && $data(-command) != "" } {
	eval $data(-command) [list [$data(entry) get]]
    }

    if {$data(-type) == "history"} {
	tixCbx::addhistory $w [$data(entry) get]
    }
}

proc tixCbx::LbButton {w y} {
    upvar #0 $w data
    $data(slb) listbox select clear
    $data(slb) listbox select from [$data(slb) listbox nearest $y]
    tixCbx::SetEntry $w
}

proc tixCbx::LbReturn {w} {
    upvar #0 $w data

    tixCbx::PopdownShell $w
    if {$data(-command) != {} && $data(-command) != "" } {
	eval $data(-command) [list [$data(entry) get]]
    }
}

proc tixCbx::LbRelease {w x y} {
    upvar #0 $w data

    if {$x < 0 || $y < 0} {
	return
    }
    set listbox [$data(slb) listbox]
    if {$x > [winfo width $listbox]} {
	return
    }
    if {$y > [winfo height $listbox]} {
	return
    }

    tixCbx::PopdownShell $w
    if {$data(-command) != {} && $data(-command) != "" } {
	eval $data(-command) [list [$data(entry) get]]
    }
}


proc tixCbx::LbArrowKeyUp {w} {
    upvar #0 $w data

    set indices [$data(slb) listbox curselection]
    if {$indices == "" } {
	$data(slb) listbox select from 0
	$data(slb) listbox select to   0
	$data(slb) listbox yview 0
    } else {
	set i [lindex $indices 0]
	if {$i > 0} {
	    incr i -1
	    $data(slb) listbox select from $i
	    $data(slb) listbox select to   $i
	    set height [$data(slb) info height]
	    set yview  [$data(slb) info yview]
	    if {$i < $yview || $i >= [expr "$yview+$height"]} {
		$data(slb) listbox yview $i
	    }
	}
    }
    tixCbx::SetEntry $w
}


proc tixCbx::LbArrowKeyDown {w} {
    upvar #0 $w data

    set indices [$data(slb) listbox curselection]
    if {$indices == "" } {
	$data(slb) listbox select from 0
	$data(slb) listbox select to   0
	$data(slb) listbox yview 0
    } else {
	set i [lindex $indices 0]
	incr i
	if {$i < [$data(slb) info totalheight]} {
	    $data(slb) listbox select from $i
	    $data(slb) listbox select to   $i
	    set height [$data(slb) info height]
	    set yview  [$data(slb) info yview]
	    if {$i < $yview || $i >= [expr "$yview+$height"]} {
		$data(slb) listbox yview [expr "$i - $height + 1"]
	    }
	}
    }
    tixCbx::SetEntry $w
}

proc tixCbx::EscKey {w} {
    upvar #0 $w data
    
    if {$data(grabbed) == "1"} {
	tixCbx::RestoreValue $w
	tixCbx::PopdownShell $w
    }
}

proc tixCbx::Expose {w} {
    upvar #0 $w data

    if {$data(first_expose) == "1"} {
	set data(first_expose) 0
	tixCbx::SetEntry $w
    }
}

#----------------------------------------------------------------------
# Internal commands
#----------------------------------------------------------------------
proc tixCbx::SetEntry {w} {
    upvar #0 $w data

    set indices [$data(slb) listbox curselection]
    if {$indices != ""} {
	set i [lindex $indices 0]
    } else {
	set i 0
    }
    if {$data(editable) == "0"} {
	$data(entry) config -state normal
    }
    $data(entry) delete 0 end
    $data(entry) insert 0 [$data(slb) listbox get $i]
    if {$data(editable) == "0"} {
	$data(entry) config -state disabled
    }
    focus $data(entry)
}

proc tixCbx::ClearEntry {w} {
    upvar #0 $w data

    $data(entry) delete 0 end
}

proc tixCbx::PopdownShell {w} {
    upvar #0 $w data

    wm withdraw $data(shell)
    focus $data(entry)
    $data(rootCmd) config -cursor {}
    grab release $data(root)
    set data(grabbed) 0
}

proc tixCbx::RestoreValue {w} {
    upvar #0 $w data

    if {$data(editable) == "0"} {
	$data(entry) config -state normal
    }
    $data(entry) delete 0 end
    $data(entry) insert 0 $data(oldvalue)
    if {$data(editable) == "0"} {
	$data(entry) config -state disabled
    }
}

# Calculating the geometry of the combo box
#
# +4 in the height because of the frames surrounding the listbox
proc tixCbx::PopupShell {w} {
    upvar #0 $w data

    # pop up the shell
    set y [expr [winfo rooty $data(entry)]+[winfo height $data(entry)]+3]
    $data(slb) config -geom $data(-entrywidth)x$data(-listboxheight)
    set height [expr "[winfo reqheight [$data(slb) listbox]]+4"]

    set x1 [winfo rootx $data(entry)]
    set x2 [expr "[winfo rootx $data(btn)] + [winfo width $data(btn)]"]
    set width  [expr "$x2 - $x1"]

    # If the listbox is below bottom of screen, put it upwards
    set scrheight [winfo screenheight .]
    set bottom [expr $y+$height]
    if {$bottom > $scrheight} {
	set y [expr $y-$height-[winfo height $data(entry)]-5]
    }

    wm geometry $data(shell) $width\x$height+$x1+$y
    wm deiconify $data(shell)
    raise $data(shell)
    focus $data(entry)

    if {[$data(slb) listbox curselection] == ""} {
	$data(slb) listbox select from 0
	$data(slb) listbox select to   0
    }
    $data(rootCmd) config -cursor arrow

    set data(grabbed) 1
    set data(oldvalue) [$data(entry) get]
    catch {
	grab -global $data(root)
    }
}

# The following two routines can emulate a "right align mode" for the
# entry in the combo box.
proc tixCbx::MonitorEntry {w total max begin end} {
    upvar #0 $w data

    if {$data(flag) == 0} {
	set data(flag) 1
	if {$total > $max && $total > [expr $end +1]} {
	    $data(entry) view [expr "$total - $max + 1"]
	}
	set data(flag) 0
    }
}

proc tixCbx::EntryAlignEnd {w} {
    upvar #0 $w data

    set data(flag) 0
    $data(entry) config \
	-scroll "tixCbx::MonitorEntry $w"
    update idletasks
    $data(entry) config -scroll {}
}
