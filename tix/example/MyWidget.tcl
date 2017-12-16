# Class     : myWidget

proc myWidget {w args} {
    upvar #0 $w data

    eval tixInt_CreateWidget $w tixLaFm TixLabelFrame $args
    return $data(wInframe)
}

proc tixLaFm::CreateClassRec {} {
    global tixLaFm

    set tixLaFm(options)       {-label -innerborder -font}
    set tixLaFm(rootOptions)   {-bg}
    set tixLaFm(-font)         {-font font Font \
	                       "-*-helvetica-bold-r-normal-*-14-*-*-*-*-*-*-*"}
    set tixLaFm(-label)        {-label label Label ""}
    set tixLaFm(-innerborder)  {-innerborder innerBorder InnerBorder 2}
}

proc tixLaFm::SetBindings {w} {

}

proc tixLaFm::InitWidgetRec {w args} {
    upvar #0 $w data
}

proc tixLaFm::ConstructWidget {w} {
    upvar #0 $w data

    frame $w.f0
    frame $w.f1 -borderwidth 2 -relief groove
    frame $w.f2 -borderwidth $data(-innerborder)

    label $w.lab \
	-font $data(-font) \
	-text $data(-label) \
	-anchor w

    pack $w.lab -in $w.f0

    pack $w.f2  -in $w.f1 -expand yes -fill both

    pack $w.f0 -side top -fill x
    pack $w.f1 -side top -fill both -expand yes

    set data(wInframe) $w.f2
    set data(wLabel)   $w.lab
}

proc tixLaFm::config-label {w arg} {
    upvar #0 $w data

    $data(wLabel) config -text $optn
    set data(-label) $optn
}
