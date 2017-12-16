/*
 * tkPhoto.h --
 *	Definitions for clients of the photo widget
 *
 * Copyright 1993 The Australian National University.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, provided that the above copyright
 * notice appears in all copies.  This software is provided without any
 * warranty, express or implied. The Australian National University
 * makes no representations about the suitability of this software for
 * any purpose.
 *
 * Author: Paul Mackerras (paulus@cs.anu.edu.au)
 *
 * $Header: /home/paulus/CaVis/othersrc/tk3.3/RCS/tkPhoto.h,v 1.5 1993/10/21 23:58:21 paulus Exp $
 */
#include <tk.h>
#include <X11/Xutil.h>		/* for XVisualInfo */

typedef void *PhotoHandle;

typedef struct image_block {
    unsigned char *ptr;		/* points to image data */
    int		width;		/* horiz # pixels in block */
    int		height;		/* vert # pixels in block */
    int		pitch;		/* # bytes stored per line */
    int		pixel_size;	/* # bytes per pixel */
    int		comp_off[3];	/* address offset to R,G,B components */
} PhotoImage;

PhotoHandle FindPhoto _ANSI_ARGS_((char *window_path));

void	PhotoPutBlock _ANSI_ARGS_((PhotoHandle handle, PhotoImage *block,
				   int dest_x, int dest_y,
				   int dest_width, int dest_height));

void	PhotoPutZoomedBlock _ANSI_ARGS_((PhotoHandle handle, PhotoImage *block,
					 int dest_x, int dest_y,
					 int dest_width, int dest_height,
					 int zoom_x, int zoom_y,
					 int decimate_x, int decimate_y));

int	PhotoGetImage _ANSI_ARGS_((PhotoHandle handle, PhotoImage *block));

void	PhotoBlank _ANSI_ARGS_((PhotoHandle handle));

void	PhotoExpand _ANSI_ARGS_((PhotoHandle handle, int width, int height ));

void	PhotoGetSize _ANSI_ARGS_((PhotoHandle handle,
				  int *widthp, int *heightp));

void	PhotoSetSize _ANSI_ARGS_((PhotoHandle handle, int width, int height));

void	PhotoGetInfo _ANSI_ARGS_((PhotoHandle handle, Tk_Window *pTkwin,
				  XVisualInfo *pVisInfo, Colormap *pCmap));

void	PhotoPutXImage _ANSI_ARGS_((PhotoHandle handle, GC gc, XImage *image,
				    int src_x, int src_y, int dest_x,
				    int dest_y, int width, int height));

int	Photo_Init _ANSI_ARGS_((Tcl_Interp *));
