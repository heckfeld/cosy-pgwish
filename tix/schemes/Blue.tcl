proc tixSetColorScheme::Blue {} {
    global tixOption

    if {[tk colormode .] == "color"} {
	set tixOption(bg)           #9090f0
	set tixOption(fg)           black

	set tixOption(dark1_bg)     #8080d0
	set tixOption(dark1_fg)     black
	set tixOption(dark2_bg)     #7070c0
	set tixOption(dark2_fg)     black

	set tixOption(light1_bg)    #9898f0
	set tixOption(light1_fg)    black
	set tixOption(light2_bg)    #a0a0ff
	set tixOption(light2_fg)    black

	set tixOption(active_bg)    $tixOption(dark1_bg)
	set tixOption(active_fg)    $tixOption(fg)
	set tixOption(disabled_fg)  gray55

	set tixOption(input1_bg)    $tixOption(dark1_bg)
	set tixOption(input2_bg)    $tixOption(bg)
	set tixOption(output1_bg)   $tixOption(dark1_bg)
	set tixOption(output2_bg)   $tixOption(bg)

	set tixOption(select_fg)    white
	set tixOption(select_bg)    black

	set tixOption(selector)	    yellow
    } else {
	set tixOption(bg)           lightgray
	set tixOption(fg)           black

	set tixOption(dark1_bg)     gray
	set tixOption(dark1_fg)     black
	set tixOption(dark2_bg)     gray50
	set tixOption(dark2_fg)     white
	
	set tixOption(light1_bg)    gray50
	set tixOption(light1_fg)    white
	set tixOption(light2_bg)    gray50
	set tixOption(light2_fg)    white

	set tixOption(active_bg)    gray
	set tixOption(active_fg)    $tixOption(fg)
	set tixOption(disabled_fg)  gray55
	
	set tixOption(input1_bg)    $tixOption(bg)
	set tixOption(input2_bg)    $tixOption(bg)
	set tixOption(output1_bg)   $tixOption(bg)
	set tixOption(output2_bg)   $tixOption(bg)
	
	set tixOption(select_fg)    white
	set tixOption(select_bg)    black

	set tixOption(selector)	    yellow
    }

}
