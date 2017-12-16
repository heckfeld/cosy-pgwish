#
# Copyright (c) 1993 by Sven Delmas
# All rights reserved.
# See the file COPYRIGHT for the copyright notes.

global tksteal_priv
set tksteal_priv(debug) 0

# this proc is taken from the beta code of the toolbox
 proc tkSteal_getarg {opts arg var} {# xf ignore me 8
  set ind [lsearch $opts [concat $arg *]]
  if {$ind==-1} {
    error "ARG ERROR: application tried to find nonexistend arg \"$arg\"!"
  }
  # The variable named var will contain the value.
  upvar 1 $var varr
  set varr [lindex [lindex $opts $ind] 4]
  if {"[lindex [lindex $opts $ind] 3]" ==
      "[lindex [lindex $opts $ind] 4]"} {
    return 0
  } {
    return 1
  }
}

# this proc is taken from the beta code of the toolbox
 proc tkSteal_parseargs {cmdline descr args} {# xf ignore me 8
  if {$args!=""} {upvar 1 [lindex $args 0] changed}
  set changed {}
  for {set i 0} {$i<[llength $cmdline]} {incr i} {
    set name [lindex $cmdline $i]
    while {[llength $name]>0} {
      if {[llength $name]==1} {
        set value [lindex $cmdline [expr $i+1]]
        incr i
      } else {
        set value [lindex $name 1]
        set name [lindex $name 0]
      }
      # Find option
      set ind [lsearch $descr [concat $name *]]
      if {$ind==-1} {
        set ind [lsearch $descr $name]
        if {$ind==-1} {
          error "unknown option \"$name\""
        }
      }
      lappend changed $name
      # Enter value
      set descr [lreplace $descr $ind $ind \
        [lreplace [lindex $descr $ind] 4 4 $value]]
      set name [lreplace $name 0 1]
    }
  }
  # New optlist
  set new {}
  foreach item $descr {
    if {[llength $item]<=5} {
      lappend new $item
    } else {
      # XXX Maybe should demand the default to be in ready format...
      # So the exec. time might be a LITTLE faster.
      #
      # Eval the type getting actions.
      set act [lindex $item 5]
      regsub -all "%\(value\)" $act [lindex $item 3] act
      set item [lreplace $item 3 3 [eval $act]]
      set act [lindex $item 5]
      regsub -all "%\(value\)" $act [lindex $item 4] act
      lappend new [lreplace $item 4 4 [eval $act]]
    }
  }
  return $new
}

global tksteal_ARGUMENTS
set tksteal_ARGUMENTS {
{-background    background    Background   #ffe4c4         #ffe4c4}
{-borderwidth   borderWidth   BorderWidth  0               0}
{-command       command       Command      {} {}}
{-foreground    foreground    Foreground   black           black}
{-font          font          Font         *-Courier-Medium-R-Normal-*-120-* *-Courier-Medium-R-Normal-*-120-*}
{-name          name          Name         {}              {}}
{-height        height        Height       20              20}
{-relief        relief        Relief       flat            flat}
{-stealwindowid stealWindowId WindowId     0               0}
{-width         width         Width        20              20}
{-windowid      windowId      WindowId     0               0}}

proc tksteal {pathName args} {# xf ignore me 8
  global tk_library
  global tksteal_priv
  global tksteal_ARGUMENTS

  # check if interpreter can access X
  catch "xaccess xwinparent -widget ."
  if {"[info commands xaccess]" == ""} {
    if {"[info commands load]" != ""} {
      if {[catch "load $tk_library/libtkXAccess.so"]} {
        if {[catch "load ./libtkXAccess.so"]} {
          error "tksteal the command xaccess is not available"
        }
      }
    } {
      error "tksteal the command xaccess is not available"
    }
  }

  # initialize the data
  set tksteal_priv($pathName,initialized) 0
  set tksteal_priv($pathName,pid) 0
  set tksteal_priv($pathName,config) \
    [tkSteal_parseargs $args $tksteal_ARGUMENTS]

  # main frame
  frame $pathName -borderwidth 0 -class TkSteal
  rename $pathName $pathName-internal
  proc $pathName args "# xf ignore me 8
return \[eval tksteal_command $pathName \$args\]"

  label $pathName.xfl -borderwidth 0 -padx 1 -pady 1
  rename $pathName.xfl $pathName-xflinternal
  proc $pathName.xfl args "# xf ignore me 8
return \[eval tksteal_command $pathName \$args\]"
  pack append $pathName $pathName.xfl {top fill expand}

  bind $pathName.xfl <Map> "# xf ignore me 8
    tksteal_start $pathName"
  bind $pathName.xfl <Destroy> "# xf ignore me 8
    tksteal_stop $pathName"
  bind $pathName.xfl <Expose> "# xf ignore me 8
    if {\$tksteal_priv($pathName,initialized)} {
      $pathName redisplay
    }"
  bind $pathName.xfl <Configure> "# xf ignore me 8
    if {\$tksteal_priv($pathName,initialized)} {
      $pathName redisplay
    }"

  eval $pathName config $args
}

 proc tksteal_start {pathName} {# xf ignore me 8
  global env
  global tksteal_priv

  if {$tksteal_priv($pathName,initialized) || "$pathName" == ""} {
    return
  }

  tkSteal_getarg $tksteal_priv($pathName,config) -background background
  tkSteal_getarg $tksteal_priv($pathName,config) -borderwidth borderwidth
  tkSteal_getarg $tksteal_priv($pathName,config) -foreground foreground
  tkSteal_getarg $tksteal_priv($pathName,config) -font font
  tkSteal_getarg $tksteal_priv($pathName,config) -command command
  tkSteal_getarg $tksteal_priv($pathName,config) -name name
  tkSteal_getarg $tksteal_priv($pathName,config) -height height
  tkSteal_getarg $tksteal_priv($pathName,config) -relief relief
  tkSteal_getarg $tksteal_priv($pathName,config) -width width
  tkSteal_getarg $tksteal_priv($pathName,config) -stealwindowid stealwindowid
  tkSteal_getarg $tksteal_priv($pathName,config) -windowid windowid

  # run command
  if {"$command" != ""} {
    set newCommand $command
    regsub -all {\$background} $newCommand $background newCommand
#    regsub -all {\$borderwidth} $newCommand $borderwidth newCommand
    regsub -all {\$foreground} $newCommand $foreground newCommand
    regsub -all {\$font} $newCommand $font newCommand
    regsub -all {\$relief} $newCommand $relief newCommand
    regsub -all {\$width} $newCommand $width newCommand
    regsub -all {\$height} $newCommand $height newCommand
    regsub -all {\$name} $newCommand $name newCommand
    regsub -all {\$windowid} $newCommand $windowid newCommand
    regsub -all {\$stealwindowid} $newCommand [format %ld [winfo id $pathName.xfl]] newCommand
    regsub -all {\$\{background\}} $newCommand $background newCommand
#    regsub -all {\$\{borderwidth\}} $newCommand $borderwidth newCommand
    regsub -all {\$\{foreground\}} $newCommand $foreground newCommand
    regsub -all {\$\{font\}} $newCommand $font newCommand
    regsub -all {\$\{relief\}} $newCommand $relief newCommand
    regsub -all {\$\{width\}} $newCommand $width newCommand
    regsub -all {\$\{height\}} $newCommand $height newCommand
    regsub -all {\$\{name\}} $newCommand $name newCommand
    regsub -all {\$\{windowid\}} $newCommand $windowid newCommand
    regsub -all {\$\{stealwindowid\}} $newCommand [format %ld [winfo id $pathName.xfl]] newCommand
    if {[catch "eval exec $newCommand &" result]} {
      error $result
    } {
      set tksteal_priv($pathName,pid) $result
    }
  }

  $pathName-internal conf -background $background
  $pathName-internal conf -borderwidth $borderwidth
  $pathName-internal conf -relief $relief
  $pathName-xflinternal conf -background $background
  $pathName-xflinternal conf -padx 0
  $pathName-xflinternal conf -pady 0
  if {"$width" != "" && $width != 0} {
    $pathName-xflinternal conf -width $width
  }
  if {"$height" != "" && $height != 0} {
    $pathName-xflinternal conf -height $height
  }
  $pathName conf -stealwindowid [format %ld [winfo id $pathName.xfl]]

  if {"$name" != ""} {
    set timeout 0
    while {$timeout < 1000} {
      set windowids [xaccess xwinchilds -windowid [xaccess xwinroot -widget .]]
      foreach window $windowids {
        if {[string match $name [xaccess xwinname -windowid $window]]} {
          $pathName conf -windowid $window
          catch "xaccess eventreparent -parentwidget $pathName.xfl -windowid $window"
          set tksteal_priv($pathName,initialized) 1
          after 2000 $pathName redisplay
          return
        }
        foreach childwindow [xaccess xwinchilds -windowid $window] {
          if {[string match $name [xaccess xwinname -windowid $childwindow]]} {
            $pathName conf -windowid $childwindow
            catch "xaccess eventreparent -parentwidget $pathName.xfl -windowid $childwindow"
            set tksteal_priv($pathName,initialized) 1
            after 2000 $pathName redisplay
            return
          }
          foreach childwindow2 [xaccess xwinchilds -windowid $childwindow] {
            if {[string match $name [xaccess xwinname -windowid $childwindow2]]} {
              $pathName conf -windowid $childwindow2
              catch "xaccess eventreparent -parentwidget $pathName.xfl -windowid $childwindow2"
              set tksteal_priv($pathName,initialized) 1
              after 2000 $pathName redisplay
              return
            }
          }
        }
      }
      incr timeout
    }
  }
  set tksteal_priv($pathName,initialized) 1
  after 2000 $pathName redisplay
}

 proc tksteal_stop {pathName} {# xf ignore me 8
  global tksteal_priv

  rename $pathName-internal ""
  rename $pathName-xflinternal ""
  if {$tksteal_priv($pathName,pid) != 0} {
    catch "exec kill $tksteal_priv($pathName,pid)"
    set tksteal_priv($pathName,pid) 0
  }
  set tksteal_priv($pathName,initialized) 0
  return ""
}

 proc tksteal_command {pathName minorCommand args} {# xf ignore me 8
  global tksteal_priv

  case $minorCommand {
    {conf*} {
      if {[llength $args] == 0} {
        set result ""
        foreach element $tksteal_priv($pathName,config) {
          lappend result [list [lindex $element 0] [lindex $element 1] [lindex $element 2] [lindex $element 3] [lindex $element 4]]
        }
        return $result
      } {
        if {[llength $args] == 1} {
          set resourceIndex [lsearch $tksteal_priv($pathName,config) $args*]
          if {$resourceIndex != -1} {
            set element [lindex $tksteal_priv($pathName,config) $resourceIndex]
            return [list [lindex $element 0] [lindex $element 1] [lindex $element 2] [lindex $element 3] [lindex $element 4]]
          } {
            error "unknown resource: $args"
          }
        } {
          set tksteal_priv($pathName,config) \
            [tkSteal_parseargs $args $tksteal_priv($pathName,config)]
          for {set counter 0} {$counter < [llength $args]} {incr counter} {
            case [lindex $args $counter] {
              {-back*} {
                tkSteal_getarg $tksteal_priv($pathName,config) \
                  -background background
                $pathName-internal conf -background $background
                $pathName-xflinternal conf -background $background
              }
              {-border*} {
                tkSteal_getarg $tksteal_priv($pathName,config) \
                  -borderwidth borderwidth
                $pathName-internal conf -borderwidth $borderwidth
              }
              {-reli*} {
                tkSteal_getarg $tksteal_priv($pathName,config) \
                  -relief relief
                $pathName-internal conf -relief $relief
              }
              {-heig*} {
                tkSteal_getarg $tksteal_priv($pathName,config) \
                  -height height
                $pathName-xflinternal conf -height $height
              }
              {-widt*} {
                tkSteal_getarg $tksteal_priv($pathName,config) \
                  -width width
                $pathName-xflinternal conf -width $width
              }
            }
            incr counter
          }
        }
      }
    }
    {redi*} {
      if {$tksteal_priv($pathName,initialized)} {
        tkSteal_getarg $tksteal_priv($pathName,config) \
          -windowid windowid
        catch "xaccess eventconfigure \
          -windowid $windowid \
          -borderwidth 0 \
          -x 0 \
          -y 0 \
          -width [winfo width $pathName] \
          -height [winfo height $pathName]"
      }
    }
    {buttonclick} {
      if {[llength $args] != 1 && [llength $args] != 2} {
        error "wrong # of args: should be \"$pathName buttonclick button ?state?\""
      }
      tkSteal_getarg $tksteal_priv($pathName,config) \
        -windowid windowid
      if {[llength $args] == 1} {
        catch "xaccess eventbuttonpress -windowid $windowid -button [lindex $args 0]"
        catch "xaccess eventbuttonrelease -windowid $windowid -button [lindex $args 0]"
      } {
        catch "xaccess eventbuttonpress -windowid $windowid -button [lindex $args 0] -state [lindex $args 1]"
        catch "xaccess eventbuttonrelease -windowid $windowid -button [lindex $args 0] -state [lindex $args 1]"
      }
    }
    {buttonpress} {
      if {[llength $args] != 1 && [llength $args] != 2} {
        error "wrong # of args: should be \"$pathName buttonpress button ?state?\""
      }
      tkSteal_getarg $tksteal_priv($pathName,config) \
        -windowid windowid
      if {[llength $args] == 1} {
        catch "xaccess eventbuttonpress -windowid $windowid -button [lindex $args 0]"
      } {
        catch "xaccess eventbuttonpress -windowid $windowid -button [lindex $args 0] -state [lindex $args 1]"
      }
    }
    {buttonrelease} {
      if {[llength $args] != 1 && [llength $args] != 2} {
        error "wrong # of args: should be \"$pathName buttonpress button ?state?\""
      }
      tkSteal_getarg $tksteal_priv($pathName,config) \
        -windowid windowid
      if {[llength $args] == 1} {
        catch "xaccess eventbuttonrelease -windowid $windowid -button [lindex $args 0]"
      } {
        catch "xaccess eventbuttonrelease -windowid $windowid -button [lindex $args 0] -state [lindex $args 1]"
      }
    }
    {keyclick} {
      if {[llength $args] != 1 && [llength $args] != 2} {
        error "wrong # of args: should be \"$pathName keytype keycode ?state?\""
      }
      tkSteal_getarg $tksteal_priv($pathName,config) \
        -windowid windowid
      if {[llength $args] == 1} {
        catch "xaccess eventkeypress -windowid $windowid -keycode [lindex $args 0]"
        catch "xaccess eventkeyrelease -windowid $windowid -keycode [lindex $args 0]"
      } {
        catch "xaccess eventkeypress -windowid $windowid -keycode [lindex $args 0] -state [lindex $args 1]"
        catch "xaccess eventkeyrelease -windowid $windowid -keycode [lindex $args 0] -state [lindex $args 1]"
      }
    }
    {keypress} {
      if {[llength $args] != 1 && [llength $args] != 2} {
        error "wrong # of args: should be \"$pathName keypress keycode ?state?\""
      }
      tkSteal_getarg $tksteal_priv($pathName,config) \
        -windowid windowid
      if {[llength $args] == 1} {
        catch "xaccess eventkeypress -windowid $windowid -keycode [lindex $args 0]"
      } {
        catch "xaccess eventkeypress -windowid $windowid -keycode [lindex $args 0] -state [lindex $args 1]"
      }
    }
    {keyrelease} {
      if {[llength $args] != 1 && [llength $args] != 2} {
        error "wrong # of args: should be \"$pathName keyrelease keycode ?state?\""
      }
      tkSteal_getarg $tksteal_priv($pathName,config) \
        -windowid windowid
      if {[llength $args] == 1} {
        catch "xaccess eventkeyrelease -windowid $windowid -keycode [lindex $args 0]"
      } {
        catch "xaccess eventkeyrelease -windowid $windowid -keycode [lindex $args 0] -state [lindex $args 1]"
      }
    }
    {sendstring} {
      if {[llength $args] != 1 && [llength $args] != 2} {
        error "wrong # of args: should be \"$pathName sendstring string ?interval?\""
      }
      tkSteal_getarg $tksteal_priv($pathName,config) \
        -windowid windowid
      if {[llength $args] == 1} {
        catch "xaccess sendstring -windowid $windowid -data \{[lindex $args 0]\}"
      } {
        catch "xaccess sendstring -windowid $windowid -data \{[lindex $args 0]\} -interval [lindex $args 1]"
      }
    }
    {motion} {
      if {[llength $args] != 1 && [llength $args] != 2} {
        error "wrong # of args: should be \"$pathName motion x y\""
      }
      tkSteal_getarg $tksteal_priv($pathName,config) \
        -windowid windowid
      catch "xaccess eventmotion -windowid $windowid -x [lindex $args 0] -y [lindex $args 1]"
    }
    {stoptksteal} {
      if {[llength $args] != 0} {
        error "wrong # of args: should be \"$pathName stoptksteal\""
      }
      tksteal_stop $pathName
    }
  }
  return ""
}

 proc tksteal_redisplay {pathName} {
  global tksteal_priv

  if {$tksteal_priv($pathName,initialized)} {
    $pathName redisplay
  }
}

# eof

