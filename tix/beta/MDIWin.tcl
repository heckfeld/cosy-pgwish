proc tixMDIWindow {w args} {
    eval tixInt_CreateWidget $w tixMdi TixMDIWindow $args
    return $w
}

proc tixMdi::CreateClassRec {} {
  global tixMdi

  # OPTIONS
  set tixMdi(rootOptions)   {}
  set tixMdi(options)       {
			     -clienttype
			     }
  set tixMdi(staticOptions) {-clienttype}

  # DEFAULT VALUES
  set tixMdi(-clienttype)   {-clienttype clientType ClientType tixMwmClient}

  # ALIASES


  # METHODS
  set tixMdi(methods)       {add place window}
}

proc tixMdi::InitWidgetRec {w class className args} {
    upvar #0 $w data
}

proc tixMdi::ConstructWidget {w} {
    upvar #0 $w data
} 

#----------------------------------------------------------------------
#                  Configuration commands
#----------------------------------------------------------------------

#----------------------------------------------------------------------
#                  public methods
#----------------------------------------------------------------------
proc tixMdi::add {w name args} {
    upvar #0 $w data

    return [eval $data(-clienttype) $w.$name $args]
}

proc tixMdi::place {w name args} {
    upvar #0 $w data

    return [eval place $w.$name $args]
}

proc tixMdi::window {w name args} {
    upvar #0 $w data

    if {$args == {}} {
	return $w.$name
    } else {
	return [eval $w.$name $args]
    }
}
