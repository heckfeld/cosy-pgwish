/* 
 * gif.c --
 *
 *	This file provides access to gif pixmaps.
 *
 * Copyright (c) 1993 by Sven Delmas
 * All rights reserved.
 * See the file COPYRIGHT for the copyright notes.
 *
 *
 * adapted from code by Alfred Kayser, DNPAP (the package is named xgif).
 *
 * Copyright by Alfred Kayser, DNPAP
 *
 *
 * minor pieces of code have been taken from the code published by
 *   jim frost and kirk johnson (xloadimage)
 *
 * Copyright 1989, 1990 Kirk L. Johnson
 *
 * Permission to use, copy, modify, distribute, and sell this
 * software and its documentation for any purpose is hereby granted
 * without fee, provided that the above copyright notice appear in
 * all copies and that both that copyright notice and this
 * permission notice appear in supporting documentation. The
 * author makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express
 * or implied warranty.
 *
 * THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, INDIRECT
 * OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * Copyright 1989, 1990 Jim Frost
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  The author makes no representations
 * about the suitability of this software for any purpose.  It is
 * provided "as is" without express or implied warranty.
 *
 * THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE
 * USE OR PERFORMANCE OF THIS SOFTWARE.
 */
  
#if defined (USE_GIF)

#ifndef lint
static char *AtFSid = "$Header: gif.c[5.0] Mon Jul 25 18:06:17 1994 garfield@mydomain.de frozen $";
#endif /* not lint */

/* include files */
#include "tkConfig.h"
#include "tcl.h"
#include "tk.h"

#define MAX_ARGS     100

#define WHERE static

typedef unsigned char byte;

/* X stuff */
WHERE Display       *theDisp;
WHERE int           theScreen, dispcells, dispWIDE, dispHIGH;
WHERE Colormap      theCmap, LocalCmap;
WHERE Window        rootW, mainW;
WHERE GC            theGC;
WHERE Visual        *theVisual;
WHERE XImage        *theImage;

/* global vars */
WHERE unsigned long  cols[256];
WHERE XColor         defs[256];
WHERE int            quiet, local, DEBUG;

#define IMAGESEP 0x2c
#define EXTENSION 0x21
#define INTERLACEMASK 0x40
#define COLORMAPMASK 0x80
#define ADDTOPIXEL(a) if (Quick) Image[Offset++]=a; else AddToPixel(a)

#define ALLOCATED 3

static int EGApalette[16][3] = {
  {0,0,0},       {0,0,128},     {0,128,0},     {0,128,128}, 
  {128,0,0},     {128,0,128},   {128,128,0},   {200,200,200},
  {100,100,100}, {100,100,255}, {100,255,100}, {100,255,255},
  {255,100,100}, {255,100,255}, {255,255,100}, {255,255,255} };

static int BitOffset,		/* Bit Offset of next code */
    XC, YC,                     /* Output X and Y coords of current pixel */
    Offset,                     /* Offset in output array */
    Pass,			/* Used by output routine if interlaced pic */
    Width, Height,		/* image dimensions */
    BytesPerScanline,		/* bytes per scanline in output raster */
    ColorMapSize,		/* number of colors */
    Background,			/* background color */
    NumUsed,			/* Number of colors really used */
    CodeSize,			/* Code size, read from GIF header */
    ReadMask,			/* Code AND mask for current code size */
    Interlace,
    HasColormap;

static byte *Image;		/* The result array */
static byte *Raster;		/* The raster data stream, unblocked */

    /* The GIF headers found in the file */
static byte gifheader[13];
static byte imageheader[9];
static byte colormap[3*256];

    /* The hash table used by the decompressor */
static int  Prefix[4096];
static int  Suffix[4096];

    /* An output array used by the decompressor */
static byte OutCode[1025];

    /* The color map, read from the GIF header */
static byte Red[256], Green[256], Blue[256], used[256];

static char id[] = "GIF";


int
ReadImageData(fp)
FILE *fp;
{
/* Read the raster data.  Here we just transpose it from the GIF array
 * to the Raster array, turning it from a series of blocks into one long
 * data stream, which makes life much easier for ReadCode().
 */
    long filesize, filepos;
    int ch;
    byte *ptr1;

    /* find the size of the file */
    filepos = ftell(fp);
    fseek(fp, 0L, 2);
    filesize = ftell(fp)-filepos;
    fseek(fp, filepos, 0);

    if (!(Raster = (byte *) malloc(filesize)))
      return 1;

    ptr1 = Raster;
    while ((ch = getc(fp))>0)
    {
	if ((int) fread(ptr1, 1, ch, fp) < (int) ch)
	  return 1;
        ptr1 += ch;
    } 
    return 0;
}


/* Fetch the next code from the raster data stream.  The codes can be
 * any length from 3 to 12 bits, packed into 8-bit bytes, so we have to
 * maintain our location in the Raster array as a BIT Offset.  We compute
 * the byte Offset into the raster array by dividing this by 8, pick up
 * three bytes, compute the bit Offset into our 24-bit chunk, shift to
 * bring the desired code to the bottom, then mask it off and return it. 
 */
int
ReadCode()
{
    int RawCode, ByteOffset, BitShift;

    ByteOffset = BitOffset / 8;
    BitShift = BitOffset % 8;
    BitOffset += CodeSize;
    if (BitShift+CodeSize<8)
	return (Raster[ByteOffset]>>BitShift) & ReadMask;
    else
    {
        RawCode = Raster[ByteOffset] + (0x100 * Raster[ByteOffset + 1]);
        if (BitShift+CodeSize >= 16)
	    RawCode += (0x10000 * Raster[ByteOffset + 2]);
        return((RawCode>>BitShift) & ReadMask);
    }
}


void
AddToPixel(Index)
byte Index;
{
    if (YC<Height) /* Might be of importance when reading interlaced gifs */
        Image[YC*BytesPerScanline+XC] = Index;
    if (!used[Index]) { used[Index]=True; NumUsed++; }
    if (++XC == Width)
    {
	XC = 0;
	if (Interlace)
	{
	    switch (Pass) 
            {
	    case 0: YC += 8; if (YC >= Height) { Pass++; YC = 4; } break;
            case 1: YC += 8; if (YC >= Height) { Pass++; YC = 2; } break;
            case 2: YC += 4; if (YC >= Height) { Pass++; YC = 1; } break;
            case 3: YC += 2; break;
            default: break;
	    }
	}
	else
	    YC++;
    }
}


int
DecodeImage()
{
/* Start reading the raster data. First we get the intial code size
 * and compute decompressor constant values, based on this code size.
 */
int Quick = 0,			/* True, when not interlaced and local Cmap */
    InitCodeSize = 0,		/* Starting code size, used during Clear */
    InCode = 0,			/* Value returned by ReadCode */
    MaxCode = 0,		/* limiting value for current code size */
    ClearCode = 0,		/* GIF clear code */
    EOFCode = 0,		/* GIF end-of-information code */
    CurCode = 0, OldCode = 0,	/* Decompressor variables */
    FreeCode = 0,		/* Decompressor, next free slot in hashtable */
    OutCount = 0,		/* Decompressor output 'stack count' */
    FinChar = 0,		/* Decompressor variable */
    BitMask = 0;		/* AND mask for data size */

    BitMask = ColorMapSize - 1;

    ClearCode = (1 << CodeSize);
    EOFCode = ClearCode + 1;
    FreeCode = ClearCode + 2;

/* The GIF spec has it that the code size is the code size used to
 * compute the above values is the code size given in the file, but the
 * code size used in compression/decompression is the code size given in
 * the file plus one. (thus the ++).
 */

    CodeSize++;
    InitCodeSize = CodeSize;
    MaxCode = (1 << CodeSize);
    ReadMask = MaxCode - 1;

/* Allocate the X Image */
    if (!(Image = (byte *) malloc(Width*Height)))
      return 1;

    BytesPerScanline = Width;

/* Decompress the file, continuing until you see the GIF EOF code.
 * One obvious enhancement is to add checking for corrupt files here.
 */
    Quick = (local && !Interlace);
    Offset = 0; 
    if (DEBUG) fprintf(stderr,"Decoding...\n");
    InCode = ReadCode();
    while (InCode != EOFCode) {

/* Clear code sets everything back to its initial value, then reads the
 * immediately subsequent code as uncompressed data.
 */

	if (InCode == ClearCode) {
	    CodeSize = InitCodeSize;
	    MaxCode = (1 << CodeSize);
	    ReadMask = MaxCode - 1;
            FreeCode = ClearCode + 2;
	    CurCode = OldCode = InCode = ReadCode();
	    FinChar = CurCode & BitMask;
	    ADDTOPIXEL(FinChar);
	}
	else {

/* If not a clear code, then must be data: save same as CurCode */

	    CurCode = InCode;

/* If greater or equal to FreeCode, not in the hash table yet;
 * repeat the last character decoded
 */

	    if (CurCode >= FreeCode) {
		CurCode = OldCode;
		OutCode[OutCount++] = FinChar;
	    }

/* Unless this code is raw data, pursue the chain pointed to by CurCode
 * through the hash table to its end; each code in the chain puts its
 * associated output code on the output queue.
 */

	    while (CurCode > BitMask) {
		if (OutCount >= 1024) {
		    fprintf(stderr,"\nCorrupt GIF file (OutCount)!\n");
                    exit(1);  
                }
		OutCode[OutCount++] = Suffix[CurCode];
		CurCode = Prefix[CurCode];
	    }

/* The last code in the chain is treated as raw data. */

	    /* OutCode[OutCount++] = FinChar = CurCode &BitMask*/;
	    FinChar = CurCode & BitMask;
	    ADDTOPIXEL(FinChar);

/* Now we put the data out to the Output routine.
 * It's been stacked LIFO, so deal with it that way...  */
	    while (OutCount>0)
		ADDTOPIXEL(OutCode[--OutCount]);

/* Build the hash table on-the-fly. No table is stored in the file. */

	    Prefix[FreeCode] = OldCode;
	    Suffix[FreeCode] = FinChar;
	    OldCode = InCode;

/* Point to the next slot in the table.  If we exceed the current
 * MaxCode value, increment the code size unless it's already 12.  If it
 * is, do nothing: the next code decompressed better be CLEAR
 */

	    FreeCode++;
	    if (FreeCode >= MaxCode) {
		if (CodeSize < 12) {
		    CodeSize++;
		    MaxCode *= 2;
		    ReadMask = (1 << CodeSize) - 1;
		}
	    }
	}
	InCode = ReadCode();
    }
    free(Raster);
    return 0;
}


int
ColorDicking()
{
    /* we've got the picture loaded, we know what colors are needed. get 'em */
    int i,j;
    register byte *ptr;

    if (!HasColormap)
    {
	if (DEBUG) fprintf(stderr,"Using EGA palette as default\n");
        for (i=0; i<ColorMapSize; i++) {
            Red[i] = EGApalette[i&15][0];
            Green[i] = EGApalette[i&15][1];
            Blue[i] = EGApalette[i&15][2];
            used[i] = True;
	    }
    }

    for (i=j=0; i<ColorMapSize; i++)
    {
        if (local || used[i])
	{
	    defs[i].red   = Red[i]<<8;
	    defs[i].green = Green[i]<<8;
	    defs[i].blue  = Blue[i]<<8;
	    defs[i].flags = DoRed | DoGreen | DoBlue;
	    defs[i].pixel = i;
        }
    }
    if (local && HasColormap)
    { 
	LocalCmap=XCreateColormap(theDisp,mainW,theVisual,AllocAll);
	XStoreColors(theDisp,LocalCmap,defs,ColorMapSize);
	return 0;
    }

    if (!quiet) fprintf(stderr,"Allocating %d colors...\n",NumUsed);

    /* Allocate the X colors for this picture */
    for (i=j=0; i<ColorMapSize; i++)
    {
        if (local || used[i])
	{
            if (!XAllocColor(theDisp,theCmap,&defs[i])
	      || defs[i].red != Red[i]<<8
	      || defs[i].green != Green[i]<<8
              || defs[i].blue != Blue[i]<<8) /* Not an exact match! */
	    { 
                j++;
		defs[i].pixel = 0xffff;
            }
	    else used[i]=ALLOCATED;
            cols[i] = defs[i].pixel;
        }
	else cols[i]=i;
    }
    if (j) 		/* failed to pull it off */
    {
        XColor ctab[256];
        int dc = (dispcells<256) ? dispcells : 256;

        if (!quiet)
	    fprintf(stderr,"Failed to allocate %d out of %d colors. \n%s",
			  j,NumUsed,  "   Searching for resembling colors\n");

        /* read in the color table */
        for (i=0; i<dc; i++) ctab[i].pixel = i;
        XQueryColors(theDisp, theCmap, ctab, dc);
                
        /* run through the used colors.  any used color that has a pixel
           value of 0xffff wasn't allocated.  for such colors, run through
           the entire X colormap and pick the closest color */

        for (i=0; i<ColorMapSize; i++)
	{
            if (used[i] && cols[i]==0xffff)   /* an unallocated pixel */
	    {
                long mdist = 0x7FFFFFFL;  /* MAXLONG */
		int close = -1;
                long d;
                long r = Red[i]<<8;
                long g = Green[i]<<8;
                long b = Blue[i]<<8;

                for (j=0; j<dc; j++)
		{
                    d = 5*abs(r-ctab[j].red)
		      + 3*abs(g-ctab[j].green) +
		        1*abs(b-ctab[j].blue);
                    if (d<mdist) { mdist=d; close=j; }
                }
                if (close<0) return 1;
                cols[i] = ctab[close].pixel;
            }
        }	
    }
    if (!quiet) fprintf(stderr, "Building XImage...\n");
    ptr = Image;
    for (i=0; i<Height; i++)
        for (j=0; j<Width; j++,ptr++) 
            *ptr = (byte) cols[*ptr];
    return 0;
}


void
ReadColormap(fp)
FILE *fp;
{
    byte *ptr=colormap;
    int i;

    if (DEBUG) fprintf(stderr,"Reading Color map...\n");
    fread(colormap, ColorMapSize, 3, fp);
    for (i = 0; i < ColorMapSize; i++) {
        Red[i] = (*ptr++);
        Green[i] = (*ptr++);
        Blue[i] = (*ptr++);
        used[i] = 0;
    }
    NumUsed=0;
}


/*****************************/
int
LoadGIF(fname)
char *fname;
/*****************************/
{
    register byte ch;
    FILE *fp;

    BitOffset = 0,		/* Bit Offset of next code */
    XC = 0, YC = 0,		/* Output X and Y coords of current pixel */
    Offset = 0,                 /* Offset in output array */
    Pass = 0;			/* Used by output routine if interlaced pic */
    ColorMapSize = 0;
    Image = NULL;

    fp = fopen(fname,"r");

    if (!fp) return 1;

    if ( (fread(gifheader, sizeof(gifheader), 1, fp)!=1)
      || (strncmp((char *) gifheader, id, 3)!=0)) {
      fclose(fp);
      return 1;
    }
    if (strncmp((char *) gifheader+3, "87a", 3) &&
	strncmp((char *) gifheader+3, "89a", 3))
	fprintf(stderr,"Warning: %s contains unknown version %c%c%c",
	   fname,gifheader[3],gifheader[4],gifheader[5]);
    HasColormap = ((gifheader[10] & COLORMAPMASK) ? True : False);
    ColorMapSize = 1 << (gifheader[10]&7)+1;

    Background = gifheader[11];		/* background color... not used. */

/* Read in global colormap. */
    if (HasColormap) ReadColormap(fp);

/* Check for image extension */
    while ((ch=getc(fp)) == EXTENSION)
    {
	getc(fp);			/* skip extension code */
	while ((ch=getc(fp))>0)
	    fseek(fp, ch, 1);		/* skip it */
    }
	
    if (ch != IMAGESEP) {
      fclose(fp);
      return 1;
    }

    fread(imageheader,sizeof(imageheader),1,fp);

    Width = imageheader[4] + 0x100 * imageheader[5];
    Height = imageheader[6] + 0x100 * imageheader[7];

    if (!quiet) 
        fprintf(stderr,"%s: %dx%dx%d\n", fname, Width, Height, ColorMapSize);

    Interlace = ((imageheader[8] & INTERLACEMASK) ? True : False);

    if (imageheader[8] & COLORMAPMASK) 
    {
        HasColormap = True;
        ColorMapSize = 1 << (imageheader[8]&7)+1;
        ReadColormap(fp);
    }
    CodeSize = getc(fp); 
    if (!ReadImageData(fp)) {
      fclose(fp);
      if (DecodeImage())
	return 1;
      return 0;
    } else {
      fclose(fp);
      return 1;
    }
}


/*
 *----------------------------------------------------------------------
 *
 * InternalGifGet --
 *
 *      This procedure is invoked to read a Gif file.
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
InternalGifGet(interp, tkwin, filename, width, height, depth, pixmap,
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
  XGCValues gcv;
  GC gc;

  DEBUG     = 0;
  Image     = NULL;
  quiet     = 1;
  local     = 0;
  LocalCmap = 0;
  theDisp   = Tk_Display(tkwin);
  theScreen = Tk_ScreenNumber(tkwin);
  theCmap   = Tk_Colormap(tkwin);
  rootW     = RootWindow(theDisp, theScreen);
  theGC     = DefaultGC(theDisp, theScreen);
  theVisual = DefaultVisual(theDisp, theScreen);
  dispcells = DisplayCells(theDisp, theScreen);
  dispWIDE  = DisplayWidth(theDisp, theScreen);
  dispHIGH  = DisplayHeight(theDisp, theScreen);

  /* load and prepare gif */
  if (LoadGIF(filename)) {
    goto error;
  }
  if (ColorDicking()) {
    goto error;
  }

  /* create pixmap */
  *pixmap = XCreatePixmap(theDisp, Tk_WindowId(tkwin),
			  Width, Height,
			  DefaultDepth(theDisp, theScreen));

  if (DefaultDepth(theDisp, theScreen) == 1) {
    gcv.function = GXcopy;
    gcv.foreground = BlackPixel(theDisp, theScreen);
    gcv.background = WhitePixel(theDisp, theScreen);
    gc = XCreateGC(theDisp, *pixmap, GCFunction | GCForeground |
		  GCBackground, &gcv);
    theImage = XCreateImage(theDisp, theVisual,
			    DefaultDepth(theDisp, theScreen),
			    XYBitmap,  0, Image,
			    Width, Height, 8, 0);
    theImage->bitmap_bit_order = MSBFirst;
    theImage->byte_order = MSBFirst;
    XPutImage(theDisp, *pixmap, gc, theImage, 0, 0, 0, 0,
	      theImage->width, theImage->height);
  } else {
    if (DefaultDepth(theDisp, theScreen) % 8) {
      goto error;
    } else {
      gcv.function = GXcopy;
      gc = XCreateGC(theDisp, *pixmap, GCFunction, &gcv);
      theImage = XCreateImage(theDisp, theVisual, 8, ZPixmap, 0,
			      Image, Width, Height, 8, Width);
      theImage->byte_order = MSBFirst;
      XPutImage(theDisp, *pixmap, gc, theImage, 0, 0, 0, 0,
		theImage->width, theImage->height);
    }
  }
  
  *width = theImage->width;
  *height = theImage->height;
  *depth = 0;
  if (clipMask == (Pixmap *) NULL) {
    *clipMask = None;
  }

  /* clean up */
  if (theImage) {
    XDestroyImage(theImage);
    theImage = NULL;
  } else {
    if (Image) {
      XFree(Image);
      Image = NULL;
    }
  }
  XFreeGC(theDisp, gc);
  return TCL_OK;

 error:
  if (theImage) {
    XDestroyImage(theImage);
    theImage = NULL;
  } else {
    if (Image) {
      XFree(Image);
      Image = NULL;
    }
  }

  return TCL_ERROR;
}

#endif

/* eof */

