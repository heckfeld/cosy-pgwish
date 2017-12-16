# itcl_wrapper
# ----------------------------------------------------------------------
# Abstract base class supplying common methods for itcl_widget and
# itcl_window wrappers.
#
#   PUBLIC ATTRIBUTES: 
#
#     None
#
#   METHODS: 
#
#     configure .......... used to query/change public attributes
#
#     subconfigure ....... configure a public variable dependent on the
#                          existence of a sub-widget; widget must have 
#                          option with same name as variable.
#
#                          Arguments:
#                              variable - the name of the public variable
#                              widget   - the name of the widget relative
#                                         to the instance, i.e. w if $this.w
#                              args     - extra commands to execute;
#                                         contents will be eval'd
#
#                          i.e. for variable v, if $this.w exists, does  
#
#                              $this.w configure -$v [virtual set $v]
#                              eval $args
#
# ----------------------------------------------------------------------
#   AUTHOR:         Jim Wight <j.k.wight@newcastle.ac.uk>
#            Lindsay Marshall <lindsay.marshall@newcastle.ac.uk>
# ----------------------------------------------------------------------
# Copyright 1994 The University of Newcastle upon Tyne (see COPYRIGHT)
# ======================================================================

itcl_class itcl_wrapper {
    # ------------------------------------------------------------------
    #  METHOD:  configure - used to return or change public attributes
    # ------------------------------------------------------------------
    method configure {args} {
	#
	# When there are no arguments the wrapped widget's options are
        # obtained before public variables. The assumption is made in 
        # searching backwards through the heritage list for public variables
        # that the object is ultimately derived from a class whose name
        # begins with itcl_, e.g. itcl_button, itcl_canvas and itcl_window.
        # These wrappers duplicate the widget options, so that is used as
        # the signal to stop further processing.
        #
	switch [llength $args] {
	  0 { set heritage [virtual info heritage]
	      set options [[virtual set tk_widget] configure]

	      foreach public [virtual info public] {
		  set option [split $public :]
		  if {[string match  "itcl_*" [lindex $option 0]]} {
		      break
		  }
		  set opt [lindex $option 2]
		  lappend options [list -$opt {} {} \
				   [virtual info public $opt -init] \
				   [virtual info public $opt -value]]
	      }
	      return $options
	    }
	  1 { if {[catch {[virtual set tk_widget] configure $args} info]} {
	        if {[set opt \
		   [virtual info public [string trimleft $args -]]] != ""} {
		      return [list $args {} {} [lindex $opt 1] [lindex $opt 2]]
		} else {
		    return {}
		}
	      } else {
		return $info
	      }
	    }
	  default {
	      eval wigwam_configure $args
	      }
        }
    }

    # ------------------------------------------------------------------
    #  METHOD:  config - same as configure; only really needed because tk
    #                    uses it in its  .../lib/tk/*tcl files.
    # ------------------------------------------------------------------
    method config {args} {
        eval [virtual set tk_widget] configure $args
    }

    # ------------------------------------------------------------------
    #  METHOD:  subconfigure - configure a sub-widget
    # ------------------------------------------------------------------
    method subconfigure {variable widget args} {
	if {[winfo exists $this.$widget]} {
	    $this.$widget configure -$variable [virtual set $variable]
	    eval $args
	}
    }
	

    # ------------------------------------------------------------------
    #  DESTRUCTOR - destroy window containing widget
    # ------------------------------------------------------------------
    destructor {
	::rename [virtual set tk_widget] {}
	tk_destroy $this
    }

    # For private use
    #
    method wigwam_configure {config} {}

    # ------------------------------------------------------------------
    #  PROC: InitializeWrapper  - generate a string, which, when
    #  evaluated in the wrapper class, will create its methods and public
    #  variables.
    # ------------------------------------------------------------------
    proc InitializeWrapper {type} {
	set out ""

	$type ._foo_

	catch {._foo_ dummy} msg
	if {[regsub {bad option.* be (.*)} $msg {\1} methods] == 0} {
	    puts {InitializeWrapper: Format of "bad option" error message has changed}
	    puts {Expected: bad option "dummy":  must/should be ...}
	    puts "     Got: $msg"
	    puts "\nPlease report this to your system administrator."
	    exit
	}      

        #
        # Create a method corresponding to each Tk widget command
        #
	regsub {(.*) or (.*)} $methods {\1 \2} methods
	regsub -all "," $methods "" methods
	foreach method $methods {
	    if {[string compare $method configure] != 0} {
		if {[string compare $method delete] == 0} {
		    append out [list method tk_delete {args} \
                                            {eval $tk_widget delete $args}] {;}
		} else {
		    append out [list method $method {args} \
                                         "eval \$tk_widget $method \$args"] {;}
		}
	    }
	}

        #
        # Create a public variable corresponding to each Tk widget option
        #
	catch {._foo_ configure} vars
	foreach option $vars {
	    set var [string range [lindex $option 0] 1 end]
	    append out [list public $var {} \
                                 "eval \$tk_widget configure -$var \\\"\$$var\\\""] {;}
	}

	#
	# Define public variables for specials -class and -screen so
	# that they don't cause any harm during configuration. The
	# real notice of these is taken in the itcl_widget constructor.
	#
        if {[lsearch {frame toplevel} $type] != -1} {
            append out [list public class {}] {;}
            if {$type == "toplevel"} {
               append out [list public screen {}] {;}
            }
        }

	destroy ._foo_

	return $out
    }
}

auto_load destroy
