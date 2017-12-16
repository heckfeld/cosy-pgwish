# Dialog
# ----------------------------------------------------------------------
# Implements a toplevel window containing a message and buttons
#
#   PUBLIC ATTRIBUTES:
#                                                                       Default
#     -text ........ the message to display .............................. [""]
#     -bitmap  ..... a bitmap to display to the left of the message . [warning]
#     -buttons ..... a list of buttons to add to the dialog .............. [ok]
#     -default ..... default button selected by the <Return> key.......... [""]
#     -padx ........ minimum horizontal margin around buttons ............. [5]
#     -pady ........ minimum vertical margin around buttons ............... [5]
#     -variable .... global variable through which to return the name of 
#                    the selected button ............ [last component of $this]
#     -auto ........ boolean specifying whether the Dialog should be
#                    automatically popped up and deleted ............... [True]
#
#   METHODS:
#
#     None.
#
#   NOTE:
#
#     A Dialog is destroyed when a button is selected if auto is True. 
#     Setting auto to False implies you want greater control, which
#     includes deciding when the widget gets destroyed. If you want it to
#     happen on selection then configuring "-popdown {$this delete}" will
#     see to it.
#
# ----------------------------------------------------------------------
#   AUTHOR:  Jim Wight        <j.k.wight@newcastle.ac.uk>
# ----------------------------------------------------------------------
# Copyright 1994 The University of Newcastle upon Tyne (see COPYRIGHT)
# ======================================================================

itcl_class Dialog {
    inherit Window

    constructor {args} {
      regsub {.*\.(.*)} $this {\1} variable
	
      eval Window::constructor $args

      ::frame $this._text 
      ::frame $this._buttons 

      message $this.text \
          -width 3i \
	  -text $text \
	  -justify center

      if {$bitmap != ""} {
          label $this.warn -bitmap $bitmap
          pack $this.warn \
	      -in $this._text \
	      -side left
	  pack [::frame $this._text.glue \
		    -width $padx -height 1] \
	       -side left
      }

      pack $this.text \
          -in $this._text \
	  -side left \
	  -fill both -expand true

      for {set button 0} {$button < [llength $buttons]} {incr button} {
	  set name [lindex $buttons $button]
	  button $this.$name \
	      -text $name \
	      -command "$this select $name"

	  if {$button > 0} {
	      pack [::frame $this._glue$button \
		         -width $padx -height 1] \
		    -in $this._buttons \
		    -side left
	  }

          if {[string compare $name $default] == 0} {
	      ::frame $this._default \
	          -relief sunken \
		  -bd 1
	      raise $this.$name $this._default

	      pack $this._default \
	          -in $this._buttons \
		  -side left \
		  -expand true
	      pack $this.$name \
	          -in $this._default \
		  -padx $padx -pady $pady

	      bind $this <Return> "$this.$name flash; $this select $name"
          } else {
	      pack $this.$name \
	          -in $this._buttons \
		  -side left \
		  -expand true
          }
      }

      pack $this._text -fill both -padx $padx -pady $pady
      pack [::frame $this._join \
	        -geometry 2x2 \
	        -relief raised \
	        -borderwidth 1] \
           -fill x
      pack $this._buttons -side bottom -fill both -padx $padx -pady $pady
      
      transient .
      withdraw
      update idletasks
      set x [expr [winfo screenwidth $this]/2 - [winfo reqwidth $this]/2 \
	     - [winfo vrootx [winfo parent $this]]]
      set y [expr [winfo screenheight $this]/2 - [winfo reqheight $this]/2 \
	     - [winfo vrooty [winfo parent $this]]]
      geometry +$x+$y

      if {$auto} {
	  deiconify
      }
    }

    method deiconify {} {
      Window::deiconify

      set focus [focus]
      grab $this
      focus $this
      eval set var $variable
      global $var
      tkwait variable $var
    }

    public bitmap    warning { subconfigure bitmap warn }

    public text      {}      { subconfigure text text }

    public buttons   ok
    public default   {}
    public padx      5
    public pady      5
    public variable  {}
    public auto      True


    # For private use from here on
    #
    method select {button} {
	eval set var $variable
	global $var

	set $var $button
	focus $focus
	withdraw
	if {$auto} {
	    delete
	}
    }

    protected focus {}
}    
