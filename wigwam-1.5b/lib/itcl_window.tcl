# itcl_window
# ----------------------------------------------------------------------
# Window wrapper class with provision for application to `.'.
#
#   PUBLIC ATTRIBUTES:
#
#     All toplevel's options.
#
#   METHODS:
#
#     None.
#
# ----------------------------------------------------------------------
#   AUTHOR:         Jim Wight <j.k.wight@newcastle.ac.uk>
#            Lindsay Marshall <lindsay.marshall@newcastle.ac.uk>
# ----------------------------------------------------------------------
# Copyright 1994 The University of Newcastle upon Tyne (see COPYRIGHT)
# ======================================================================

itcl_class itcl_window {
    inherit itcl_wrapper

    constructor {args} {
	set tk_widget $this-win-

	if {![string match "." $this]} {
	    ::rename $this $this-tmp-
	    ::toplevel $this  -class [$this-tmp- info class]
	    ::rename $this $tk_widget
	    ::rename $this-tmp- $this
	    ::bind $this <Destroy> "catch {rename $tk_widget {}}"
	} else {
	    ::rename $this $this-class-
	    ::proc . {args} {
 		eval .-class- $args
	    }
	}

	if {![string match "" $args]} {
	    eval configure $args
	}
    }

    eval [InitializeWrapper toplevel]

    protected tk_widget
}    

rename . .-win-
proc . {args} {
    eval .-win- $args
}
