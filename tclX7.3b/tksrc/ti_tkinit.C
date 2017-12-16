////////////////////////////////////////////////////////////////////////////////
// (C) Copyright 1995, EasternGraphics GmbH i.G.        		      //
// All Rights Reserved.                                                       //
//                                                                            //
// Redistribution and use in source and binary forms, with or without         //
// modification, are permitted provided that the following conditions         //
// are met:                                                                   //
// 1. No commercial use.                                                      //
// 2. Redistributions of source code must retain the above copyright          //
//    notice, this list of conditions and the following disclaimer.           //
// 3. All advertising materials mentioning features or use of this software   //
//    must display the following acknowledgement:                             //
//      This product includes software developed by EasternGraphics GmbH.     //
// 4. The name of EasternGraphics GmbH should not be used in advertising or   //
//    publicity pertaining to distribution of the software without specific,  //
//    written prior permission.                                               //
//                                                                            //
// THIS SOFTWARE IS PROVIDED TO YOU ``AS IS'' AND ANY EXPRESS OR  IMPLIED     //
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF       //
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN    //
// NO EVENT SHALL EASTERN-GRAPHICS GMBH  BE LIABLE FOR ANY DIRECT, INDIRECT,  //
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT   //
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,  //
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY      //
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT        //
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF   //
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.          //
//                                                                            //
// For more information please contact:                                       //
//            email: {ekki,wicht,tiger}@prakinf.tu-ilmenau.de     	      //
////////////////////////////////////////////////////////////////////////////////

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <GL/glx.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <tcl.h>
#include <tk.h>

#include "ti_tiger.h"
#include "ti_list.h"
#include "ti_strng.h"

extern Tcl_Interp *ti_Ip;
static Tcl_HashTable ti_WinTab;

unsigned int ti_w = 400, ti_h = 400;
unsigned int ti_maw = 400, ti_mah = 400;
unsigned int ti_miw = 400, ti_mih = 400;

class TI_OpenGLWindow;
class TI_OpenGLWidget {
friend class TI_OpenGLWindow;
    Display *dpy;
    Window win;
    int scr;
    XVisualInfo *vis;
    Colormap cmap;
    GLXContext context;

    Tk_Window tkwin;
    TI_String name;

    int err, w, h;

    XVisualInfo *getVis() const;
  public:
    TI_OpenGLWidget(const char*, int, int);
    ~TI_OpenGLWidget();
    
    void width(int _w) { w = _w; }
    int get_width() const { return w; }
    void height(int _h) { h = _h; }
    int get_height() const { return h; }

    void activate();
    void swapbuffers();
    int error() const { return err; }
    const char *getName() const { return name; } 
    
    // tcl interface 
    static int classCMD(ClientData, Tcl_Interp *, int, char *[]); 
    static int objectCMD(ClientData, Tcl_Interp *, int, char *[]);
    static void deleteCMD(ClientData);
};

XVisualInfo *TI_OpenGLWidget::getVis() const {
    GLenum glconf[20];
    int ind = 0;

    if (ti_Singlebuffer == GL_FALSE) 
	glconf[ind++] = (GLenum)GLX_DOUBLEBUFFER;

    if (ti_RGBA == GL_TRUE) {
	glconf[ind++] = (GLenum)GLX_RGBA; 
	glconf[ind++] = (GLenum)GLX_RED_SIZE;
	glconf[ind++] = (GLenum)1; 
	glconf[ind++] = (GLenum)GLX_GREEN_SIZE;
	glconf[ind++] = (GLenum)1; 
	glconf[ind++] = (GLenum)GLX_BLUE_SIZE;
	glconf[ind++] = (GLenum)1; 

	if (ti_Alpha == GL_TRUE) {
	    glconf[ind++] = (GLenum)GLX_ALPHA_SIZE;
	    glconf[ind++] = (GLenum)1;
	}
	
	if (ti_Accum == GL_TRUE) {
	    glconf[ind++] = (GLenum)GLX_ACCUM_RED_SIZE;
	    glconf[ind++] = (GLenum)1;
	    glconf[ind++] = (GLenum)GLX_ACCUM_GREEN_SIZE;
	    glconf[ind++] = (GLenum)1;
	    glconf[ind++] = (GLenum)GLX_ACCUM_BLUE_SIZE;
	    glconf[ind++] = (GLenum)1;
	    glconf[ind++] = (GLenum)GLX_ACCUM_ALPHA_SIZE;
	    glconf[ind++] = (GLenum)1;
	}
    } else {
	glconf[ind++] = (GLenum)GLX_BUFFER_SIZE;
	glconf[ind++] = (GLenum)1;
    }

    if (ti_Depth == GL_TRUE) {
	glconf[ind++] = (GLenum)GLX_DEPTH_SIZE;
	glconf[ind++] = (GLenum)1;
    }
    
    if (ti_Stencil == GL_TRUE) {
	glconf[ind++] = (GLenum)GLX_STENCIL_SIZE;
	glconf[ind++] = (GLenum)1;
    }

    glconf[ind] = (GLenum)None;

    return glXChooseVisual(dpy, scr, (int *)glconf);
}

TI_OpenGLWidget::TI_OpenGLWidget(const char *_n, int _w, int _h): name(_n) {
    err = 1;
    w = _w;
    h = _h;
    tkwin = Tk_CreateWindowFromPath(ti_Ip, Tk_MainWindow(ti_Ip), (char*)name, (char *)0);
    if (tkwin == NULL) 
	return;
    Tk_SetClass(tkwin, "TI_OpenGLWidget");
    dpy = Tk_Display(tkwin);
    scr = Tk_ScreenNumber(tkwin);
    win = Tk_WindowId(tkwin);

    // check out X extension:
    int erb, evb; // error base and event base 
    if (!glXQueryExtension(dpy, &erb, &evb))  
	ti_Output->fatal( "X server has no OpenGL GLX extension.");
    if ((vis=getVis())==0) 
	ti_Output->fatal("No appropriate visual !");
    
    // the colormap
    cmap = XCreateColormap(dpy, RootWindow(dpy, scr), vis->visual, AllocNone);
    if (Tk_SetWindowVisual(tkwin ,vis->visual, vis->depth, cmap)==0)  
	ti_Output->fatal ("Can`t set Tk_Window visual !");
    if((context = glXCreateContext(Tk_Display(tkwin), vis, 0, GL_TRUE))==0) 
	ti_Output->fatal ("Could not create a graphics context !");
    Tk_GeometryRequest(tkwin, get_width(), get_height());
    err = 0;
}

TI_OpenGLWidget::~TI_OpenGLWidget() {
    Tcl_DeleteCommand( ti_Ip, name );
    if (tkwin) Tk_DestroyWindow(tkwin);
    if (dpy) {
	if (cmap) XFreeColormap(dpy, cmap);
	if (vis) XFree(vis);
    }
}

void TI_OpenGLWidget::activate() {
    if (!Tk_IsMapped(tkwin)) return;
    if (!glXMakeCurrent(Tk_Display(tkwin), Tk_WindowId(tkwin), context))
	ti_Output->fatal("Can't connect context and window !");
}

void  TI_OpenGLWidget::swapbuffers() {
    glXSwapBuffers(Tk_Display(tkwin), Tk_WindowId(tkwin));
}

int TI_OpenGLWidget::classCMD(ClientData, Tcl_Interp *ip, int argc, char *argv[]) { 
    if (argc!=4) {
	Tcl_AppendResult(ip, "Bad syntax. Must be: ", argv[0], " <name> <width> <heigh>.", 0);
	return TCL_ERROR;
    }
    int w=0, h=0;
    if (!(sscanf(argv[2], "%i", &w) && sscanf(argv[3], "%i", &h))) {
	Tcl_AppendResult( ip, "Bad parameters. <width> <height> must be integers. ", 0);
	return TCL_ERROR;
    }
    TI_OpenGLWidget *widget = new TI_OpenGLWidget(argv[1], w, h);
    if (widget->error()) { 
	delete widget; 
	return TCL_ERROR;
    }
    Tcl_CreateCommand(ip, argv[1], TI_OpenGLWidget::objectCMD, (ClientData)widget, 
		      TI_OpenGLWidget::deleteCMD);
    Tcl_SetResult(ip, argv[1], TCL_VOLATILE);
    return TCL_OK;
}

int TI_OpenGLWidget::objectCMD(ClientData cd, Tcl_Interp *ip, int argc, char *argv[]) {
    TI_OpenGLWidget *w = (TI_OpenGLWidget*)cd;

    if (argc<2) return TCL_OK;
    Tcl_ResetResult(ip);
    if (strcmp(argv[1], "-activate")==0) w->activate();
    if (strcmp(argv[1], "-swapbuffers")==0) w->swapbuffers();
    return TCL_OK;
}

void TI_OpenGLWidget::deleteCMD(ClientData cd) {
    TI_OpenGLWidget *w = (TI_OpenGLWidget*)cd;
    delete cd;
}

void prefsize(GLint w, GLint h) {
    ti_w = (unsigned int)w;
    ti_h = (unsigned int)h;
}

void maxsize(GLint w, GLint h) {
    ti_maw = (unsigned int)w;
    ti_mah = (unsigned int)h;
}

void minsize(GLint w, GLint h) {
    ti_miw = (unsigned int)w;
    ti_mih = (unsigned int)h;
}


static const char *TI_WindowNameBase = ".opgl";
static TI_OpenGLWindow *actWin = 0;

class TI_OpenGLWindow {
    static TI_OpenGLWindow *findOpenGLWindow(int);
  public:
    // name of toplevel, window and OpenGL widget
    TI_String tn, wn;
    GLint id;

    TI_OpenGLWindow(const char *_tn, const char *_wn, GLint _id) 
	:tn(_tn), wn(_wn), id(_id) {};
    ~TI_OpenGLWindow();
    static int create(const char *);
    
    // window commands
    static GLint winopen(const char *);
    static void winset(GLint);
    static GLint winget();
    static char *widget();
    static void winclose(GLint);
    static void swapbuffers();
};

TI_OpenGLWindow::create(const char *name) {
    Tcl_HashEntry *entry = 0; 
    GLint idx = -1;
    TI_String tn(100), wn(100);
    GLint id;
    
    do { // try to find a valid name for a toplevel widget
	tn = TI_WindowNameBase;
	tn += ti_setGLint( ++idx );
	entry = Tcl_FindHashEntry( &ti_WinTab, (char*)tn );
    } while ( entry!=0 );
    id = idx;
    
    // create a new toplevel 
    TI_String cmd;
    cmd += "toplevel "; 
    cmd += tn;
    cmd += "; wm geometry "; 
    cmd += tn; cmd += " "; 
    cmd += ti_setGLint( ti_w ); 
    cmd += "x"; 
    cmd +=  ti_setGLint( ti_h );
    cmd += "; wm title ";
    cmd += tn; cmd += " \""; 
    cmd += name;
    cmd += "\""; 
    if (ti_miw<ti_w && ti_mih<ti_h) {
	cmd += "; wm minsize "; 
	cmd += tn; cmd += " "; 
	cmd += ti_setGLint( ti_miw ); 
	cmd += " "; 
	cmd +=  ti_setGLint( ti_mih );
    }
    if (ti_maw>ti_w && ti_mah>ti_h) {
	cmd += "; wm maxsize "; 
	cmd += tn; cmd += " "; 
	cmd += ti_setGLint( ti_maw ); 
	cmd += " "; 
	cmd +=  ti_setGLint( ti_mah );
    }
    
    // create the OpenGL widget
    wn = tn;
    wn += ".widget"; 

    cmd += "; openGL "; 
    cmd += wn; 
    cmd += " ";
    cmd += ti_setGLint( ti_w );
    cmd += " ";
    cmd += ti_setGLint( ti_h );
    cmd += "; pack ";
    cmd += wn; 
    cmd += " -fill both -expand yes; update; ";
    cmd += wn; 
    cmd += " -activate\n";
    Tcl_VarEval(ti_Ip, (char*)cmd, 0);

    entry = Tcl_CreateHashEntry( &ti_WinTab, (char*)tn, &idx );
    if (entry) {
	TI_OpenGLWindow *win = new TI_OpenGLWindow( tn, wn, id );
	Tcl_SetHashValue( entry, win );
	actWin = win;
    }
    return id;
}

TI_OpenGLWindow::~TI_OpenGLWindow() {
    Tcl_HashEntry *entry = Tcl_FindHashEntry( &ti_WinTab, (char*)tn );
    if (entry) {
	TI_String cmd(100);
	cmd += "destroy "; cmd += tn; cmd += "\n";
	Tcl_VarEval(ti_Ip, (char*)cmd, 0);
	Tcl_DeleteHashEntry( entry );
    }
}

TI_OpenGLWindow *TI_OpenGLWindow::findOpenGLWindow( int id ) {
    TI_String tn;
    TI_OpenGLWindow *w = 0;
    tn = TI_WindowNameBase;
    tn += ti_setGLint( id );
    Tcl_HashEntry *entry = Tcl_FindHashEntry( &ti_WinTab, (char*)tn);
    if (entry) w = (TI_OpenGLWindow*)entry->clientData;
    return w;
}

GLint TI_OpenGLWindow::winopen(const char *name) { 
    return TI_OpenGLWindow::create(name); 
}

void TI_OpenGLWindow::winset(int id) {
    TI_OpenGLWindow *w = TI_OpenGLWindow::findOpenGLWindow( id );
    if ( w ) { 
	TI_String cmd(100);
	cmd += w->wn;
	cmd += " -activate\n";
	Tcl_VarEval( ti_Ip, (char*)cmd, 0);
    }
}

GLint TI_OpenGLWindow::winget() { return actWin ? actWin->id : -1; }

char *TI_OpenGLWindow::widget() {
    TI_String wn;
    if (actWin) wn = actWin->wn;
    return wn;
}

void TI_OpenGLWindow::winclose(int id) {
    TI_OpenGLWindow *w = TI_OpenGLWindow::findOpenGLWindow( id );
    if ( w ) { if (w == actWin) actWin = 0; delete w; }
}

void TI_OpenGLWindow::swapbuffers() {
    if (actWin) { 
	TI_String cmd(100);
	cmd += actWin->wn;
	cmd += " -swapbuffers\n";
	Tcl_VarEval(ti_Ip, (char*)cmd, 0);
    }
}

GLint winopen(const char *name) { return TI_OpenGLWindow::winopen(name); }

void winset(GLint id) { TI_OpenGLWindow::winset(id); }

GLint winget() { return TI_OpenGLWindow::winget(); }

char *widget() { return TI_OpenGLWindow::widget(); }

void winclose(GLint id) { TI_OpenGLWindow::winclose(id); }

void swapbuffers() { TI_OpenGLWindow::swapbuffers(); }

void ti_initOpenGLW(Tcl_Interp *ip) {
    Tcl_InitHashTable( &ti_WinTab, TCL_STRING_KEYS );
    Tcl_CreateCommand(ip, "openGL", TI_OpenGLWidget::classCMD, 0, 0);
}

