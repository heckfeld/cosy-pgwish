proc tixInitOptionDatabase {} {

global tixOption


# general stuff, everything gets these defaults unless otherwise specified
#-------------------------------------------------------------------------
option add *Font		$tixOption(font)	userDefault
option add *Background		$tixOption(bg)		userDefault
option add *Foreground		$tixOption(fg)		userDefault
option add *font		$tixOption(font)	userDefault
option add *background		$tixOption(bg)		userDefault
option add *foreground		$tixOption(fg)		userDefault
option add *activeBackground	$tixOption(active_bg)	userDefault
option add *activeForeground	$tixOption(active_fg) 	userDefault
option add *disabledForeground	$tixOption(disabled_fg)	userDefault
option add *anchor		w 			userDefault
option add *selectBackground	$tixOption(select_bg)	userDefault
option add *selectForeground	$tixOption(select_fg)	userDefault
option add *selectBorderWidth	0			userDefault
# buttons
#--------
option add  *Button.borderWidth	$tixOption(border1) 	userDefault
option add  *Button.anchor	c 			userDefault

# check buttons
#--------------
option add *Checkbutton.selector	$tixOption(selector)	userDefault
option add *Checkbutton.borderWidth	$tixOption(border1) 	userDefault

# radio buttons
#--------------
option add *Radiobutton.selector	$tixOption(selector)	userDefault
option add *Radiobutton.borderWidth	$tixOption(border1) 	userDefault

# entries
#--------
option add *Entry.relief		sunken			 userDefault
option add *Entry.background		$tixOption(input1_bg)	 userDefault
option add *Entry.foreground		black			 userDefault
option add *Entry.insertBackground	black			 userDefault

# canvas
#-------

# frame
#------

# labels
#-------
option add *Label.anchor		w			userDefault
option add *Label.borderWidth		0			userDefault
option add *Label.font                  $tixOption(bold_font)	userDefault

# listboxs
#---------
option add *Listbox.background		$tixOption(dark1_bg)	userDefault
option add *Listbox.relief		sunken 			userDefault
 
# menus
#------
option add *Menu*font			$tixOption(menu_font)	userDefault
option add *Menu*selector		$tixOption(selector)	userDefault
 
# menus buttons
#--------------
option add *Menubutton*font		$tixOption(menu_font)	userDefault

# scales
#-------
option add *Scale.foreground		$tixOption(fg)		userDefault
option add *Scale.activeForeground	$tixOption(bg)		userDefault
option add *Scale.background		$tixOption(bg)		userDefault
option add *Scale.sliderForeground	$tixOption(bg)		userDefault
option add *Scale.sliderBackground	$tixOption(dark1_bg)	userDefault
option add *Scale.font			$tixOption(italic_font)	userDefault
 
# scrollbars
option add *Scrollbar.foreground	$tixOption(bg)		userDefault
option add *Scrollbar.activeForeground	$tixOption(bg) 		userDefault
option add *Scrollbar.background	$tixOption(dark1_bg) 	userDefault
option add *Scrollbar.relief		sunken 			userDefault

# texts
option add *Text.background		$tixOption(input1_bg)	userDefault
option add *Text.relief			sunken			userDefault
option add *Text.borderWidth		$tixOption(border1)	userDefault
#----------------------------------------------------------------------
# TIX WIDGETS
#----------------------------------------------------------------------
# GENERAL
option add *listboxBg			$tixOption(dark1_bg)	userDefault
option add *listboxFont			$tixOption(font)	userDefault
option add *entryBg			$tixOption(input1_bg)	userDefault
option add *entryFont			$tixOption(font)	userDefault
option add *buttonBg			$tixOption(bg)		userDefault
option add *scrollbarFg			$tixOption(bg)		userDefault
option add *scrollbarBg			$tixOption(dark1_bg) 	userDefault


# CONTROL
option add *TixControl.value 		0			userDefault
option add *TixControl.borderWidth 	1			userDefault
option add *TixControl.relief 	        sunken			userDefault
option add *TixControl.labelFont        $tixOption(bold_font)	userDefault


# COMBOBOX
option add *TixCombobox.entryFont	$tixOption(font) 	userDefault


# DLG_BTNS
option add *TixDlgBtns.borderWidth 	1			userDefault
option add *TixDlgBtns.relief 		raised			userDefault
option add *TixDlgBtns.anchor 		c			userDefault
option add *TixDlgBtns.padx 		10			userDefault
option add *TixDlgBtns.pady 		10			userDefault
option add *TixDlgBtns.buttonbg		$tixOption(bg)		userDefault


# FILE_SELECT_BOX
option add *TixFileSelectbox.relief 	raised			userDefault
option add *TixFileSelectbox.borderWidth 1			userDefault
option add *TixFileSelectbox.font 	$tixOption(bold_font)	userDefault


# LABEL_FRAME
option add *TixLabelFrame.font 		$tixOption(bold_font)	userDefault
option add *TixLabelFrame.anchor 	center			userDefault
option add *TixLabelFrame.borderWidth 	2			userDefault
option add *TixLabelFrame.padX 		2			userDefault
option add *TixLabelFrame.padY 		2			userDefault
option add *TixLabelFrame.anchor 	sw			userDefault


# POPUP MENU
option add *TixPopupMenu*Menubutton.background $tixOption(dark1_bg) userDefault


# SCROLLED_LISTBOX
option add *TixScrolledListbox.scbWidth 15			userDefault


# SELECT
option add *TixSelect.borderWidth 	2			userDefault
option add *TixSelect.relief      	sunken			userDefault
option add *TixSelect.buttonBg      	$tixOption(bg)		userDefault
option add *TixSelect.selectedBg      	$tixOption(dark1_bg)	userDefault


# STD_DLG_BTNS
option add *TixStdDlgBtns.borderWidth 	1			userDefault
option add *TixStdDlgBtns.buttonBg	$tixOption(bg)		userDefault
option add *TixStdDlgBtns.relief 	raised			userDefault


}
