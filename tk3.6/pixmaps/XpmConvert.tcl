#!/usr/local/bin/X11/wish
global argc
global argv

if {$argc < 1} {
  puts stderr "No filenames specified!"
  catch "destroy ."
  catch "exit 1"
}

foreach fileName $argv {
  if {"[file extension $fileName]" != ".xpm"} {
    continue
  }

  # read source
  set result ""
  set inFd [open $fileName r]
  set contents [read $inFd]
  close $inFd
 
  # convert contents
  foreach line [split $contents "\n"] {
    set resultLine [string trim $line]
    if {[string match "static char*" $resultLine]} {
      continue
    }
    if {![string match "/*/" $resultLine] &&
        "$resultLine" != ""} {
      regsub -all {^\"} $resultLine "" resultLine
      regsub -all {\",$} $resultLine "" resultLine
      regsub -all {\"\};$} $resultLine "" resultLine
      if {[string match "*\{*" $resultLine] ||
          [string match "*\}*" $resultLine] ||
          [string match "*\[*" $resultLine] ||
          [string match "*\]*" $resultLine]} {
        puts stderr "Tcl syntax characters in Xpm definition."
        puts stderr "Please replace all \{\}\[\] characters in"
        puts stderr "the Xpm file."
        catch "destroy ."
        catch "exit 1"
      }
      lappend result $resultLine
    }
  }

  # write destination
  set outFd [open ${fileName}l w]
  set tmpFileName $fileName
  regsub -all {\.} $tmpFileName {_} tmpFileName
  puts $outFd "/* XPM */"
  puts $outFd "static char $tmpFileName\[\] = {\"" nonewline
  puts $outFd $result nonewline
  puts $outFd "\"};"
  close $outFd
}

catch "destroy ."
catch "exit 0"

# eof
