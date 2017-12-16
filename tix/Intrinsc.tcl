# The Tix Intrinsics

proc tixInt_CreateWidget {w class className args} {
    upvar #0 $w data
    upvar #0 $class classRec

    # CREATE CLASS RECORD IF NECESSARY
    if {[info globals $class] == ""} {
	tixInt_CreateClassRec $class
    }

    tixInt_CreateWidgetRec        $w $class $className
    eval $class::CreateWidgetRoot $w
    tixInt_MkWidgetCommand        $w

    if {$classRec(rootOptions) == {}} {
	foreach option [$data(rootCmd) config] {
	    lappend classRec(rootOptions) [lindex $option 0]
	}
    }

    # CREATE WIDGET RECORD
    eval tixInt_LoadOptions       $w $class $className $args

    # INITIALIZE
    eval $class::InitWidgetRec    $w $class $className $args 
    eval $class::ConstructWidget  $w
    eval $class::SetBindings      $w

    bind $w <Destroy> "+tixDeleteWidgetCmd $w"
}

proc tixInt_CreateClassRec {class} {
    $class::CreateClassRec
    tixInt_CreateDefMethods $class
}

proc tixInt_CreateWidgetRec {w class className} {
    upvar #0 $w data
    upvar #0 $class classRec

    set data(class)     $class
    set data(className) $className
    set data(root)      $w
}

proc tixInt_LoadOptions {w class className args} {
    upvar #0 $w data
    upvar #0 $class classRec

    # SET UP THE WIDGET RECORD ACCORDING TO DEFAULT VALUES

    foreach option $classRec(options) {
	if {[lindex $classRec($option) 0] == "="} {
	    continue
	}

	set o_name    [lindex $classRec($option) 1]
	set o_class   [lindex $classRec($option) 2]
	set o_default [lindex $classRec($option) 3]

	if {![catch "option get $w $o_name $o_class" userDef]} {
	    if {$userDef != ""} {
		set data($option) $userDef
	    } else {
		set data($option) $o_default
	    }
	} else {
	    set data($option) $o_default
	}
    }

    # SET UP THE WIDGET RECORD ACCORDING TO COMMAND ARGUMENTS
    set len  [llength $args]
    set len2 [expr {$len - 2}]
    set i 0
    set root_options {}

    while {$i <= $len2} {
	set option [lindex $args $i]
	incr i
	set arg [lindex $args $i]
	incr i
	
	if {[lsearch $classRec(options) $option] != "-1"} {
	    if {[lindex $classRec($option) 0] != "="} {
		set data($option) $arg
	    } else {
		set realOption [lindex $classRec($option) 1]
		set data($realOption) $arg
	    }
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

    if {$i != $len} {
	error "Odd number of config parameters applied to $data(root)"
    }
}

proc tixInt_MkWidgetCommand {w} {
    upvar #0 $w data

    set data(rootCmd) $w:root
    tixRenameWidgetCmd $w $data(rootCmd)

    proc $w {method args} {
	global tix_priv
	# the name of the data structure is the same as the procedure
	#
	set w [lindex [info level 0] 0]
	if [catch "tixInt_WidgetCmd $w $method $args" ret] {
	    if {$tix_priv(debug)} {
		error $ret
	    } else {
		return -code error $ret
	    }
	} else {
	    return $ret
	}
    }
}

proc tixInt_WidgetCmd {w method args} {
    global tix_priv
    upvar #0 $w data
    upvar #0 $data(class) classRec

    case $method in {
	#------------------------------------------------------------
	# Config is a special method that needs special attention
	#------------------------------------------------------------
	{config configure} {
	    if {$args == {}} {
		# Query all options
		return [tixInt_QueryAllOptions $w]
	    }
	    if {[llength $args] == 1} {
		# query the value of an option
		if [catch "tixInt_QueryOption $w [lindex $args 0]" ret] {
		    return -code error $e_msg
		} else {
		    return $ret
		}
	    } else {
		if [catch {eval tixInt_ApplyOptions $w $args} e_msg] {
		    return -code error $e_msg
		}
	    }
	}
	default {
	    # 1. Execute the method as if it is a method of the class
	    # 2. If error happens in the execution of 1, check if
	    #    the method is supported by the class
	    # 3. If yes, then simply output the error message from 1.
	    # 4. If no, then try execute as if the method belogs 
	    #    to the root widget
	    # 5. If error in 4, check the following
	    # 6. If the method does belong to the root widget
	    #    simple print out the error message from 4.
	    # 7. If the method does not belong to the root widget
	    #    print the set of methods avilable from both the class
	    #    and the root widget


	    # 1. If this is a Intrinsics type method, call the method
	    #    command
	    if {[lsearch $classRec(methods) $method] != "-1"} {
		return [eval $data(class)::$method $w $args]
	    } else {
		# Execute this as a command of the root widget
		if [catch "eval $data(rootCmd) $method $args" ret] {
		    if {[lrange $ret 0 1] != "bad option"} {
			# The root widget command execution error
			error $ret
		    } else {
			# There is no such root widget command
			if {$classRec(methods) != ""} {
			    foreach method $classRec(methods) {
				append ret ", $method"
			    }
			}
			error $ret
		    }
		} else {
		    # An root-widget command executed OK
		    return $ret
		}
	    }
	}
    }
}


proc tixInt_ListOptions {w} {
    upvar #0 $w data
    upvar #0 $data(class) classRec

    return [lsort "$classRec(rootOptions) $classRec(options)"]
}

proc tixInt_QueryAllOptions {w} {
    upvar #0 $w data
    upvar #0 $data(class) classRec

    set options {}

    foreach option [$data(rootCmd) config] {
	set flag [lindex $option 0]
	if {[lsearch $classRec(options) $flag] == "-1"} {
	    lappend options $option
	}
    }

    foreach option $classRec(options) {
	if {[lindex $classRec($option) 0] != "="} {
	    set opt $classRec($option)
	    lappend opt $data($option)
	    lappend options $opt
	}
    }

    return [lsort $options]
}

proc tixInt_QueryOption {w option} {
    upvar #0 $w data
    upvar #0 $data(class) classRec

    if {[lsearch $classRec(options) $option] != -1} {
	if {[lindex $classRec($option) 0] == "="} {
	    set option [lindex $classRec($option) 1]
	}
	set opt $classRec($option)
	return [lappend opt $data($option)]
    } else {
	if [catch {return [$data(rootCmd) config $option]} err_msg] {
	    if {[string range $err_msg 0 13] == "unknown option"} {
		error "unknown option $option. Should be: \
                           [tixInt_ListOptions $w]"
	    } else {
		error $err_msg
	    }
	}
    }
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

    while {$i <= $len2} {
	set option [lindex $args $i]
	incr i
	set arg [lindex $args $i]
	incr i

	if {[lsearch $classRec(options) $option] != "-1"} {
	    if {[lsearch $classRec(staticOptions) $option] != "-1"} {
		error "cannot change static option \"$option\""
	    }

	    if {[lindex $classRec($option) 0] == "="} {
		set option [lindex $classRec($option) 1]
	    }

	    if {[info commands $data(class)::config$option] != ""} {
		$data(class)::config$option $w $arg
	    } else {
  	      if {[info commands $data(class)::configWidget] != ""} {
		  $data(class)::configWidget $w $option $arg
	      }
            }

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

    if {$i != $len} {
	error "value for \"$option\" missing"
    }
}

proc tixInt_DefCreateWidgetRoot {w args} {
    upvar #0 $w data

    frame $w -class $data(className)
}

# CREATE THE DEFAULT METHODS
proc tixInt_CreateDefMethods {class} {

    if {"[info procs $class::CreateWidgetRoot]" == ""} {
	if {[auto_load $class::CreateWidgetRoot] == 0} {
	    proc $class::CreateWidgetRoot {w args} {
		tixInt_DefCreateWidgetRoot $w $args
	    }
	}
    }

    if {"[info procs $class::InitWidgetRec]" == ""} {
	if {[auto_load $class::InitWidgetRec] == 0} {
	    proc $class::InitWidgetRec {w class className args} {}
	}
    }

    if {"[info procs $class::SetBindings]" == ""} {
	if {[auto_load $class::SetBindings] == 0} {
	    proc $class::SetBindings {w} {}
	}
    }
}

proc tixRenameWidgetCmd {w new_w} {
    if {[info commands $new_w] != {}} {
	rename $new_w {}
    }
    rename $w $new_w
}

proc tixDeleteWidgetCmd {w} {
    if {[info commands $w] != {}} {
	rename $w {}
    }
}
