proc tk_entryMoveCursor {w offset} {
    $w icursor [expr [$w index insert]+$offset]
}
