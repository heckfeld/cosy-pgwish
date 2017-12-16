set TIX_LIBRARY ..
lappend auto_path $TIX_LIBRARY
tixInit -libdir $TIX_LIBRARY -scheme SGIGray -fontset 14Point -binding Motif

tixMDIWindow .m -bg steelblue
pack .m -fill both -expand yes

.m add win1 -title hello -width 100 -height 100
.m place win1 -x 10 -y 10

button .b -text maximize -command ".m window win1 maximize"
pack .b

wm minsize . 0 0
wm geometry . 400x300+100+100



