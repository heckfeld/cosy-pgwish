/*
 * raw256.c --
 *
 *      This file provides access to raw pixmaps.
 *
 * by Frank Sauer (sauer@iitcsun3.med.miami.edu)
 *
 * Copyright (c) 1993 by Sven Delmas
 * All rights reserved.
 * See the file COPYRIGHT for the copyright notes.
 *
 * routines for reading raw 256x256  8 or 16-bit GrayScale MRI images
 * Note that 16 bit really is 12 bit, of which the lower 4 bits are dropped
 * after reading.
 *
 * NOTE images are currently being DOUBLED, so read into 512x512 matrix!!!
 */

#if defined (USE_RAW256)

#ifndef lint
static char *AtFSid = "$Header: raw256.c[5.0] Thu May  5 00:14:03 1994 garfield@mydomain.de frozen $";
#endif /* not lint */

#include "tkConfig.h"
#include "tk.h"
#include <sys/types.h>
#include <sys/stat.h>

typedef unsigned char byte;

static Colormap cmap = (Colormap) NULL;

static
unsigned int sendRawImageToX(disp, scrn, visual, image, pixmap)
     Display      *disp;
     int           scrn;
     Visual       *visual;
     byte         *image;
     Pixmap       *pixmap;
{
  unsigned int  a, x, y, ddepth;
  byte         *pixptr;
  XColor        xcolor;
  GC            gc;
  XImage       *ximage;
 
 
  ddepth= DefaultDepth(disp, scrn);
  *pixmap= XCreatePixmap(disp, RootWindow(disp, scrn), 512,
                         512, ddepth);
  gc= XCreateGC(disp, *pixmap, 0, 0);
 
 
  pixptr= image;
 
  ximage= XCreateImage(disp, visual, ddepth, ZPixmap, (int) NULL, (char *) 0, 512, 512, 8, (int) NULL);  ximage->data = (char *) malloc(ximage->bytes_per_line * 512);
 
  for(y=0;y<512;y++)
    for (x=0;x<512;x++) {
      a = (int) pixptr[y*512+x];
      XPutPixel(ximage, x,y,a);
  }
 
  XPutImage(disp, *pixmap, gc, ximage, 0, 0, 0, 0, 512, 512);
  XDestroyImage(ximage); /* waste not want not */
  XFreeGC(disp, gc);
 
  return(1);
}
 
 
static
byte *rawload(fullname)
     char         *fullname;

{ FILE *fp;
  byte *image;
  int x,y,bpp;
  unsigned int size,v;
  byte *b2;
  struct stat st;
 
  fp = fopen(fullname, "r");
  if( fp == NULL ){
    return NULL;
  }
 
  b2 = (byte *)&v;
  stat(fullname,&st);
  bpp = st.st_size/65536;
  if (((bpp != 1) && (bpp != 2)) || (st.st_size % 65536 != 0)) {
    fclose(fp);
    return NULL;
  }
  image = (byte *)malloc(4*65536);
  if (image != NULL) {
    for(y=0;y<512;y += 2)
      for(x=0;x<512;x += 2) {
        fread (&v, bpp, 1, fp);
        image[y*512+x] = (byte)(b2[0]);
        image[y*512+x+1] = (byte)(b2[0]);
        image[(y+1)*512+x] = (byte)(b2[0]);
        image[(y+1)*512+x+1] = (byte)(b2[0]);
      }
  }
  fclose(fp);
  return(image);
}
 
/*
 *----------------------------------------------------------------------
 *
 * InternalRaw256Get --
 *
 *      This procedure is invoked to read a Raw 16bit 256x256 MRI image
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
InternalRaw256Get(interp, tkwin, filename, width, height, depth,
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
  Display *display;
  byte *image; 
  XColor gray_scale[256];        /* Gray-scale values */
  int screen;
  int i;

  display = Tk_Display(tkwin);
  screen = DefaultScreen(display);
  if (cmap == (Colormap) NULL) {
    cmap = XCreateColormap(display, Tk_WindowId(tkwin),
			   DefaultVisual(display,screen), AllocAll);
    for (i=0;i<256;i++)
      { gray_scale[i].red = i+256*i;
	gray_scale[i].green = i+256*i;
	gray_scale[i].blue = i+256*i;
	gray_scale[i].pixel = i;
	gray_scale[i].flags = DoRed|DoBlue|DoGreen;
      }
  }
  
  image = rawload(filename);
  if (image == NULL)
     return TCL_ERROR;
 
  Tk_SetWindowColormap(Tk_WindowId(tkwin), cmap);
 
  sendRawImageToX(display, screen, DefaultVisual(display,screen),
		  image, pixmap);

  *width = 512;
  *height = 512;
  *depth = 8;
  free(image);
  return TCL_OK;
}

#endif

/* eof */

