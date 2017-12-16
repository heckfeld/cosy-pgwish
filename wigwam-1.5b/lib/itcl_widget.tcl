# itcl_widget
# ----------------------------------------------------------------------
# Implements an [incr tcl] wrapper for a Tk widget.
#
#   PUBLIC ATTRIBUTES: 
#
#     None
#
#   METHODS: 
#
#     None
#
# ----------------------------------------------------------------------
#   AUTHOR:         Jim Wight <j.k.wight@newcastle.ac.uk>
#            Lindsay Marshall <lindsay.marshall@newcastle.ac.uk>
# ----------------------------------------------------------------------
# Copyright 1994 The University of Newcastle upon Tyne (see COPYRIGHT)
# ======================================================================

itcl_class itcl_widget {
    inherit itcl_wrapper

    # ------------------------------------------------------------------
    #  CONSTRUCTOR - create new widget
    # ------------------------------------------------------------------
    constructor {args} {
	set heritage [virtual info heritage]
	set class [virtual info class]
	
	if {[set cp [lsearch $args -class]] != -1} {
            set class "-class [lindex $args [expr $cp + 1]]"
        } elseif {[lsearch $heritage itcl_frame] != -1 ||
		                     [lsearch $heritage itcl_toplevel] != -1} {
	    set class "-class $class"
	} else {
	    set class {}
	}

        set screen {}
        if {[set sp [lsearch $args -screen]] != -1} {
            set screen "-screen [lindex $args [expr $sp + 1]]"
        }

        #
        #  Create a widget with the same name as this object
        #
	set tk_widget $this-win-
	::rename $this $this-tmp-
        #
        # Assumes inheritance from itcl_<widget> and that itcl_<widget> is
	# one earlier in heritage list than itcl_widget.
	# Generates ::widget $this [-class class] [-screen screen] command.
        #
        set index [expr [lsearch $heritage itcl_widget] - 1]
	eval ::[string range [lindex $heritage $index] 5 end] \
	                                                  $this $class $screen
	::rename $this $tk_widget
	::rename $this-tmp- $this
	::bind $this <Destroy> "catch {rename $tk_widget {}}"

	if {[string compare $args ""] != 0} {
	    eval configure $args
	}
    }

    # ------------------------------------------------------------------
    #  PROTECTED
    #    tk_widget ..... the real widget
    # ------------------------------------------------------------------
    protected tk_widget
}

proc ItclClassFromWidget {type} {
    if {[itcl_info classes itcl_$type] == ""} {
	eval \
	    "itcl_class itcl_$type {
                 inherit itcl_widget
  
                 constructor {args} {
                     eval itcl_widget::constructor \$args
                 }

                eval \[InitializeWrapper $type\]
           }"
       } 
}
