lappend auto_path .. 
tixInit -libdir ..  -scheme SGIGray -fontset 14Point -binding Motif


proc WindowPlacement_Init { userDefaults appDefaults {appLibLoc 0} } {

    global windowPlaces

    global windowPlacesChanged
    set windowPlacesChanged 0

    #
    # Load the $appDefaults file first
    #
    # Look in ./ first, then $appLibLoc (if specified),
    #   then /usr/local/dvc/, then /usr/lib/X11/app-defaults,
    #   then /usr/openwin/lib/app-defaults, then issue a warning
    #
    if [file exists ./$appDefaults] {
	WindowPlacement_Load "./$appDefaults"
    } elseif [string match 0 appLibLoc] {
	if [file exists $appLibLoc/$appDefaults] {
	    WindowPlacement_Load "$appLibLoc/$appDefaults"
	}
    } elseif {[file exists /usr/local]
	&& [file exists /usr/local/dvc]
	&& [file exists /usr/local/dvc/$appDefaults]} {

	WindowPlacement_Load "/usr/local/dvc/$appDefaults"
    } elseif {[file exists /usr/lib/X11]
	&& [file exists /usr/lib/X11/app-defaults]
	&& [file exists /usr/lib/X11/app-defaults/$appDefaults]} {

	WindowPlacement_Load "/usr/lib/X11/app-defaults/$appDefaults"
    } elseif {[file exists /usr/openwin]
	&& [file exists /usr/openwin/lib]
	&& [file exists /usr/openwin/lib/app-defaults]
	&& [file exists /usr/openwin/lib/app-defaults/$appDefaults]} {

	WindowPlacement_Load "/usr/openwin/lib/app-defaults/$appDefaults"
    } else {
	# puts stderr "WindowPlacement_Init: Warning can't find application defaults \"$appDefaults\""
    }

    #
    # Now load the $userDefaults file
    #
    # Look in ./ first, then ~/  (don't issue a warning if we can't find them)
    #
    global windowUserDefaults

    if [file exists ./$userDefaults] {
	WindowPlacement_Load "./$userDefaults"
	set windowUserDefaults "./$userDefaults"
    } elseif [file exists ~/$userDefaults] {
	WindowPlacement_Load "~/$userDefaults"
	set windowUserDefaults "~/$userDefaults"
    } else {
	set windowUserDefaults "~/$userDefaults"
    }
}

proc WindowPlacement_Load { file_name {secondary_array 0}} {

    #
    # WindowPlacement_Load only deals with comment lines (those with
    # a '!' as the first character, or window names. No '*'s, appName's,
    # etc.
    #

    global windowPlaces
    global WindowsG

    if {($secondary_array != 0)} {
	global $secondary_array
    }

    if [catch { set file_id [open $file_name r] } err] {
	puts stderr "WindowPlacement_Load: Error opening file $file_name: $err"
    } else {
	while {[gets $file_id line] >= 0} {
	    set colon_loc [string first : $line]

	    if {($colon_loc == -1)
		|| ([string length $line] == 0)
		|| [string match "\!" [string index $line 0]]} {

		#
		# Skip this line as it either is a comment, or
		# we can't find the ':' to parse it.
		#
	    } else {
		set window_name [string range $line 0 \
		    [expr $colon_loc - 1]]

		set setting [string trim \
		    [string range $line [expr $colon_loc + 1] end]]

		set windowPlaces($window_name) $setting
		set WindowsG($window_name,geometry) $setting
		set WindowsG($window_name,geometry_set) 1

		if {($secondary_array != 0)} {
		    set $secondary_array($window_name) $setting
		}
	    }
	}

	close $file_id
    }
}

proc window.save_location { window_name } {
    #
    # This only saves the x and y location of the window
    #
    global windowPlaces

    set geom [wm geometry $window_name]
    set new_geom [string range $geom [string first "+" $geom] end]

    set changed 0
    if [
	catch {
	   if {([string compare $new_geom $windowPlaces($window_name)] != 0)} {
		set changed 1
	   }
	}
    ] { set changed 1 }

    if {$changed} {
	global windowPlacesChanged
	set windowPlacesChanged 1

	set windowPlaces($window_name) $new_geom
    }
}

proc window.save_geometry { window_name } {
    #
    # This saves width, height, x and y
    #
    global windowPlaces

    set new_geom [wm geometry $window_name]

    set changed 0
    if [
	catch {
	   if {([string compare $new_geom $windowPlaces($window_name)] != 0)} {
		set changed 1
	   }
	}
    ] {set changed 1}

    if {$changed} {
	global windowPlacesChanged
	set windowPlacesChanged 1

	set windowPlaces($window_name) $new_geom
    }
}

proc WindowPlacement_Save {} {
    global windowPlacesChanged

    if {($windowPlacesChanged != 0)} {
	global windowPlaces
	global windowUserDefaults

	if [catch { set file_id [open $windowUserDefaults w+] } err] {
	    puts stderr \
		"WindowPlacement_Save: Error opening file $windowUserDefaults: $err"
	} else {
	    foreach i [array names windowPlaces] {
		puts $file_id "$i:\t\t$windowPlaces($i)"
	    }
	}

	close $file_id
    }
}


#----------------------------------------------------------------------
# Procedure: window.set_geometry
# Arguments:
#     w: Window for which to set the geometry.
#----------------------------------------------------------------------
proc window.set_geometry {w} {
    global WindowsG

    set geom [wm geometry $w]
    set WindowsG($w,geometry) $geom
    if {! [info exists WindowsG($w,geometry_set)]} {
	wm geometry $w $geom
	set WindowsG($w,geometry_set) 1
    }
    wm withdraw $w
    wm deiconify $w
}

#----------------------------------------------------------------------
# Procedure: window.takeover
# Arguments:
#    w: Window pathname
#    name: Title of window
#----------------------------------------------------------------------
proc window.takeover {w name} {
    global WindowsG

    if {$w != "."} {
	if {[winfo exists $w] == 1} {
	    set geom [wm geometry $w]
	    set WindowsG($w,geometry) $geom
	    if {! [info exists WindowsG($w,geometry_set)]} {
		wm geometry $w $geom
		set WindowsG($w,geometry_set) 1
	    }
	    if [catch {wm raise $w}] {
		wm withdraw $w
	    }
	    wm deiconify $w

	    wm title $w $name
	    wm iconname $w $name
	    return -1;
        }   
	# Put default window positions for everything 
        toplevel $w
# 	set WindowsG($w,geometry) +250+250
#	set WindowsG($w,geometry_set) 1
	wm geometry $w +250+250

	# Unless the specific window sets what it wants to do on an
	# attempt to destroy the window, don't do anything.  Otherwise,
	# we may end up in a bad state.  Windows that are created should
	# override this default behavior.
	wm protocol $w WM_DELETE_WINDOW "DoNothing"

	set CurrentCallee null;
    }
    wm title $w $name
    wm iconname $w $name
    if {$w == "."} { set w "" }

    return $w
}

proc window.configure {w} {
    global WindowsG
 
    set exists 0
    catch {if {$WindowsG($w,exists) == 1} {set exists 1}}

    if {$exists == 0} {
	return
    }

    if [winfo ismapped $w] {
	set geom [wm geometry $w]
	catch {if {$WindowsG($w,geometry_location_only) == 1} {
	    set geom [string range $geom [string first "+" $geom] end]
	}}
	set WindowsG($w,geometry) $geom
	set WindowsG($w,geometry_set) 1

        window.save_location $w
    }
}

proc window.premap {w} {
    window.map $w 1
}

proc window.map {w {premap 0}} {
    global WindowsG
    set ret [catch {
	if {$WindowsG($w,exists) == 1} {
	    return
	}
    }]
    if {$ret == 2} {return}

    set WindowsG($w,exists) 1
    if [info exists WindowsG($w,geometry_set)] {
	set ret [catch {
	    if {$WindowsG($w,geometry_dont_set)} {
		return
	    }
	}]
	if {$ret == 2} {return}

	if {$premap == 0} {
	    after 0 wm geometry $w $WindowsG($w,geometry)
	} else {
	    wm geometry $w $WindowsG($w,geometry)
	}
    } else {
	set ret [catch {if {$WindowsG($w,geometry_dont_set)} { return }}]
	if {$ret == 2} {return}

	if {$premap == 0} {
	    after 0 wm geometry $w +250+250
	} else {
	    wm geometry $w +250+250
	}
    }
    window.configure $w
}

proc window.dont_set_geom {w {dont 1}} {
    global WindowsG
    if {$dont == 0} {
	catch {unset WindowsG($w,geometry_dont_set)}
    } else {
	set WindowsG($w,geometry_dont_set) 1
    }
}

proc window.set_location_only {w {loc_only 1}} {
    global WindowsG
    if {$loc_only == 1} {
	set WindowsG($w,geometry_location_only) 1
    } else {
	catch {unset WindowsG($w,geometry_location_only)}
    }
}

proc window.destroy {w} {
    global WindowsG
    set WindowsG($w,exists) 0
}






# Create the top level session manager for the video conference
#
# Arguments:
#    name -     String for name of window
#    w -	Name to use for top-level window.
#

set Directory(who) [list {Gordon Chaffee} {John Borezcy} {Brian Smith} {Radhika Malpani} {Andrew Swan}]

set UserPrefs(speedDialList) {{Larry Rowe} {Jimmy Carter} {John Ousterhout}}

proc dvc_dirSearch {who} {
    return {}
}
proc dvc_dirSetUid {uid who} {
}

proc dvc_dirNames {} {
    global Directory
    return $Directory(who)
}

proc listbox.selection {w} {
    set index [$w curselection]
    if { $index == "" } {
	return {}
    }
    return [$w get $index]
}


proc mkCall {name {w .calldbox} {vcpanelid -1}} {
    global UserPrefs
    global CallG


    # set w [ window.takeover $w $name ]
    # if { $w == "-1" } {
    #	#window exists -- its is deiconified and popped up
    #	return;
    #}
    toplevel $w
    wm title $w
    wm minsize $w 1 1


    # options frame -------------------------------------------
    set opts $w.optionframe
    frame $opts
    checkbutton $opts.audio -text "Audio" \
	-variable CallAttrsG(audio) -relief flat
    checkbutton $opts.video -text "Video" \
	-variable CallAttrsG(video) -relief flat
    pack $opts.audio $opts.video -side left -anchor w
    button $opts.vidtest -text "Check Local Camera" \
	-command livewin.checkopen
    pack $opts.vidtest -side right -anchor e

    # search frame -------------------------------------------
    frame $w.nameframe -borderwidth 2 -relief groove
    entry $w.nameframe.entry -relief sunken
    bind  $w.nameframe.entry <Return> \
	" SearchList \$CallG(activepage).listframe.list $w.nameframe.entry SEARCH $w.errmsg.msg"
    bind  $w.nameframe.entry <KeyRelease> \
	"+SearchList \$CallG(activepage).listframe.list $w.nameframe.entry SEARCH $w.errmsg.msg"
    bind  $w.nameframe.entry <Control-n> \
	"+SearchList \$CallG(activepage).listframe.list $w.nameframe.entry NEXT $w.errmsg.msg"
    bind  $w.nameframe.entry <Control-p> \
	"+SearchList \$CallG(activepage).listframe.list $w.nameframe.entry PREVIOUS $w.errmsg.msg"

    focus $w.nameframe.entry;


    button $w.nameframe.b1 -text "Search" -command \
      "SearchList \$CallG(activepage).listframe.list $w.nameframe.entry SEARCH $w.errmsg.msg"
    button $w.nameframe.b2 -text "Next" -command \
      "SearchList \$CallG(activepage).listframe.list $w.nameframe.entry NEXT $w.errmsg.msg"
    button $w.nameframe.b3 -text "Prev" -command \
      "SearchList \$CallG(activepage).listframe.list $w.nameframe.entry PREVIOUS $w.errmsg.msg"

    pack $w.nameframe.b1 $w.nameframe.b2 $w.nameframe.b3 -side left \
	-fill x -padx 2 -pady 5
    pack $w.nameframe.entry -side left -expand yes -fill x -padx 2 -pady 5

    #error message frame -----------------------------------------------------
    frame $w.errmsg -bd 1
    label $w.errmsg.msg -text "     "
    pack $w.errmsg.msg -side left 


#list frame for people ----------------------------------

    set book [tixNotebook $w.book]

    set pages {{dir Directory} {speed "Speed Dial"} {active Active}}
    foreach pair $pages {
	set type [lindex $pair 0]
	set tab_label [lindex $pair 1]
	set page $book.$type
	frame $page

	frame $page.listframe -bd 10

	#button frame ----------------------------------
	frame $page.buttonframe -borderwidth 10
	button $page.buttonframe.call -text "Call" \
	    -command "MakeCall \[listbox.selection $page.listframe.list\] $w.errmsg.msg $vcpanelid"

	frame $page.buttonframe.default -relief sunken -bd 1
	raise $page.buttonframe.call $page.buttonframe.default

	button $page.buttonframe.info -text "Info" \
	    -command "ShowInfo \[listbox.selection $page.listframe.list\] $w.errmsg.msg $page.buttonframe.info .info"
	button $page.buttonframe.redial -text "Redial" \
	    -command "redialCmd $w.errmsg.msg"
	button $page.buttonframe.glance -text "Glance" \
	    -command "Glance \[listbox.selection $page.listframe.list\] $w.errmsg.msg"
	button $page.buttonframe.update -text "Update" \
	    -command "UpdateList.$type $page.listframe.list"

	if {$type == "speed"} {
	    button $page.buttonframe.speed -text " Delete  " \
		-command "speeddial.delete \[listbox.selection $page.listframe.list\] $w.errmsg.msg"
	} else {
	    button $page.buttonframe.speed -text "SpeedAdd" \
		-command "speeddial.add \[listbox.selection $page.listframe.list\] $w.errmsg.msg"
	}
	wm protocol $w WM_DELETE_WINDOW "mkCallCancel $w"
	button $page.buttonframe.cancel -text "Cancel" \
	    -command "mkCallCancel $w"

	pack $page.buttonframe.call -in $page.buttonframe.default -pady 4 -padx 4 \
	    -fill both -expand yes
	pack $page.buttonframe.default $page.buttonframe.glance \
	    $page.buttonframe.redial $page.buttonframe.info \
	    $page.buttonframe.update\
	    $page.buttonframe.speed $page.buttonframe.cancel \
	    -fill both -pady 5 -expand yes

	set deactivate_list [list $page.buttonframe.call $page.buttonframe.glance $page.buttonframe.info $page.buttonframe.speed]

	#people list frame ------------------------------------------------------
	scrollbar $page.listframe.scroll -relief sunken \
	    -command "$page.listframe.list yview"
	listbox $page.listframe.list -yscroll "$page.listframe.scroll set" \
	    -relief sunken -setgrid 1 -geometry 40x10 -exportselection no

	set CallG($page.listframe.list,deactivate_list) $deactivate_list
	set CallG($page.listframe.list,length) 1

	bind $page.listframe.list <Double-1> \
	    "button.press $page.buttonframe.call"
	bind $page.listframe.list <3> "tkext_listboxSelect %W %y"
	bind $page.listframe.list <B3-Motion> \
	    "%W select from \[%W nearest %y\]"
	bind $page.listframe.list <Double-3> \
	    "button.press $page.buttonframe.glance"

	tk_listboxSingleSelect $page.listframe.list

	pack $page.listframe.scroll -side right -fill y
	pack $page.listframe.list -side left -expand yes -fill both

	UpdateList.$type $page.listframe.list
	if {$type == "speed"} {
	    set CallG(speedlbox) $page.listframe.list
	}

	pack $page.listframe -side left -fill both -expand yes
	pack $page.buttonframe -side right 
    }	
    window.premap $w

    update idletasks

    set pages {{active Active} {speed "Speed Dial"} {dir Directory}}
    foreach pair $pages {
	set type [lindex $pair 0]
	set tab_label [lindex $pair 1]
	set page $book.$type
	$w.book addpage $page $tab_label \
	    "displayMsg $w.errmsg.msg {}; set CallG(activepage) $page;"
    }
    pack $w.book -expand yes -fill both

    pack $w.nameframe -side bottom -fill x 
    pack $opts -side bottom -anchor nw -fill x
    pack $w.errmsg -side bottom -fill x 

    # If we are adding a new person, then make the dbox modal
    if {$vcpanelid != -1} {
	tkwait visibility $w
	while {[catch {grab $w}]} {}
	tkwait window $w
    }

}

#----------------------------------------------------------------------
# Procedure: MakeCall
# Arguments:
#   who: Name of person to call.  May be a real name or a username
#   msgwindow: Window to put failed messages into
#   vcpanelid: -1 if this is an initial call, the video phone call id if it
#          is an addition to an existing call.
# Return: -1 if call completely failed, 0 if it initialization is occurring
#----------------------------------------------------------------------
proc MakeCall {who msgwindow vcpanelid} {
    global MyInfoG
    global UserPrefs

    displayMsg $msgwindow ""
    if {$who == ""} {
	displayMsg $msgwindow "Please select somebody"
	return
    }
    set topwin [winfo toplevel $msgwindow]
    set uid [dvc_dirSearch $who]
    if {$uid == -1} {
	displayMsg $msgwindow "The directory does not list $who"
	return -1
    }

    # If user tries to call himself at this local process, don't allow it
    if {$uid == $MyInfoG(uid)} {
	displayMsg $msgwindow "Calling yourself is prohibited!"
	return -1
    }
    
    set UserPrefs(redialperson) $who

    if {$uid == "0_0_0"} {
	displayMsg $msgwindow "$who is not running a DVC Session Manager"
	return -1
    }

    if {$vcpanelid != -1} {
	global $vcpanelid
	set group [set ${vcpanelid}(callgroup)]
	global $group
	# Check if the person we want to add is already in the group
	set index [lsearch [set ${group}(who)] "$uid *"]
	if {$index != -1} {
	    displayMsg $msgwindow "$who is already a party in this group."
	    return -1
	}
	set callgroup [dvc_callInitiate $uid call $group]
    } else {
	set callgroup [dvc_callInitiate $uid call]
    }

    # We aren't through yet.  The other party can accept or deny our request
    if {$callgroup != "0_0_0"} {

	dvc_dirSetUidName $uid $who

	# Now place a dialog box that says we are ringing the party.
	mkRinging $callgroup $who $uid
	
    } else {
	# The person's current address is invalid.
	# XXX: In the future, here is where to put other messages of 
	# communicating with someone (using talk-like messages).  For
	# now, put up an old-fashioned dialog box
        displayMsg $msgwindow "$who is not running a DVC Session Manager"
	return -1
    }
    if {$vcpanelid != -1} {
	mkCallCancel $topwin
    }

    return 0
}

#----------------------------------------------------------------------
# Procedure: mkRinging
# Arguments:
#     w : "Ringing $who" window
#     pg: Process group to destroy
# Purpose:
#     If a call initiator wishes to hangup before a call is connected,
#     the hangup goes through here
#----------------------------------------------------------------------
proc mkRinging {pg who {uid null}} {
    global WindowsG

    set w ".${pg}_ringing"
    set name "Ringing $who"
    # Unless we are taking over the top level, check if the window exists
    if [winfo exists $w] {
	destroy $w
    }
    toplevel $w
    set WindowsG($w,geometry) +250+250
    set WindowsG($w,geometry_set) 1
    wm geometry $w $WindowsG($w,geometry)

    wm title $w $name
    wm iconname $w $name
    if {$w == "."} { set w "" }

#   wm minsize $w 1 1

    frame $w.f -bd 10
    label $w.f.l1 -text "Waiting for"
    label $w.f.l2 -text "$who"
    label $w.f.l3 -text "to answer"
    label $w.f.l4 -text " "
    pack $w.f.l1 $w.f.l2 $w.f.l3 $w.f.l4 -side top

    wm protocol $w WM_DELETE_WINDOW "callCancel $w $pg $uid"
    button $w.f.b -text "Hangup" -command "callCancel $w $pg $uid"
    pack $w.f.b -side bottom
    pack $w.f

    window.premap $w
}

#----------------------------------------------------------------------
# Procedure: callAccepted
# Arguments:
#     pg: Process group
#     uid: id of person who accepted
# Purpose:
#     This procedure is called from dvc_callAccept.  It takes care of
#     removing windows that were created during the ringing stage.
# Return: 0 if OK, -1 if there is a problem
#----------------------------------------------------------------------
proc callAccepted {pg {uid null}} {
    if [catch {destroy ".${pg}_ringing"}] {
	return -1
    }
}

#----------------------------------------------------------------------
# Procedure: callRejected
# Arguments:
#     pg: Process group
#     uid: id of person who rejected the call.  (In a group, this is not set)
# Purpose:
#     This procedure is called from dvc_callAccept.  It takes care of
#     removing windows that were created during the ringing stage.
# Notes:
#     Takes over the "Ringing $who" dialog and changes it to hold
#     a rejection message
#----------------------------------------------------------------------
proc callRejected {pg {uid null}} {
    set w ".${pg}_ringing"

    if [winfo exists $w] {
	if [catch {wm raise $w}] {
	    wm withdraw $w
	    wm deiconify $w
	}
	# Take over the window that had the Waiting for 
	set who [lindex [$w.f.l2 configure -text] 4]
	$w.f.l1 configure -text "$who"
	$w.f.l2 configure -text "is not available"
	$w.f.l3 configure -text "right now."
#	$w.f.l4 configure -text " "
	pack $w.f.l1 $w.f.l2 $w.f.l3 $w.f.l4 -side top

	$w.f.b configure -text "OK" -command "destroy $w"
    }
}

#----------------------------------------------------------------------
# Procedure: callCancel
# Arguments:
#     w : "Ringing $who" window
#     pg: Process group to destroy
# Purpose:
#     If a call initiator wishes to hangup before a call is connected,
#     the hangup goes through here
#----------------------------------------------------------------------
proc callCancel {w pg uid} {
    destroy $w
    dvc_callCancel $pg $uid
}

#----------------------------------------------------------------------
# Procedure: callTerminate
# Arguments:
#     w : "Ringing $who" window
#     pg: Process group to destroy
# Purpose:
#     If a call initiator wishes to hangup before a call is connected,
#     the hangup goes through here
#----------------------------------------------------------------------
#proc callTerminate {pg} {
#   dismissVPCPanelsOfGroup $pg
#}

proc mkCallCancel {win} {
    destroy $win
}

proc UpdateList {l namelist} {
    global CallG

    $l delete 0 end
    foreach arg $namelist {
	$l insert end $arg
    }
    set len [llength $namelist]
    if {$CallG($l,length) > 0 && $len == 0} {
	set buttonlist $CallG($l,deactivate_list)
	foreach button $buttonlist {
	    $button configure -state disabled
	}
    } elseif {$CallG($l,length) == 0 && $len > 0} {
	set buttonlist $CallG($l,deactivate_list)
	foreach button $buttonlist {
	    $button configure -state normal
	}
    }
    set CallG($l,length) $len
    $l select from 0
}
    
# Keep the list of displayed names uptodate
proc UpdateList.dir {l} {
    UpdateList $l [dvc_dirNames]
}

proc UpdateList.speed {l} {
    global UserPrefs
    UpdateList $l $UserPrefs(speedDialList)
}

proc UpdateList.active {l} {
    set allnames [dvc_dirNames]
    set activelist {}
    foreach arg $allnames {
        set uid [dvc_dirSearch $arg]
        if {$uid != "0_0_0"} {
	    lappend activelist $arg
        }
    }
    UpdateList $l $activelist
}

proc mkCallRestrict {name {w .restrictdbox}} {
    # Unless we are taking over the top level, check if the window exists
    set w [ window.takeover $w $name ]
    if { $w == "-1" } {
	#window exists -- its is deiconified and popped up
	return;
    }
    wm minsize $w 1 1

    frame $w.listframe -borderwidth 10
    label $w.listframe.label -text "People"
    pack $w.listframe.label -side top

    frame $w.buttonframe -borderwidth 10
    button $w.buttonframe.restrict -text "Don't Answer" \
	-command "RestrictCaller \[list \[$w.nameframe.entry get\]\]"

    frame $w.buttonframe.default -relief sunken -bd 1
    raise $w.buttonframe.restrict $w.buttonframe.default

    button $w.buttonframe.search -text "Search ..."
    wm protocol $w WM_DELETE_WINDOW "mkCallCancel $w"
    button $w.buttonframe.cancel -text "Cancel" -command "mkCallCancel $w"
    pack $w.buttonframe.restrict -in $w.buttonframe.default -pady 2 -padx 2 -fill x
    pack $w.buttonframe.default $w.buttonframe.search \
         $w.buttonframe.cancel -fill x -pady 5

    frame $w.nameframe -borderwidth 10
    label $w.nameframe.label -text "Name"
    entry $w.nameframe.entry -relief sunken

    pack $w.nameframe.label -side left -fill x
    pack $w.nameframe.entry -fill x -padx 5

    pack $w.nameframe -side bottom -fill x
    pack $w.buttonframe -side right
    pack $w.listframe -side left -fill both -expand yes

    scrollbar $w.listframe.scroll -relief sunken \
	-command "$w.listframe.list yview"
    listbox $w.listframe.list -yscroll "$w.listframe.scroll set" \
        -relief sunken -setgrid 1

    bind $w.listframe.list <ButtonRelease-1> \
        "$w.nameframe.entry delete 0 end; \
         catch \{$w.nameframe.entry insert 0 \[listbox.selection $page.listframe.list\] \}"

    tk_listboxSingleSelect $w.listframe.list

    pack $w.listframe.scroll -side right -fill y
    pack $w.listframe.list -side left -expand yes -fill both

    window.premap $w
}


#--------------------------------------------------------------------------
#
# proc SearchList
# Arguments : listboxname entryname mode ( mode = SEARCH/NEXT)
# searches the listbox for entries matching the given pattern in the entry widget
# mode SEARCH selects the first entry to match the pattern
# mode NEXT will select the next entry to match the pattern till they are no more
# new entries.
# pattern matching is of the glob style
#
#--------------------------------------------------------------------------

proc SearchList { lbox entry mode msgwindow } {
    global CallG
    
    displayMsg $msgwindow ""
    if { $mode == "SEARCH" } {
	set CallG(pattern) [string tolower [$entry get] ]*
	set CallG($lbox,nxtIndex) 0
	set pistIndex 0
    }
    if { $mode != "PREVIOUS" } {
	if { ! [info exists CallG($lbox,prvIndex)] } {
	    set CallG(pattern) [string tolower [$entry get] ]*
	    set CallG($lbox,prvIndex) 0
	    set CallG($lbox,nxtIndex) 0
	}
	set listsize [ $lbox size];
	while { $CallG($lbox,nxtIndex) != $listsize } {
	    set listval [string tolower [$lbox get $CallG($lbox,nxtIndex)]];
	    if { [string match $CallG(pattern) $listval ] == 1 } {
		#pattern matched
		$lbox select from $CallG($lbox,nxtIndex);
		$lbox yview $CallG($lbox,nxtIndex);
		set CallG($lbox,prvIndex) [expr $CallG($lbox,nxtIndex)-1];
		set CallG($lbox,nxtIndex) [expr $CallG($lbox,nxtIndex)+1];
		return;
	    }
	    set CallG($lbox,nxtIndex) [expr $CallG($lbox,nxtIndex)+1];
	}
	displayMsg $msgwindow "No matching entry "
    } else {
	if { ! [info exists CallG($lbox,prvIndex)] } {
	    set CallG(pattern) [string tolower [$entry get] ]*
	    set CallG($lbox,prvIndex) 0
	    set CallG($lbox,nxtIndex) 0
	}
	while { $CallG($lbox,prvIndex) >= 0 } {
	    set listval [string tolower [ $lbox get $CallG($lbox,prvIndex) ] ];
	    if { [string match $CallG(pattern) $listval ] == 1 } {
		#pattern matched
		$lbox select from $CallG($lbox,prvIndex);
		$lbox yview $CallG($lbox,prvIndex);
		set CallG($lbox,nxtIndex) [expr $CallG($lbox,prvIndex)+1];
		set CallG($lbox,prvIndex) [expr $CallG($lbox,prvIndex)-1];
		return;
	    }
	    set CallG($lbox,prvIndex) [expr $CallG($lbox,prvIndex)-1];
	}
	displayMsg $msgwindow "No matching entry "
    }

}

#-------------------------------------------------------------------------
#
#proc redialCmd
# Arguments :
# name of person last called, window to display message
#
#-------------------------------------------------------------------------

proc redialCmd { msgwindow } {
    global UserPrefs
    if {! [info exists UserPrefs(redialperson)] } {
	displayMsg $msgwindow "Sorry - no one has been dialed"
	return;
    }
    displayMsg $msgwindow "Calling $UserPrefs(redialperson) ......"
    MakeCall $UserPrefs(redialperson) $msgwindow -1
}

#-------------------------------------------------------------------------
#
# proc speeddial.add
#
#-------------------------------------------------------------------------

proc speeddial.add {name msgwindow} {
    global UserPrefs
    global CallG

    displayMsg $msgwindow ""
    if {$name == ""} {
	displayMsg $msgwindow "Please select somebody"
	return
    }
	
    #check entry not already in list
    set index [ lsearch $UserPrefs(speedDialList) $name* ]
    displayMsg $msgwindow ""
    if { $index != "-1" } {
	displayMsg $msgwindow "Person selected is already in speed dial list"
	return;
    }

    set lbox $CallG(speedlbox)
    set listsize [$lbox size];
    $lbox insert end $name
    lappend UserPrefs(speedDialList) $name;
    if {$listsize == 0} {
	UpdateList $lbox $UserPrefs(speedDialList)
    }
    set count 0
}



#-------------------------------------------------------------------------
#
# proc speeddial.delete
#
#-------------------------------------------------------------------------

proc speeddial.delete {name msgwindow} {
    global UserPrefs
    global CallG

    displayMsg $msgwindow ""
    if {$name == ""} {
	displayMsg $msgwindow "Please select somebody"
	return
    }
	
    set index [ lsearch $UserPrefs(speedDialList) $name* ]
    if { $index == "-1" } {
	displayMsg $msgwindow "Cannot delete entry - not in speed dial list "
	return;
    }

    set UserPrefs(speedDialList) [lreplace $UserPrefs(speedDialList) $index $index] ;

    set lbox $CallG(speedlbox)

    set listsize [$lbox size];
    set currindex [$lbox curselection]
    for {set i 0} {$i < $listsize} {incr i} {
	set listval [$lbox get $i]
	if {$listval == $name} {
	    $lbox delete $i $i
	    if {$currindex == $i && $listsize > 1} {
		incr listsize -1
		if {$i == $listsize} {
		    set i [expr $i-1]
		}
		$lbox select from $i
	    } else {
		UpdateList $lbox {}
	    }
	    return;
	}
    }
    displayMsg $msgwindow "No matching entry "
}

#------------------------------------------------------------------------
#
# proc ShowInfo
#
# Notes: Changed to no longer gray out the infobutton.  Reason: If the
#        info dialog box gets hidden behind another window, the user
#        should be able to press the "Info" button to get the window to
#	 reappear -- GCC
#------------------------------------------------------------------------

proc ShowInfo {name msgwindow infobutton w} {
    global PInfoG;
    global CallG

    if {$name == ""} {
	displayMsg $msgwindow "Please select somebody"
	return;
    }
    set person [NS_call getUserInfo $name]
    foreach i $person {
	if { [lindex $i 0] != "uid" } {
	    set PInfoG([lindex $i 0]) [lindex $i 1];
	}
    }
#   $infobutton config -state disabled

    # Unless we are taking over the top level, check if the window exists
    if {$w != "."} {
	if {[winfo exists $w] == 1} {
	    destroy $w
        }
        toplevel $w
    }
    wm title $w $name
    wm iconname $w $name
    if {$w == "."} { set w "" }

#   wm minsize $w 1 1

    frame $w.frame -bd 10
    frame $w.nameframe
    label $w.nameframe.label -text "Name" -anchor e -width 10
    label $w.nameframe.entry -text $PInfoG(realname) \
	-font -Adobe-Helvetica-Medium-R-Normal--*-120-* 

    frame $w.userframe
    label $w.userframe.label -text "Login Name" -anchor e -width 10
    label $w.userframe.entry -text $PInfoG(username) \
	-font -Adobe-Helvetica-Medium-R-Normal--*-120-*  -anchor w

    frame $w.hostframe
    label $w.hostframe.label -text "Host Name" -anchor e -width 10
    label $w.hostframe.entry -text $PInfoG(hostname) \
	-font -Adobe-Helvetica-Medium-R-Normal--*-120-*  -anchor w

    pack $w.nameframe.label -side left 
    pack $w.nameframe.entry  -padx 5 -side left 
    pack $w.nameframe -in $w.frame -side top -fill x -pady 2

    pack $w.userframe.label -side left -fill x 
    pack $w.userframe.entry  -fill x  -padx 5
    pack $w.userframe -in $w.frame -side top -fill x -pady 2

    pack $w.hostframe.label -side left -fill x
    pack $w.hostframe.entry  -fill x  -padx 5
    pack $w.hostframe -in $w.frame -side top -fill x -pady 2

    pack $w.frame -fill x

#access frame ---------------------------
    frame $w.radioframe -borderwidth 10
    label $w.radioframe.label -text "Access"
    frame $w.radioframe.frame -relief groove -bd 2

    set access [ list anybody confirm restricted ]
    set accesstext [list "Anybody Can Call" "Confirm Calls" "Restricted"]
    set index 0
    while { $index < 3 } {
	set name [lindex $access $index]
	set textval [lindex $accesstext $index]
    	radiobutton $w.radioframe.frame.$name -text $textval -state disabled \
		-variable PInfoG(access) -relief flat -value $name -anchor w
	if { $name == $PInfoG(access) } {
		$w.radioframe.frame.$name config -state normal
	}
    	pack $w.radioframe.frame.$name \
	-side top -pady 2  -fill x -anchor w 
	set index [ expr $index+1 ]
    }
    pack $w.radioframe.label $w.radioframe.frame -anchor nw

# button frame --------------------------
    frame $w.buttonframe -borderwidth 10
    wm protocol $w WM_DELETE_WINDOW "destroy $w; \
		  $infobutton config -state normal";
    button $w.buttonframe.ok -text "OK" \
	-command "destroy $w; 
		  $infobutton config -state normal";

    frame $w.buttonframe.default -relief sunken -bd 1
    raise $w.buttonframe.ok $w.buttonframe.default

    pack $w.buttonframe.ok -in $w.buttonframe.default \
        -pady 4 -padx 4 -fill x -ipadx 2
    pack $w.buttonframe.default -pady 10 -fill x -ipadx 2 -side bottom
    pack $w.buttonframe -side right -fill y -anchor center

    pack $w.radioframe -side left -anchor nw -expand yes -fill x

# state -----------------------------------------
    frame $w.stateframe -borderwidth 10
    label $w.stateframe.label -text "State"
    frame $w.stateframe.frame -relief groove -bd 2

    set avail [ list avail notavail busy ]
    set availtext [list "Available " "Not Available " "Busy"]
    set index 0
    while { $index < 3 } {
	set name [lindex $avail $index]
	set textval [lindex $availtext $index]
    	radiobutton $w.stateframe.frame.$name -text $textval -state disabled \
		-variable PInfoG(avail) -relief flat -value $name -anchor w
	if { $name == $PInfoG(avail) } {
		$w.stateframe.frame.$name config -state normal
	}
    	pack $w.stateframe.frame.$name -side top -pady 2  -fill x -anchor w 
	set index [ expr $index+1 ]
    }

	pack $w.stateframe.label $w.stateframe.frame -anchor nw
	pack $w.stateframe -side left -anchor nw -expand yes -fill x

#glance ok ------------------------------

    frame $w.glanceframe -borderwidth 10
    label $w.glanceframe.label -text "Glance Ok? "
    frame $w.glanceframe.frame -relief groove -bd 2

    set glance_ok [ list true false ]
    set glance_oktext [list "Yes " "No " ]
    set index 0
    while { $index < 2 } {
	set name [lindex $glance_ok $index]
	set textval [lindex $glance_oktext $index]
    	radiobutton $w.glanceframe.frame.$name -text $textval -state disabled \
		-variable PInfoG(glance_ok) -relief flat -value $name -anchor w
	if { $name == $PInfoG(glance_ok) } {
		$w.glanceframe.frame.$name config -state normal
	}
    	pack $w.glanceframe.frame.$name -side top -pady 2  -fill x -anchor w 
	set index [ expr $index+1 ]
    }

    pack $w.glanceframe.label $w.glanceframe.frame -anchor nw
    pack $w.glanceframe -side left -anchor nw -expand yes -fill x

    window.premap $w
}


proc displayMsg { msgwindow msg } {
    global MessageG

    if {! [info exists MessageG($msgwindow,id)]} {
	set MessageG($msgwindow,id) 0
    }
    incr MessageG($msgwindow,id)
    $msgwindow config -text $msg;

    if {$msg != ""} {
	# Only reset the message window if the window still exists and the
	# same message is in the window that we put there.
	after 10000 \
	    "if { \[winfo exists $msgwindow\] &&
	          \$MessageG($msgwindow,id) == $MessageG($msgwindow,id)} {
	        $msgwindow config -text \"\"
             }"  
    }
}


set ConfirmG(id) 0
#----------------------------------------------------------------------
# Procedure: mkConfirmDialog
# Arguments: w -- pathname for window
#	   : name -- name of dialog box
#          : type -- which bitmap type to use
#	   : width -- what width to use
#	   : modal -- Is it a (locally) modal dialog?
#	   : message -- Message to display
# Returns: 0 to refuse to accept a call, 1 to accept it
#----------------------------------------------------------------------
proc mkConfirmDialog {group callerid fromwho} {
    global ConfirmG

    incr ConfirmG(id)
    set w ".confirm_${callerid}"
    set name "Confirm Call"
    set message "Do you wish to accept a call from $fromwho?"
    set ConfirmG(var$ConfirmG(id)) 0
    set type question
    set width 3i
    set modal 1

    # XXX: We may wish to periodically pop modal windows to the top just
    # to be sure that they were seen

    # Unless we are taking over the top level, check if the window exists
    if {$w != "."} {
	if {[winfo exists $w] == 1} {
	    # NOTE: Destroy the window first because of the tkwait at the
	    # bottom
	    destroy $w
        }
#       toplevel $w
    }

#    wm title $w $name
#    wm iconname $w $name
#    wm group $w .

    window.takeover $w $name

    frame $w.f1
    frame $w.f1.finfo -borderwidth 10
    label $w.f1.finfo.l -bitmap $type
    pack $w.f1.finfo.l -fill y -expand yes
    pack $w.f1.finfo -side left -anchor w -expand yes -fill y

    message $w.f1.m -width $width -text $message

    pack $w.f1.m -side top -pady 4

    frame $w.f1.bframe
    button $w.f1.bframe.yes -text "Yes" \
        -command "set ConfirmG(var$ConfirmG(id)) 1; destroy $w;"

    frame $w.f1.bframe.default -relief sunken -bd 1
    raise $w.f1.bframe.yes $w.f1.bframe.default

    wm protocol $w WM_DELETE_WINDOW "set ConfirmG(var$ConfirmG(id)) 0; destroy $w"
    button $w.f1.bframe.no -text "No" -command "set ConfirmG(var$ConfirmG(id)) 0; destroy $w;"
    pack $w.f1.bframe.yes -in $w.f1.bframe.default \
        -pady 4 -padx 4 -fill x -ipadx 2
    pack $w.f1.bframe.default $w.f1.bframe.no -ipadx 2 -padx 4 -pady 2 -side left
    pack $w.f1.bframe -side bottom
    pack $w.f1 -side right -fill y -anchor center

    pack $w.f1

    if {$modal} {
	tkwait visibility $w
	while {[catch {grab $w}]} {}
	tkwait window $w
    }
    return $ConfirmG(var$ConfirmG(id))
}

#----------------------------------------------------------------------
# Procedure: confirmCancel
# Arguments:
#   pg:  Group
#   uid: Who
#----------------------------------------------------------------------
proc confirmCancel {group uid} {

    set localpg ${group}_local
    global $localpg

    if [catch {set ${localpg}(created)}] {
	set created 0
    } else {
	set created 1
    }

    dvc_callTerminate $group $created

    set w ".confirm_${uid}"
    if [winfo exists $w] {
	destroy $w
    }
}

#----------------------------------------------------------------------
# Procedure: Glance
# Arguments:
#    
#----------------------------------------------------------------------
proc Glance {who msgwindow} {
    global MyInfoG

    displayMsg $msgwindow ""
    if {$who == ""} {
	displayMsg $msgwindow "Please select somebody"
	return
    }
    set uid [dvc_dirSearch $who]
    if {$uid == -1} {
	displayMsg $msgwindow "The directory does not list $who"
	return -1
    }
	
    if {$uid == "0_0_0"} {
	displayMsg $msgwindow "$who is not running a DVC Session Manager"
	return -1
    }

    # If user tries to glance at himself in this local process, don't allow it
    if {$uid == $MyInfoG(uid)} {
	displayMsg $msgwindow "Look in the mirror!"
	return -1
    }

    if {[dvc_glance.1 $uid] == 1} {
	set glanceInfo [GlancePanel $who $msgwindow]
	set glanceWinId [lindex $glanceInfo 1]
	set glanceWin [lindex $glanceInfo 0]
	set ret [dvc_glance.2 $uid $glanceWinId]
	if {$ret != 0} {
	    $glanceWin.menu.dismiss configure \
		-command "dvc_glanceStopReceive $ret; if \[winfo exists $glanceWin\] \{destroy $glanceWin\}"
	}
    } else {
	displayMsg $msgwindow "$who does not allow glances."
    }
}

proc GlancePanel {name msgwin {w .glancewin}} {
    if [winfo exists $w] {
	button.activate $w.menu.dismiss
    }
    set x [window.takeover $w $name]
    wm protocol $w WM_DELETE_WINDOW "button.press $w.menu.dismiss"

    frame $w.f -bd 10
    if [winfo exists $w.f.c] {
	destroy $w.f.c
    }
    canvas $w.f.c -width 320 -height 240 -relief ridge -bd 2
    pack $w.f.c -side top -expand yes -fill both
    pack $w.f
    frame $w.menu
    button $w.menu.dismiss -text "Dismiss" -command "destroy $w"
    button $w.menu.call -text "Call" -command \
      "MakeCall [list $name] $msgwin -1; button.press $w.menu.dismiss"
    pack $w.menu.call $w.menu.dismiss -padx .5i -ipadx 3 -pady 10 -side left
    pack $w.menu -side bottom
    window.premap $w
    update idletasks
    return [list $w [winfo id $w.f.c]]
}

proc tixInt_ApplyOptions {w args} {
    upvar #0 $w data
    upvar #0 $data(class) classRec

    set len  [llength $args]
    set len2 [expr {$len - 2}]
    set i    0

    if {$len == 1} {
	return [eval tixInt_QueryOption $w $args]
    }

    if {[info proc $data(class)::config_begin] != ""} {
	$data(class)::config_begin $w
    }

    while {$i <= $len2} {
	set option [lindex $args $i]
	incr i
	set arg [lindex $args $i]
	incr i

	if {[lsearch $classRec(options) $option] != "-1"} {
	    if {[lindex $classRec($option) 0] == "="} {
		set option [lindex $classRec($option) 1]
	    }
	    $data(class)::config$option $w $arg
	    set data($option) $arg
	} else {
	    if [catch {$data(rootCmd) config $option $arg} err_msg] {
		if {[string range $err_msg 0 13] == "unknown option"} {
		    error "unknown option $option. Should be: \
                           [tixInt_ListOptions $w]"
		} else {
		    error $err_msg
		}
	    }
	}
    }

    if {[info proc $data(class)::config_apply] != ""} {
	$data(class)::config_apply $w
    }

    if {$i != $len} {
	error "value for \"$option\" missing"
    }
}

wm withdraw .
# Start
mkCall "Make Call"

