#include <netwm.h>
#include <kwin.h>
#include <tk.h>

void
setWorkspace ( Tk_Window *winPtr, char *name) {

    QString wksp ( name);
    NETRootInfo info( Tk_Display(winPtr),
		      NET::DesktopNames | NET::NumberOfDesktops );


    for( int i = 0; i < info.numberOfDesktops(); ++i) {
	if( QString::compare( wksp, QString::fromUtf8(info.desktopName(i+1))) == 0)
	    KWin::setOnDesktop( Tk_WindowId(winPtr), i);
    }
}
