# FileNominator
# ----------------------------------------------------------------------
# Implements a file selector widget
#
#   PUBLIC ATTRIBUTES:
#
#     -select ........ prefix of a command to execute when the user commits
#                      themself to a nomination, either by double clicking
#                      an entry in the list, clicking the "select" button,
#                      selecting an entry from the "path" menu or hitting
#                      Return in the nomination entry field. The command is
#                      invoked with the full path name of the nomination as
#                      argument.
#
#     -cancel ........ command to execute when the cancel button is clicked.
#
#     -filter ........ the filter expression
#
#     -include ....... prefix of command for determining which directory
#                      entries to display. The command is invoked once for
#                      each entry with the entry name as argument. It should
#                      return true if the entry is to be included. The
#                      entries offered for inclusion are those remaining
#                      after application of the filter in the interface.
#
#     -directory ..... specifies the directory to be displayed
#
#     -cwd ........... specifies whether working directory should be
#                      changed when a directory is selected
#
#     -padx .......... spacing around components in the X direction
#     -pady .......... spacing around components in the Y direction
#
#   METHODS:
#
#     clear .......... clear the selection and nomination entry field.
#
# ----------------------------------------------------------------------
#   AUTHOR:  Jim Wight <j.k.wight@newcastle.ac.uk>
# ----------------------------------------------------------------------
# Copyright 1994 The University of Newcastle upon Tyne (see COPYRIGHT)
# ======================================================================

itcl_class FileNominator {
    inherit itcl_frame

    constructor {args} {
	global ${this}filter ${this}nomination ${this}dirs

	set directory [pwd]/
	eval itcl_frame::constructor $args

	# The directory listing
	#
	Viewport $this.view "listbox $this.list -geometry 22x12 \
                                           -borderwidth 1 -relief sunken" \
                 -borderwidth 1 -relief raised
	tk_listboxSingleSelect $this.list
	bind $this.view <Enter> \
                  "$this configure -focusWin \[focus\]; focus $this.nomination"
	bind $this.view <Leave> "focus \[$this info public focusWin -value\]"
	bind $this.list <ButtonRelease-1> "$this choose"
	bind $this.list <Double-ButtonRelease-1> \
                     "$this.select activate; update; $this.nomination invoke; \
                      catch \"$this.select deactivate\""
	bind $this.list <ButtonRelease-2> \
	                      "[bind Listbox <ButtonPress-1>]; \
                               [bind $this.list <ButtonRelease-1>]; \
                               [bind $this.list <Double-ButtonRelease-1>]"
	pack $this.view -padx $padx -pady $pady -expand true -fill both

	# An invisible frame containing the filter label and entry widgets 
	#
	frame $this.filtframe

	MenuButton $this.lfilter -text options \
	          -menu $this.lfilter.menu -borderwidth 2 -relief raised
	menu $this.lfilter.menu
	$this.lfilter.menu add radiobutton -label "List directories" \
                                              -variable ${this}dirs -value list
	$this.lfilter.menu add radiobutton \
	    -label "Return directory names" -variable ${this}dirs -value return

	set ${this}dirs list

	EmacsEntry $this.filter -width 1 -command "$this refilter" \
	                        -textvariable ${this}filter -relief sunken
	#set ${this}filter $filter
	pack $this.filtframe -fill x
	pack $this.lfilter -in $this.filtframe \
                           -side left -padx $padx -pady $pady
	pack $this.filter -in $this.filtframe \
                     -side left -fill both -expand true -padx $padx -pady $pady

	# The nomination entry widget. The FileNominator always returns a
	# value by returning the contents of this widget, regardless of
        # how it came to be there, e.g. by selection from the displayed
	# directory, by selection from the "path" menu, or by being typed in.
	#
	EmacsEntry $this.nomination -command "$this select" \
                                 -relief sunken -textvariable ${this}nomination
	bind $this.nomination <Any-Key> \
                	 "[bind Entry <Any-Key>]; $this.list select clear"
	bind $this.nomination <Key-Delete> "+$this.list select clear"
	bind $this.nomination <Return> "$this.select activate; update; \
                                        [bind $this.nomination <Return>]; \
                                        catch \"$this.select deactivate\""
	pack $this.nomination -fill x -padx $padx -pady $pady


	# An invisible frame containing the select, path and cancel buttons
	#
	frame $this.fsel
	pack $this.fsel -side left

	button $this.select -text select -command "$this.nomination invoke"
	pack $this.select -in $this.fsel -side left \
                                         -fill y -padx $padx -pady $pady

	MenuButton $this.path -text path -menu $this.path.menu \
	                                          -borderwidth 2 -relief raised
 	menu $this.path.menu -postcommand "$this poppath"

 	pack $this.path -in $this.fsel -side left -padx $padx -pady $pady

	button $this.cancel -text cancel -command "$this cancel"
	pack $this.cancel -in $this.fsel -side left \
                                         -fill y -padx $padx -pady $pady

	# Generate the initial directory listing
	#
        directory $directory
    }

    method clear {} {
	global ${this}nomination

	set ${this}nomination ""
	$this.list select clear
    }

    public select    {}
    public cancel    {}
    public filter    *        { global ${this}filter
			        set ${this}filter $filter
				if {[itcl_info objects $this.view] != ""} {
				    refilter 
				}
			      }
    public include   {takeme}
    public directory {}       { directory $directory }
    public cwd       {0}      { set cwd [string match {[Tt]rue} $cwd] }
    public padx      5
    public pady      5


    # Private methods and variables beyond here
    #
    method select {args} {
	global ${this}dirs

	if {[string match "" $args]} {
	    return
	}

	if {[string match "/" [string index $args 0]]} {
	    set path $args
	} else {
	    set path $directory$args
	}

	if {[file isdir $path]} {
	    if {[string match "list" [set ${this}dirs]]} {
		directory $path
		return
	    } 
	}

	if {![string match "" $select]} {
	    eval $select $path
	}
    }

    method cancel {} {
	if {![string match "" $cancel]} {eval $cancel}
    }
    
    method refilter {args} {
	directory $directory
    }

    method populate {args} {
	global ${this}filter

	if {[string match "" [set ${this}filter]]} {
	    set ${this}filter *
	}

	set list [lsort [eval glob -nocomplain \{[set ${this}filter]\}]]
	$this.list delete 0 end
	$this.list insert end "./"
	$this.list insert end "../"
	if {[string match "" $include]} {
	    set include takeme
	}
	for {set e 0} {$e < [llength $list]} {incr e} {
	    set entry [lindex $list $e]
	    if {[eval $include \"$entry\"]} {
		if {[file isdir $entry]} {
		    $this.list insert end ${entry}/
		} else {
		    if {[file readable $entry]} {
			$this.list insert end ${entry}
		    }
		}
	    }
	}
    }

    method directory {dir} {
	global ${this}nomination

	set here [pwd]
	if {![catch "cd $dir"]} {
	    if {[winfo exists $this.view]} {
		populate
		set ${this}nomination {}
	    }
	    set directory [pwd]/
	    if {!$cwd} { cd $here }
	}
    }

    method choose {} {
	global ${this}nomination

	set ${this}nomination \
                           [$this.list get [$this.list curselection]]
    }
    
    method poppath {} {
	# directory always has a trailing /
	set path [split [file dirname $directory] /]

	if {[string match "" [lindex $path 1]]} {
	    set path [lreplace $path 0 0]
	}
	$this.path.menu delete 0 last
	for {set i 0} {$i < [llength $path]} {incr i} {
	    $this.path.menu add command -label [lindex $path $i]/ \
	                                   -command "$this pathdir $i"
	}
	$this.path.menu add separator; incr i
	$this.path.menu add command -label {$HOME} \
	                               -command "$this pathenv $i"
    }

    method pathdir {index} {
	set path {}
	for {set i 0} {$i <= $index} {incr i} {
	    lappend path [lindex [$this.path.menu entryconfigure $i -label] 4]
	}
	nominate [join $path ""]
    }

    method pathenv {index} {
	global env ${this}nomination

	set var [string range \
               [lindex [$this.path.menu entryconfigure $index -label] 4] 1 end]
	if {[catch "set env($var)"] == 0} {
	    nominate [set env($var)]
	}
    }

    method nominate {value} {
	global ${this}nomination

	set ${this}nomination $value
	$this.select activate; update
	$this.nomination invoke
	catch "$this.select deactivate"
    }

    method takeme {file} {return 1}

    public focusWin {}
}
