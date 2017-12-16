proc tixSetColorScheme::TK {} {

    global tixOption

    if {[tk colormode .] == "color"} {
	set tixOption(bg)           bisque1
	set tixOption(fg)           black

	set tixOption(dark1_bg)     bisque3
	set tixOption(dark1_fg)     black
	set tixOption(dark2_bg)     bisque4
	set tixOption(dark2_fg)     black

	set tixOption(light1_bg)    bisque1
	set tixOption(light1_fg)    white
	set tixOption(light2_bg)    bisque1
	set tixOption(light2_fg)    white

	set tixOption(active_bg)    $tixOption(dark1_bg)
	set tixOption(active_fg)    $tixOption(fg)
	set tixOption(disabled_fg)  gray55

	set tixOption(input1_bg)    bisque2
	set tixOption(input2_bg)    bisque2
	set tixOption(output1_bg)   $tixOption(dark1_bg)
	set tixOption(output2_bg)   $tixOption(bg)

	set tixOption(select_fg)    black
	set tixOption(select_bg)    lightblue

	set tixOption(selector)	    yellow
    } else {
	tixSetColorScheme::GrayScale
    }
}
