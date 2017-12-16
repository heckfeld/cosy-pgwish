proc tixLabelFrame {w args} {
    upvar #0 $w data

    eval tixInt_CreateWidget $w tixLaFm TixLabelFrame $args
    return $data(wInframe)
}

proc tixLaFm::CreateClassRec {} {
    global tixLaFm

    # OPTIONS
    set tixLaFm(rootOptions)   {}
    set tixLaFm(options)       {-anchor -label -padx -pady
				-font -background -foreground -bg -fg
			       }
    set tixLaFm(staticOptions) {}
    # DEFAULT VALUES
    set tixLaFm(-anchor)       {-anchor anchor Anchor center}
    set tixLaFm(-background)   {-background background Background #ffe4c4}
    set tixLaFm(-foreground)   {-foreground foreground Foreground black}
    set tixLaFm(-font)         {-font font Font \
	                       "-*-helvetica-bold-r-normal-*-14-*-*-*-*-*-*-*"}
    set tixLaFm(-label)        {-label label Label ""}
    set tixLaFm(-padx)         {-padx padX Pad 0}
    set tixLaFm(-pady)         {-padY padY Pad 0}

    # ALIASES
    set tixLaFm(-bg)           {= -background}
    set tixLaFm(-fg)           {= -foreground}
    # METHODS
    set tixLaFm(methods)       {}
}

proc tixLaFm::ConstructWidget {w} {
    upvar #0 $w data

    frame $w.f1 -borderwidth 2 -bg $data(-background) -relief groove
    frame $w.f2 -borderwidth 0 -bg $data(-background)

    label $w.lab \
	-font $data(-font) \
	-text $data(-label) \
	-fg $data(-foreground) \
	-bg $data(-background) \
	-anchor $data(-anchor)\
	-bd 0 -padx 0 -pady 0

    pack $w.lab -fill x
    pack $w.f2 -in $w.f1 -fill both -expand yes \
	-padx $data(-padx) -pady $data(-pady)
    pack $w.f1 -fill both -expand yes

    set data(wInframe) $w.f2
    set data(wLabel)   $w.lab
}

proc tixLaFm::config-anchor {w arg} {
    upvar #0 $w data

    $data(wLabel) config -anchor $arg
}

proc tixLaFm::config-background {w arg} {
    upvar #0 $w data

    $data(wLabel) config -bg $arg
    $data(rootCmd) config -bg $arg
    $w.f1 config -bg $arg
    $w.f2 config -bg $arg
}

proc tixLaFm::config-foreground {w arg} {
    upvar #0 $w data

    $data(wLabel) config -fg $arg
}

proc tixLaFm::config-font {w arg} {
    upvar #0 $w data

    $data(wLabel) config -font $arg
}

proc tixLaFm::config-padx {w arg} {
    upvar #0 $w data

    pack $data(wInframe) -padx $arg
}

proc tixLaFm::config-pady {w arg} {
    upvar #0 $w data

    pack $data(wInframe) -pady $arg
}

proc tixLaFm::config-label {w arg} {
    upvar #0 $w data

    $data(wLabel) config -text $arg
}
