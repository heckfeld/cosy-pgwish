/* 
 * bitmap.c --
 *
 *	This file provides access to bitmaps.
 *
 * Copyright (c) 1993 by Sven Delmas
 * All rights reserved.
 * See the file COPYRIGHT for the copyright notes.
 *
 */

#ifndef lint
static char *AtFSid = "$Header: bitmap.c[5.0] Mon Jul 25 18:06:16 1994 garfield@mydomain.de frozen $";
#endif /* not lint */

#include "tkConfig.h"
#include "tk.h"

/* directly stolen from Xlib :-) */
/* $XConsortium: XWrBitF.c,v 1.9 91/02/01 16:34:58 gildea Exp $ */
/* Copyright, 1987, Massachusetts Institute of Technology */

/*
Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in advertising or
publicity pertaining to distribution of the software without specific,
written prior permission.  M.I.T. makes no representations about the
suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.
*/

char *bitmap_formatImage(image, resultsize)
XImage *image;
int *resultsize;
{
  register int x, c, b;
  register char *ptr;
  int y;
  char *data;
  int width, height;
  int bytes_per_line;

  width = image->width;
  height = image->height;

  bytes_per_line = (width+7)/8;
  *resultsize = bytes_per_line * height;           /* Calculate size of data */

  data = (char *) ckalloc( *resultsize );          /* Get space for data */
  if (!data)
    return(0);

  /*
   * The slow but robust brute force method of converting the image:
   */
  ptr = data;
  c = 0; b=1;
  for (y=0; y<height; y++) {
    for (x=0; x<width;) {
      if (XGetPixel(image, x, y))
        c |= b;
      b <<= 1;
      if (!(++x & 7)) {
        *(ptr++)=c;
        c=0; b=1;
      }
    }
    if (x & 7) {
      *(ptr++)=c;
      c=0; b=1;
    }
  }

  return(data);
}

/*
 *----------------------------------------------------------------------
 *
 * InternalBitmapGet --
 *
 *      This procedure is invoked to read a bitmap file.
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
InternalBitmapGet(interp, tkwin, filename, width, height, depth,
		  pixmap, clipMask)
     Tcl_Interp *interp;         /* Current interpreter. */
     Tk_Window tkwin;            /* The window */
     char *filename;             /* The filename to load. */
     unsigned int *width;        /* The pixmap width. */
     unsigned int *height;       /* The pixmap height. */
     unsigned int *depth;        /* The pixmap depth. */
     Pixmap *pixmap;             /* The resulting pixmap. */
     Pixmap *clipMask;           /* The resulting clipmask. */
{
  int dummy2;

  if (XReadBitmapFile(Tk_Display(tkwin), Tk_WindowId(tkwin), filename,
		      width, height, pixmap, &dummy2,
		      &dummy2) != BitmapSuccess) {
    return TCL_ERROR;
  }
  *clipMask = None;
  *depth = 1;
  return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * InternalBitmapPut --
 *
 *      This procedure is invoked to save a bitmap file.
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
InternalBitmapPut(interp, tkwin, filename, width, height, depth,
		  pixmap, clipMask)
     Tcl_Interp *interp;         /* Current interpreter. */
     Tk_Window tkwin;            /* The window */
     char *filename;             /* The filename to load. */
     unsigned int width;         /* The pixmap width. */
     unsigned int height;        /* The pixmap height. */
     unsigned int depth;         /* The pixmap depth. */
     Pixmap pixmap;              /* The resulting pixmap. */
     Pixmap clipMask;            /* The resulting clipmask. */
{
  if (depth != 1) {
    return TCL_ERROR;
  }
  if (XWriteBitmapFile(Tk_Display(tkwin), filename, pixmap,
		       width, height, -1, -1) == BitmapSuccess) {
    return TCL_OK;
  }
  return TCL_ERROR;
}

/*
 *----------------------------------------------------------------------
 *
 * InternalBitmapToPixmap --
 *
 *      This procedure is invoked to create a bitmap from data.
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
InternalBitmapToPixmap(interp, tkwin, source, width, height, depth,
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
  if (depth != 1) {
    return TCL_ERROR;
  }
  *pixmap =
    XCreateBitmapFromData(Tk_Display(tkwin), Tk_WindowId(tkwin),
			  source, width, height);
  *clipMask = None;
  return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * InternalBitmapToAscii --
 *
 *      This procedure is invoked to create bitmap ascii data.
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
InternalBitmapToAscii(interp, tkwin, pixmap, width, height, depth)
     Tcl_Interp *interp;         /* Current interpreter. */
     Tk_Window tkwin;            /* any window on screen where pixmap lives */
     Pixmap pixmap;              /* The resulting pixmap. */
     unsigned int width;         /* The pixmap width. */
     unsigned int height;        /* The pixmap height. */
     unsigned int depth;         /* The pixmap depth. */
{
  int d, counter, byte;
  char tmpBuffer[30];
  char *bitmapData, *tmpBitmapData;
  XImage *image;
  
  if (depth != 1) {
    return TCL_ERROR;
  }
  /* Convert bitmap to an image */
  image = XGetImage(Tk_Display(tkwin), pixmap, 0, 0, width, height,
		    1L, XYPixmap);
  /* Get standard format for data */
  bitmapData = bitmap_formatImage(image, &counter);
  XDestroyImage(image);
  if (bitmapData) {
    sprintf(tmpBuffer, "%d %d 1 bitmap", width, height);
    Tcl_AppendResult(interp, "{", tmpBuffer, "} {",
		     (char *) NULL);
    for (byte = 0, tmpBitmapData = bitmapData; byte < counter;
	 byte++, tmpBitmapData++) {
      if (byte) {
	Tcl_AppendResult(interp, ", ", (char *) NULL);
      }
      d = *tmpBitmapData;
      if (d < 0) {
	d += 256;
      }
      sprintf(tmpBuffer, "0x%02x", d);
      Tcl_AppendResult(interp, tmpBuffer, (char *) NULL);
    }
    Tcl_AppendResult(interp, "} ", (char *) NULL);
  } else {
    strcpy(interp->result, "");
  }
  ckfree((char *) bitmapData);
  return TCL_OK;
}

/* eof */

