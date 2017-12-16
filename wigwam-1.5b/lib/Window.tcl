# Window
# ----------------------------------------------------------------------
# Implements a toplevel window with popup and popdown callbacks. 
# Can also be applied to `.'
#
#   PUBLIC ATTRIBUTES:
#
#     -popup ......... a command to execute when the window is popped up
#     -popdown ....... a command to execute when the window is popped down
#
#     All wm options, e.g.
#         Window .help -title "Help" -minsize {1 1}
#
#   METHODS:
#
#     All wm options, e.g.
#         Window .
#         . iconify
#
# ----------------------------------------------------------------------
#   AUTHOR:         Jim Wight <j.k.wight@newcastle.ac.uk>
#            Lindsay Marshall <lindsay.marshall@newcastle.ac.uk>
# ----------------------------------------------------------------------
# Copyright 1994 The University of Newcastle upon Tyne (see COPYRIGHT)
# ======================================================================

itcl_class Window {
    inherit itcl_window

    constructor {args} {
        eval itcl_window::constructor $args

        ::bind $this <Map> "$this popup"
        ::bind $this <Unmap> "$this popdown"
    }

    public popup   {}
    public popdown {}

    # For private use
    #
    method popup {} {
	if {![string match "" $popup]} {eval $popup}
    }

    method popdown {} {
	if {![string match "" $popdown]} {eval $popdown}
    }

    catch {wm foo .} msg
    if {[regsub {unknown or ambiguous option.*must be (.*)} $msg {\1} methods] == 0} {
	puts stdout {Window: Format of "unknown option" error message has changed}
	puts stdout {Expected: unknown or ambiguous option "dummy":  must be etc.}
	puts stdout "     Got: $msg"
	puts stdout "\nPlease report this to your system administrator."
	exit
    }      
    
    regsub {(.*) or (.*)} $methods {\1 \2} methods
    regsub -all "," $methods "" methods
    foreach method $methods {
	eval [list method $method {args} "eval wm $method \$this \$args"]

	switch $method {
	    aspect       -
	    grid         -
	    iconposition -
	    maxsize      -
	    minsize      -
	    protocol     {eval [list public $method {} "eval wm $method \$this \$$method"]}
	    deiconify    -
	    frame        -
	    iconify      -
	    state        -
	    withdraw     {eval [list public $method {} "wm $method \$this"]}
            geometry     {} ; # Clashes with toplevel's option
	    default      {eval [list public $method {} "wm $method \$this \$$method"]}
        }
    }    
}
