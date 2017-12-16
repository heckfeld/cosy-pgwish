#
# Copyright (c) 1993 by Sven Delmas
# All rights reserved.
# See the file COPYRIGHT for the copyright notes.

global tkemacs_priv
set tkemacs_priv(debug) 0

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

global tkemacs_ARGUMENTS
set tkemacs_ARGUMENTS {
{-background        background        Background        #ffe4c4       #ffe4c4}
{-borderwidth       borderWidth       BorderWidth       0             0}
{-cursor            cursor            Cursor            xterm         xterm}
{-debug             debug             Debug             0             0}
{-exportselection   exportSelection   ExportSelection   1             1}
{-font              font              Font              *-Courier-Medium-R-Normal-*-120-* *-Courier-Medium-R-Normal-*-120-*}
{-foreground        foreground        Foreground        black         black}
{-insertbackground  insertBackground  Foreground        black         black}
{-insertborderwidth insertBorderWidth BorderWidth       0             0}
{-insertofftime     insertOffTime     OffTime           300           300}
{-insertontime      insertOnTime      OnTime            600           600}
{-insertwidth       insertWidth       InsertWidth       2             2}
{-padx              padX              Pad               1             1}
{-pady              padY              Pad               1             1}
{-relief            relief            Relief            flat          flat}
{-selectbackground  selectBackground  Foreground        #b2dfee       #b2dfee}
{-selectborderwidth selectBorderwidth Borderwidth       1             1}
{-selectforeground  selectForeground  Background        black         black}
{-setgrid           setGrid           SetGrid           1             1}
{-state             state             State             normal        normal}
{-wrap              wrap              Wrap              char          char}
{-xscrollcommand    xScrollCommand    ScrollCommand     {}            {}}
{-yscrollcommand    yScrollCommand    ScrollCommand     {}            {}}

{-command           command           Command           {TkEmacs}     {TkEmacs}}
{-endsource         endSource         EndSource         {}            {}}
{-errorcallback     errorCallback     ErrorCallback     {}            {}}
{-file              file              File              {}            {}}
{-geometry          geometry          Geometry          {}            {}}
{-height            height            Height            300           300}
{-lispfile          lispFile          LispFile          {tkemacs.el}  {tkemacs.el}}
{-name              name              Name              {TKEMACS*}    {TKEMACS*}}
{-pollinterval      pollInterval      PollInterval      {}            {}}
{-reparent          reparent          Reparent          1             1}
{-startupsource     startupSource     StartupSource     {}            {}}
{-timeout           timeout           Timeout           10000         10000}
{-useadvise         useAdvise         UseAdvise         0             0}
{-width             width             Width             550           550}}

proc tkemacs {pathName args} {# xf ignore me 8
  global tk_library
  global tkemacs_priv
  global tkemacs_ARGUMENTS

  # initialize the data
  set tkemacs_priv(prefix) ""
  set tkemacs_priv($pathName,initialized) 0
  set tkemacs_priv($pathName,emacsAvailable) 0
  set tkemacs_priv($pathName,windowId) 0
  set tkemacs_priv($pathName,pid) 0
  set tkemacs_priv($pathName,handle) ""
  set tkemacs_priv($pathName,config) \
    [tkSteal_parseargs $args $tkemacs_ARGUMENTS]
  tkSteal_getarg $tkemacs_priv($pathName,config) \
    -reparent reparent

  # create parent widget
  if {$reparent} {
    frame $pathName -borderwidth 0 -class TkEmacs
    rename $pathName $pathName-internal
  }
  proc $pathName args "# xf ignore me 8
return \[eval tkemacs_command $pathName \$args\]"

  if {$reparent} {
    frame $pathName.xfl -borderwidth 0 -width 1 -height 1
    rename $pathName.xfl $pathName-xflinternal
    pack append $pathName $pathName.xfl {top fill expand}
  }
  proc $pathName.xfl args "# xf ignore me 8
return \[eval tkemacs_command $pathName \$args\]"

  if {$reparent} {
    # check if interpreter can access X
    catch "xaccess xwinparent -widget ."
    if {"[info commands xaccess]" == ""} {
      if {"[info commands load]" != ""} {
        if {[catch "load $tk_library/libtkXAccess.so"]} {
          if {[catch "load ./libtkXAccess.so"]} {
            error "tkemacs: the command xaccess is not available"
          }
        }
      } {
        error "tkemacs: the command xaccess is not available"
      }
    }
  }

  if {$reparent} {
    bind $pathName <Map> "# xf ignore me 8
      tkemacs_start %W $args"
    bind $pathName <Destroy> {# xf ignore me 8
      tkemacs_stop %W}
    bind $pathName <Expose> {# xf ignore me 8
      if {"$tkemacs_priv(%W,windowId)" != "" &&
          $tkemacs_priv(%W,windowId) != 0 &&
          $tkemacs_priv(%W,emacsAvailable) != 0} {
        catch "xaccess eventexpose \
          -windowid $tkemacs_priv(%W,windowId)"
      }}
    bind $pathName <Configure> {# xf ignore me 8
      if {"$tkemacs_priv(%W,windowId)" != "" &&
          $tkemacs_priv(%W,windowId) != 0 &&
          $tkemacs_priv(%W,emacsAvailable) != 0} {
        catch "xaccess eventconfigure \
          -windowid $tkemacs_priv(%W,windowId) \
          -borderwidth 0 \
          -x 0 \
          -y 0 \
          -width [winfo width %W] \
          -height [winfo height %W]"
      }}
  } {
    eval tkemacs_start $pathName $args
  }
}

 proc tkemacs_start {pathName args} {# xf ignore me 8
  global dp_version
  global tkemacs_priv

  if {$tkemacs_priv($pathName,initialized) || "$pathName" == ""} {
    return
  }

  tkSteal_getarg $tkemacs_priv($pathName,config) \
    -useadvise useadvise
  tkSteal_getarg $tkemacs_priv($pathName,config) \
    -background background
  tkSteal_getarg $tkemacs_priv($pathName,config) \
    -borderwidth borderwidth
  tkSteal_getarg $tkemacs_priv($pathName,config) \
    -command command
  tkSteal_getarg $tkemacs_priv($pathName,config) \
    -lispfile lispfile
  tkSteal_getarg $tkemacs_priv($pathName,config) \
    -startupsource startupsource
  tkSteal_getarg $tkemacs_priv($pathName,config) \
    -background background
  tkSteal_getarg $tkemacs_priv($pathName,config) \
    -insertbackground insertbackground
  tkSteal_getarg $tkemacs_priv($pathName,config) \
    -foreground foreground
  tkSteal_getarg $tkemacs_priv($pathName,config) \
    -font font
  tkSteal_getarg $tkemacs_priv($pathName,config) \
    -relief relief
  tkSteal_getarg $tkemacs_priv($pathName,config) \
    -selectbackground selectbackground
  tkSteal_getarg $tkemacs_priv($pathName,config) \
    -selectforeground selectforeground
  tkSteal_getarg $tkemacs_priv($pathName,config) \
    -debug debug
  tkSteal_getarg $tkemacs_priv($pathName,config) \
    -width width
  tkSteal_getarg $tkemacs_priv($pathName,config) \
    -height height
  tkSteal_getarg $tkemacs_priv($pathName,config) \
    -reparent reparent
  set tkemacs_priv(debug) $debug

  if {"$command" == "" ||
      (![auto_execok [lindex $command 0]] &&
       ![file executable [lindex $command 0]])} {
    error "tkemacs: no valid emacs command specified \"$command\""
  }

  # create file handle procedures
  set newBody [info body tkemacs_accept]
  regsub -all WIDGET_NAME $newBody $pathName newBody
  eval "proc tkemacs_accept_$pathName \{condition fileId\} \{$newBody\}"
  set newBody [info body tkemacs_read]
  regsub -all WIDGET_NAME $newBody $pathName newBody
  eval "proc tkemacs_read_$pathName \{condition fileId\} \{$newBody\}"

  # establish connection
  catch "dp_filehandler stdin"
  catch "filehandler stdin"
  if {"[info commands dp_connect]" != "" &&
      "[info commands dp_filehandler]" != ""} {
    set tkemacs_priv(prefix) dp_
    if {[info exists dp_version]} {
      if {$dp_version >= 3.0} {
        if {[catch "dp_connect -server 0" result]} {
          error $result
        }   
      } {
        if {[catch "dp_connect -server \"\" 0" result]} {
          error $result
        }   
      }
    } {
      if {[catch "dp_connect -server \"\" 0" result]} {
        error $result
      }   
    }
    set server [lindex $result 0]
    set port [lindex $result 1]
  } {
    set tkemacs_priv(prefix) ""
    if {"[info commands connect]" == "" ||
        "[info commands filehandler]" == ""} {
      error "tkemacs: requires the tclRawTCP or tclDP package"
    }
    if {[catch "connect -server \"\" 0" result]} {
      error $result
    }

    if {[llength $result] == 2} {
      set server [lindex $result 0]
      set port [lindex $result 1]
    } {
      global connect_info
      set server $result
      set port $connect_info($server)
    }
  }
  if {$tkemacs_priv(debug)} {
    puts stdout "$pathName: Server: $server"
    puts stdout "$pathName: Port: $port"
  }

  if {[catch "$tkemacs_priv(prefix)filehandler $server r tkemacs_accept_$pathName" result]} {
    error $result
  }

  # run emacs
  set newCommand $command
  if {$tkemacs_priv(debug)} {
    set geometry 80x20+10+10
  } {
    set geometry 200x100+10000+10000
  }
  if {[llength $newCommand] == 1} {
    if {[string match lemac* [lindex $command 0]]} {
      append newCommand { -title TKEMACS -geometry $geometry -l $lispfile -tkwidget $pathName.xfl localhost $port}
    } {
      append newCommand { -rn TKEMACS -geometry $geometry -l $lispfile -tkwidget $pathName.xfl localhost $port}
    }
  }
  regsub -all {\$geometry} $newCommand $geometry newCommand
  regsub -all {\$\{geometry\}} $newCommand $geometry newCommand
  regsub -all {\$pathName} $newCommand $pathName newCommand
  regsub -all {\$\{pathName\}} $newCommand $pathName newCommand
  regsub -all {\$port} $newCommand $port newCommand
  regsub -all {\$\{port\}} $newCommand $port newCommand
  regsub -all {\$lispfile} $newCommand $lispfile newCommand
  regsub -all {\$\{lispfile\}} $newCommand $lispfile newCommand
  set tkemacs_priv($pathName,wait) 1
  if {[catch "eval exec $newCommand &" result]} {
    error $result
  } {
    set tkemacs_priv($pathName,pid) $result
  }
  tkwait variable tkemacs_priv($pathName,initialized)
  tkemacs_wait $pathName

  if {$reparent} {
    $pathName-internal conf -background $background
    $pathName-internal conf -borderwidth $borderwidth
    $pathName-internal conf -relief $relief
    $pathName-xflinternal conf -width $width
    $pathName-xflinternal conf -height $height
    $pathName-xflinternal conf -background $background 
  }

  tkemacs_evalString $pathName \
    "(tk-set-border-width-to-zero)"
  if {$useadvise && $reparent} {
    tkemacs_evalString $pathName \
      "(tk-advise-change-view-functions)"
    tkemacs_evalString $pathName \
      "(tk-activate-view-change-notification)"
  } {
    tkemacs_evalString $pathName \
      "(tk-unadvise-change-view-functions)"
    tkemacs_evalString $pathName \
      "(tk-deactivate-view-change-notification)"
  }
  tkemacs_evalString $pathName \
    "(tk-set-background \"$background\")"
  tkemacs_evalString $pathName \
    "(tk-set-cursor-background \"$insertbackground\")"
  tkemacs_evalString $pathName \
    "(tk-set-foreground \"$foreground\")"
  tkemacs_evalString $pathName \
    "(tk-set-highlight-background \"$selectbackground\")"
  tkemacs_evalString $pathName \
    "(tk-set-highlight-foreground \"$selectforeground\")"
  tkemacs_evalString $pathName \
    "(tk-set-font \"$font\")"

  if {"$startupsource" != ""} {
    tkemacs_evalString $pathName $startupsource
  }

  eval $pathName config $args
}

 proc tkemacs_stop {pathName} {# xf ignore me 8
  global tkemacs_priv
  global tkVersion

  rename $pathName-internal ""
  rename $pathName-xflinternal ""
  if {$tkemacs_priv($pathName,pid) != 0 || $tkVersion < 3.3} {
    tkSteal_getarg $tkemacs_priv($pathName,config) -endsource endsource
    if {"$endsource" != ""} {
      tkemacs_evalString $pathName $endsource
    }
    tkemacs_evalString $pathName "(tk-kill-emacs)" "CMD: " 0

    # wait some time to allow emacs to finish
    set tkemacs_priv($pathName,finish) 0
    after 3000 "set tkemacs_priv($pathName,finish) 1"
    tkwait variable tkemacs_priv($pathName,finish)

    set tkemacs_priv($pathName,initialized) 0
    set tkemacs_priv($pathName,pid) 0
    set tkemacs_priv($pathName,windowId) 0
    catch "$tkemacs_priv(prefix)filehandler $tkemacs_priv($pathName,handle)"
    catch "close $tkemacs_priv($pathName,handle)"
  }
  return ""
}

 proc tkemacs_command {pathName minorCommand args} {# xf ignore me 8
  global tkemacs_priv

  set sendType ""
  set sendCommand ""
  set waitFor 1
  case $minorCommand {
    {conf*} {
      if {"$tkemacs_priv($pathName,windowId)" != "" &&
          $tkemacs_priv($pathName,windowId) != 0 &&
          !$tkemacs_priv($pathName,emacsAvailable)} {
        update idletask
        set tkemacs_priv($pathName,emacsAvailable) 1
        catch "xaccess eventconfigure \
          -windowid $tkemacs_priv($pathName,windowId) \
          -borderwidth 0 \
          -x 0 \
          -y 0 \
          -width [winfo width $pathName] \
          -height [winfo height $pathName]"
      }
      if {[llength $args] == 0} {
        set result ""
        foreach element $tkemacs_priv($pathName,config) {
          lappend result [list [lindex $element 0] [lindex $element 1] [lindex $element 2] [lindex $element 3] [lindex $element 4]]
        }
        return $result
      } {
        if {[llength $args] == 1} {
          set resourceIndex [lsearch $tkemacs_priv($pathName,config) $args*]
          if {$resourceIndex != -1} {
            set element [lindex $tkemacs_priv($pathName,config) $resourceIndex]
            return [list [lindex $element 0] [lindex $element 1] [lindex $element 2] [lindex $element 3] [lindex $element 4]]
          } {
            error "unknown resource: $args"
          }
        } {
          set tkemacs_priv($pathName,config) \
            [tkSteal_parseargs $args $tkemacs_priv($pathName,config)]
          for {set counter 0} {$counter < [llength $args]} {incr counter} {
            case [lindex $args $counter] {
              {-usea*} {
                tkSteal_getarg $tkemacs_priv($pathName,config) \
                  -useadvise useadvise
                tkSteal_getarg $tkemacs_priv($pathName,config) \
                  -reparent reparent
                if {$useadvise && $reparent} {
                  tkemacs_evalString $pathName \
                    "(tk-advise-change-view-functions)"
                  tkemacs_evalString $pathName \
                    "(tk-activate-view-change-notification)"
                } {
                  tkemacs_evalString $pathName \
                    "(tk-unadvise-change-view-functions)"
                  tkemacs_evalString $pathName \
                    "(tk-deactivate-view-change-notification)"
                }
              }
              {-border*} {
                tkSteal_getarg $tkemacs_priv($pathName,config) \
                  -reparent reparent
                if {$reparent} {
                  tkSteal_getarg $tkemacs_priv($pathName,config) \
                    -borderwidth borderwidth
                  $pathName-internal conf -borderwidth $borderwidth
                }
              }
              {-back*} {
                tkSteal_getarg $tkemacs_priv($pathName,config) \
                  -background background
                tkSteal_getarg $tkemacs_priv($pathName,config) \
                  -reparent reparent
                if {$reparent} {
                  $pathName-internal conf -background $background
                  $pathName-xflinternal conf -background $background
                }
                tkemacs_evalString $pathName \
                  "(tk-set-background \"$background\")"
              }
              {-cursorco*} {
                tkSteal_getarg $tkemacs_priv($pathName,config) \
                  -insertbackground insertbackground
                tkemacs_evalString $pathName \
                  "(tk-set-cursor-color \"$insertbackground\")"
              }
              {-debug} {
                tkSteal_getarg $tkemacs_priv($pathName,config) \
                  -debug debug
                set tkemacs_priv(debug) $debug
              }
              {-file} {
                tkSteal_getarg $tkemacs_priv($pathName,config) \
                  -file file
                tkemacs_evalString $pathName \
                  "(find-file \"$file\")"
              }
              {-font} {
                tkSteal_getarg $tkemacs_priv($pathName,config) \
                  -font font
                tkemacs_evalString $pathName \
                  "(tk-set-font \"$font\")"
              }
              {-fore*} {
                tkSteal_getarg $tkemacs_priv($pathName,config) \
                  -foreground foreground
                tkemacs_evalString $pathName \
                  "(tk-set-foreground \"$foreground\")"
              }
              {-heigh*} {
                tkSteal_getarg $tkemacs_priv($pathName,config) \
                  -height height
                $pathName-xflinternal conf -height $height
              }
              {-insertback*} {
                tkSteal_getarg $tkemacs_priv($pathName,config) \
                  -insertbackground insertbackground
                tkemacs_evalString $pathName \
                  "(tk-set-cursor-background \"$insertbackground\")"
              }
              {-reli*} {
                tkSteal_getarg $tkemacs_priv($pathName,config) \
                  -reparent reparent
                if {$reparent} {
                  tkSteal_getarg $tkemacs_priv($pathName,config) \
                    -relief relief
                  $pathName-internal conf -relief $relief
                }
              }
              {-selectback*} {
                tkSteal_getarg $tkemacs_priv($pathName,config) \
                  -selectbackground selectbackground
                tkemacs_evalString $pathName \
                  "(tk-set-highlight-background \"$selectbackground\")"
              }
              {-selectfore*} {
                tkSteal_getarg $tkemacs_priv($pathName,config) \
                  -selectforeground selectforeground
                tkemacs_evalString $pathName \
                  "(tk-set-highlight-foreground \"$selectforeground\")"
              }
              {-widt*} {
                tkSteal_getarg $tkemacs_priv($pathName,config) \
                  -width width
                $pathName-xflinternal conf -width $width
              }
            }
            incr counter
          }
        }
      }
    }
    {compare} {
      if {[llength $args] != 3} {
        error "wrong # of args: should be \"$pathName compare index1 op index2\""
      }
      set sendType "CMD: "
      set sendCommand "(tk-compare-index \"[lindex $args 0]\" \"[lindex $args 1]\" \"[lindex $args 2]\")"
    }
    {debug} {
      if {[llength $args] > 1} {
        error "wrong # of args: should be \"$pathName debug ?boolean?\""
      }
      if {[llength $args] == 0} {
        return $tkemacs_priv(debug)
      } {
        set tkemacs_priv(debug) [lindex $args 0]
      }
    }
    {delete} {
      if {[llength $args] < 1 || [llength $args] > 2} {
        error "wrong # of args: should be \"$pathName delete index1 ?index2?\""
      }
      set sendType "CMD: "
      if {[llength $args] == 1} {
        set sendCommand "(tk-delete-text \"[lindex $args 0]\")"
      } {
        set sendCommand "(tk-delete-text \"[lindex $args 0]\" \"[lindex $args 1]\")"
      }
    }
    {get} {
      if {[llength $args] > 2} {
        error "wrong # of args: should be \"$pathName get ?index1? ?index2?\""
      }
      set sendType "CMD: "
      if {[llength $args] == 0} {
        set sendCommand "(tk-get-buffer-string)"
      } {
        if {[llength $args] == 1} {
          set sendCommand "(tk-get-buffer-string \"[lindex $args 0]\")"
        } {
          set sendCommand "(tk-get-buffer-string \"[lindex $args 0]\" \"[lindex $args 1]\")"
        }
      }
    }
    {index} {
      if {[llength $args] != 1} {
        error "wrong # of args: should be \"$pathName index index\""
      }
      set sendType "CMD: "
      set sendCommand "(tk-translate-position \"[lindex $args 0]\")"
    }
    {insert} {
      if {[llength $args] != 2} {
        error "wrong # of args: should be \"$pathName insert index string\""
      }
      set sendType "CMD: "
      set sendCommand "(tk-insert-string \"[lindex $args 0]\" \"[lindex $args 1]\")"
    }
    {mark} {
      if {[llength $args] < 1} {
        error "wrong # of args: should be \"$patchName mark option ?arg arg ...?\""
      }
      if {"[lindex $args 0]" == "set" && "[lindex $args 1]" == "insert"} {
        set sendType "CMD: "
        set sendCommand "(tk-set-cursor \"[lindex $args 2]\")"
      }
    }
    {redisplay} {
      if {[llength $args] != 0} {
        error "wrong # of args: should be \"$pathName redisplay\""
      }
    }
    {reparent} {
      tkSteal_getarg $tkemacs_priv($pathName,config) \
        -reparent reparent
      if {!$reparent} {
        return
      }
      if {[llength $args] == 1} {
        set tkemacs_priv($pathName,windowId) [lindex $args 0]
        catch "xaccess eventreparent \
          -windowid $tkemacs_priv($pathName,windowId) \
          -parentwindowid [format %ld [winfo id $pathName.xfl]]"
      } {
        tkSteal_getarg $tkemacs_priv($pathName,config) \
          -name name
        if {"$name" == ""} {
          error "to reparent automatically you have to specify -name"
        }
        set timeout 0
        while {$timeout < 1000} {
          set windowids [xaccess xwinchilds -windowid [xaccess xwinroot -widget .]]
          foreach window $windowids {
            if {[string match $name [xaccess xwinname -windowid $window]]} {
              set tkemacs_priv($pathName,windowId) $window 
              catch "xaccess eventreparent \
                -windowid $tkemacs_priv($pathName,windowId) \
                -parentwindowid [format %ld [winfo id $pathName.xfl]]"
              return            
            }
            foreach childwindow [xaccess xwinchilds -windowid $window] {
              if {[string match $name [xaccess xwinname -windowid $childwindow]]} {
                set tkemacs_priv($pathName,windowId) $childwindow
                catch "xaccess eventreparent \
                  -windowid $tkemacs_priv($pathName,windowId) \
                  -parentwindowid [format %ld [winfo id $pathName.xfl]]"
                return            
              }
              foreach childwindow2 [xaccess xwinchilds -windowid $childwindow] {
                if {[string match $name [xaccess xwinname -windowid $childwindow2]]} {
                  set tkemacs_priv($pathName,windowId) $childwindow2
                  catch "xaccess eventreparent \
                    -windowid $tkemacs_priv($pathName,windowId) \
                    -parentwindowid [format %ld [winfo id $pathName.xfl]]"
                  return            
                }
              }
            }
          }
          incr timeout
        }
      }
    }
    {reparentEmacs19} {
      tkSteal_getarg $tkemacs_priv($pathName,config) \
        -reparent reparent
      if {!$reparent} {
        return
      }
      if {[llength $args] != 1} {
        error "wrong # of args: should be \"$pathName reparent windowId\""
      }
      set tkemacs_priv($pathName,windowId) [lindex $args 0]
      catch "xaccess eventreparent \
        -windowid $tkemacs_priv($pathName,windowId) \
        -parentwindowid [format %ld [winfo id $pathName.xfl]]"
    }
    {reparentLEmacs} {
      tkSteal_getarg $tkemacs_priv($pathName,config) \
        -reparent reparent
      if {!$reparent} {
        return
      }
      if {[llength $args] != 1} {
        error "wrong # of args: should be \"$pathName reparent windowId\""
      }
      set tkemacs_priv($pathName,windowId) \
        [xaccess xwinparent -window [xaccess xwinparent -window [lindex $args 0]]]
      catch "xaccess eventreparent \
        -windowid $tkemacs_priv($pathName,windowId) \
        -parentwindowid [format %ld [winfo id $pathName.xfl]]"
    }
    {reset} {
      if {[llength $args] != 0} {
        error "wrong # of args: should be \"$pathName reset\""
      }
      set type "RST: "
      set waitFor 0
    }
    {scan} {
    }
    {send} {
      if {[llength $args] != 1} {
        error "wrong # of args: should be \"$pathName send lisp-command\""
      }
      set sendType "CMD: "
      set sendCommand [lindex $args 0]
    }
    {sendnowait} {
      if {[llength $args] != 1} {
        error "wrong # of args: should be \"$pathName send lisp-command\""
      }
      set sendType "CMD: "
      set sendCommand [lindex $args 0]
      set waitFor 0
    }
    {setxscroll} {
      if {[llength $args] != 4} {
        error "wrong # of args: should be \"$pathName setxscroll totalUnits windowUnits firstUnit lastUnit\""
      }
      tkSteal_getarg $tkemacs_priv($pathName,config) \
        -xscrollcommand xscrollcommand
      if {"$xscrollcommand" != ""} {
        eval $xscrollcommand $args
      }
    }
    {setyscroll} {
      if {[llength $args] != 4} {
        error "wrong # of args: should be \"$pathName setyscroll totalUnits windowUnits firstUnit lastUnit\""
      }
      tkSteal_getarg $tkemacs_priv($pathName,config) \
        -yscrollcommand yscrollcommand
      if {"$yscrollcommand" != ""} {
        eval $yscrollcommand $args
      }
    }
    {stopemacs} {
      if {[llength $args] != 0} {
        error "wrong # of args: should be \"$pathName stopemacs\""
      }
      tkSteal_getarg $tkemacs_priv($pathName,config) \
        -reparent reparent
      if {$reparent} {
        set tkemacs_priv($pathName,initialized) 0
        set tkemacs_priv($pathName,pid) 0
        set tkemacs_priv($pathName,windowId) 0
        catch "$tkemacs_priv(prefix)filehandler $tkemacs_priv($pathName,handle)"
        catch "close $tkemacs_priv($pathName,handle)"
      } {
        tkemacs_stop $pathName
      }
    }
    {tag} {
    }
    {xview} {
      if {[llength $args] != 1} {
        error "wrong # of args: should be \"$pathName xview offset\""
      }
      set sendType "CMD: "
      set sendCommand "(tk-set-x-view [lindex $args 0])"
    }
    {yview} {
      if {[llength $args] != 1} {
        error "wrong # of args: should be \"$pathName yview offset\""
      }
      set sendType "CMD: "
      set sendCommand "(tk-set-y-view [lindex $args 0])"
    }
    {default} {
      error "$pathName: unknown command specified\n$args"
    }
  }

  if {"$sendCommand" != "" && "$sendType" != ""} {
    return [tkemacs_evalString $pathName $sendCommand $sendType $waitFor]
  }
  return ""
}

 proc tkemacs_accept {condition fileId} {# xf ignore me 8
  global dp_version
  global tkemacs_priv

  if {$tkemacs_priv(debug)} {
    puts stdout "WIDGET_NAME: Accept at: $fileId"
  }
  # accept the connection
  if {[catch "$tkemacs_priv(prefix)accept $fileId" result]} {
    error $result
  }
  if {[info exists dp_version]} {
    if {$dp_version >= 3.0} {
      set result [lindex $result 0]
    }
  }
  set tkemacs_priv(WIDGET_NAME,handle) $result
  if {[catch "$tkemacs_priv(prefix)filehandler $tkemacs_priv(WIDGET_NAME,handle) r tkemacs_read_WIDGET_NAME" result]} {
    error $result
  }
  update idletask
  catch "$tkemacs_priv(prefix)filehandler $fileId"
  catch "close $fileId"
  set tkemacs_priv(WIDGET_NAME,initialized) 1
}

 proc tkemacs_read {condition fileId} {# xf ignore me 8
  # read data
  tkemacs_read_data WIDGET_NAME $fileId
}

 proc tkemacs_read_data {pathName fileId} {# xf ignore me 8
  global tkemacs_priv

  # read first line
  set readData ""
  set tkemacs_priv($pathName,result) ""
  if {"[info exists tkemacs_priv(prefix)]" == ""} {
    set tkemacs_priv(prefix) ""
  }
  if {[catch "gets $fileId" readData]} {
    catch "$tkemacs_priv(prefix)filehandler $fileId"
    catch "close $fileId"
    return
  }
  if {"$readData" == ""} {
    if {[eof $fileId]} {
      catch "$tkemacs_priv(prefix)filehandler $fileId"
      catch "close $fileId"
      return
    }
  }

  # now read the following lines
  set currentType 0
  set result ""
  while {1} {
    if {"$readData" != ""} {
      if {$tkemacs_priv(debug)} {
        puts stderr "$pathName: Receive from $fileId: $readData"
      }
      if {[string match "CMD: *" $readData] && $currentType == 0 &&
          "$result" == ""} {
        # the first CMD: token
        set currentType 1
        set result [string range $readData 5 end]
      } {
        if {[string match "RET: *" $readData] && $currentType == 0 &&
            "$result" == ""} {
          # the first RET: token
          set currentType 2
          set result [string range $readData 5 end]
        } {
          if {[string match "ERR: *" $readData]} {
            # an error message
            tkSteal_getarg $tkemacs_priv($pathName,config) \
              -errorcallback errorcallback
            if {"$errorcallback" != ""} {
              catch "$errorcallback $pathName \{$readData\}"
            }
            set result ""
            set tkemacs_priv($pathName,wait) 0
            return
          } {
            if {[string match "END*" $readData] && $currentType != 0} {
              # the end of this message
              if {"$result" != ""} {
                if {$currentType == 1} {
                  if {$tkemacs_priv(debug)} {
                    puts stderr "$pathName: Eval: $result"
                  }
                  if {[catch "$result" res]} {
                    tkSteal_getarg $tkemacs_priv($pathName,config) \
                      -errorcallback errorcallback
                    if {"$errorcallback" != ""} {
                      catch "$errorcallback $pathName \{$res\}"
                    }
                  }
                } {
                  if {$currentType == 2} {
                    if {$tkemacs_priv(debug)} {
                      puts stderr "$pathName: Return: $result"
                    }
                    set tkemacs_priv($pathName,result) $result
                    set tkemacs_priv($pathName,wait) 0
                  } {
                    set tkemacs_priv($pathName,result) ""
                    set tkemacs_priv($pathName,wait) 0
                  }
                }
              }
              return
            } {
              if {[string match "RST:*" $readData]} {
                set tkemacs_priv($pathName,result) ""
                set tkemacs_priv($pathName,wait) 0
                return
              } {
                # data that is appended to the already read message
                append result "\n[string range $readData 5 end]"
              }
            }
          }
        }
      }
    }
    # next message line
    if {[catch "gets $fileId" readData]} {
      catch "$tkemacs_priv(prefix)filehandler $fileId"
      catch "close $fileId"
      return
    }
  }
}

 proc tkemacs_sendData {pathName fileId type data} {# xf ignore me 8
  global tkemacs_priv

  # write to handle
  if {[string match "RST:*" $type]} {
    puts $fileId "$type"
    set tkemacs_priv($pathName,wait) 0
  }
  if {"$data" != ""} {
    set sendData ""
    foreach line [split $data "\n"] {
      append sendData "$type$line\n"
    }
    append sendData "END: "
    if {$tkemacs_priv(debug)} {
      puts stderr "$pathName: Send to $fileId: $sendData"
    }
    catch "puts $fileId \{$sendData\}"
  }
}

 proc tkemacs_evalString {pathName sendCommand {sendType "CMD: "} {waitFor 1}} {# xf ignore me 8
  global tkemacs_priv

  if {!$tkemacs_priv($pathName,initialized) ||
      "$tkemacs_priv($pathName,handle)" == ""} {
    return
  }
  set tkemacs_priv($pathName,wait) $waitFor
  if {[catch "tkemacs_sendData $pathName $tkemacs_priv($pathName,handle) \
        \"$sendType\" \{$sendCommand\}" result]} {
    tkSteal_getarg $tkemacs_priv($pathName,config) -errorcallback errorcallback
    if {"$errorcallback" != ""} {
      catch "$errorcallback $pathName \{$result\}"
    }
    tkemacs_wait $pathName
    error $result
  }
  tkemacs_wait $pathName
  return $tkemacs_priv($pathName,result)
}

 proc tkemacs_wait {pathName} {# xf ignore me 8
  global tkemacs_priv

  if {$tkemacs_priv($pathName,wait)} {
    tkSteal_getarg $tkemacs_priv($pathName,config) -timeout timeout
    after $timeout "set tkemacs_priv($pathName,wait) 0; set tkemacs_priv($pathName,result) {}"
    tkwait variable tkemacs_priv($pathName,wait)
  }
}

# eof

