# FileNominatorWindow
# ----------------------------------------------------------------------
# Implements a file nominator widget window with gridded geometry
#
#   PUBLIC ATTRIBUTES:
#
#     -directory ..... specifies the directory to be displayed
#
#   METHODS:
#
#     None.
#
# ----------------------------------------------------------------------
#   AUTHOR:  Jim Wight <j.k.wight@newcastle.ac.uk>
# ----------------------------------------------------------------------
# Copyright 1994 The University of Newcastle upon Tyne (see COPYRIGHT)
# ======================================================================

itcl_class FileNominatorWindow {
    inherit Window

    constructor {args} {
	eval Window::constructor $args

	if {$directory != ""} {
	    FileNominator $this.nominator -directory $directory
	} else {
	    FileNominator $this.nominator
	}
	
	$this.nominator.list configure -setgrid true

	pack $this.nominator -fill both -expand true
    }

    public directory {} { subconfigure directory nominator }
}    


