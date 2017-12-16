# RadioButton
# ----------------------------------------------------------------------
# A radiobutton variant that handles radio groups without the need for
# explicit reference to global variables.
# 
#
#   PUBLIC ATTRIBUTES:
#
#     group .......... name of the radio group to which the button belongs.
#                      
#     variable ....... Don't use this; use group instead.
#                      It exists purely to enable the radiobutton option, in
#                      case used, to work with group, not against it. Its
#                      value will be interpreted as a group name, not a global
#                      variable. The global variable that is actually used has
#                      a manufactured name and should never need to be accessed
#                      directly - the class methods should always be used. (If
#                      you don't like the behaviour then go use radiobutton).
#
#   METHODS:
#
#     group .......... returns a list of the pathnames of members (in no
#                      particular order) in the caller's group
#
#     selected ....... returns the name (as given by -value) of the selected
#                      member in the caller's group 
#
#     isSelected ..... returns selected state of caller
#
#
#   PROCEDURES:
#
#     members ........ returns a list of the pathnames of members of the
#                      specified group
#
#     chosen ......... returns the name of the selected member of the 
#                      specified group
#
# ----------------------------------------------------------------------
#   AUTHOR:  Jim Wight        <j.k.wight@newcastle.ac.uk>
# ----------------------------------------------------------------------
# Copyright 1994 The University of Newcastle upon Tyne (see COPYRIGHT)
# ======================================================================

itcl_class RadioButton {
    inherit itcl_radiobutton

    constructor {args} {
	eval itcl_radiobutton::constructor $args

	if {$group == ""} {
	    configure -group [lindex [previous configure -variable] 4]
	}
    }

    destructor {
	global _r_b_v$_group

	unset _members($_group,$this)
	if {[members $_group] == ""} {
	    unset _r_b_v$_group
	}
    }

    public group {} {
	global _r_b_v$_group

	if {$_group != ""} {
	    unset _members($_group,$this)
	    if {[members $_group] == ""} {
		catch "unset _r_b_v$_group"
	    }
	}
	set _members($group,$this) 1
	set _group $group
	$tk_widget configure -variable _r_b_v$group
    }

    public variable {} {
	configure -group $variable
    }

    # ------------------------------------------------------------------
    #  METHOD group - return list of members belonging to caller's group
    # ------------------------------------------------------------------
    method group {} {
	return [members $_group]
    }

    # ------------------------------------------------------------------
    #  METHOD isSelected - return state of caller
    # ------------------------------------------------------------------
    method isSelected {} {
	return [string match [selected] [lindex [configure -value] 4]]
    }

    # ------------------------------------------------------------------
    #  METHOD selected - return selected member in caller's group
    # ------------------------------------------------------------------
    method selected {} {
	global [set globvar [lindex [configure -variable] 4]]

	return [set $globvar]
    }

    # ------------------------------------------------------------------
    #  PROC members - return members in the specified group
    # ------------------------------------------------------------------
    proc members {group} {
	set quorum {}
	foreach name [array names _members] {
	    if {[regsub "$group,(.*)" $name {\1} member]} {
		lappend quorum $member
	    }
	}
	return $quorum
    }

    # ------------------------------------------------------------------
    #  PROC chosen - return selected member in the specified group
    # ------------------------------------------------------------------
    proc chosen {group} {
	if {[set names [members $group]] != ""} {
	    return [[lindex $names 0] selected]
	} else {
	    return {}
	}
    }

    protected _group {}

    common _members

    set   _members(0) 1;  # Force it to be an array so that members doesn't
    unset _members(0);    # foul up if called before any instances created.
    
}
