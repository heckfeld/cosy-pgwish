#
# Copyright (c) 1993 by Sven Delmas
# All rights reserved.
# See the file COPYRIGHT for the copyright notes.

global tkgs_priv
set tkgs_priv(debug) 0

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

global tkgs_ARGUMENTS
set tkgs_ARGUMENTS {
{-background  background   Background   #ffe4c4         #ffe4c4}
{-bbox        bbox         Bbox         {0 0 1000 1000} {0 0 1000 1000}}
{-borderwidth borderWidth  BorderWidth  0               0}
{-command     command      Command      {gs -dQUIET -dNOPAUSE -g${width}x${height} $file} {gs -dQUIET -dNOPAUSE -g${width}x${height} $file}}
{-file        file         File         {}              {}}
{-height      height       Height       20              20}
{-orient      orient       Orientation  0               0}
{-relief      relief       Relief       flat            flat}
{-width       width        Width        20              20}
{-xdpi        xDpi         XDpi         100             100}
{-ydpi        yDpi         YDpi         100             100}}

proc tkgs {pathName args} {# xf ignore me 8
  global tk_library
  global tkgs_priv
  global tkgs_ARGUMENTS

  # check if interpreter can access X
  catch "xaccess xwinparent -widget ."
  if {"[info commands xaccess]" == ""} {
    if {"[info commands load]" != ""} {
      if {[catch "load $tk_library/libtkXAccess.so"]} {
        if {[catch "load ./libtkXAccess.so"]} {
          error "tkgs: the command xaccess is not available"
        }
      }
    } {
      error "tkgs: the command xaccess is not available"
    }
  }

  # check if interpreter can handle pixmaps
  if {"[info commands pinfo]" == ""} {
    error "tkgs: the command pinfo is not available"
  }

  # initialize the data
  set tkgs_priv($pathName,initialized) 0
  set tkgs_priv($pathName,pid) 0
  set tkgs_priv($pathName,targetWindow) 0
  set tkgs_priv($pathName,config) \
    [tkSteal_parseargs $args $tkgs_ARGUMENTS]

  # main frame
  frame $pathName -borderwidth 0 -class TkGS
  rename $pathName $pathName-internal
  proc $pathName args "# xf ignore me 8
return \[eval tkgs_command $pathName \$args\]"

  label $pathName.xfl -borderwidth 0 -padx 1 -pady 1
  rename $pathName.xfl $pathName-xflinternal
  proc $pathName.xfl args "# xf ignore me 8
return \[eval tkgs_command $pathName \$args\]"
  pack append $pathName $pathName.xfl {top fill expand}

  bind $pathName.xfl <Map> "# xf ignore me 8
    tkgs_start $pathName"
  bind $pathName.xfl <Destroy> "# xf ignore me 8
    tkgs_stop $pathName"
  bind $pathName.xfl <Expose> "# xf ignore me 8
    if {\$tkgs_priv($pathName,initialized)} {
      $pathName redisplay
    }"
  bind $pathName.xfl <Configure> "# xf ignore me 8
    if {\$tkgs_priv($pathName,initialized)} {
      $pathName redisplay
    }"
  bind $pathName.xfl <Property> "# xf ignore me 8
    if {\$tkgs_priv($pathName,initialized)} {
      $pathName redisplay
      after 4000 tkgs_redisplay $pathName
    }"

  eval $pathName config $args
}

 proc tkgs_start {pathName} {# xf ignore me 8
  global env
  global tkgs_priv

  if {$tkgs_priv($pathName,initialized) || "$pathName" == ""} {
    return
  }

  tkSteal_getarg $tkgs_priv($pathName,config) -bbox bbox
  tkSteal_getarg $tkgs_priv($pathName,config) -background background
  tkSteal_getarg $tkgs_priv($pathName,config) -borderwidth borderwidth
  tkSteal_getarg $tkgs_priv($pathName,config) -command command
  tkSteal_getarg $tkgs_priv($pathName,config) -file file
  tkSteal_getarg $tkgs_priv($pathName,config) -height height
  tkSteal_getarg $tkgs_priv($pathName,config) -orient orient
  tkSteal_getarg $tkgs_priv($pathName,config) -relief relief
  tkSteal_getarg $tkgs_priv($pathName,config) -width width
  tkSteal_getarg $tkgs_priv($pathName,config) -xdpi xdpi
  tkSteal_getarg $tkgs_priv($pathName,config) -ydpi ydpi

  if {"$command" == "" || ![auto_execok [lindex $command 0]]} {
    error "tkgs: no valid gs command specified \"$command\""
  }
  if {$width <= 0 || $height <= 0} {
    error "tkgs: no correct size specified \"$widthx$height\""
  }
  if {[llength $bbox] != 4} {
    error "tkgs: no correct bbox specified \"$bbox\""
  }

  set data [list [list $width $height 1 1] [list . c white]]
  for {set j 0} {$j < $height} {incr j} {
    append dataline "."
  }
  for {set i 0} {$i < $width} {incr i} {
    append data " {$dataline}"
  }
  $pathName-xflinternal conf -bitmap {}
  catch "pinfo undefine $pathName-bitmap"
  pinfo define $pathName-bitmap [list [list $width $height 0 xpm3]]

  # run gs
  if {[file exists $file]} {
    set env(GHOSTVIEW) "[format %ld [winfo id $pathName.xfl]] [pinfo get $pathName-bitmap]"
    catch "xaccess propchange -widget $pathName.xfl \
      -propertyname GHOSTVIEW -format 32 \
      -data \"0 $orient $bbox $xdpi $ydpi 0 0 0 0 $width $height\""
    set newCommand $command
    regsub -all {\$width} $newCommand $width newCommand
    regsub -all {\$height} $newCommand $height newCommand
    regsub -all {\$file} $newCommand $file newCommand
    regsub -all {\$\{width\}} $newCommand $width newCommand
    regsub -all {\$\{height\}} $newCommand $height newCommand
    regsub -all {\$\{file\}} $newCommand $file newCommand
    if {[catch "eval exec $newCommand &" result]} {
      error $result
    } {
      set tkgs_priv($pathName,pid) $result
    }
  }

  $pathName-internal conf -background $background
  $pathName-internal conf -borderwidth $borderwidth
  $pathName-internal conf -relief $relief
  $pathName-xflinternal conf -background $background
  $pathName-xflinternal conf -padx 0
  $pathName-xflinternal conf -pady 0

  set tkgs_priv($pathName,initialized) 1
  after 2000 $pathName redisplay
}

 proc tkgs_stop {pathName} {# xf ignore me 8
  global tkgs_priv

  rename $pathName-internal ""
  rename $pathName-xflinternal ""
  if {$tkgs_priv($pathName,pid) != 0} {
    catch "exec kill $tkgs_priv($pathName,pid)"
    set tkgs_priv($pathName,pid) 0
  }
  catch "pinfo undefine $pathName-bitmap"
  set tkgs_priv($pathName,initialized) 0
  return ""
}

 proc tkgs_command {pathName minorCommand args} {# xf ignore me 8
  global tkgs_priv

  set restartGS 0
  case $minorCommand {
    {conf*} {
      if {[llength $args] == 0} {
        set result ""
        foreach element $tkgs_priv($pathName,config) {
          lappend result [list [lindex $element 0] [lindex $element 1] [lindex $element 2] [lindex $element 3] [lindex $element 4]]
        }
        return $result
      } {
        if {[llength $args] == 1} {
          set resourceIndex [lsearch $tkgs_priv($pathName,config) $args*]
          if {$resourceIndex != -1} {
            set element [lindex $tkgs_priv($pathName,config) $resourceIndex]
            return [list [lindex $element 0] [lindex $element 1] [lindex $element 2] [lindex $element 3] [lindex $element 4]]
          } {
            error "unknown resource: $args"
          }
        } {
          if {[lsearch $args {-bbox}] != -1 ||
              [lsearch $args {-command}] != -1 ||
              [lsearch $args {-file}] != -1 ||
              [lsearch $args {-height}] != -1 ||
              [lsearch $args {-orient}] != -1 ||
              [lsearch $args {-width}] != -1 ||
              [lsearch $args {-xdpi}] != -1 ||
              [lsearch $args {-ydpi}] != -1} {
            set restartGS 1
          }
          set tkgs_priv($pathName,config) \
            [tkSteal_parseargs $args $tkgs_priv($pathName,config)]
          for {set counter 0} {$counter < [llength $args]} {incr counter} {
            case [lindex $args $counter] {
              {-back*} {
                tkSteal_getarg $tkgs_priv($pathName,config) \
                  -background background
                $pathName-internal conf -background $background
                $pathName-xflinternal conf -background $background
              }
              {-border*} {
                tkSteal_getarg $tkgs_priv($pathName,config) \
                  -borderwidth borderwidth
                $pathName-internal conf -borderwidth $borderwidth
              }
              {-reli*} {
                tkSteal_getarg $tkgs_priv($pathName,config) \
                  -relief relief
                $pathName-internal conf -relief $relief
              }
            }
            incr counter
          }
        }
      }
    }
    {next} {
      if {$tkgs_priv($pathName,initialized) &&
          $tkgs_priv($pathName,targetWindow)} {
        catch "xaccess eventclientmessage -windowid $tkgs_priv($pathName,targetWindow) -format 32 -messagetypename NEXT"
      }
    }
    {redi*} {
      if {$tkgs_priv($pathName,initialized)} {
        $pathName-xflinternal config -bitmap $pathName-bitmap
      } {
        after 1000 $pathName redisplay
      }
    }
  }
  if {$restartGS && $tkgs_priv($pathName,initialized)} {
    tkgs_stop $pathName
    tkgs_start $pathName
  }
  return ""
}

 proc tkgs_redisplay {pathName} {
  global tkgs_priv

  if {$tkgs_priv($pathName,initialized)} {
    $pathName redisplay
  }
}

# eof

