set TIX_LIBRARY ..
lappend auto_path $TIX_LIBRARY
tixInit -libdir $TIX_LIBRARY -scheme SGIGray -fontset 14Point -binding Motif

proc cmd {sizes} {
    puts "The sizes of the panes are $sizes"

    puts [lindex $sizes 0]
}

tixPanedWindow .p -orientation vertical -command cmd
.p add 1 -size 100 -min 
.p add 2 -size 100 -min 30
.p add 3 -size 30 -min 30 -max 30
.p add 4 -size 100 -min 50 -max 100

tixScrolledListbox .p.1.lst
pack  .p.1.lst -expand yes -fill both

pack .p -expand yes -fill both
.p update

wm geometry . +100+100
wm minsize . 0 0
