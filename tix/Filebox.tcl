proc tixFileSelectbox {w args} {
    eval tixInt_CreateWidget $w tixFsb TixFileSelectbox $args
    return $w
}

proc tixFileSelectBox {w args} {
    eval tixInt_CreateWidget $w tixFsb TixFileSelectbox $args
    return $w
}

proc tixFsb::CreateClassRec {} {
  global tixFsb

  # OPTIONS
  set tixFsb(rootOptions)   {}
  set tixFsb(options)       {-command -browsecmd -pattern -selection
		             -filter -grab
			     -entrybg -entryfont
			     -listboxbg -listboxfont
			     -font -background -foreground -bg -fg
			    }
  set tixFsb(staticOptions) {}
  # DEFAULT VALUES
  set tixFsb(-browsecmd)    {-browsecmd browseCmd BrowseCmd {}}
  set tixFsb(-command)      {-command command Command {}}
  set tixFsb(-selection)    {-selection selection Selection {}}
  set tixFsb(-filter)       {-filter filter Filter {}}
  set tixFsb(-pattern)      {-pattern pattern Pattern *}
  set tixFsb(-grab)         {-grab grab Grab global}

  # Sub widget options
  set tixFsb(-font)         {-font font Font \
	                     "-*-helvetica-bold-r-normal-*-14-*-*-*-*-*-*-*"}
  set tixFsb(-background)   {-background background Background #ffe4c4}
  set tixFsb(-foreground)   {-foreground foreground Foreground black}
  set tixFsb(-entrybg)      {-entrybg entryBg Background #ffe4c4}
  set tixFsb(-entryfont)    {-entryfont entryFont Font \
	                     "-*-helvetica-medium-r-normal-*-14-*-*-*-*-*-*-*"}
  set tixFsb(-listboxbg)    {-listboxbg listboxBg Background #ffe4c4}
  set tixFsb(-listboxfont)  {-listboxfont listboxFont Font \
	                     "-*-helvetica-medium-r-normal-*-14-*-*-*-*-*-*-*"}

  # ALIASES
  set tixFsb(-bg)           {= -background}
  set tixFsb(-fg)           {= -foreground}

  # METHODS
  set tixFsb(methods)       {filter invoke}
}


proc tixFsb::InitWidgetRec {w class className args} {
    upvar #0 $w data

    set data(pwd)     [pwd]
    if {$data(-filter) == {}} {
	set data(-filter) $data(pwd)/$data(-pattern)
    }
    set data(exposed)   0
    set data(flag)      0
    set data(w_dirMenu) $w.dirmenu
}

proc tixFsb::SetBindings {w} {
    upvar #0 $w data

    bind $w <Expose> "tixFsb::Expose $w"
}

#----------------------------------------------------------------------
#		Construct widget
#----------------------------------------------------------------------
proc tixFsb::ConstructWidget {w} {
    upvar #0 $w data

    set frame1 [tixFsb::CreateFrame1 $w]
    set frame2 [tixFsb::CreateFrame2 $w]
    set frame3 [tixFsb::CreateFrame3 $w]

    pack $frame1 -in $w -side top -fill x
    pack $frame3 -in $w -side bottom -fill x
    pack $frame2 -in $w -side top -fill both -expand yes

    # Binding of List box 1 (dir)
    #----------------------------
    set list1 [$data(box1) listbox]
    bind $list1 <1>         "tixFsb::SelectDir $w %W %y"
    bind $list1 <B1-Motion> "tixFsb::SelectDir $w %W %y"
    bind $list1 <Double-ButtonPress-1> "tixFsb::InvokeDir $w %W %y"
    global tix_priv
    if [info exists tix_priv(as_loaded)] {
	tixAutoS_BindListboxSingle $list1
    }

    # Binding of List box 2 (file)
    #-----------------------------
    set list2 [$data(box2) listbox]
    bind $list2 <1>         "tixFsb::SelectFile $w %W %y"
    bind $list2 <B1-Motion> "tixFsb::SelectFile $w %W %y"
    bind $list2 <Double-ButtonPress-1> "$w invoke"
    if [info exists tix_priv(as_loaded)] {
	tixAutoS_BindListboxSingle $list2
    }
}

proc tixFsb::CreateFrame1 {w} {
    upvar #0 $w data

    frame $w.f1 -border 10 -bg $data(-background)
    label $w.f1.l1 -text "Filter"\
	-font $data(-font) \
	-bg $data(-background) \
	-fg $data(-foreground)
    tixCombobox $w.f1.cbx -type history \
	-command "$data(root) filter" -height 5 -scrollbar auto -anchor e \
	-entrybg $data(-entrybg) \
	-entryfont $data(-entryfont) \
	-listboxbg $data(-listboxbg) \
	-listboxfont $data(-listboxfont) \
	-fg $data(-foreground)
    pack $w.f1.l1 $w.f1.cbx -side top -expand yes -fill both

    set data(w_cbxFil) $w.f1.cbx
    lappend data(subwidgets) $w.f1 $w.f1.l1 $w.f1.cbx
    lappend data(labels) $w.f1.l1
 
    $w.f1.cbx addhistory $data(pwd)/*
    return $w.f1
}

proc tixFsb::CreateFrame2 {w} {
    upvar #0 $w data

    frame $w.f2 -border 10 -bg $data(-background)
    #     THE LEFT FRAME
    #-----------------------
    frame $w.f2a -bg $data(-background)
    label $w.f2a.lab -text "Directories" \
	-bg $data(-background) \
	-fg $data(-foreground) \
	-font $data(-font)
    tixScrolledListbox $w.f2a.sbox -geometry 8x6 \
	-bg $data(-background) \
	-listboxbg $data(-listboxbg) \
	-font $data(-listboxfont) \
	-fg $data(-foreground)

    pack $w.f2a.lab   -side top                -fill x
    pack $w.f2a.sbox  -side bottom -expand yes -fill both

    #     THE RIGHT FRAME
    #-----------------------
    frame $w.f2b -bg $data(-background)
    label $w.f2b.lab -text "Files" \
	-bg $data(-background) \
	-fg $data(-foreground) \
	-font $data(-font)
    tixScrolledListbox $w.f2b.sbox -geom 8x6 \
	-bg $data(-background) \
	-listboxbg $data(-listboxbg) \
	-font $data(-listboxfont) \
	-fg $data(-foreground)


    pack $w.f2b.lab  -side top                -fill x
    pack $w.f2b.sbox -side bottom -expand yes -fill both

    # used to make some width between the listboxes
    #----------------------------------------------
    label $w.f2labx -width 1 -bg $data(-background)

    pack $w.f2a    -in $w.f2 -side left -fill both -expand yes
    pack $w.f2labx -in $w.f2 -side left -fill y
    pack $w.f2b    -in $w.f2 -side left -fill both -expand yes

    set data(box1)  $w.f2a.sbox
    set data(box2)  $w.f2b.sbox
    set data(dummy) $w.f2a.lab

    lappend data(subwidgets) $w.f2 \
	$w.f2a $w.f2a.lab $w.f2a.sbox \
	$w.f2b $w.f2b.lab $w.f2b.sbox \
	$w.f2labx
    lappend data(labels) $w.f2a.lab $w.f2b.lab 

    return $w.f2
}

proc tixFsb::CreateFrame3 {w} {
    upvar #0 $w data

    frame $w.f3 -border 10 -bg $data(-background)
    label $w.f3.l1 -text "Selection" \
	-font $data(-font) \
	-bg $data(-background) \
	-fg $data(-foreground)
    tixCombobox $w.f3.cbx -type history \
	-command "$data(root) invoke" -height 5 -scrollbar auto -anchor e \
	-entrybg $data(-entrybg) \
	-entryfont $data(-entryfont) \
	-listboxbg $data(-listboxbg) \
	-listboxfont $data(-listboxfont) \
	-fg $data(-foreground)
    pack $w.f3.l1 $w.f3.cbx -side top -fill both

    set data(w_cbxSel) $w.f3.cbx

    lappend data(subwidgets) $w.f3 $w.f3.l1 $w.f3.cbx
    lappend data(labels) $w.f3.l1

    return $w.f3
}

#----------------------------------------------------------------------
#                           CONFIG OPTIONS
#----------------------------------------------------------------------
# Set the filter to the real  directory (resolve all symlinks)
#
proc tixFsb::config-filter {w arg} {
    upvar #0 $w data

    set tmp [pwd]
    catch {
	cd [file dirname $arg]
	set data(pwd) [pwd]
    }
    cd $tmp

    set data(-pattern) [file tail $arg]
    if {$data(pwd) == "/"} {
	set data(-filter) /$data(-pattern)
    } else {
	set data(-filter) $data(pwd)/$data(-pattern)
    }

    $data(w_cbxFil) entry set $data(-filter)
}

proc tixFsb::config-selection {w arg} {
    upvar #0 $w data
    $data(w_cbxSel) entry set $arg
}

proc tixFsb::config-pattern {w arg} {
    upvar #0 $w data
    set dir [file dirname [$data(w_cbxFil) entry get]]
    if {$dir == "/"} {
	$data(w_cbxFil) entry set /$arg
    } else {
	$data(w_cbxFil) entry set $dir/$arg
    }
}

proc tixFsb::config-background {w arg} {
    upvar #0 $w data

    $data(rootCmd) config -bg $arg 
    foreach subw $data(subwidgets) {
	$subw config -bg $arg
    }
    $data(box1) config -scrollbarfg $arg
    $data(box2) config -scrollbarfg $arg
}

proc tixFsb::config-foreground {w arg} {
    upvar #0 $w data

    foreach label $data(labels) {
	$label config -fg $arg
    }
    $data(w_cbxFil) config -fg $arg
    $data(w_cbxSel) config -fg $arg
    $data(box1) config -fg $arg
    $data(box2) config -fg $arg
}

proc tixFsb::config-font {w arg} {
    upvar #0 $w data

    foreach label $data(labels) {
	$label config -font $arg
    }
}

proc tixFsb::config-listboxbg {w arg} {
    upvar #0 $w data

    $data(w_cbxFil) config -listboxbg $arg
    $data(w_cbxSel) config -listboxbg $arg
    $data(box1) config -listboxbg $arg
    $data(box2) config -listboxbg $arg
    $data(box1) config -scrollbarbg $arg
    $data(box2) config -scrollbarbg $arg
}

proc tixFsb::config-listboxfont {w arg} {
    upvar #0 $w data

    $data(w_cbxFil) config -listboxfont $arg
    $data(w_cbxSel) config -listboxfont $arg
    $data(box2) config -font $arg
    $data(box2) config -font $arg
}

proc tixFsb::config-entrybg {w arg} {
    upvar #0 $w data

    $data(w_cbxFil) config -entrybg $arg
    $data(w_cbxSel) config -entrybg $arg
}

proc tixFsb::config-listboxfont {w arg} {
    upvar #0 $w data

    $data(w_cbxFil) config -entryfont $arg
    $data(w_cbxSel) config -entryfont $arg
}

#----------------------------------------------------------------------
#                    INTERNAL METHODS
#----------------------------------------------------------------------
proc tixFsb::LoadDirIntoLists {w} {
    upvar #0 $w data

    $data(box1) listbox delete 0 end
    $data(box2) listbox delete 0 end

    set tmp [pwd]
    cd $data(pwd)

    foreach i [lsort [glob -nocomplain * .*]] {
	set fname [file tail $i]
	if [file isdirectory $data(pwd)/$fname] {
	    $data(box1) listbox insert end $fname
	}
    }

    set data(-filter) [$data(w_cbxFil) entry get]

    set top 0

    # force glob to list the .* files. However, since the use might not
    # be interested in them, put the "normal" files at the top of the listbox

    if {$data(-pattern) == "*"} {
	foreach i [lsort [glob -nocomplain * .*]] {
	    set fname [file tail $i]
	    if {![file isdirectory $data(pwd)/$fname]} {
		$data(box2) listbox insert end $fname
		if {[string index $fname 0] == "."} {
		    incr top
		}
	    }
	}
    } else {
	foreach i [lsort [glob -nocomplain $data(-filter)]] {
	    set fname [file tail $i]
	    if {![file isdirectory $data(pwd)/$fname]} {
		$data(box2) listbox insert end $fname
	    }
	}
    }

    $data(box2) listbox yview $top
    cd $tmp
}

proc tixFsb::LoadDir {w} {
    upvar #0 $w data
    global TIX_SOURCE_DIR

    # Sets a grab to $data(dummy), which is a label, so that the 
    # application won't respond to any event during the time the file names
    # are being loaded in. I use a nasty global grab here. I will try to 
    # do it with a localk grab but so far I haven't succeeded.
    update idletasks

    case $data(-grab) {
	global {
	    grab -global $data(dummy)
	    $data(dummy) config -cursor \
		"@$TIX_SOURCE_DIR/bitmaps/hourglass \
		 $TIX_SOURCE_DIR/bitmaps/hourglass \
		 red white"
	}
	local {
	    grab $data(dummy)
	}
    }

    catch {
	# Just in case some error happens inside grab, don't
	# want to make the X server inaccessible

	update idletasks
	catch {
	    tixFsb::LoadDirIntoLists $w
	} err

	# If the user enters an invalid directory, clean up the mess, restore
	# the filter and attempt to reload the files
	#--------------------------------------------------------------------
	if {$err != {}} {
	    set data(-filter) [$data(w_cbxFil) entry get]
	    set ftail [file tail $data(-filter)]
	    if {$data(pwd) == "/"} {
		set data(-filter) /$ftail
	    } else {
		set data(-filter) $data(pwd)/$ftail
	    }
	    $data(w_cbxFil) entry set $data(-filter)
	    catch {
		tixFsb::LoadDirIntoLists $w
	    }
	}

	tixFsb::MkDirMenu $w

	# process (and discard) all events when the file names were being 
	# loaded in, then release the grab

	update
	$data(dummy) config -cursor {}
	grab release $data(dummy) 
    } err

    if {$err != {}} {
	grab release $data(dummy) 
	$data(dummy) config -cursor {}
	error $err
    }
}

proc tixFsb::MkDirMenu {w} {
    upvar #0 $w data

    if [winfo exists $data(w_dirMenu)] {
	destroy $data(w_dirMenu)
    }
    tixCreatePopupMenu $data(w_dirMenu) Directories "tixFsb::MkDirMenuProc $w"
    tixPupBindParents $data(w_dirMenu) [$data(w_cbxFil) entry]
}

proc tixFsb::MkDirMenuProc {w m} {
    upvar #0 $w data

    set f $data(pwd)

    menu $m
    while {$f != "/"} {
	set name "$f/$data(-pattern)"
	$m add command -label $name -command "tixFsb::MenuInvokeDir $w $name"
	set f [file dirname $f]
    }
    $m add command -label $f/$data(-pattern)
}

proc tixFsb::SelectDir {w dir_list y} {
    upvar #0 $w data

    set data(-filter) [$data(w_cbxFil) entry get]
    set data(-pattern) [file tail $data(-filter)]

    $dir_list select from [$dir_list nearest $y]
    $dir_list select to [$dir_list nearest $y]
    set subdir [$dir_list get [$dir_list nearest $y]]
    if {$data(pwd) == "/"} {
	set data(-filter) /$subdir/$data(-pattern)
    } else {
	set data(-filter) $data(pwd)/$subdir/$data(-pattern)
    }
    $data(w_cbxFil) entry set $data(-filter)
}

proc tixFsb::InvokeDir {w dir_list y} {
    upvar #0 $w data

    set data(-filter) [$data(w_cbxFil) entry get]
    set data(-pattern) [file tail $data(-filter)]

    $dir_list select from [$dir_list nearest $y]
    $dir_list select to [$dir_list nearest $y]
    set tmp [pwd]
    catch {
	cd $data(pwd)
	cd [$dir_list get [$dir_list nearest $y]]
	set data(pwd) [pwd]
    }
    cd $tmp

    if {$data(pwd) == "/"} {
	set data(-filter) /$data(-pattern)
    } else {
	set data(-filter) $data(pwd)/$data(-pattern)
    }
    $data(w_cbxFil) entry set $data(-filter)
    tixFsb::LoadDir $w
}

proc tixFsb::MenuInvokeDir {w filter} {
    upvar #0 $w data

    set data(-filter) $filter
    set data(pwd) [file dirname $filter]
    $data(w_cbxFil) entry set $filter

    tixFsb::LoadDir $w
}

proc tixFsb::filter {w args} {
    upvar #0 $w data

    set data(-filter)  [$data(w_cbxFil) entry get]
    set data(-pattern) [file tail $data(-filter)]
    set tmp [pwd]
    catch {
	cd $data(pwd)
	cd [file dirname $data(-filter)]
	set data(pwd) [pwd]
    }
    cd $tmp

    tixFsb::LoadDir $w
}

proc tixFsb::SelectFile {w file_list y} {
    upvar #0 $w data

    $file_list select from [$file_list nearest $y]
    $file_list select to [$file_list nearest $y]
    set selected [$file_list get [$file_list nearest $y]]

    if {$data(pwd) == "/"} {
	$data(w_cbxSel) entry set /$selected
	set $data(-selection) /$selected
    } else {
	$data(w_cbxSel) entry set $data(pwd)/$selected
	set $data(-selection) $data(pwd)/$selected
    }
    if {$data(-browsecmd) != {}} {
	eval $data(-browsecmd) [$data(w_cbxSel) entry get]
    }
}

proc tixFsb::invoke {w args} {
    upvar #0 $w data

    $data(w_cbxFil) addhistory $data(-filter)
    set data(-filter) [$data(w_cbxFil) entry get]

    $data(w_cbxSel) addhistory [$data(w_cbxSel) entry get]
    set data(-selection) [$data(w_cbxSel) entry get]

    if {$data(-command) != {}} {
	eval $data(-command) [list [$data(w_cbxSel) entry get]]
    }
}

proc tixFsb::Expose {w} {
    upvar #0 $w data

    if {$data(exposed) == 0} {
	set data(exposed) 1
	$data(w_cbxFil) entry set $data(-filter)
	tixFsb::LoadDir $w
	$data(w_cbxFil) align
    }
}

#----------------------------------------------------------------------
#
#
#              C O N V E N I E N C E   R O U T I N E S 
#
#
#----------------------------------------------------------------------
proc tixMkFileDialog {
    w
    okcmd
    {helpcmd      {}} } \
{
    toplevel $w
    wm minsize $w 10 10

    tixStdDlgBtns $w.btns
    tixFileSelectbox $w.fsb -command "wm withdraw $w; $okcmd"

    $w.btns button ok     config -command "$w.fsb invoke"
    $w.btns button apply  config -command "$w.fsb filter" -text Filter
    $w.btns button cancel config -command "wm withdraw $w"
    if {$helpcmd == {}} {
	$w.btns button help config -state disabled
    } else {
	$w.btns button help config -command $helpcmd
    }
    wm protocol $w WM_DELETE_WINDOW "wm withdraw $w"
    pack $w.btns  -side bottom -fill both
    pack $w.fsb   -fill both -expand yes

    return $w.fsb
}
