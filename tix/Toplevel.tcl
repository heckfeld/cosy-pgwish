proc tixCreateToplevel {w {type -mapped}} {
    upvar #0 $w data

    toplevel $w
    wm minsize $w 0 0
    if {$type == "-withdrawn"} {
	wm withdraw $w
    }

    bind $w <Destroy>    [bind Toplevel <Destroy>]
    bind $w <Map>        [bind Toplevel <Map>]
    bind $w <Unmap>      [bind Toplevel <Unmap>]
    bind $w <Visibility> [bind Toplevel <Visibility>]
    bind $w <Destroy>    "+_tixToplevelDestroy $w"
    bind $w <Map>        "+_tixToplevelMap $w"
    bind $w <Unmap>      "+_tixToplevelUnmap $w"
    bind $w <Visibility> "+_tixToplevelVisibility $w"
}

proc _tixToplevelDestroy {$w} {
    upvar #0 $w data

    unset data
}

proc _tixToplevelUnmap {w} {
    upvar #0 $w data

    foreach dlg $data(dialogs) {
	set data($dlg,geom) [winfo geometry $dlg]
	wm withdraw $dlg
    }
}

proc _tixToplevelMap {w} {
    upvar #0 $w data

    foreach dlg $data(dialogs) {
	wm geometry $dlg $data($dlg,geom)
	wm deiconify $dlg
    }
}

proc _tixToplevelVisibility {w} {
    upvar #0 $w data

    foreach dlg $data(dialogs) {
	raise $dlg $w
    }
}

proc tixCreateDialogShell {w {type -mapped}} {
    toplevel $w
    set parent [winfo parent $w]
    upvar #0 $parent data

    wm minsize $w 0 0
    wm withdraw $w
    update
    mwm transfor $w [winfo parent $w]
    lappend data(dialogs) $w
    bind $w <Destroy> "_tixDialogDestroy $w"

    if {$type != "-withdrawn"} {
	wm deiconify $w
    }
}

proc _tixDialogDestroy {w} {
    set parent [winfo parent $w]
    upvar #0 $parent data

    catch {unset $data($w,geom)}
}


if {[info commands mwm] == {}} {
    proc mwm {args} {}
}

proc _tixInitMainWindow {w} {
    upvar #0 $w data

    set data(dialogs) {}

    bind $w <Destroy>    +[bind Toplevel <Destroy>]
    bind $w <Map>        +[bind Toplevel <Map>]
    bind $w <Unmap>      +[bind Toplevel <Unmap>]
    bind $w <Visibility> +[bind Toplevel <Visibility>]
    bind $w <Destroy>    "+_tixToplevelDestroy $w"
    bind $w <Map>        "+_tixToplevelMap $w"
    bind $w <Unmap>      "+_tixToplevelUnmap $w"
    bind $w <Visibility> "+_tixToplevelVisibility $w"
}

_tixInitMainWindow .
