/* 
 * xpm3.c --
 *
 *	This file provides access to xpm3 pixmaps.
 *
 * Copyright (c) 1993 by Sven Delmas
 * All rights reserved.
 * See the file COPYRIGHT for the copyright notes.
 *
 */

#if defined (USE_XPM3)

#ifndef lint
static char *AtFSid = "$Header: xpm3.c[5.0] Tue Sep 13 20:20:45 1994 garfield@mydomain.de frozen $";
#endif /* not lint */

#include "tkConfig.h"
#include "tk.h"
#include "X11/xpm.h"

/*
 *----------------------------------------------------------------------
 *
 * InternalXpm3Get --
 *
 *      This procedure is invoked to read a Xpm3 file.
 *
 * Results:
 *      A standard Tcl result.
 *
 * Side effects:
 *      None
 *
 *----------------------------------------------------------------------
 */

int
InternalXpm3Get(interp, tkwin, filename, width, height, depth, pixmap,
		clipMask)
     Tcl_Interp *interp;         /* Current interpreter. */
     Tk_Window tkwin;            /* The window */
     char *filename;             /* The filename to load. */
     unsigned int *width;        /* The pixmap width. */
     unsigned int *height;       /* The pixmap height. */
     unsigned int *depth;        /* The pixmap depth. */
     Pixmap *pixmap;             /* The resulting pixmap. */
     Pixmap *clipMask;           /* The resulting clipmask. */
{
  Colormap cmap;
  XpmAttributes xpm_attributes;

  cmap = Tk_Colormap(tkwin);
  xpm_attributes.valuemask = 0;
  xpm_attributes.valuemask |= XpmCloseness;
  xpm_attributes.closeness = 150000;
  xpm_attributes.valuemask |= XpmColormap;
  xpm_attributes.colormap = cmap;
  if (XpmReadFileToPixmap(Tk_Display(tkwin), Tk_WindowId(tkwin),
			  filename, pixmap, clipMask,
			  &xpm_attributes) != BitmapSuccess) { 
    return TCL_ERROR;
  }
  if (clipMask == (Pixmap *) NULL) {
    *clipMask = None;
  }
  *width = xpm_attributes.width;
  *height = xpm_attributes.height;
  *depth = 0;
  XpmFreeAttributes(&xpm_attributes);
  return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * InternalXpm3Put --
 *
 *      This procedure is invoked to save a xpm3 file.
 *
 * Results:
 *      A standard Tcl result.
 *
 * Side effects:
 *      None
 *
 *----------------------------------------------------------------------
 */

int
InternalXpm3Put(interp, tkwin, filename, width, height, depth, pixmap,
		clipMask)
     Tcl_Interp *interp;         /* Current interpreter. */
     Tk_Window tkwin;            /* The window */
     char *filename;             /* The filename to load. */
     unsigned int width;         /* The pixmap width. */
     unsigned int height;        /* The pixmap height. */
     unsigned int depth;         /* The pixmap depth. */
     Pixmap pixmap;              /* The resulting pixmap. */
     Pixmap clipMask;            /* The resulting clipmask. */
{
  Colormap cmap;
  XpmAttributes xpm_attributes;

  cmap = Tk_Colormap(tkwin);
  xpm_attributes.valuemask = 0;
  xpm_attributes.valuemask |= XpmSize;
  xpm_attributes.width = width;
  xpm_attributes.height = height;
  xpm_attributes.valuemask |= XpmColormap;
  xpm_attributes.colormap = cmap;
  if (XpmWriteFileFromPixmap(Tk_Display(tkwin), filename, pixmap,
			     clipMask, &xpm_attributes) == XpmSuccess) {
    XpmFreeAttributes(&xpm_attributes);
    return TCL_OK;
  }
  XpmFreeAttributes(&xpm_attributes);
  return TCL_ERROR;
}

/*
 *----------------------------------------------------------------------
 *
 * InternalXpm3ToPixmap --
 *
 *      This procedure is invoked to create pixmap from ascii data.
 *
 * Results:
 *      A standard Tcl result.
 *
 * Side effects:
 *      None
 *
 *----------------------------------------------------------------------
 */

int
InternalXpm3ToPixmap(interp, tkwin, source, width, height, depth,
		     pixmap, clipMask)
     Tcl_Interp *interp;         /* Current interpreter. */
     Tk_Window tkwin;            /* The window */
     char *source;               /* Pixmap source. */
     unsigned int width;         /* The pixmap width. */
     unsigned int height;        /* The pixmap height. */
     unsigned int depth;         /* The pixmap depth. */
     Pixmap *pixmap;             /* The resulting pixmap. */
     Pixmap *clipMask;           /* The resulting clipMask. */
{
  int listCounter = 0;
  char **list;
  XpmAttributes xpm_attributes;

  if (Tcl_SplitList(interp, source,
		    &listCounter, &list) == TCL_OK) {
    xpm_attributes.valuemask = 0;
    xpm_attributes.valuemask |= XpmCloseness;
    xpm_attributes.closeness = 150000;
    if (XpmCreatePixmapFromData(Tk_Display(tkwin), Tk_WindowId(tkwin),
				list, pixmap, clipMask,
				&xpm_attributes) != BitmapSuccess) {
      ckfree((char *) list);
      XpmFreeAttributes(&xpm_attributes);
      return TCL_ERROR;
    }
    XpmFreeAttributes(&xpm_attributes);
    ckfree((char *) list);
  } else {
    Tcl_AppendResult(interp, "wrong pixmap data format\n",
		     (char *) NULL);
    return TCL_ERROR;
  }
  return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * InternalXpm3ToAscii --
 *
 *      This procedure is invoked to create pixmap ascii data.
 *
 * Results:
 *      A standard Tcl result.
 *
 * Side effects:
 *      None
 *
 *----------------------------------------------------------------------
 */

int
InternalXpm3ToAscii(interp, tkwin, pixmap, width, height, depth)
     Tcl_Interp *interp;         /* Current interpreter. */
     Tk_Window tkwin;            /* any window on screen where pixmap lives */
     Pixmap pixmap;              /* The resulting pixmap. */
     unsigned int width;         /* The pixmap width. */
     unsigned int height;        /* The pixmap height. */
     unsigned int depth;         /* The pixmap depth. */
{
  int counter, ncolors;
  char tmpBuffer[30];
  char **pixmapData, **tmpPixmapData;
  XpmAttributes xpm_attributes;
    
  xpm_attributes.valuemask = 0;
  if (Tk_GetPixmapClipMask(Tk_Display(tkwin), pixmap) != None) {
    XpmCreateDataFromPixmap(Tk_Display(tkwin), &pixmapData, pixmap,
			    Tk_GetPixmapClipMask(Tk_Display(tkwin), pixmap),
			    &xpm_attributes);
  } else {
    XpmCreateDataFromPixmap(Tk_Display(tkwin), &pixmapData, pixmap,
			    (Pixmap) NULL, &xpm_attributes);
  }
  if (pixmapData && *pixmapData) {
    tmpPixmapData = pixmapData;
    sscanf(*tmpPixmapData, "%d %d %d %d", &width, &height,
	   &ncolors, &counter);
    counter = height + ncolors + 1;
    sprintf(tmpBuffer, "%d %d 0 xpm3", width, height);
    Tcl_AppendResult(interp, "{", tmpBuffer, "} {",
		     (char *) NULL);
    while(counter > 0) {
      Tcl_AppendResult(interp, "{", *tmpPixmapData, "} ",
		       (char *) NULL);
      tmpPixmapData++;
      counter--;
    }
    Tcl_AppendResult(interp, "}", (char *) NULL);
  } else {
    strcpy(interp->result, "");
  }
  ckfree((char *) pixmapData);
  return TCL_OK;
}

#endif

/* eof */

