#----------------------------------------------------------------------
#
#         S t a n d a r d   D i a l o g   B u t t o n s
#
#----------------------------------------------------------------------
proc tixStdDlgBtns {w args} {
    eval tixDlgBtns $w $args
    $w add ok     -text OK     -width 6
    $w add apply  -text Apply  -width 6
    $w add cancel -text Cancel -width 6
    $w add help   -text Help   -width 6
    return $w
}
