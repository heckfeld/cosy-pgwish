#----------------------------------------------------------------------
# The Tix Package stub file. It loads in the Tix Package.
#
#----------------------------------------------------------------------
proc tixInit {args} {
    global tix_priv

    # The default options

    set option(-beta)     true
    set option(-binding)  TK
    set option(-debug)    false
    set option(-fontset)  TK
    set option(-libdir)   "[info library]/../tix"
    set option(-scheme)   Default

    set validOptions {-beta -binding -debug -fontset -libdir -scheme}

    tixHandleOptions option $validOptions $args

    #-------------------------------
    # Initialization of the package
    #-------------------------------
    global TIX_SOURCE_DIR auto_path

    set TIX_SOURCE_DIR $option(-libdir)

    lappend auto_path $TIX_SOURCE_DIR/schemes
    lappend auto_path $TIX_SOURCE_DIR/bindings

    # Enable/Disable beta widgets
    #
    if {$option(-beta) == "true"} {
	lappend auto_path $TIX_SOURCE_DIR/beta
    }

    # Enable/Disable Intrinsics debugging
    #
    if {$option(-debug) == "true"} {
	set tix_priv(debug) 1
    } else {
	set tix_priv(debug) 0
    }

    #-----------------------------------
    # Initialization of options database
    #-----------------------------------
    # Load the fontset
    #
    if [auto_load tixSetFontset::$option(-fontset)] {
	tixSetFontset::$option(-fontset)
    } else {
	return -code error "unknown fontset option \"$option(-fontset)\""
    }

    # Load the bindings
    #
    if [auto_load tixSetColorScheme::$option(-scheme)] {
	tixSetColorScheme::$option(-scheme)
    } else {
	return -code error "unknown color scheme option \"$option(-scheme)\""
    }

    tixInitOptionDatabase

    #----------------------------------
    # Initialization of binding package
    #----------------------------------
    if [auto_load tixSetBinding::$option(-binding)] {
	tixSetBinding::$option(-binding)
    } else {
	return -code error "unknown binding option \"$option(-binding)\""
    }
}
