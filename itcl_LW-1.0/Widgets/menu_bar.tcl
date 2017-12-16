# -*-TCL-*-  TCL mode for Emacs
#
# $Id: menu_bar.tcl,v 1.1.5.1 1994/06/05 17:33:12 mark Exp $
#
# tcl_file: menu_bar.tcl
#--------------------------------------------------------------
# Description: Common Menu support
#	Permits defining of menu panes using a simple format,
#	and then creating menu bars and/or attaching the menu panes
#	to widgets as a popup menu
#
# PROCEDURES:
#	mm_Menu {name entryList} ....... defines menu pane
#	mm_MenuBar {name paneList} ..... create a menu bar from pre-defined panes
#	mm_MenuAttach {w pane} ......... attach a menu pane to a widget $w (<B3>)
#
# entryList is a list of items, each of them is of format {type options...}
#	where type is one of the : title label menu or tk "menu" items
#	      options is tk menu items options
#
# $mm_menu(Menu) in entryList is replaced with the menu widget name
# $mm_menu(Item) in entryList is replaced with the current item number
#
# paneList is a list of items, each of them is of format 
#	{label number pane [side]} 
#
# There is an EXAMPLE at the end of the file. To run it type
#
#        ( setenv MM_MENU_EXAMPLE ; wish -f menu_bar.tcl ) 
#
#--------------------------------------------------------------
# Author        : Mark Sterin  ($Locker:  $)
# Creation date : Mar 3, 1994
#
# Last check-in date: $Date: 1994/06/05 17:33:12 $
#
# (c) Lannet Ltd.  1994
#--------------------------------------------------------------
#

#--------------------------------------------------------------------
# PROCEDURE:  mm_Menu - define menu pane
#--------------------------------------------------------------------
proc mm_Menu {name deflist} {
    global _mm_pd_Menu
    set name [string tolower $name]
    set _mm_pd_Menu($name) $deflist
}

    
#--------------------------------------------------------------------
# PROCEDURE:  mm_MenuBar - create menu bar
#--------------------------------------------------------------------
proc mm_MenuBar {bar_name deflist } {
    global _mm_pd_Menu
    # check bar_name doesn't exists
    if [info exists $bar_name] { 
	error "$bar_name already exists"
    }
    set bar_name [string tolower $bar_name]
    frame $bar_name -class MenuBar -borderwidth 2 -relief groove
    pack $bar_name -fill x -side top -anc nw

    set num 0
    foreach i $deflist {
	set text [lindex $i 0]
	set und  [lindex $i 1]
	set pane_name [string tolower [lindex $i 2]]
	set side [lindex $i 3]
	set btn_name  $bar_name.b_$pane_name
	set menu_name $btn_name.$pane_name

	if { "x$side" == "x" } { set side left }

	menubutton $btn_name \
	    -text "$text" \
	    -menu $menu_name \
	    -underline $und
	pack $btn_name -side $side -padx 1 -ipadx 1

	_mm_MenuMakePane $menu_name $pane_name

	lappend menu_bar_list $btn_name
    }

    #
    # Should be the right parent here !!! - BUG
    #
    bind [winfo toplevel $bar_name] <Any-FocusIn> "
	if {(\"%d\" == \"NotifyVirtual\") && (\"%m\" == \"NotifyNormal\")} \"
	    focus $bar_name
	\"
    "
    bind $bar_name <Destroy> "bind [winfo toplevel $bar_name] <Any-FocusIn> {}"

    eval tk_menuBar $bar_name $menu_bar_list
}



#--------------------------------------------------------------------
# PROCEDURE:  mm_MenuAttach - attach a menu bar to a widget
#--------------------------------------------------------------------
# Attaches menu to name
proc mm_MenuAttach {name pane_name} {
    set pane_name [string tolower $pane_name]
    set menu .menuPopup_$pane_name

    _mm_MenuMakePane $menu $pane_name 0

    bind $name <3> "_mm_MenuPost $menu %X %Y"
}

#--------------------------------------------------------------------
# PROCEDURE:  mm_MenuButton - create a menu button and attach menu bar
#--------------------------------------------------------------------
# Attaches menu to name
proc mm_MenuButton {btn_name pane_name args} {
    set pane_name [string tolower $pane_name]
    set menu_name $btn_name.menuPopup

    eval menubutton $btn_name \
	-menu $menu_name \
	$args

    _mm_MenuMakePane $menu_name $pane_name 0

}

#--------------------------------------------------------------------
# PROCEDURE:  _mm_MenuMakePane - create a menu pane
#--------------------------------------------------------------------
proc _mm_MenuMakePane { menu pane { in_menu_bar 1 } } {
    if {[winfo exists $menu]} return

    global _mm_pd_Menu
    menu $menu
    
    set pane_name [string tolower $pane]
    
    # substiture $mm_menu(Menu)
    regsub -all {\$mm_menu\(Menu\)} $_mm_pd_Menu($pane_name) $menu menu_def

    set item -1
    foreach j $menu_def {
	incr item

	# substiture $mm_menu(Item)
	regsub -all {\$mm_menu\(Item\)} $j $item i

	case [lindex $i 0] in {
	    title {
		eval $menu add command -state disabled [lrange $i  1 100]
		$menu add separator
	    }
	    label {
		eval $menu add command -state disabled [lrange $i 1 100]
	    }
	    menu {
		set nested_pane [lindex $i 1]
		set rest [lrange $i 2 100]
		if { [string tolower $nested_pane] == [string tolower $pane] } {
		    error "_mm_MenuMakePane: loop in menu panes for \"$nested_pane\""
		}
		set nested_menu $menu.[string tolower $nested_pane]
		_mm_MenuMakePane $nested_menu $nested_pane $in_menu_bar
		eval $menu add cascade -menu $nested_menu $rest
	    }
	    default {
		eval $menu add $i
	    }
	}
    }

    if { ! $in_menu_bar } {
	bind $menu <B3-ButtonRelease> "_mm_MenuUnpost $menu"
	bind $menu <B3-Motion> "$menu activate @%y"
    }
}

#--------------------------------------------------------------------
# PROCEDURE:  _mm_MenuPost - post a popup menu 
#--------------------------------------------------------------------
proc _mm_MenuPost {menu x y} {
  $menu post $x $y
  $menu activate @$y
  grab set $menu
}

#--------------------------------------------------------------------
# PROCEDURE:  _mm_MenuUnpost - unpost popup menu
#--------------------------------------------------------------------
proc _mm_MenuUnpost {menu} {
  grab release $menu
  $menu unpost
  $menu invoke active
}

#
#----------------------------------------------------------------------
# 			EXAMPLE OF USAGE
#----------------------------------------------------------------------
#

if { [lsearch -exact [array names env] MM_MENU_EXAMPLE] != -1 } {
    # Clean the desk
    foreach i [winfo child . ] { catch { destroy  $i }}
    
    #
    # Define menu panes
    #
    mm_Menu example::help {
	{command -label "About"         -command "Help about"}
	{command -label "Help"          -command Help}
	{command -label "Help on Help"  -command Help}
    }
    mm_Menu example::view {
	{command -label "About" -under 0 -command command}
	{command -label "Help"  -under 0 -command Help}
    }
    
    # may have title, label or any of menu entries
    mm_Menu example::file {
	{ title -label "This is a menu title " -background white}
	{ menu example::view -label "nested menu" -underline 2}
	{ command -label "Open" \
	      -command {
		  puts "Open , index = [$mm_menu(Menu) index active]"
	      }
	}
	{ separator }
	{ radio -label Test }
	{ radio -label Demo }
	{ separator }
	{ check -label Check }
	{ check -label Log }
	{ separator }
	{ command -label "Close" \
	      -command {
		  puts "Close, index = [$mm_menu(Menu) index active]"
	      }
	}
	{ command -bitmap questhead -command  "Help help"}
	{ command -label "EXIT" \
	      -command  { 
		  if  {[tk_dialog .dlg_exit {Exit Confirmation} \
			    {Really exit ? } questhead 1 OK CANCEL] == 0 } {
				exit
			    }
	      }
	}
	{ label -label "This is a label"}
    }
    
    # 
    # Create Menu bar for the . 
    # This command should be _before_ any widget creation in window
    #
    mm_MenuBar .menu {
	{"File" 0 example::file}
	{"View" 0 example::view}
	{"Help" 1 example::help right}
    }
    
    #
    # Create label& entry 
    #
    label .f -text "Label - press B3"  -bd 2 -anc center
    pack .f -expand yes -fill both -ipadx 4 -ipady 4 -pady 3 -padx 3
    bind .f <Enter> {.f config -relief groove}
    bind .f <Leave> {.f config -relief flat}
    
    entry .e -bd 2 -textvar e; set e  "Entry: Press B3 to popup menu"
    pack .e -expand yes -fill both -ipadx 4 -ipady 4 -pady 3 -padx 3
    bind .e <Enter> {.e config -relief groove}
    bind .e <Leave> {.e config -relief flat}
    
    #
    # Attach popup menus to widgets
    #
    mm_MenuAttach .f example::file
    mm_MenuAttach .e example::file
}

#
#---------------------------------------------------------------
#
# Modification History:
#
# $Log: menu_bar.tcl,v $
# Revision 1.1.5.1  1994/06/05  17:33:12  mark
# Automatic Branch Creation
#
# Revision 1.1  1994/06/05  17:33:12  mark
# Initial revision
#
#

