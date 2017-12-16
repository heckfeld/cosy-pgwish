proc tixSetColorScheme::Default {} {

    global tixOption

    if {[tk colormode .] == "color"} {
	set tixOption(bg)           white
	set tixOption(fg)           black

	set tixOption(dark1_bg)     white
	set tixOption(dark1_fg)     black
	set tixOption(dark2_bg)     white
	set tixOption(dark2_fg)     black

	set tixOption(light1_bg)    white
	set tixOption(light1_fg)    white
	set tixOption(light2_bg)    white
	set tixOption(light2_fg)    white

	set tixOption(active_bg)    black
	set tixOption(active_fg)    white
	set tixOption(disabled_fg)  white

	set tixOption(input1_bg)    white
	set tixOption(input2_bg)    white
	set tixOption(output1_bg)   black
	set tixOption(output2_bg)   black

	set tixOption(select_fg)    white
	set tixOption(select_bg)    black

	set tixOption(selector)	    black
    } else {
	tixSetColorScheme::GrayScale
    }
    
}
