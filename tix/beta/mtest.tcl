set TIX_LIBRARY ..
lappend auto_path $TIX_LIBRARY
tixInit -libdir $TIX_LIBRARY -scheme SGIGray -fontset 14Point -binding Motif

tixMwmClient .m1 -bd 2 -titlebd 1 -title MDIWindow -parent . -hintwidth 1
place .m1 -x 20 -y 30

tixMwmClient .m2 -bd 2 -titlebd 1 -title MDIWindow -parent . -hintwidth 1
place .m2 -x 40 -y 40

. config -bg rosybrown
wm minsize . 0 0
wm geometry . 400x300+100+100



