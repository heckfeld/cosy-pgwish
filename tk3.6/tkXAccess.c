/* 
 * tkXAccess.c --
 *
 *	This module implements functionality required for sending
 *      X events to windows and do other Xlib stuff.
 *
 * Copyright (c) 1993 by Sven Delmas
 * All rights reserved.
 * See the file COPYRIGHT for the copyright notes.
 *
 */

#if defined(USE_XACCESS)

#ifndef lint
static char *AtFSid = "$Header: tkXAccess.c[5.0] Thu Jul 28 19:12:54 1994 garfield@mydomain.de frozen $";
#endif /* not lint */

#include "tkConfig.h"
#include "tk.h"
#include "tkXAccess.h"

/*
  I do not know why, but several window managers require multiple
  reparent events. Right now a value of 25 should be enough.
  */
#define REPARENT_LOOPS 25

typedef struct {
  char *name;
  Atom type;
} AtomList;

static int continueXAccess;

static AtomList propertyTable[] = {
    {"ARC",             XA_ARC},
    {"ATOM",            XA_ATOM},
    {"BITMAP",          XA_BITMAP},
    {"CARDINAL",        XA_CARDINAL},
    {"COLORMAP",        XA_COLORMAP},
    {"CURSOR",          XA_CURSOR},
    {"DRAWABLE",        XA_DRAWABLE},
    {"FONT",            XA_FONT},
    {"INTEGER",         XA_INTEGER},
    {"PIXMAP",          XA_PIXMAP},
    {"POINT",           XA_POINT},
    {"RECTANGLE",       XA_RECTANGLE},
    {"RGB_COLOR_MAP",   XA_RGB_COLOR_MAP},
    {"STRING",          XA_STRING},
    {"WINDOW",          XA_WINDOW},
    {"VISUALID",        XA_VISUALID},
    {"WM_COMMAND",      XA_WM_COMMAND},
    {"WM_HINTS",        XA_WM_HINTS},
    {"WM_ICON_SIZE",    XA_WM_ICON_SIZE},
    {"WM_SIZE_HINTS",   XA_WM_SIZE_HINTS},
    {"",                0},
};

/*
 * external functions used by TkSteal.
 */

/*
 *----------------------------------------------------------------------
 *
 * LocalXErrorHandler --
 *
 *      This procedure prevents X errors.
 *
 * Results:
 *      0
 *
 * Side effects:
 *      X errors are ignored.
 *
 *----------------------------------------------------------------------
 */
static int
LocalXErrorHandler(clientData, errorPtr)
     ClientData clientData;
     XErrorEvent *errorPtr;
{
  continueXAccess = 0;
  return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * TkXAccessMatchModifierState --
 *
 *      This procedure matches a string to the corresponding
 *      modifier state.
 *
 * Results:
 *      The state of the modifier.
 *
 * Side effects:
 *      None
 *
 *----------------------------------------------------------------------
 */
static int
TkXAccessMatchModifierState(name)
     char *name;
{
  int paramLength = 0;

  paramLength = strlen(name);
  if ((strncmp(name, "Any", paramLength) == 0) &&
      (paramLength >= 2)) {
    return(0);
  } else if ((strncmp(name, "Shift", paramLength) == 0) &&
	     (paramLength >= 2)) {
    return(ShiftMask);
  } else if ((strncmp(name, "Lock", paramLength) == 0) &&
	     (paramLength >= 2)) {
    return(LockMask);
  } else if ((strncmp(name, "Control", paramLength) == 0) &&
	     (paramLength >= 2)) {
    return(ControlMask);
  } else if ((strncmp(name, "Meta", paramLength) == 0) &&
	     (paramLength == 4)) {
    return(Mod1Mask);
  } else if ((strncmp(name, "Mod1", paramLength) == 0) &&
	     (paramLength >= 4)) {
    return(Mod1Mask);
  } else if ((strncmp(name, "Mod2", paramLength) == 0) &&
	     (paramLength >= 4)) {
    return(Mod2Mask);
  } else if ((strncmp(name, "Mod3", paramLength) == 0) &&
	     (paramLength >= 4)) {
    return(Mod3Mask);
  } else if ((strncmp(name, "Mod4", paramLength) == 0) &&
	     (paramLength >= 4)) {
    return(Mod4Mask);
  } else if ((strncmp(name, "Mod5", paramLength) == 0) &&
	     (paramLength >= 4)) {
    return(Mod5Mask);
  } else if ((strncmp(name, "Mod2", paramLength) == 0) &&
	     (paramLength >= 4)) {
    return(Mod2Mask);
  } else if ((strncmp(name, "Button1", paramLength) == 0) &&
	     (paramLength >= 7)) {
    return(Button1Mask);
  } else if ((strncmp(name, "Button2", paramLength) == 0) &&
	     (paramLength >= 7)) {
    return(Button2Mask);
  } else if ((strncmp(name, "Button3", paramLength) == 0) &&
	     (paramLength >= 7)) {
    return(Button3Mask);
  } else if ((strncmp(name, "Button4", paramLength) == 0) &&
	     (paramLength >= 7)) {
    return(Button4Mask);
  } else if ((strncmp(name, "Button5", paramLength) == 0) &&
	     (paramLength >= 7)) {
    return(Button5Mask);
  }
  return -1;
}

/*
 *----------------------------------------------------------------------
 *
 * TkXAccessCmd --
 *
 *      This procedure gives access to various X-lib calls.
 *
 * Results:
 *      Returns a standard Tcl completion code, and leaves an error
 *      message in interp->result if an error occurs.
 *
 * Side effects:
 *      Depends on the performed action.
 *
 *----------------------------------------------------------------------
 */
int
TkXAccessCmd(clientData, interp, argc, argv)
     ClientData clientData;	/* Main window associated with
				 * interpreter. */
     Tcl_Interp *interp;	/* Current interpreter. */
     int argc;			/* Number of arguments. */
     char **argv;		/* Argument strings. */
{
  char c;
  char *interval = (char *) NULL;
  char *property_return;
  char *win_name;
  char buf[30];
  
  int actual_format = 0;
  int border_width = 0;
  int counter = 0;
  int counter3 = 0;
  int dataindex = 0;
  int datalen = 0;
  int delete = 0;
  int format = 8;
  int height = 0;
  int keysymsPerKeyCode = 0;
  int length = 0;
  int maxKeyCode = 0;
  int minKeyCode = 0;
  int mode = PropModeReplace;
  int num_elements = 0;
  int override_redirect = True;
  int paramLength = 0;
  int relative = 0;
  int volume = 0;
  int width = 0;
  int x = -1002;
  int x_ret = -1002;
  int x_root = -1002;
  int x_root_ret = -1002;
  int y = -1002;
  int y_ret = -1002;
  int y_root = -1002;
  int y_root_ret = -1002;

  unsigned int button = 0;
  unsigned int keycode = 0;
  unsigned int mask_ret = 0;
  unsigned int modeModifierMask = 0;
  unsigned int num_child = 0;
  unsigned int permanentState = 0;
  unsigned int state = 0;
  unsigned int tmpKeycode = 0;

  long interval_value = 0L;
  long long_length = 100000L;
  long long_offset = 0L;

  unsigned long bytes_after = 0L;
  unsigned long counter2 = 0L;
  unsigned long lcounter = 0L;
  unsigned long long_num_elements = 0L;
  
  double interval_steps = 0;
  double x_tmp = 0;
  double y_tmp = 0;

  struct timeval time;
  struct timeval startTime;

  Window above = (Window) None;
  Window child_ret = (Window) None;
  Window parent = (Window) None;
  Window parent_ret = (Window) None;
  Window root_ret = (Window) None;
  Window window = (Window) None;
  Window *child_list;
  
  Atom actual_type = (Atom) NULL;
  Atom message_type = (Atom) NULL;
  Atom property  = (Atom) NULL;
  Atom req_type = (Atom) XA_STRING;
  Atom type = (Atom) XA_STRING;
  Atom *atomList;

  KeySym *mainKeyMap;
  KeySym keySym;

  KeyCode *keyCode;
  
  XModifierKeymap *modifierMap;
  
  XEvent xevent;
  XButtonEvent *xbutton;
  XClientMessageEvent *xclient;
  XConfigureEvent *xconfig;
  XExposeEvent *xexpose;
  XKeyEvent *xkey;

  Tk_Window abovetkwin = (Tk_Window) NULL;
  Tk_Window parenttkwin = (Tk_Window) NULL;
  Tk_Window tkrootwin = (Tk_Window) clientData;
  Tk_Window tkwin = (Tk_Window) NULL;

  Tk_ErrorHandler handler1;
  Tk_ErrorHandler handler2;
  Tk_ErrorHandler handler3;
    
  if (argc < 2) {
    Tcl_ResetResult(interp);
    Tcl_AppendResult(interp, "wrong # args: should be \"",
		     argv[0], " command ?arg arg....?\"",
		     (char *) NULL);
    return TCL_ERROR;
  }

  while (counter < argc) {
    length = strlen(argv[counter]);
    if ((strncmp(argv[counter], "-abovewidget", length) == 0) &&
	(counter < argc) && (length >= 9)) {
      counter++;
      if ((abovetkwin = Tk_NameToWindow(interp, argv[counter],
					tkrootwin)) != NULL) {
	above = Tk_WindowId(abovetkwin);
      }
    } else if ((strncmp(argv[counter], "-abovewindowid", length) == 0) &&
	(counter < argc) && (length >= 9)) {
      counter++;
      above = strtoul(argv[counter], (char **) NULL, 0);
    } else if ((strncmp(argv[counter], "-borderwidth", length) == 0) &&
	(counter < argc) && (length >= 3)) {
      counter++;
      border_width = atoi(argv[counter]);
    } else if ((strncmp(argv[counter], "-button", length) == 0) &&
	(counter < argc) && (length >= 4)) {
      counter++;
      paramLength = strlen(argv[counter]);
      if ((strncmp(argv[counter], "1", paramLength) == 0) &&
	  (paramLength == 1)) {
	button = Button1;
      } else if ((strncmp(argv[counter], "2", paramLength) == 0) &&
	  (paramLength == 1)) {
	button = Button2;
      } else if ((strncmp(argv[counter], "3", paramLength) == 0) &&
	  (paramLength == 1)) {
	button = Button3;
      } else if ((strncmp(argv[counter], "4", paramLength) == 0) &&
	  (paramLength == 1)) {
	button = Button4;
      } else if ((strncmp(argv[counter], "5", paramLength) == 0) &&
	  (paramLength == 1)) {
	button = Button5;
      }
    } else if ((strncmp(argv[counter], "-data", length) == 0) &&
        (counter < argc) && (length >= 3)) {
      counter++;
      dataindex = counter;
      datalen = strlen(argv[dataindex]);
    } else if ((strncmp(argv[counter], "-delete", length) == 0) &&
        (counter < argc) && (length >= 3)) {
      counter++;
      delete = atoi(argv[counter]);
    } else if ((strncmp(argv[counter], "-format", length) == 0) &&
	(counter < argc) && (length >= 3)) {
      counter++;
      format = atoi(argv[counter]);
    } else if ((strncmp(argv[counter], "-height", length) == 0) &&
	(counter < argc) && (length >= 3)) {
      counter++;
      height = atoi(argv[counter]);
    } else if ((strncmp(argv[counter], "-interval", length) == 0) &&
        (counter < argc) && (length >= 3)) {
      counter++;
      interval = argv[counter];
    } else if ((strncmp(argv[counter], "-keycode", length) == 0) &&
	(counter < argc) && (length >= 4)) {
      counter++;
      keycode = XKeysymToKeycode(Tk_Display(tkrootwin),
				 XStringToKeysym(argv[counter]));
    } else if ((strncmp(argv[counter], "-longlength", length) == 0) &&
        (counter < argc) && (length >= 6)) {
      counter++;
      long_length = strtoul(argv[counter], (char **) NULL, 0);
    } else if ((strncmp(argv[counter], "-longoffset", length) == 0) &&
        (counter < argc) && (length >= 6)) {
      counter++;
      long_offset = strtoul(argv[counter], (char **) NULL, 0);
    } else if ((strncmp(argv[counter], "-messagetype", length) == 0) &&
	(counter < argc) && (length >= 12)) {
      counter++;
      message_type = strtoul(argv[counter], (char **) NULL, 0);
    } else if ((strncmp(argv[counter], "-messagetypename", length) == 0) &&
	(counter < argc) && (length >= 13)) {
      counter++;
      message_type = Tk_InternAtom(tkrootwin, argv[counter]);
    } else if ((strncmp(argv[counter], "-numelements", length) == 0) &&
        (counter < argc) && (length >= 3)) {
      counter++;
      num_elements = atoi(argv[counter]);
    } else if ((strncmp(argv[counter], "-overrideredirect", length) == 0) &&
	(counter < argc) && (length >= 3)) {
      counter++;
      override_redirect = atoi(argv[counter]);
    } else if ((strncmp(argv[counter], "-parentwidget", length) == 0) &&
	(counter < argc) && (length >= 10)) {
      counter++;
      if ((parenttkwin = Tk_NameToWindow(interp, argv[counter],
					 tkrootwin)) != NULL) {
	parent = Tk_WindowId(parenttkwin);
      }
    } else if ((strncmp(argv[counter], "-parentwindowid", length) == 0) &&
	(counter < argc) && (length >= 10)) {
      counter++;
      parent = strtoul(argv[counter], (char **) NULL, 0);
    } else if ((strncmp(argv[counter], "-property", length) == 0) &&
        (counter < argc) && (length >= 9)) {
      counter++;
      property = strtoul(argv[counter], (char **) NULL, 0);
    } else if ((strncmp(argv[counter], "-propertyname", length) == 0) &&
        (counter < argc) && (length >= 10)) {
      counter++;
      property = Tk_InternAtom(tkrootwin, argv[counter]);
    } else if ((strncmp(argv[counter], "-propmode", length) == 0) &&
        (counter < argc) && (length >= 6)) {
      counter++;
      if (strcmp(argv[counter], "replace") == 0) {
        mode = PropModeReplace;
      } else if (strcmp(argv[counter], "prepend") == 0) {
        mode = PropModePrepend;
      } else if (strcmp(argv[counter], "append") == 0) {
        mode = PropModeAppend;
      }
    } else if ((strncmp(argv[counter], "-proptype", length) == 0) &&
        (counter < argc) && (length >= 6)) {
      counter++;
      counter2 = 0;
      while (1) {
        if (strcmp(propertyTable[counter2].name, "") == 0) {
          break;
        }
        if (strcmp(propertyTable[counter2].name, argv[counter]) == 0) {
          type = propertyTable[counter2].type;
          req_type = propertyTable[counter2].type;
          break;
        }
        counter2++;
      }
    } else if ((strncmp(argv[counter], "-relative", length) == 0) &&
        (counter < argc) && (length >= 3)) {
      counter++;
      relative = atoi(argv[counter]);
    } else if ((strncmp(argv[counter], "-state", length) == 0) &&
	(counter < argc) && (length >= 4)) {
      counter++;
      if (TkXAccessMatchModifierState(argv[counter]) != -1) {
	state |= TkXAccessMatchModifierState(argv[counter]);
      }
    } else if ((strncmp(argv[counter], "-volume", length) == 0) &&
	(counter < argc) && (length >= 2)) {
      counter++;
      volume = atoi(argv[counter]);
    } else if ((strncmp(argv[counter], "-widget", length) == 0) &&
	(counter < argc) && (length >= 4)) {
      counter++;
      if ((tkwin = Tk_NameToWindow(interp, argv[counter],
				   tkrootwin)) != NULL) {
	window = Tk_WindowId(tkwin);
      }
    } else if ((strncmp(argv[counter], "-width", length) == 0) &&
	(counter < argc) && (length >= 3)) {
      counter++;
      width = atoi(argv[counter]);
    } else if ((strncmp(argv[counter], "-windowid", length) == 0) &&
	(counter < argc) && (length >= 4)) {
      counter++;
      window = strtoul(argv[counter], (char **) NULL, 0);
    } else if ((strncmp(argv[counter], "-x", length) == 0) &&
	(counter < argc) && (length == 2)) {
      counter++;
      x = atoi(argv[counter]);
    } else if ((strncmp(argv[counter], "-xroot", length) == 0) &&
	(counter < argc) && (length >= 3)) {
      counter++;
      x_root = atoi(argv[counter]);
    } else if ((strncmp(argv[counter], "-y", length) == 0) &&
	(counter < argc) && (length == 2)) {
      counter++;
      y = atoi(argv[counter]);
    } else if ((strncmp(argv[counter], "-yroot", length) == 0) &&
	(counter < argc) && (length >= 3)) {
      counter++;
      y_root = atoi(argv[counter]);
    }
    counter++;
  }

  continueXAccess = 1;
  property_return = (char *) NULL;
  child_list = (Window *) NULL;
  atomList = (Atom *) NULL;
  handler1 = Tk_CreateErrorHandler(Tk_Display(tkrootwin), BadWindow,
				   -1, -1, LocalXErrorHandler,
				   (ClientData) NULL);
  handler2 = Tk_CreateErrorHandler(Tk_Display(tkrootwin), BadAccess,
				   X_GetProperty, -1,
				   LocalXErrorHandler, (ClientData)
				   NULL);
  handler3 = Tk_CreateErrorHandler(Tk_Display(tkrootwin), BadAtom,
				   X_GetProperty, -1,
				   LocalXErrorHandler, (ClientData)
				   NULL);
  length = strlen(argv[1]);
  if (window == (Window) NULL &&
      !(((strncmp(argv[1], "xbell", length) == 0) && length >= 2) ||
	((strncmp(argv[1], "eventmotion", length) == 0) && length >= 7))) {
    Tcl_ResetResult(interp);
    Tcl_AppendResult(interp, argv[0], ": no target window specified",
		     (char *) NULL);
    goto error;
  }
  if (format != 8 && format != 16 && format != 32) {
    Tcl_ResetResult(interp);
    Tcl_AppendResult(interp, argv[0],
		     ": wrong format value (must be 8, 16 or 32)",
		     (char *) NULL);
    goto error;
  }

  if (window != (Window) NULL) {
    XQueryTree(Tk_Display(tkrootwin), window, &root_ret, &parent_ret,
	       &child_list, &num_child);
    if (!continueXAccess) goto Xerror;
    XQueryPointer(Tk_Display(tkrootwin), window, &root_ret,
		  &child_ret, &x_root_ret, &y_root_ret, &x_ret,
		  &y_ret, &mask_ret);
    if (!continueXAccess) goto Xerror;
  } else {
    XQueryTree(Tk_Display(tkrootwin), Tk_WindowId(tkrootwin),
	       &root_ret, &parent_ret, &child_list, &num_child);
    if (!continueXAccess) goto Xerror;
    XQueryPointer(Tk_Display(tkrootwin), Tk_WindowId(tkrootwin),
		  &root_ret, &child_ret, &x_root_ret, &y_root_ret,
		  &x_ret, &y_ret, &mask_ret);
    if (!continueXAccess) goto Xerror;
  }
  (void) gettimeofday(&time, (struct timezone *) NULL);
  if (num_elements == 0) {
    num_elements = datalen;
  }
  if (width == 0 && tkwin != (Tk_Window) NULL) {
    width = Tk_Width(tkwin);
  }
  if (height == 0 && tkwin != (Tk_Window) NULL) {
    height = Tk_Height(tkwin);
  }
  if (x == -1002) {
    x = x_ret;
  }
  if (x_root == -1002) {
    x_root = x_root_ret;
  }
  if (y == -1002) {
    y = y_ret;
  }
  if (y_root == -1002) {
    y_root = y_root_ret;
  }

  c = argv[1][0];
  if ((c == 'e') &&
      (strncmp(argv[1], "eventbuttonpress", length) == 0) &&
      (length >= 12)) {
    xevent.type = ButtonPress;
    xbutton = (XButtonPressedEvent*) &xevent;
    xbutton->display = Tk_Display(tkrootwin);
    xbutton->window = window;
    xbutton->root = root_ret;
    xbutton->subwindow = child_ret;
    xbutton->time = (time.tv_sec * 1000) + time.tv_usec;
    xbutton->x = x;
    xbutton->y = y;
    xbutton->x_root = x_root;
    xbutton->y_root = y_root;
    xbutton->state = state;
    xbutton->button = button;
    xbutton->same_screen = True;
    XSendEvent(Tk_Display(tkrootwin), window, True, 0xffffL, &xevent);
    if (!continueXAccess) goto Xerror;
  } else if ((c == 'e') &&
	     (strncmp(argv[1], "eventbuttonrelease", length) == 0) &&
             (length >= 12)) {
    xevent.type = ButtonRelease;
    xbutton = (XButtonReleasedEvent*) &xevent;
    xbutton->display = Tk_Display(tkrootwin);
    xbutton->window = window;
    xbutton->root = root_ret;
    xbutton->subwindow = child_ret;
    xbutton->time = (time.tv_sec * 1000) + time.tv_usec;
    xbutton->x = x;
    xbutton->y = y;
    xbutton->x_root = x_root;
    xbutton->y_root = y_root;
    xbutton->state = state;
    xbutton->button = button;
    xbutton->same_screen = True;
    XSendEvent(Tk_Display(tkrootwin), window, True, 0xffffL, &xevent);
    if (!continueXAccess) goto Xerror;
  } else if ((c == 'e') &&
	     (strncmp(argv[1], "eventclientmessage", length) == 0) &&
	     (length >= 7)) {
    xevent.type = ClientMessage;
    xclient = (XClientMessageEvent*) &xevent;
    xclient->display = Tk_Display(tkrootwin);
    xclient->window = window;
    xclient->message_type = message_type;
    xclient->format = format;
    XSendEvent(Tk_Display(tkrootwin), window, True, 0xffffL, &xevent);
    if (!continueXAccess) goto Xerror;
  } else if ((c == 'e') &&
	     (strncmp(argv[1], "eventconfigure", length) == 0) &&
             (length >= 7)) {
    xevent.type = ConfigureNotify;
    xconfig = (XConfigureEvent*) &xevent;
    xconfig->display = Tk_Display(tkrootwin);
    xconfig->event = window;
    xconfig->window = window;
    xconfig->x = x;
    xconfig->y = y;
    xconfig->width = width;
    xconfig->height = height;
    xconfig->border_width = border_width;
    xconfig->above = above;
    xconfig->override_redirect = override_redirect;
    XSendEvent(Tk_Display(tkrootwin), window, True, 0xffffL, &xevent);
    if (!continueXAccess) goto Xerror;
  } else if ((c == 'e') &&
	     (strncmp(argv[1], "eventexpose", length) == 0) &&
             (length >= 6)) {
    xevent.type = Expose;
    xexpose = (XExposeEvent*) &xevent;
    xexpose->count = 0;
    xexpose->display = Tk_Display(tkrootwin);
    xexpose->window = window;
    xexpose->x = x;
    xexpose->y = y;
    xexpose->width = width;
    xexpose->height = height;
    XSendEvent(Tk_Display(tkrootwin), window, True, 0xffffL, &xevent);
    if (!continueXAccess) goto Xerror;
  } else if ((c == 'e') &&
	     (strncmp(argv[1], "eventkeypress", length) == 0) &&
             (length >= 9)) {
    xevent.type = KeyPress;
    xkey = (XKeyPressedEvent*) &xevent;
    xkey->display = Tk_Display(tkrootwin);
    xkey->window = window;
    xkey->root = root_ret;
    xkey->subwindow = child_ret;
    xkey->time = (time.tv_sec * 1000) + time.tv_usec;
    xkey->x = x;
    xkey->y = y;
    xkey->x_root = x_root;
    xkey->y_root = y_root;
    xkey->state = state;
    xkey->keycode = keycode;
    xkey->same_screen = True;
    XSendEvent(Tk_Display(tkrootwin), window, True, 0xffffL, &xevent);
    if (!continueXAccess) goto Xerror;
  } else if ((c == 'e') &&
	     (strncmp(argv[1], "eventkeyrelease", length) == 0) &&
             (length >= 9)) {
    xevent.type = KeyRelease;
    xkey = (XKeyReleasedEvent*) &xevent;
    xkey->display = Tk_Display(tkrootwin);
    xkey->window = window;
    xkey->root = root_ret;
    xkey->subwindow = child_ret;
    xkey->time = (time.tv_sec * 1000) + time.tv_usec;
    xkey->x = x;
    xkey->y = y;
    xkey->x_root = x_root;
    xkey->y_root = y_root;
    xkey->state = state;
    xkey->keycode = keycode;
    xkey->same_screen = True;
    XSendEvent(Tk_Display(tkrootwin), window, True, 0xffffL, &xevent);
    if (!continueXAccess) goto Xerror;
  } else if ((c == 'e') && (strncmp(argv[1], "eventmap", length) == 0) &&
             (length >= 7)) {
    XMapWindow(Tk_Display(tkrootwin), window);
    if (!continueXAccess) goto Xerror;
  } else if ((c == 'e') &&
	     (strncmp(argv[1], "eventmotion", length) == 0) &&
             (length >= 7)) {
    if (interval != (char *) NULL) {
      interval_steps = atof(interval);
    }
    XQueryPointer(Tk_Display(tkrootwin), Tk_WindowId(tkrootwin),
		  &root_ret, &child_ret, &x_root_ret, &y_root_ret,
		  &x_ret, &y_ret, &mask_ret);
    if (!continueXAccess) goto Xerror;
    if (window == (Window) NULL) {
      if (!relative) {
	x -= x_root_ret;
	y -= y_root_ret;
      }
    }
    if (interval_steps > 0) {
      x_tmp = x_ret;
      y_tmp = y_ret;
      while (x_tmp != x || y_tmp != y) {
	if (x_tmp < x) {
	  x_tmp += interval_steps;
	  if (x_tmp > x) {
	    x_tmp = x;
	  }
	} else {
	  x_tmp -= interval_steps;
	  if (x_tmp < x) {
	    x_tmp = x;
	  }
	}
	if (y_tmp < y) {
	  y_tmp += interval_steps;
	  if (y_tmp > y) {
	    y_tmp = y;
	  }
	} else {
	  y_tmp -= interval_steps;
	  if (y_tmp < y) {
	    y_tmp = y;
	  }
	}
	XWarpPointer(Tk_Display(tkrootwin), (Window) None, (Window) window,
		     0, 0, 0, 0, x_tmp, y_tmp);
	if (!continueXAccess) goto Xerror;
	XSync(Tk_Display(tkrootwin), False);
      }
    } else {
      XWarpPointer(Tk_Display(tkrootwin), (Window) None, (Window) window,
		   0, 0, 0, 0, x, y);
      if (!continueXAccess) goto Xerror;
    }
  } else if ((c == 'e') &&
	     (strncmp(argv[1], "eventreparent", length) == 0) &&  
	     (length >= 6)) {
    if (parent == (Window) NULL) {
      Tcl_ResetResult(interp);
      Tcl_AppendResult(interp, argv[0], ": no parent window specified",
	  	       (char *) NULL);
      goto error;
    }
    XSync(Tk_Display(tkrootwin), False);
    XWithdrawWindow(Tk_Display(tkrootwin), window,
		    Tk_ScreenNumber(tkrootwin));
    XSync(Tk_Display(tkrootwin), False);
    for (counter = 0; counter < REPARENT_LOOPS; counter++) {
      XReparentWindow(Tk_Display(tkrootwin), window, parent, 0, 0);
      XSync(Tk_Display(tkrootwin), False);
      /*
	I hoped this code would allow a correct working reparenting for
	window managers that require multiple reparent events.
	Unfortunately the parent is updated even when the reparenting
	fails. So me just loop many times to guarentee a correct
	reparenting.
	*/
#if 0      
      if (child_list != (Window *) NULL) {
	XFree(child_list);
      }
      XQueryTree(Tk_Display(tkrootwin), window, &root_ret, &parent_ret,
		 &child_list, &num_child);
      XSync(Tk_Display(tkrootwin), False);
      if (parent == parent_ret) {
	break;
      }
#endif
    }
    XMapWindow(Tk_Display(tkrootwin), window);
    XSync(Tk_Display(tkrootwin), False);
    if (!continueXAccess) goto Xerror;
  } else if ((c == 'e') &&
	     (strncmp(argv[1], "eventunmap", length) == 0) &&  
	     (length >= 6)) {
    XUnmapWindow(Tk_Display(tkrootwin), window);
    if (!continueXAccess) goto Xerror;
  } else if ((c == 'p') && (strncmp(argv[1], "propchange", length) == 0) &&
      (length >= 5)) {
    if (dataindex == 0) {
      Tcl_ResetResult(interp);
      Tcl_AppendResult(interp, argv[0], ": no data defined",
                       (char *) NULL);
      goto error;
    }
    if (property == (Atom) NULL) {
      Tcl_ResetResult(interp);
      Tcl_AppendResult(interp, argv[0], ": no property specified",
                       (char *) NULL);
      goto error;
    }
    XChangeProperty(Tk_Display(tkrootwin), window, property, type,
                    format, mode, argv[dataindex], num_elements);
    if (!continueXAccess) goto Xerror;
  } else if ((c == 'p') && (strncmp(argv[1], "propdelete", length) == 0) &&
      (length >= 5)) {
    XDeleteProperty(Tk_Display(tkrootwin), window, property);
  } else if ((c == 'p') && (strncmp(argv[1], "propget", length) == 0) &&
      (length >= 5)) {
    if (property == (Atom) NULL) {
      atomList = XListProperties(Tk_Display(tkrootwin), window,
                                 &num_elements);
      if (!continueXAccess) goto Xerror;
      if (atomList != (Atom *) NULL) {
        Tcl_ResetResult(interp);
        for (counter = 0; counter < num_elements; counter++) {
          sprintf(buf, "%ld ", atomList[counter]);
          Tcl_AppendResult(interp, buf, (char *) NULL);
        }
        XFree((Atom *) atomList);
      }
    } else {
      if (XGetWindowProperty(Tk_Display(tkrootwin), window, property,
                             long_offset, long_length, delete, req_type,
                             &actual_type, &actual_format,
                             &long_num_elements, &bytes_after,
                             (unsigned char **) &property_return) != Success) {
        Tcl_ResetResult(interp);
        Tcl_AppendResult(interp, argv[0], ": could not read property",
                         (char *) NULL);
	goto error;
      }
      if (actual_type == None) {
        Tcl_ResetResult(interp);
        Tcl_AppendResult(interp, argv[0], ": could not read property",
                         (char *) NULL);
	goto error;
      }
      if (actual_type != XA_STRING) {
        Tcl_ResetResult(interp);
        Tcl_AppendResult(interp, argv[0], ": can only retrieve XA_STRING type",
                         (char *) NULL);
	goto error;
      }
      if (!continueXAccess) goto Xerror;

      counter2 = 0;
      Tcl_ResetResult(interp);
      Tcl_AppendResult(interp,
                       XGetAtomName(Tk_Display(tkrootwin), property),
                       (char *) NULL);
      Tcl_AppendResult(interp, " ", (char *) NULL);
      while (1) {
        if (strcmp(propertyTable[counter2].name, "") == 0) {
          Tcl_AppendResult(interp, "UNDEFINED", (char *) NULL);
          break;
        }
        if (propertyTable[counter2].type == actual_type) {
          Tcl_AppendResult(interp,
                           propertyTable[counter2].name,
                           (char *) NULL);
          break;
        }
        counter2++;
      }
      Tcl_AppendResult(interp, " {", (char *) NULL);
      for (lcounter = 0; lcounter < long_num_elements; lcounter++) {
        sprintf(buf, "%c", property_return[lcounter]);
        Tcl_AppendResult(interp, buf, (char *) NULL);
        if (property_return[lcounter] == '\0' &&
            lcounter < long_num_elements-1) {
          Tcl_AppendResult(interp, "} {", (char *) NULL);
        }
      }
      Tcl_AppendResult(interp, "}", (char *) NULL);
      XFree(property_return);
    }
  } else if ((c == 's') && (strncmp(argv[1], "sendstring", length) == 0) &&  
	     (length >= 2)) {
    if (dataindex == 0) {
      Tcl_ResetResult(interp);
      Tcl_AppendResult(interp, argv[0], ": no data defined",
                       (char *) NULL);
      goto error;
    }
    if (interval != (char *) NULL) {
      interval_value = atol(interval);
    }
    if ((modifierMap = XGetModifierMapping(Tk_Display(tkrootwin))) == NULL) {
      Tcl_ResetResult(interp);
      Tcl_AppendResult(interp, argv[0], ": unable to get ",
                       "modifier mapping", (char *) NULL);
      goto error;
    }
    /* Thanks go to John Ousterhout who implemented this in */
    /* tkBind.c... so I took it from there instead of reimplementing. */
    keyCode = modifierMap->modifiermap;
    for (counter3 = 0; counter3 < 8 * modifierMap->max_keypermod;
	 counter3++, keyCode++) {
      if (*keyCode == 0) {
	continue;
      }
      if (XKeycodeToKeysym(Tk_Display(tkrootwin), *keyCode, 0)
	  == XK_Mode_switch) {
	modeModifierMask |= ShiftMask << (counter3/modifierMap->max_keypermod);
      }
    }
    XFreeModifiermap(modifierMap);
    XDisplayKeycodes(Tk_Display(tkrootwin), &minKeyCode, &maxKeyCode);
    if ((mainKeyMap = XGetKeyboardMapping(Tk_Display(tkrootwin),
					  minKeyCode, (maxKeyCode -
						       minKeyCode + 1),
					  &keysymsPerKeyCode)) == NULL) {
      Tcl_ResetResult(interp);
      Tcl_AppendResult(interp, argv[0], ": unable to get ",
                       "keyboard mapping", (char *) NULL);
      goto error;
    }
    for (counter = 0; counter < (int) strlen(argv[dataindex]);
	 counter++) {
      if (argv[dataindex][counter] == '<') {
	counter++;
	if (counter == (int) strlen(argv[dataindex])) {
	  Tcl_ResetResult(interp);
	  Tcl_AppendResult(interp, argv[0], ": malformed data defined: \"",
			   argv[dataindex], "\"", (char *) NULL);
	  goto error;
	}
	if (argv[dataindex][counter] != '<') {
	  if (argv[dataindex][counter] == 'P' &&
	      argv[dataindex][counter+1] == 'r' &&
	      argv[dataindex][counter+2] == 'e' &&
	      argv[dataindex][counter+3] == 's' &&
	      argv[dataindex][counter+4] == 's' &&
	      argv[dataindex][counter+5] == '-') {
	    counter += 6;
	    if (counter == (int) strlen(argv[dataindex])) {
	      Tcl_ResetResult(interp);
	      Tcl_AppendResult(interp, argv[0], ": malformed data defined: \"",
			       argv[dataindex], "\"", (char *) NULL);
	      goto error;
	    }
	    counter2 = 0;
	    while (counter < (int) strlen(argv[dataindex]) &&
		   argv[dataindex][counter] != '>') {
	      buf[counter2++] = argv[dataindex][counter++];
	    }
	    buf[counter2] = '\0';
	    if (TkXAccessMatchModifierState(buf) != -1) {
	      permanentState |= TkXAccessMatchModifierState(buf);
	    }
	    continue;
	  } else {
	    if (argv[dataindex][counter] == 'R' &&
		argv[dataindex][counter+1] == 'e' &&
		argv[dataindex][counter+2] == 'l' &&
		argv[dataindex][counter+3] == 'e' &&
		argv[dataindex][counter+4] == 'a' &&
		argv[dataindex][counter+5] == 's' &&
		argv[dataindex][counter+6] == 'e' &&
		argv[dataindex][counter+7] == '-') {
	      counter += 8;
	      if (counter == (int) strlen(argv[dataindex])) {
		Tcl_ResetResult(interp);
		Tcl_AppendResult(interp, argv[0],
				 ": malformed data defined: \"",
				 argv[dataindex], "\"", (char *) NULL);
		goto error;
	      }
	      counter2 = 0;
	      while (counter < (int) strlen(argv[dataindex]) &&
		     argv[dataindex][counter] != '>') {
		buf[counter2++] = argv[dataindex][counter++];
	      }
	      buf[counter2] = '\0';
	      if (TkXAccessMatchModifierState(buf) != -1) {
		permanentState &= ~TkXAccessMatchModifierState(buf);
	      }
	      continue;
	    } else {
	      counter2 = 0;
	      while (counter < (int) strlen(argv[dataindex]) &&
		     argv[dataindex][counter] != '>') {
		buf[counter2++] = argv[dataindex][counter++];
	      }
	      buf[counter2] = '\0';
	      if (TkXAccessMatchModifierState(buf) != -1) {
		state |= TkXAccessMatchModifierState(buf);
		continue;
	      }
	    }
	  }
	} else {
	  buf[0] = argv[dataindex][counter];
	  buf[1] = '\0';
	}
      } else {
	buf[0] = argv[dataindex][counter];
	buf[1] = '\0';
      }
      /* now we map certain special character. Is there another way */
      /* to do this (ok besides using a ascii indexed table ??? */
      if (strlen(buf) == 1) {
	switch (buf[0]) {
	case '^':
	  strcpy(buf, "asciicircum");
	  break;
	case '~':
	  strcpy(buf, "asciitilde");
	  break;
	case ' ':
	  strcpy(buf, "space");
	  break;
	case '!':
	  strcpy(buf, "exclam");
	  break;
	case '"':
	  strcpy(buf, "quotedbl");
	  break;
	case '@':
	  strcpy(buf, "at");
	  break;
	case '$':
	  strcpy(buf, "dollar");
	  break;
	case '%':
	  strcpy(buf, "percent");
	  break;
	case '&':
	  strcpy(buf, "ampersand");
	  break;
	case '/':
	  strcpy(buf, "slash");
	  break;
	case '(':
	  strcpy(buf, "parenleft");
	  break;
	case ')':
	  strcpy(buf, "parenright");
	  break;
	case '{':
	  strcpy(buf, "braceleft");
	  break;
	case '}':
	  strcpy(buf, "braceright");
	  break;
	case '[':
	  strcpy(buf, "bracketleft");
	  break;
	case ']':
	  strcpy(buf, "bracketright");
	  break;
	case '=':
	  strcpy(buf, "equal");
	  break;
	case '?':
	  strcpy(buf, "question");
	  break;
	case '\\':
	  strcpy(buf, "backslash");
	  break;
	case '\'':
	  strcpy(buf, "apostrophe");
	  break;
	case '`':
	  strcpy(buf, "grave");
	  break;
	case '+':
	  strcpy(buf, "plus");
	  break;
	case '*':
	  strcpy(buf, "asterisk");
	  break;
	case '#':
	  strcpy(buf, "numbersign");
	  break;
	case '|':
	  strcpy(buf, "bar");
	  break;
	case ',':
	  strcpy(buf, "comma");
	  break;
	case ';':
	  strcpy(buf, "semicolon");
	  break;
	case '.':
	  strcpy(buf, "period");
	  break;
	case ':':
	  strcpy(buf, "colon");
	  break;
	case '-':
	  strcpy(buf, "minus");
	  break;
	case '_':
	  strcpy(buf, "underscore");
	  break;
	case '<':
	  strcpy(buf, "less");
	  break;
	case '>':
	  strcpy(buf, "greater");
	  break;
	case '\t':
	  strcpy(buf, "Tab");
	  break;
	case '\r':
	  strcpy(buf, "Return");
	  break;
	case '\n':
	  strcpy(buf, "Return");
	  break;
	case '\b':
	  strcpy(buf, "BackSpace");
	  break;
	}
      }
      
      keycode = 0;
      tmpKeycode = XKeysymToKeycode(Tk_Display(tkrootwin),
				    XStringToKeysym(buf));
      keySym = XKeycodeToKeysym(Tk_Display(tkrootwin), tmpKeycode, 0);
      if (XKeysymToString(keySym) != NULL &&
	  strcmp(buf, XKeysymToString(keySym)) == 0) {
	keycode = tmpKeycode;
      }
      if (keycode == 0 && keysymsPerKeyCode > 0) {
	keySym = XKeycodeToKeysym(Tk_Display(tkrootwin), tmpKeycode, 1);
	if (XKeysymToString(keySym) != NULL &&
	    strcmp(buf, XKeysymToString(keySym)) == 0) {
	  keycode = tmpKeycode;
	  state |= ShiftMask;
	}
      }
      if (keycode == 0 && keysymsPerKeyCode > 1) {
	keySym = XKeycodeToKeysym(Tk_Display(tkrootwin), tmpKeycode, 2);
	if (XKeysymToString(keySym) != NULL &&
	    strcmp(buf, XKeysymToString(keySym)) == 0) {
	  keycode = tmpKeycode;
	  state |= modeModifierMask;
	}
      }
      if (keycode == 0 && keysymsPerKeyCode > 2) {
	keySym = XKeycodeToKeysym(Tk_Display(tkrootwin), tmpKeycode, 3);
	if (XKeysymToString(keySym) != NULL &&
	    strcmp(buf, XKeysymToString(keySym)) == 0) {
	  keycode = tmpKeycode;
	  state |= ShiftMask;
	  state |= modeModifierMask;
	}
      }
      if (keycode != 0) {
	(void) gettimeofday(&time, (struct timezone *) NULL);
	xevent.type = KeyPress;
	xkey = (XKeyPressedEvent*) &xevent;
	xkey->display = Tk_Display(tkrootwin);
	xkey->window = window;
	xkey->root = root_ret;
	xkey->subwindow = child_ret;
	xkey->time = (time.tv_sec * 1000) + time.tv_usec;
	xkey->x = x;
	xkey->y = y;
	xkey->x_root = x_root;
	xkey->y_root = y_root;
	xkey->state = state | permanentState;
	xkey->keycode = keycode;
	xkey->same_screen = True;
	XSendEvent(Tk_Display(tkrootwin), window, True, 0xffffL, &xevent);
	XSync(Tk_Display(tkrootwin), False);
	if (!continueXAccess) goto Xerror;
	(void) gettimeofday(&time, (struct timezone *) NULL);
	xevent.type = KeyRelease;
	xkey = (XKeyReleasedEvent*) &xevent;
	xkey->display = Tk_Display(tkrootwin);
	xkey->window = window;
	xkey->root = root_ret;
	xkey->subwindow = child_ret;
	xkey->time = (time.tv_sec * 1000) + time.tv_usec;
	xkey->x = x;
	xkey->y = y;
	xkey->x_root = x_root;
	xkey->y_root = y_root;
	xkey->state = state | permanentState;
	xkey->keycode = keycode;
	xkey->same_screen = True;
	XSendEvent(Tk_Display(tkrootwin), window, True, 0xffffL, &xevent);
	XSync(Tk_Display(tkrootwin), False);
	if (!continueXAccess) goto Xerror;
	if (interval_value > 0) {
	  (void) gettimeofday(&startTime, (struct timezone *) NULL);
	  startTime.tv_sec += interval_value;
	  while (1) {
	    (void) gettimeofday(&time, (struct timezone *) NULL);
	    if ((startTime.tv_sec < time.tv_sec) ||
		((startTime.tv_sec == time.tv_sec) &&
		 (startTime.tv_usec < time.tv_usec))) {
	      break;
	    }
	  }
	}
      }
      state = 0;
    }
    XFree ((char *) mainKeyMap);
  } else if ((c == 'x') && (strncmp(argv[1], "xbell", length) == 0) &&  
	     (length >= 2)) {
    if (volume < -100 || volume > 100) {
      volume = 0;
    }
    if (interval == (char *) NULL) {
      XBell(Tk_Display(tkrootwin), volume);
      if (!continueXAccess) goto Xerror;
    } else {
      counter = 0;
      while (counter < (size_t) strlen(interval)) {
	counter3 = 0;
	for (; counter < (size_t) strlen(interval); counter++) {
	  if (interval[counter] == (char ) NULL ||
	      interval[counter] == ' ') {
	    break;
	  }
	  buf[counter3++] = interval[counter];
	}
	buf[counter3] = '\0';
	interval_value = atol(buf);
	if (interval_value > 0) {
	  (void) gettimeofday(&startTime, (struct timezone *) NULL);
	  startTime.tv_sec += interval_value;
	  while (1) {
	    (void) gettimeofday(&time, (struct timezone *) NULL);
	    if ((startTime.tv_sec < time.tv_sec) ||
		((startTime.tv_sec == time.tv_sec) &&
		 (startTime.tv_usec < time.tv_usec))) {
	      break;
	    }
	  }
	  XBell(Tk_Display(tkrootwin), volume);
	  XSync(Tk_Display(tkrootwin), False);
	  if (!continueXAccess) goto Xerror;
	} else {
	  break;
	}
	counter++;
      }
    }
  } else if ((c == 'x') && (strncmp(argv[1], "xwinchilds", length) == 0) &&  
	     (length >= 5)) {
    Tcl_ResetResult(interp);
    for (counter = 0; counter < num_child; counter++) {
      if (counter == num_child - 1) {
        sprintf(buf, "%ld", child_list[counter]);
      } else {
        sprintf(buf, "%ld ", child_list[counter]);
      }
      Tcl_AppendResult(interp, buf, (char *) NULL);
    }
  } else if ((c == 'x') && (strncmp(argv[1], "xwinname", length) == 0) &&  
	     (length >= 5)) {
    Tcl_ResetResult(interp);
    if (!XFetchName(Tk_Display(tkrootwin), window, &win_name)) {
      if (!continueXAccess) goto Xerror;
      Tcl_AppendResult(interp, "", (char *) NULL);
    } else {
      if (!continueXAccess) goto Xerror;
      Tcl_AppendResult(interp, win_name, (char *) NULL);
    }
    XFree(win_name);
  } else if ((c == 'x') && (strncmp(argv[1], "xwinparent", length) == 0) &&  
	     (length >= 5)) {
    Tcl_ResetResult(interp);
    sprintf(buf, "%ld", parent_ret);
    Tcl_AppendResult(interp, buf, (char *) NULL);
  } else if ((c == 'x') && (strncmp(argv[1], "xwinroot", length) == 0) &&  
	     (length >= 5)) {
    Tcl_ResetResult(interp);
    sprintf(buf, "%ld", root_ret);
    Tcl_AppendResult(interp, buf, (char *) NULL);
  } else {
    Tcl_ResetResult(interp);
    Tcl_AppendResult(interp, argv[0], ": unknown command \"", argv[1], "\"",
		     (char *) NULL);
    goto error;
  }
  XFlush(Tk_Display(tkrootwin));
  XFree(child_list);
  Tk_DeleteErrorHandler(handler1);
  Tk_DeleteErrorHandler(handler2);
  Tk_DeleteErrorHandler(handler3);
  return TCL_OK;

 Xerror:
  XFlush(Tk_Display(tkrootwin));
  Tcl_ResetResult(interp);
  Tcl_AppendResult(interp, argv[0], ": X protocol error", (char *) NULL);
  
 error:
  if (property_return != (char *) NULL) {
    XFree(property_return);
  }
  if (child_list != (Window *) NULL) {
    XFree(child_list);
  }
  if (atomList != (Atom *) NULL) {
    XFree((Atom *) atomList);
  }
  Tk_DeleteErrorHandler(handler1);
  Tk_DeleteErrorHandler(handler2);
  Tk_DeleteErrorHandler(handler3);
  return TCL_ERROR;
}

/*
 *----------------------------------------------------------------------
 *
 * TkSteal_Init --
 *
 *      This procedure performs application-specific initialization.
 *      Most applications, especially those that incorporate additional
 *      packages, will have their own version of this procedure.
 *
 * Results:
 *      Returns a standard Tcl completion code, and leaves an error
 *      message in interp->result if an error occurs.
 *
 * Side effects:
 *      Depends on the startup script.
 *
 *----------------------------------------------------------------------
 */
int
TkSteal_Init(interp)
    Tcl_Interp *interp;         /* Interpreter for application. */
{
  /*
   * Call Tcl_CreateCommand for application-specific commands.
   */
#if (TK_MAJOR_VERSION >= 3) && (TK_MINOR_VERSION >= 3)
  Tcl_CreateCommand(interp, "xaccess", TkXAccessCmd,
    (ClientData) Tk_MainWindow(interp), (Tcl_CmdDeleteProc *) NULL);
#endif
  
  return TCL_OK;
}

#endif

/* eof */

