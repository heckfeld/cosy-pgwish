# This is the primitive widget. It is just a frame with proper inheritance
# wrapping. All new Tix widgets will be derived from this widget


proc tixPrimitive {} {



}

proc tixLaFm::CreateClassRec {} {
    global tixLaFm

    # OPTIONS
    set tixLaFm(rootOptions)   {}
    set tixLaFm(options)       {-anchor -label -padx -pady
				-font -background -foreground -bg -fg
			       }
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

