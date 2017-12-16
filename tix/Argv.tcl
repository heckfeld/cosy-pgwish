#------------------------------------------------------------
# If the flag is included in the param list, return its value
# Otherwise throw error
#
# Must catch the error
#-------------------------------------------------------------
proc tixGetArg {pkg_name flag default args} {
    global argv

    set args [concat $args $argv]
    set len [llength $args]
    set i 0

    while {$i < $len} {
	set arg [lindex $args $i]
	if {$flag == $arg} {
	    set params {}
	    incr i
	    while {$i < $len} {
		set param [lindex $args $i]
		if {[string index $param 0] != "-"} {
		    lappend params $param
		} else {
		    break
		}
		incr i
	    }
	    return $params
	}
	incr i
    }
    return $default
}

proc tixExistArg {pkg_name flag args} {
    global argv

    lappend args $argv

    if {[lsearch $flag $args] != "-1"} {
	return 1
    } else {
	return 0
    }
}

