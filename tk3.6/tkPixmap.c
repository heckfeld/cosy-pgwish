/* 
 * tkPixmap.c --
 *
 *	This file maintains a database of read-only pixmaps for the Tk
 *	toolkit.  This allows pixmaps to be shared between widgets and
 *	also avoids interactions with the X server.
 *
 * Copyright (c) 1993 by Sven Delmas
 * All rights reserved.
 * See the file COPYRIGHT for the copyright notes.
 *
 *
 * This source is based upon the file tkBitmap.c from:
 *
 * John Ousterhout
 *
 * Copyright (c) 1993 The Regents of the University of California.
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that the
 * above copyright notice and the following two paragraphs appear in
 * all copies of this software.
 * 
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
 * CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

#ifndef lint
static char *AtFSid = "$Header: tkPixmap.c[5.1] Wed Oct  5 23:46:42 1994 garfield@mydomain.de frozen $";
#endif /* not lint */

#include "tkConfig.h"
#include "tk.h"

/*
 * The includes below are for pre-defined bitmaps.
 */

#include "bitmaps/gray25"
#include "bitmaps/gray50"
#if (TK_MAJOR_VERSION >= 3) && (TK_MINOR_VERSION >= 3)
#include "bitmaps/error"
#include "bitmaps/hourglass"
#include "bitmaps/info"
#include "bitmaps/questhead"
#include "bitmaps/question"
#include "bitmaps/warning"
#endif

#if defined(LOAD_PIXMAPS)
#include "pixmaps/bomb.xpml"
#include "pixmaps/exclaim.xpml"
#include "pixmaps/exit.xpml"
#include "pixmaps/hourglass.xpml"
#include "pixmaps/info.xpml"
#include "pixmaps/no_entry.xpml"
#include "pixmaps/preserver.xpml"
#include "pixmaps/question.xpml"
#include "pixmaps/stop.xpml"
#include "pixmaps/tick.xpml"
#endif

/*
 * One of the following data structures exists for each bitmap that is
 * currently in use.  Each structure is indexed with both "idTable" and
 * "nameTable".
 */

typedef struct {
    Pixmap bitmap;		/* X identifier for bitmap.  None means this
				 * bitmap was created by Tk_DefinePixmap
				 * and it isn't currently in use. */
    Pixmap clipMask;		/* X identifier for clipMask. */
    unsigned int width, height;	/* Dimensions of bitmap. */
    unsigned int depth;         /* Depth of pixmap */
    Display *display;		/* Display for which bitmap is valid. */
    int refCount;		/* Number of active uses of bitmap. */
    int cached;		        /* 1 to cache pixmap... this means */
				/* they are not free'ed. */
    Tcl_HashEntry *hashPtr;	/* Entry in nameTable for this structure
				 * (needed when deleting). */
} TkBitmap;

/*
 * Hash table to map from a textual description of a bitmap to the
 * TkBitmap record for the bitmap, and key structure used in that
 * hash table:
 */

static Tcl_HashTable nameTable;
typedef struct {
    Tk_Uid name;		/* Textual name for desired bitmap. */
    Display *display;		/* Display for which bitmap will be used. */
} NameKey;

/*
 * Hash table that maps from <display + bitmap id> to the TkBitmap structure
 * for the pixmap.  This table is used by Tk_FreePixmap.
 */

static Tcl_HashTable idTable;
typedef struct {
    Display *display;           /* Display for which bitmap was allocated. */
    Pixmap pixmap;              /* X identifier for pixmap. */
} IdKey;

/*
 * For each call to Tk_DefinePixmap one of the following structures is
 * created to hold information about the pixmap.
 */

typedef struct {
    char *source;		/* Bits for bitmap. */
    char *format;		/* The format of the source data. */
    Pixmap pixmap;              /* Pre-defined pixmap. */
    Pixmap clipMask;            /* Pre-defined clipmask. */
    unsigned int width, height;	/* Dimensions of bitmap. */
    unsigned int depth;         /* Depth of pixmap */
} PredefBitmap;

/*
 * Hash table create by Tk_DefinePixmap to map from a name to a
 * collection of in-core data about a bitmap.  The table is
 * indexed by the address of the data for the bitmap, and the entries
 * contain pointers to PredefBitmap structures.
 */

static Tcl_HashTable predefTable;

/*
 * Hash table used by Tk_GetPixmapFromData to map from a collection
 * of in-core data about a bitmap to a Tk_Uid giving an automatically-
 * generated name for the bitmap:
 */

static Tcl_HashTable dataTable;
typedef struct {
    char *source;		/* Bitmap bits. */
    unsigned int width, height;	/* Dimensions of bitmap. */
} DataKey;

static int initialized = 0;	/* 0 means static structures haven't been
				 * initialized yet. */

static int cacheDefault = 0;    /* 0 means no caching, 1 means new */
                                /* pixmaps are by default cached. */

/*
 * Forward declarations for procedures defined in this file:
 */

static void		PixmapInit _ANSI_ARGS_(());

extern int              InternalBitmapGet();
extern int              InternalBitmapPut();
extern int              InternalBitmapToPixmap();
extern int              InternalBitmapToAscii();

#if defined(USE_XPM3)
extern int              InternalXpm3Get();
extern int              InternalXpm3Put();
extern int              InternalXpm3ToPixmap();
extern int              InternalXpm3ToAscii();
#endif


#if defined(USE_GIF)
extern int              InternalGifGet();
#endif


#if defined(USE_RAW256)
extern int              InternalRaw256Get();
#endif


#if defined(USE_DPS)
extern int              InternalEpsGet();
#endif


/*
 * definition of supported graphic types
*/
typedef struct {
    char *formatName;
    int (*getFunction)();
    int (*putFunction)();
    int (*toPixmapFunction)();
    int (*toAsciiFunction)();
} GraphicTypes;

static GraphicTypes graphicTypes[] = {
#if defined(USE_XPM3)
  {"xpm3", InternalXpm3Get, InternalXpm3Put,
     InternalXpm3ToPixmap, InternalXpm3ToAscii},
#endif
#if defined(USE_GIF)
  {"gif", InternalGifGet, NULL, NULL, NULL},
#endif
#if defined(USE_DPS)
  {"eps", InternalEpsGet, NULL, NULL, NULL},
#endif
#if defined(USE_RAW256)
  {"raw256", InternalRaw256Get, NULL, NULL, NULL},
#endif
  /* bitmap is the last, because XCreateBitmapFromData is lying when */
  /* it could not create a proper bitmap from the data, and no error */
  /* code is provided. So this routine is the last one to try. */
  {"bitmap", InternalBitmapGet, InternalBitmapPut,
     InternalBitmapToPixmap, InternalBitmapToAscii},
  {"", NULL, NULL, NULL, NULL}
};


/*
 *----------------------------------------------------------------------
 *
 * Tk_GetPixmap --
 *
 *	Given a string describing a bitmap or pixmap, locate (or create
 *      if necessary) a pixmap that fits the description.
 *
 * Results:
 *	The return value is the X identifer for the desired pixmap,
 *      unless string couldn't be parsed correctly.  In this case,
 *      None is returned and an error message is left in
 *      interp->result.  The caller should never modify the pixmap
 *      that is returned, and should eventually call Tk_FreePixmap
 *      when the pixmap is no longer needed.
 *
 * Side effects:
 *	The pixmap is added to an internal database with a reference count.
 *	For each call to this procedure, there should eventually be a call
 *	to Tk_FreePixmap, so that the database can be cleaned up when pixmaps
 *	aren't needed anymore.
 *
 *----------------------------------------------------------------------
 */
Pixmap
Tk_GetPixmap(interp, tkwin, string)
    Tcl_Interp *interp;		/* Interpreter to use for error reporting. */
    Tk_Window tkwin;		/* Window in which pixmap will be used. */
    Tk_Uid string;		/* Description of pixmap.  See manual entry
				 * for details on legal syntax. */
{
    NameKey nameKey;
    IdKey idKey;
    Tcl_HashEntry *nameHashPtr, *idHashPtr, *predefHashPtr;
    register TkBitmap *bitmapPtr;
    PredefBitmap *predefPtr;
    int new;
    Pixmap bitmap = (Pixmap) NULL;
    Pixmap clipMask = (Pixmap) NULL;
    unsigned int width, height, depth;
    int listCounter = 0;

    if (!initialized) {
      PixmapInit();
    }

    nameKey.name = string;
    nameKey.display = Tk_Display(tkwin);
    nameHashPtr = Tcl_CreateHashEntry(&nameTable, (char *) &nameKey, &new);
    if (!new) {
      bitmapPtr = (TkBitmap *) Tcl_GetHashValue(nameHashPtr);
      bitmapPtr->refCount++;
      bitmapPtr->cached = cacheDefault;
      return bitmapPtr->bitmap;
    }

    /*
     * No suitable bitmap exists.  Create a new bitmap from the
     * information contained in the string.  If the string starts
     * with "@" then the rest of the string is a file name containing
     * the bitmap.  Otherwise the string must refer to a bitmap
     * defined by a call to Tk_DefinePixmap.
     */

    /* first try for a display-specific version */
    predefHashPtr = Tcl_FindHashEntry(&predefTable, (char *) &nameKey);
    if (predefHashPtr == NULL) {
      /* try for a non-display specific version */
      nameKey.display = (Display*) NULL;
      predefHashPtr = Tcl_FindHashEntry(&predefTable, (char *) &nameKey);
    }

    Tk_MakeWindowExist(tkwin);
    if (*string == '@' && predefHashPtr == NULL) {
#if (TK_MAJOR_VERSION >= 3) && (TK_MINOR_VERSION >= 3)
      Tcl_DString buffer;
      
      string = Tcl_TildeSubst(interp, string + 1, &buffer);
#else
      string = Tcl_TildeSubst(interp, string + 1);
#endif
      if (string == NULL) {
#if (TK_MAJOR_VERSION >= 3) && (TK_MINOR_VERSION >= 3)
	Tcl_DStringFree(&buffer);
#endif
	goto error;
      }

      while (1) {
	if (strlen(graphicTypes[listCounter].formatName) == 0) {
	  Tcl_AppendResult(interp, "error reading picture file \"",
			   string, "\"", (char *) NULL);
#if (TK_MAJOR_VERSION >= 3) && (TK_MINOR_VERSION >= 3)
          Tcl_DStringFree(&buffer);
#endif
	  goto error;
	}
	if (graphicTypes[listCounter].getFunction != NULL) {
	  width = 0;
	  height = 0;
	  depth = 0;
	  bitmap = (Pixmap) NULL;
	  clipMask = (Pixmap) NULL;
	  if (graphicTypes[listCounter].getFunction(interp, tkwin,
		    string, &width, &height, &depth,
		    &bitmap, &clipMask) == TCL_OK) {
	    break;
	  }
	}
	listCounter++;
      }
#if (TK_MAJOR_VERSION >= 3) && (TK_MINOR_VERSION >= 3)
      Tcl_DStringFree(&buffer);
#endif
      if (depth == 0) {
	depth = Tk_Depth(tkwin);
      }
    } else {
      if (predefHashPtr == NULL) {
	/* give up */
	Tcl_AppendResult(interp, "pixmap \"", string,
			 "\" not defined", (char *) NULL);
	goto error;
      }
      predefPtr = (PredefBitmap *) Tcl_GetHashValue(predefHashPtr);
      width = predefPtr->width;
      height = predefPtr->height;
      depth = predefPtr->depth;
      clipMask = None;
      if (depth == 0) {
	depth = Tk_Depth(tkwin);
      }
      if (predefPtr->source != NULL) {   /* create a predefined pixmap */
	while (1) {
	  if (strlen(graphicTypes[listCounter].formatName) == 0) {
	    Tcl_AppendResult(interp, "wrong pixmap data format",
			     (char *) NULL);
	    goto error;
	  }
	  if (graphicTypes[listCounter].toPixmapFunction != NULL &&
	      (strlen(predefPtr->format) == 0 ||
	       strcmp(graphicTypes[listCounter].formatName,
		      predefPtr->format) == 0)) {
	    if (graphicTypes[listCounter].toPixmapFunction(interp,
		  tkwin, predefPtr->source, width, height, depth,
		  &bitmap, &clipMask) == TCL_OK) {
	      break;
	    }
	  }
	  listCounter++;
	}
      } else {      /* use a already created predefined pixmap */
	if (predefPtr->pixmap != None) {
	  bitmap = predefPtr->pixmap;
	  clipMask = predefPtr->clipMask;
	} else {    /* create an empty pixmap */
	  bitmap =
	    XCreatePixmap(Tk_Display(tkwin), Tk_WindowId(tkwin),
			  width, height, depth);
	  if (depth == DefaultDepthOfScreen(Tk_Screen(tkwin))) {
	    XFillRectangle(Tk_Display(tkwin), bitmap,
			   DefaultGCOfScreen(Tk_Screen(tkwin)),
			   0, 0, width, height);
	  }
	}
      }
    }
    
    /*
     * Add information about this bitmap to our database.
     */

    bitmapPtr = (TkBitmap *) ckalloc(sizeof(TkBitmap));
    bitmapPtr->bitmap = bitmap;
    bitmapPtr->clipMask = clipMask;
    bitmapPtr->width = width;
    bitmapPtr->height = height;
    bitmapPtr->depth = depth;
    bitmapPtr->display = Tk_Display(tkwin);
    bitmapPtr->refCount = 1;
    bitmapPtr->cached = cacheDefault;
    bitmapPtr->hashPtr = nameHashPtr;
    idKey.display = Tk_Display(tkwin);
    idKey.pixmap = bitmap;
    idHashPtr = Tcl_CreateHashEntry(&idTable, (char *) &idKey,
				    &new);
    if (!new) {
      panic("bitmap already registered in Tk_GetPixmap");
    }
    Tcl_SetHashValue(nameHashPtr, bitmapPtr);
    Tcl_SetHashValue(idHashPtr, bitmapPtr);
    return bitmapPtr->bitmap;

    error:
    Tcl_DeleteHashEntry(nameHashPtr);
    return None;
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_GetPixmapClipMask --
 *
 *	Given a display and a pixmap id describing a bitmap or pixmap,
 *      locate a pixmap that fits the description and return the
 *      clipMask.
 *
 * Results:
 *	The return value is the clipMask of the pixmap or NULL.
 *
 *----------------------------------------------------------------------
 */
Pixmap
Tk_GetPixmapClipMask(display, pixmap)
    Display *display;		/* Display for which pixmap was
				 * allocated. */
    Pixmap pixmap;		/* Description of pixmap.  See manual entry
				 * for details on legal syntax. */
{
    IdKey idKey;
    Tcl_HashEntry *idHashPtr;
    register TkBitmap *bitmapPtr;

    if (!initialized) {
      panic("Tk_GetPixmapClipMask called uninitialized");
    }

    idKey.display = display;
    idKey.pixmap = pixmap;
    idHashPtr = Tcl_FindHashEntry(&idTable, (char *) &idKey);
    if (idHashPtr == NULL) {
      return None;
    }
    bitmapPtr = (TkBitmap *) Tcl_GetHashValue(idHashPtr);
    return bitmapPtr->clipMask;
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_PixmapExists --
 *
 *	Given a display and a pixmap id describing a bitmap or pixmap,
 *      locate a pixmap that fits the description and return True
 *      if it was found, and False if not.
 *
 * Results:
 *	The return value is True or False.
 *
 *----------------------------------------------------------------------
 */
int
Tk_PixmapExists(display, pixmap)
    Display *display;		/* Display for which pixmap was
				 * allocated. */
    Pixmap pixmap;		/* Description of pixmap.  See manual entry
				 * for details on legal syntax. */
{
    IdKey idKey;
    Tcl_HashEntry *idHashPtr;

    if (!initialized) {
      panic("Tk_PixmapExists called uninitialized");
    }

    idKey.display = display;
    idKey.pixmap = pixmap;
    idHashPtr = Tcl_FindHashEntry(&idTable, (char *) &idKey);
    if (idHashPtr == NULL) {
      return False;
    }
    return True;
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_DefinePixmap --
 *
 *	This procedure associates a textual name with a binary pixmap
 *	description, so that the name may be used to refer to the
 *	pixmap in future calls to Tk_GetPixmap. The pixmap can
 *      be pre-created by the user, or can be created later
 *      by Tk_GetPixmap().  Since pixmaps are display-specific,
 *      a user supplied pixmap will be associated with tkwin's display.
 *      If pixmap is given as None, then a new pixmap will be created
 *      by Tk_GetPixmap for each unique display.
 *
 * Results:
 *	A standard Tcl result.  If an error occurs then TCL_ERROR is
 *	returned and a message is left in interp->result.
 *
 * Side effects:
 *	"Name" is entered into the pixmap table and may be used from
 *	here on to refer to the given pixmap.
 *
 *----------------------------------------------------------------------
 */

int
Tk_DefinePixmap(interp, name, tkwin, pixmap, clipMask, source, width,
		height, depth, format)
    Tcl_Interp *interp;		/* Interpreter to use for error reporting. */
    Tk_Uid name;		/* Name to use for bitmap.  Must not already
				 * be defined as a bitmap. */
    Tk_Window tkwin;            /* any window on screen where pixmap lives */
    Pixmap pixmap;              /* pixmap to associate with name,
				 * or None to create a new pixmap */
    Pixmap clipMask;            /* bitmap to associate with name,
				 * used as clipmask */
    char *source;		/* Address of bits for bitmap. */
    unsigned int width;		/* Width of bitmap. */
    unsigned int height;	/* Height of bitmap. */
    unsigned int depth;         /* Depth of pixmap, or 0 for 
                                 * default depth of screen */
    char *format;               /* the format of the pixmap data, or */
                                /* an empty string, if the routine */
                                /* should determine the type by */
                                /* herselfe (may be not the correct */
				/* one). */
{
    int new;
    Tcl_HashEntry *predefHashPtr;
    PredefBitmap *predefPtr;
    NameKey nameKey;

    if (!initialized) {
      PixmapInit();
    }

    nameKey.name = name;
    if (tkwin == NULL) {
      nameKey.display = NULL;
    } else {
      nameKey.display = (pixmap != None) ? Tk_Display(tkwin) : NULL;
    }
    predefHashPtr = Tcl_CreateHashEntry(&predefTable,
					(char *) &nameKey, &new);
    if (!new) {
      Tcl_AppendResult(interp, "bitmap \"", name,
		       "\" is already defined", (char *) NULL);
      return TCL_ERROR;
    }
    predefPtr = (PredefBitmap *) ckalloc(sizeof(PredefBitmap));
    predefPtr->source = source;
    predefPtr->format = format;
    predefPtr->pixmap = pixmap;
    if (clipMask != (Pixmap) NULL && clipMask != None) {
      predefPtr->clipMask = clipMask;
    } else {
      predefPtr->clipMask = None;
    }
    predefPtr->width = width;
    predefPtr->height = height;
    predefPtr->depth = depth;
    Tcl_SetHashValue(predefHashPtr, predefPtr);
    return TCL_OK;
}

/*
 *--------------------------------------------------------------
 *
 * Tk_NameOfPixmap --
 *
 *	Given a pixmap, return a textual string identifying the
 *	pixmap.
 *
 * Results:
 *	The return value is the string name associated with pixmap.
 *
 * Side effects:
 *	None.
 *
 *--------------------------------------------------------------
 */

Tk_Uid
Tk_NameOfPixmap(display, bitmap)
    Display *display;                   /* Display for which pixmap
				         * was allocated. */
    Pixmap bitmap;			/* Bitmap whose name is wanted. */
{
    IdKey idKey;
    Tcl_HashEntry *idHashPtr;
    TkBitmap *bitmapPtr;

    if (!initialized) {
      panic("Tk_NameOfPixmap called uninitialized");
    }

    idKey.display = display;
    idKey.pixmap = bitmap;
    idHashPtr = Tcl_FindHashEntry(&idTable, (char *) &idKey);
    if (idHashPtr == NULL) {
      return Tk_GetUid("");
    }
    bitmapPtr = (TkBitmap *) Tcl_GetHashValue(idHashPtr);
    if (bitmapPtr->hashPtr == NULL) {
      /* the bitmap has been un-defined */
      return Tk_GetUid("");
    }
    return ((NameKey *) bitmapPtr->hashPtr->key.words)->name;
}

/*
 *--------------------------------------------------------------
 *
 * Tk_PixmapOfName --
 *
 *	Given a textual string identifying the pixmap, return
 *      the pixmap id.
 *
 * Results:
 *	The return value is the pixmap id.
 *
 * Side effects:
 *	None.
 *
 *--------------------------------------------------------------
 */

Pixmap
Tk_PixmapOfName(name, tkwin)
    Tk_Uid name;		/* Name of bitmap/pixmap */
    Tk_Window tkwin;		/* Window in which bitmap will be used. */
{
    NameKey nameKey;
    Tcl_HashEntry *nameHashPtr;
    TkBitmap *bitmapPtr;
    
    if (!initialized) {
      panic("Tk_PixmapOfName called uninitialized");
    }

    nameKey.name = name;
    nameKey.display = (Display*) NULL;
    nameHashPtr = Tcl_FindHashEntry(&nameTable, (char*) &nameKey);
    if (nameHashPtr != NULL) {
      bitmapPtr = (TkBitmap *) Tcl_GetHashValue(nameHashPtr);
      if (bitmapPtr->hashPtr != NULL) {
	return bitmapPtr->bitmap;
      }
    } else {
      nameKey.display = Tk_Display(tkwin);
      nameHashPtr = Tcl_FindHashEntry(&nameTable, (char*) &nameKey);
      if (nameHashPtr != NULL) {
	bitmapPtr = (TkBitmap *) Tcl_GetHashValue(nameHashPtr);
	if (bitmapPtr->hashPtr != NULL) {
	  return bitmapPtr->bitmap;
	}
      }
    }
    return None;
}

/*
 *--------------------------------------------------------------
 *
 * Tk_SizeOfPixmap --
 *
 *	Given a pixmap managed by this module, returns the width
 *	and height of the pixmap.
 *
 * Results:
 *	The words at *widthPtr and *heightPtr are filled in with
 *	the dimenstions of pixmap.
 *
 * Side effects:
 *	If pixmap isn't managed by this module then the procedure
 *	panics.
 *
 *--------------------------------------------------------------
 */

void
Tk_SizeOfPixmap(display, bitmap, widthPtr, heightPtr)
    Display *display;                   /* Display for which pixmap
					 * was allocated. */
    Pixmap bitmap;			/* Bitmap whose size is wanted. */
    unsigned int *widthPtr;		/* Store bitmap width here. */
    unsigned int *heightPtr;		/* Store bitmap height here. */
{
    IdKey idKey;
    Tcl_HashEntry *idHashPtr;
    TkBitmap *bitmapPtr;

    if (!initialized) {
      panic("Tk_SizeOfPixmap called uninitialized");
    }

    idKey.display = display;
    idKey.pixmap = bitmap;
    idHashPtr = Tcl_FindHashEntry(&idTable, (char *) &idKey);
    if (idHashPtr == NULL) {
      return;
    }
    bitmapPtr = (TkBitmap *) Tcl_GetHashValue(idHashPtr);
    *widthPtr = bitmapPtr->width;
    *heightPtr = bitmapPtr->height;
}

/*
 *--------------------------------------------------------------
 *
 * Tk_DepthOfPixmap --
 *
 *	Given a pixmap managed by this module, returns the depth
 *	of the pixmap.
 *
 * Results:
 *	The depth of the pixmap.
 *
 * Side effects:
 *	If pixmap isn't managed by this module then the procedure
 *	panics.
 *
 *--------------------------------------------------------------
 */

int
Tk_DepthOfPixmap(display, pixmap)
    Display *display;                   /* Display for which pixmap
					 * was allocated. */
    Pixmap pixmap;			/* Pixmap whose size is wanted. */
{
    IdKey idKey;
    Tcl_HashEntry *idHashPtr;
    TkBitmap *bitmapPtr;

    if (!initialized) {
      panic("Tk_DepthOfPixmap called uninitialized");
    }

    idKey.display = display;
    idKey.pixmap = pixmap;
    idHashPtr = Tcl_FindHashEntry(&idTable, (char *) &idKey);
    if (idHashPtr == NULL) {
      return 0;
    }
    bitmapPtr = (TkBitmap *) Tcl_GetHashValue(idHashPtr);
    return bitmapPtr->depth;
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_FreePixmap --
 *
 *	This procedure is called to release a pixmap allocated by
 *	Tk_GetPixmap.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The reference count associated with bitmap is decremented, and
 *	it is officially deallocated if no-one is using it anymore.
 *
 *----------------------------------------------------------------------
 */

void
Tk_FreePixmap(display, bitmap)
    Display *display;                   /* Display for which pixmap
					 * was allocated. */
    Pixmap bitmap;			/* Bitmap to be released. */
{
    IdKey idKey;
    Tcl_HashEntry *idHashPtr;
    register TkBitmap *bitmapPtr;

    if (!initialized) {
      panic("Tk_FreePixmap called uninitialized");
    }

    idKey.display = display;
    idKey.pixmap = bitmap;
    idHashPtr = Tcl_FindHashEntry(&idTable, (char *) &idKey);
    if (idHashPtr == NULL) {
      return;
    }
    bitmapPtr = (TkBitmap *) Tcl_GetHashValue(idHashPtr);
    bitmapPtr->refCount--;
    if (bitmapPtr->refCount <= 0 && !bitmapPtr->cached) {
      XFreePixmap(bitmapPtr->display, bitmapPtr->bitmap);
      if (bitmapPtr->clipMask != None) {
	XFreePixmap(bitmapPtr->display, bitmapPtr->clipMask);
      }
      Tcl_DeleteHashEntry(idHashPtr);
      if (bitmapPtr->hashPtr != NULL) {
	/* If hashPtr is NULL, the bitmap has been undefined,
	 * and already removed from the name table */
	Tcl_DeleteHashEntry(bitmapPtr->hashPtr);
      }
      ckfree((char *) bitmapPtr);
    }
}

/*
 *--------------------------------------------------------------
 *
 * Tk_CacheStatusOfPixmap --
 *
 *	Given a pixmap managed by this module, returns the current
 *	cache status of the pixmap.
 *
 * Results:
 *	The cache status of the pixmap.
 *
 * Side effects:
 *	If pixmap isn't managed by this module then the procedure
 *	panics..
 *
 *--------------------------------------------------------------
 */

int
Tk_CacheStatusOfPixmap(display, pixmap)
    Display *display;                   /* Display for which pixmap
					 * was allocated. */
    Pixmap pixmap;			/* Pixmap whose size is wanted. */
{
    IdKey idKey;
    Tcl_HashEntry *idHashPtr;
    TkBitmap *bitmapPtr;

    if (!initialized) {
      panic("Tk_CacheStatusOfPixmap called uninitialized");
    }

    idKey.display = display;
    idKey.pixmap = pixmap;
    idHashPtr = Tcl_FindHashEntry(&idTable, (char *) &idKey);
    if (idHashPtr == NULL) {
      return 0;
    }
    bitmapPtr = (TkBitmap *) Tcl_GetHashValue(idHashPtr);
    return bitmapPtr->cached;
}

/*
 *--------------------------------------------------------------
 *
 * Tk_SetCacheStatusOfPixmap --
 *
 *	Given a pixmap managed by this module and a new cache status,
 *	set the new cache status.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	If pixmap isn't managed by this module then the procedure
 *	panics..
 *
 *--------------------------------------------------------------
 */

void
Tk_SetCacheStatusOfPixmap(display, pixmap, status)
    Display *display;                   /* Display for which pixmap
					 * was allocated. */
    Pixmap pixmap;			/* Pixmap whose size is wanted. */
    int status;                         /* The new cache status. */
{
    IdKey idKey;
    Tcl_HashEntry *idHashPtr;
    TkBitmap *bitmapPtr;

    if (!initialized) {
      panic("Tk_SetCacheStatusOfPixmap called uninitialized");
    }

    idKey.display = display;
    idKey.pixmap = pixmap;
    idHashPtr = Tcl_FindHashEntry(&idTable, (char *) &idKey);
    if (idHashPtr == NULL) {
      return;
    }
    bitmapPtr = (TkBitmap *) Tcl_GetHashValue(idHashPtr);
    bitmapPtr->cached = status;
    if (!status && bitmapPtr->refCount <= 0) {
      Tk_FreePixmap(display, pixmap);
    }
}

/*
 *--------------------------------------------------------------
 *
 * Tk_DefaultCacheStatus --
 *
 *	Returns the current default cache status.
 *
 * Results:
 *	The default cache status.
 *
 * Side effects:
 *	None.
 *
 *--------------------------------------------------------------
 */

int
Tk_DefaultCacheStatus()
{
  return cacheDefault;
}

/*
 *--------------------------------------------------------------
 *
 * Tk_SetDefaultCacheStatus --
 *
 *	Given a new default cache status, set this status.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *--------------------------------------------------------------
 */

void
Tk_SetDefaultCacheStatus(status)
    int status;                         /* The new cache status. */
{
  cacheDefault = status;
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_GetPixmapFromData --
 *
 *	Given a description of the bits for a pixmap, make a pixmap that
 *	has the given properties.
 *
 * Results:
 *	The return value is the X identifer for the desired pixmap,
 *	unless it couldn't be created properly. In this case, None is
 *      returned and an error message is left in interp->result.  The
 *      caller should never modify the bitmap that is returned, and
 *      should eventually call Tk_FreePixmap when the pixmap is no
 *      longer needed.
 *
 * Side effects:
 *	The pixmap is added to an internal database with a reference count.
 *	For each call to this procedure, there should eventually be a call
 *	to Tk_FreePixmap, so that the database can be cleaned up when pixmaps
 *	aren't needed anymore.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
Pixmap
Tk_GetPixmapFromData(interp, tkwin, name, source, width, height,
		     depth, format)
    Tcl_Interp *interp;		/* Interpreter to use for error reporting. */
    Tk_Window tkwin;		/* Window in which pixmap will be used. */
    Tk_Uid name;		/* Name to use for bitmap.  Must not already
				 * be defined as a bitmap. */
    char *source;		/* Pixmap data for pixmap. */
    unsigned int width, height;	/* Dimensions of pixmap. */
    unsigned int depth;	        /* Depth of pixmap. */
    char *format;               /* the format of the pixmap data, or */
                                /* an empty string, if the routine */
                                /* should determine the type by */
                                /* herselfe (may be not the correct */
				/* one). */
{
    DataKey dataKey;
    Tcl_HashEntry *dataHashPtr;
    int new;
    static int autoNumber = 0;
    static int emptyNumber = 0;
    char *tmpString, string[20];

    if (!initialized) {
      PixmapInit();
    }

    if (!strcmp(source, "empty")) {
      tmpString = ckalloc(8);
      sprintf(tmpString, "_tk%d", emptyNumber++);
      dataKey.source = tmpString;
    } else {
      dataKey.source = source;
    }
    dataKey.width = width;
    dataKey.height = height;
    dataHashPtr = Tcl_CreateHashEntry(&dataTable, (char *) &dataKey,
				      &new);
    if (!new) {
      name = (Tk_Uid) Tcl_GetHashValue(dataHashPtr);
    } else {
      if (!(name && *name)) {
	autoNumber++;
	sprintf(string, "_tk%d", autoNumber);
	name = Tk_GetUid(string);
      }
      Tcl_SetHashValue(dataHashPtr, name);
      if (!strcmp(source, "empty")) {
        if (Tk_DefinePixmap(interp, name, tkwin, None, None, NULL,
			    width, height, depth, format) != TCL_OK) {
	  Tcl_DeleteHashEntry(dataHashPtr);
	  return TCL_ERROR;
	}
      } else {
        if (Tk_DefinePixmap(interp, name, tkwin, None, None, source,
			    width, height, depth, format) != TCL_OK) {
	  Tcl_DeleteHashEntry(dataHashPtr);
	  return TCL_ERROR;
	}
      }
    }
    return Tk_GetPixmap(interp, tkwin, name);
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_SavePixmap --
 *
 *	This procedure save a given pixmap to a file. The
 *	format of the file can be selected.
 *
 * Results:
 *	A standard Tcl result.  If an error occurs then TCL_ERROR is
 *	returned.
 *
 * Side effects:
 *	A file named "filename" is created/overwritten
 *
 *----------------------------------------------------------------------
 */

int
Tk_SavePixmap(interp, tkwin, filename, savePixmap, clipMask,
	      width, height, format)
     Tcl_Interp *interp;		/* Interpreter to use for
					 * error reporting. */
     Tk_Window tkwin;
     char *filename;
     char *format;
     Pixmap savePixmap;
     Pixmap clipMask;
     unsigned int width;
     unsigned int height;
{
  int listCounter = 0;
  
  Tk_MakeWindowExist(tkwin);
  while (1) {
    if (strlen(graphicTypes[listCounter].formatName) == 0) {
      Tcl_AppendResult(interp, "error writing bitmap file \"",
		       filename, "\"", (char *) NULL);
      break;
    }
    if (strcmp(format, graphicTypes[listCounter].formatName) == 0) {
      if (graphicTypes[listCounter].putFunction != NULL) {
	if (graphicTypes[listCounter].putFunction(interp, tkwin,
		  filename, width, height, Tk_Depth(tkwin),
		  savePixmap, clipMask) == TCL_OK) { 
	  return TCL_OK;
	}
      }
    }
    listCounter++;
  }
  return TCL_ERROR;
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_UndefinePixmap --
 *
 *	This procedure removes any association "name" with
 *	a bitmap or pixmap.  This can be used to undefine
 *	names defined by Tk_DefinePixmap. If tkwin is NULL,
 *      only display-independent pixmaps will be removed,
 *      otherwise both display-independent and the pixmap
 *      associated with tkwin will be undefined.
 *
 * Results:
 *	A standard Tcl result.  If an error occurs then TCL_ERROR is
 *	returned and a message is left in interp->result.
 *
 * Side effects:
 *	"Name" is removed from the predef table.
 *
 *----------------------------------------------------------------------
 */

int
Tk_UndefinePixmap(interp, name, tkwin)
    Tcl_Interp *interp;		/* Interpreter to use for error reporting. */
    Tk_Uid name;		/* Name of bitmap/pixmap to undefine */
    Tk_Window tkwin;            /* any window on screen where pixmap lives */
{
    NameKey nameKey;
    Tcl_HashEntry *predefHashPtr, *nameHashPtr, *hPtr;
    TkBitmap *bitmapPtr;
    Tcl_HashSearch search;

    if (!initialized) {
      PixmapInit();
    }

    for(hPtr = Tcl_FirstHashEntry(&dataTable, &search);
	hPtr != NULL; hPtr = Tcl_NextHashEntry(&search)) {
      if (strcmp(name, (char *) Tcl_GetHashValue(hPtr)) == 0) {
	Tcl_DeleteHashEntry(hPtr);
      }
    }

    nameKey.name = name;
    nameKey.display = (Display*) NULL;
    predefHashPtr = Tcl_FindHashEntry(&predefTable, (char*) &nameKey);
    if (predefHashPtr != NULL) {
      Tcl_DeleteHashEntry(predefHashPtr);
    }

    if (tkwin != NULL) {
      nameKey.display = Tk_Display(tkwin);
      predefHashPtr = Tcl_FindHashEntry(&predefTable, (char*) &nameKey);
      if (predefHashPtr != NULL) {
	Tcl_DeleteHashEntry(predefHashPtr);
      }
    }

    /*
     * Remove it from the name table if it is there (it might not
     * have been created yet, in which case we wont find it).  We
     * delete it from the name table and mark the hashPtr as NULL
     * so that we know it has been deleted.  The pixmap still exists,
     * and will later be freed and removed from idTable by Tk_FreeBitmap().
     *
     * If tkwin was passed as NULL, we are in trouble here.  We can't
     * remove it from the nameTable since we dont have a display to
     * hash on.  For now just ignore the problem.  The worst that will
     * happen is that, if the pixmap is still outstanding, it subsequent
     * gets will continue to return the old version and not the new one.
     */
    nameHashPtr = Tcl_FindHashEntry(&nameTable, (char *) &nameKey);
    if (nameHashPtr != NULL) {
      bitmapPtr = (TkBitmap *) Tcl_GetHashValue(nameHashPtr);
      bitmapPtr->hashPtr = (Tcl_HashEntry*) NULL;
      Tcl_DeleteHashEntry(nameHashPtr);
    }
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * tkBitmap replacements
 *
 *	Replace the already existing tkBitmap routines.
 *
 *----------------------------------------------------------------------
 */
Pixmap
Tk_GetBitmap(interp, tkwin, string)
    Tcl_Interp *interp;         /* Interpreter to use for error reporting. */
    Tk_Window tkwin;            /* Window in which bitmap will be used. */
    Tk_Uid string;              /* Description of bitmap.  See manual entry
                                 * for details on legal syntax. */
{
  Pixmap returnValue;
  
  if ((returnValue = Tk_GetPixmap(interp, tkwin, string)) != None) {
    if (Tk_DepthOfPixmap(Tk_Display(tkwin), returnValue) == 1) {
      return returnValue;
    } else {
      return None;
    }
  } else {
    return None;
  }
}

int
Tk_DefineBitmap(interp, name, source, width, height)
    Tcl_Interp *interp;         /* Interpreter to use for error reporting. */
    Tk_Uid name;                /* Name to use for bitmap.  Must not already
                                 * be defined as a bitmap. */
    char *source;               /* Address of bits for bitmap. */
    unsigned int width;         /* Width of bitmap. */
    unsigned int height;        /* Height of bitmap. */
{
  return Tk_DefinePixmap(interp, name, NULL, None, None, source,
			 width, height, 1, "bitmap");
}

Tk_Uid
Tk_NameOfBitmap(display, bitmap)
    Display *display;                   /* Display for which pixmap
				         * was allocated. */
    Pixmap bitmap;                      /* Bitmap whose name is wanted. */
{
  if (Tk_DepthOfPixmap(display, bitmap) == 1) {
    return Tk_NameOfPixmap(display, bitmap);
  } else {
    return Tk_GetUid("");
  }
}

void
Tk_SizeOfBitmap(display, bitmap, widthPtr, heightPtr)
    Display *display;                   /* Display for which pixmap
				         * was allocated. */
    Pixmap bitmap;                      /* Bitmap whose size is wanted. */
    unsigned int *widthPtr;             /* Store bitmap width here. */
    unsigned int *heightPtr;            /* Store bitmap height here. */
{
  if (Tk_DepthOfPixmap(display, bitmap) == 1) {
    Tk_SizeOfPixmap(display, bitmap, widthPtr, heightPtr);
  } else {
    *widthPtr = 0;
    *heightPtr = 0;
  }
}

void
Tk_FreeBitmap(display, bitmap)
    Display *display;                   /* Display for which pixmap
				         * was allocated. */
    Pixmap bitmap;                      /* Bitmap to be released. */
{
  if (Tk_DepthOfPixmap(display, bitmap) == 1) {
    Tk_FreePixmap(display, bitmap);
  }
}

Pixmap
Tk_GetBitmapFromData(interp, tkwin, source, width, height)
    Tcl_Interp *interp;         /* Interpreter to use for error reporting. */
    Tk_Window tkwin;            /* Window in which bitmap will be used. */
    char *source;               /* Bitmap data for bitmap shape. */
    unsigned int width, height; /* Dimensions of bitmap. */
{
  Pixmap returnValue;
  
  if ((returnValue = Tk_GetPixmapFromData(interp, tkwin, NULL, source,
					  width, height, 1,
					  "bitmap")) != None) {
    if (Tk_DepthOfPixmap(Tk_Display(tkwin), returnValue) == 1) {
      return returnValue;
    } else {
      return None;
    }
  } else {
    return None;
  }
}

/*
 *----------------------------------------------------------------------
 *
 * PixmapInit --
 *
 *	Initialize the structures used for pixmap management.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Read the code.
 *
 *----------------------------------------------------------------------
 */

static void
PixmapInit()
{
    Tcl_Interp *dummy;

    dummy = Tcl_CreateInterp();
    initialized = 1;
    Tcl_InitHashTable(&nameTable, sizeof(NameKey)/sizeof(int));
    Tcl_InitHashTable(&dataTable, sizeof(DataKey)/sizeof(int));
    Tcl_InitHashTable(&predefTable, sizeof(NameKey)/sizeof(int));

    /*
     * The call below is tricky:  can't use sizeof(IdKey) because it
     * gets padded with extra unpredictable bytes on some 64-bit
     * machines.
     */

    Tcl_InitHashTable(&idTable,
		      (sizeof(Display *) + sizeof(Pixmap)) /sizeof(int));

    Tk_DefinePixmap(dummy, Tk_GetUid("gray50"), NULL, None, None,
		    gray50_bits, gray50_width, gray50_height, 1,
		    "bitmap");
    Tk_DefinePixmap(dummy, Tk_GetUid("gray25"), NULL, None, None,
		    gray25_bits, gray25_width, gray25_height, 1,
		    "bitmap"); 
#if (TK_MAJOR_VERSION >= 3) && (TK_MINOR_VERSION >= 3)
    Tk_DefinePixmap(dummy, Tk_GetUid("error"), NULL, None, None,
		    error_bits, error_width, error_height, 1,
		    "bitmap");
    Tk_DefinePixmap(dummy, Tk_GetUid("hourglass"), NULL, None, None,
		    hourglass_bits, hourglass_width, hourglass_height, 1,
		    "bitmap"); 
    Tk_DefinePixmap(dummy, Tk_GetUid("info"), NULL, None, None,
		    info_bits, info_width, info_height, 1,
		    "bitmap");
    Tk_DefinePixmap(dummy, Tk_GetUid("questhead"), NULL, None, None,
		    questhead_bits, questhead_width, questhead_height, 1,
		    "bitmap"); 
    Tk_DefinePixmap(dummy, Tk_GetUid("question"), NULL, None, None,
		    question_bits, question_width, question_height, 1,
		    "bitmap");
    Tk_DefinePixmap(dummy, Tk_GetUid("warning"), NULL, None, None,
		    warning_bits, warning_width, warning_height, 1,
		    "bitmap"); 
#endif
    
#if defined(LOAD_PIXMAPS)
    Tk_DefinePixmap(dummy, Tk_GetUid("bomb.xpm"), NULL, None, None,
		    bomb_xpm, 32, 32, 0, "xpm3"); 
    Tk_DefinePixmap(dummy, Tk_GetUid("exclaim.xpm"), NULL, None, None,
		    exclaim_xpm, 32, 32, 0, "xpm3"); 
    Tk_DefinePixmap(dummy, Tk_GetUid("exit.xpm"), NULL, None, None,
		    exit_xpm, 32, 32, 0, "xpm3"); 
    Tk_DefinePixmap(dummy, Tk_GetUid("hourglass.xpm"), NULL, None, None,
		    hourglass_xpm, 32, 32, 0, "xpm3"); 
    Tk_DefinePixmap(dummy, Tk_GetUid("info.xpm"), NULL, None, None,
		    info_xpm, 32, 32, 0, "xpm3"); 
    Tk_DefinePixmap(dummy, Tk_GetUid("no_entry.xpm"), NULL, None, None,
		    no_entry_xpm, 32, 32, 0, "xpm3"); 
    Tk_DefinePixmap(dummy, Tk_GetUid("preserver.xpm"), NULL, None, None,
		    preserver_xpm, 32, 32, 0, "xpm3"); 
    Tk_DefinePixmap(dummy, Tk_GetUid("question.xpm"), NULL, None, None,
		    question_xpm, 32, 32, 0, "xpm3"); 
    Tk_DefinePixmap(dummy, Tk_GetUid("stop.xpm"), NULL, None, None,
		    stop_xpm, 32, 32, 0, "xpm3"); 
    Tk_DefinePixmap(dummy, Tk_GetUid("tick.xpm"), NULL, None, None,
		    tick_xpm, 32, 32, 0, "xpm3"); 
#endif

    Tcl_DeleteInterp(dummy);
}

#if defined(USE_PINFO)
/*
 *----------------------------------------------------------------------
 *
 * Tk_PinfoCmd --
 *
 *      This procedure is invoked to process the "pinfo" Tcl command.
 *      See the user documentation for details on what it does.
 *
 * Results:
 *      A standard Tcl result.
 *
 * Side effects:
 *      See the user documentation.
 *
 *----------------------------------------------------------------------
 */

int
Tk_PinfoCmd(clientData, interp, argc, argv)
    ClientData clientData;      /* Main window associated with
                                 * interpreter. */
    Tcl_Interp *interp;         /* Current interpreter. */
    int argc;                   /* Number of arguments. */
    char **argv;                /* Argument strings. */
{
    Tk_Window tkwin = (Tk_Window) clientData;
    int length;
    char c, formatBuffer[40];
    char *bitmapData = (char *) NULL;
    char **list, *bitmapDataPtr, *bitmapSourcePtr, *bitmapToPtr;
    Tcl_HashEntry *idHashPtr;
    Tcl_HashSearch search;
    NameKey *ret;
    Pixmap pixmap = (Pixmap) NULL;
    int depth, boolean;
    int listCounter = 0;
    unsigned int height, width;

    if (!initialized) {
      PixmapInit();
    }

    if (argc < 2) {
      Tcl_AppendResult(interp, "wrong # args: should be \"",
		       argv[0], " command ?pixmapName? ?arg?\"",
		       (char *) NULL);
      return TCL_ERROR;
    }

    c = argv[1][0];
    length = strlen(argv[1]);
    if ((c == 'c') && (strncmp(argv[1], "cache", length) == 0) &&
	length > 2) {
      if (argc == 2) {
	for (idHashPtr = Tcl_FirstHashEntry(&nameTable, &search);
	     idHashPtr != NULL; idHashPtr = Tcl_NextHashEntry(&search)) {
	  ret = (NameKey *) Tcl_GetHashKey(&nameTable, idHashPtr);
	  pixmap = Tk_PixmapOfName(Tk_GetUid(ret->name), tkwin);
	  if (pixmap != None) {
	    if (Tk_CacheStatusOfPixmap(Tk_Display(tkwin), pixmap)) {
	      Tcl_AppendResult(interp, ret->name, " ",
			       (char *) NULL);
	    }
	  }
	}
      } else {
	if (argc == 3) {
	  pixmap = Tk_PixmapOfName(Tk_GetUid(argv[2]), tkwin);
	  if (pixmap != None) {
	    sprintf(interp->result, "%d",
		    Tk_CacheStatusOfPixmap(Tk_Display(tkwin), pixmap));
	  } else {
	    Tcl_AppendResult(interp, "unknown pixmap: \"",
			     argv[2], "\"", (char *) NULL);
	    return TCL_ERROR;
	  }
	} else {
	  if (argc == 4) {
	    pixmap = Tk_PixmapOfName(Tk_GetUid(argv[2]), tkwin);
	    if (pixmap != None) {
	      if (Tcl_GetBoolean(interp, argv[3], &boolean) != TCL_OK) {
		return TCL_ERROR;
	      }
	      Tk_SetCacheStatusOfPixmap(Tk_Display(tkwin), pixmap,
					boolean);
	    } else {
	      Tcl_AppendResult(interp, "unknown pixmap: \"",
			       argv[2], "\"", (char *) NULL);
	      return TCL_ERROR;
	    }
	  } else {
	    Tcl_AppendResult(interp, "wrong # args: should be \"",
			     argv[0], " cache ?pixmapName? ?status?\"",
			     (char *) NULL);
	    return TCL_ERROR;
	  }
	}
      }
    } else if ((c == 'c') && (strncmp(argv[1], "cacheconfig", length) == 0) &&
	       length > 2) {
      if (argc == 3) {
	if (strcmp(argv[2], "-defaultstatus") == 0) {
	  sprintf(interp->result, "%d", Tk_DefaultCacheStatus());
	} else {
	  Tcl_AppendResult(interp, "unknown pinfo cacheconfig option: \"",
			   argv[2], "\"", (char *) NULL);
	  return TCL_ERROR;
	}
      } else {
	if (argc == 4) {
	  if (strcmp(argv[2], "-defaultstatus") == 0) {
	    if (Tcl_GetBoolean(interp, argv[3], &boolean) != TCL_OK) {
	      return TCL_ERROR;
	    }
	    Tk_SetDefaultCacheStatus(boolean);
	  } else {
	    Tcl_AppendResult(interp, "unknown pinfo cacheconfig option: \"",
			     argv[2], "\"", (char *) NULL);
	    return TCL_ERROR;
	  }
	} else {
	  Tcl_AppendResult(interp, "wrong # args: should be \"",
			   argv[0], " cacheconfig option ?value?\"",
			   (char *) NULL);
	  return TCL_ERROR;
	}
      }
    } else if ((c == 'c') && (strncmp(argv[1], "clipmaskid", length) == 0) &&
	length > 8) {
      if (argc != 3) {
	Tcl_AppendResult(interp, "wrong # args: should be \"",
			 argv[0], " clipmaskid pixmapName\"", (char *) NULL);
	return TCL_ERROR;
      }
      pixmap = Tk_PixmapOfName(Tk_GetUid(argv[2]), tkwin);
      if (pixmap != None) {
	if (Tk_GetPixmapClipMask(Tk_Display(tkwin), pixmap) != None) {
	  sprintf(interp->result, "%ld",
		  Tk_GetPixmapClipMask(Tk_Display(tkwin), pixmap));
	} else {
	  sprintf(interp->result, "none");
	}
      } else {
	Tcl_AppendResult(interp, "unknown pixmap: \"",
			 argv[2], "\"", (char *) NULL);
	return TCL_ERROR;
      }
    } else if ((c == 'c') && (strncmp(argv[1], "clipmaskdata", length) == 0) &&
	       length > 8) {
      if (argc != 3) {
	Tcl_AppendResult(interp, "wrong # args: should be \"",
			 argv[0], " clipmaskdata pixmapName\"", (char *) NULL);
	return TCL_ERROR;
      }
      pixmap = Tk_PixmapOfName(Tk_GetUid(argv[2]), tkwin);
      if (pixmap != None) {
	Tk_SizeOfPixmap(Tk_Display(tkwin), pixmap, &width, &height);
	if (Tk_GetPixmapClipMask(Tk_Display(tkwin), pixmap) != None) {
	  pixmap = Tk_GetPixmapClipMask(Tk_Display(tkwin), pixmap);
	  InternalBitmapToAscii(interp, tkwin, pixmap, width, height, 1);
	} else {
	  sprintf(interp->result, "none");
	}
      } else {
	Tcl_AppendResult(interp, "unknown pixmap: \"",
			 argv[2], "\"", (char *) NULL);
	return TCL_ERROR;
      }
    } else if ((c == 'd') && (strncmp(argv[1], "data", length) == 0) &&
	       length > 2) {
      if (argc != 4) {
	Tcl_AppendResult(interp, "wrong # args: should be \"",
			 argv[0], " data pixmapName format\"", (char *) NULL);
	return TCL_ERROR;
      }
      pixmap = Tk_PixmapOfName(Tk_GetUid(argv[2]), tkwin);
      if (pixmap != None) {
	Tk_SizeOfPixmap(Tk_Display(tkwin), pixmap, &width, &height);
	while (1) {
	  if (strlen(graphicTypes[listCounter].formatName) == 0) {
	    Tcl_AppendResult(interp, "could not create pixmap data in \"",
			     argv[3], "\" format", (char *) NULL);
	    return TCL_ERROR;
	  }
	  if (strcmp(argv[3], graphicTypes[listCounter].formatName) == 0) {
	    if (graphicTypes[listCounter].toAsciiFunction != NULL) {
	      if (graphicTypes[listCounter].toAsciiFunction(interp, 
		    tkwin, pixmap, width, height,
		    Tk_DepthOfPixmap(Tk_Display(tkwin), pixmap)) == TCL_OK) {
		break;
	      }
	    }
	  }
	  listCounter++;
	}
      } else {
	Tcl_AppendResult(interp, "unknown pixmap: \"",
			 argv[2], "\"", (char *) NULL);
	return TCL_ERROR;
      }
    } else if ((c == 'd') && (strncmp(argv[1], "define", length) == 0) &&
	       length > 3) {
      if (argc != 4) {
	Tcl_AppendResult(interp, "wrong # args: should be \"",
			 argv[0], " define pixmapName dataList\"",
			 (char *) NULL);
	return TCL_ERROR;
      }
      pixmap = Tk_PixmapOfName(Tk_GetUid(argv[2]), tkwin);
      if (Tcl_SplitList(interp, argv[3], &listCounter, &list) == TCL_OK) {
	if (listCounter != 2 && listCounter != 1) {
	  ckfree((char *) list);
	  Tcl_AppendResult(interp, "wrong pixmap data format.",
			   (char *) NULL);
	  return TCL_ERROR;
	}
	sscanf(*list, "%d %d %d %s", &width, &height, &depth, formatBuffer);
	if (width == 0 || height == 0) {
	  ckfree((char *) list);
	  Tcl_AppendResult(interp, "wrong pixmap data format:",
			   " no correct size values", (char *) NULL);
	  return TCL_ERROR;
	}
	if (listCounter == 2 && (int) strlen(*(list+1)) > 0) {
	  if (depth == 1) {
	    bitmapSourcePtr = *(list+1);
	    bitmapData = ckalloc(2 + sizeof(int) * (width * height));
	    bitmapDataPtr = bitmapData;
	    while (*bitmapSourcePtr != '\0') {
	      while (*bitmapSourcePtr && *bitmapSourcePtr == ' ') {
		bitmapSourcePtr++;
	      }
	      if ((bitmapToPtr = strchr(bitmapSourcePtr, ',')) == NULL) {
		*bitmapDataPtr = strtol(bitmapSourcePtr,NULL,0);
		bitmapDataPtr++;
		*bitmapDataPtr = '\0';
		break;
	      }
	      *bitmapDataPtr = strtol(bitmapSourcePtr,NULL,0);
	      bitmapDataPtr++;
	      bitmapSourcePtr = bitmapToPtr + 1;
	    }
	    pixmap = Tk_GetPixmapFromData(interp, tkwin,
					  Tk_GetUid(argv[2]), 
					  bitmapData, width, height,
					  depth, formatBuffer);
	  } else {
	    bitmapData = ckalloc(2 + (sizeof(char) * strlen(*(list+1))));
	    strcpy(bitmapData, *(list+1));
	    pixmap = Tk_GetPixmapFromData(interp, tkwin,
					  Tk_GetUid(argv[2]),
					  bitmapData, width, height,
					  depth, formatBuffer);
	  }
	} else {
	  pixmap = Tk_GetPixmapFromData(interp, tkwin,
					Tk_GetUid(argv[2]),
					"empty", width, height, depth,
					formatBuffer);
	}
	ckfree((char *) list);
      } else {
	Tcl_AppendResult(interp, "wrong pixmap data format", (char *) NULL);
	return TCL_ERROR;
      }
      if (pixmap != None) {
        sprintf(interp->result, "%ld", pixmap);
      } else {
	Tcl_AppendResult(interp, "could not define pixmap: \"",
			 argv[2], "\"", (char *) NULL);
	return TCL_ERROR;
      }
    } else if ((c == 'd') && (strncmp(argv[1], "depth", length) == 0) &&
	       length > 3) {
      if (argc != 3) {
	Tcl_AppendResult(interp, "wrong # args: should be \"",
			 argv[0], " depth pixmapName\"", (char *) NULL);
	return TCL_ERROR;
      }
      pixmap = Tk_PixmapOfName(Tk_GetUid(argv[2]), tkwin);
      if (pixmap != None) {
	depth = Tk_DepthOfPixmap(Tk_Display(tkwin), pixmap);
      } else {
	Tcl_AppendResult(interp, "unknown pixmap: \"",
			 argv[2], "\"", (char *) NULL);
	return TCL_ERROR;
      }
      sprintf(interp->result, "%d", depth);
    } else if ((c == 'f') && (strncmp(argv[1], "formats", length) == 0)) {
      if (argc != 2 && argc != 3) {
	Tcl_AppendResult(interp, "wrong # args: should be \"",
			 argv[0], " formats ?specifier?\"", (char *) NULL);
	return TCL_ERROR;
      }
      if (argc == 2) {
	while (1) {
	  if (strlen(graphicTypes[listCounter].formatName) == 0) {
	    break;
	  }
	  Tcl_AppendResult(interp, graphicTypes[listCounter].formatName,
			   " ", (char *) NULL);
	  listCounter++;
	}
      } else {
	while (1) {
	  if (strlen(graphicTypes[listCounter].formatName) == 0) {
	    break;
	  }
	  if (strcmp(argv[2], "all") == 0 &&
	      graphicTypes[listCounter].getFunction != NULL &&
	      graphicTypes[listCounter].putFunction != NULL &&
	      graphicTypes[listCounter].toAsciiFunction != NULL &&
	      graphicTypes[listCounter].toPixmapFunction != NULL) {
	    Tcl_AppendResult(interp,
			     graphicTypes[listCounter].formatName,
			     " ", (char *) NULL);
	  } else {
	    if (strcmp(argv[2], "get") == 0 &&
		graphicTypes[listCounter].getFunction != NULL) {
	      Tcl_AppendResult(interp,
			       graphicTypes[listCounter].formatName,
			       " ", (char *) NULL);
	    } else if (strcmp(argv[2], "put") == 0 &&
		       graphicTypes[listCounter].putFunction != NULL) {
	      Tcl_AppendResult(interp,
			       graphicTypes[listCounter].formatName,
			       " ", (char *) NULL);
	    } else if (strcmp(argv[2], "toascii") == 0 &&
		       graphicTypes[listCounter].toAsciiFunction != NULL) {
	      Tcl_AppendResult(interp,
			       graphicTypes[listCounter].formatName,
			       " ", (char *) NULL);
	    } else if (strcmp(argv[2], "topixmap") == 0 &&
		       graphicTypes[listCounter].toPixmapFunction != NULL) {
	      Tcl_AppendResult(interp,
			       graphicTypes[listCounter].formatName,
			       " ", (char *) NULL);
	    }
	  }
	  listCounter++;
	}
      }
    } else if ((c == 'g') && (strncmp(argv[1], "get", length) == 0)) {
      if (argc != 3) {
	Tcl_AppendResult(interp, "wrong # args: should be \"",
			 argv[0], " get pixmapName\"", (char *) NULL);
	return TCL_ERROR;
      }
      pixmap = Tk_PixmapOfName(Tk_GetUid(argv[2]), tkwin);
      if (pixmap == None) {
	pixmap = Tk_GetPixmap(interp, tkwin, Tk_GetUid(argv[2]));
      }
      if (pixmap != None) {
        sprintf(interp->result, "%ld", pixmap);
      } else {
	Tcl_AppendResult(interp, "could not get pixmap: \"",
			 argv[2], "\"", (char *) NULL);
	return TCL_ERROR;
      }      
    } else if ((c == 'h') && (strncmp(argv[1], "height", length) == 0)) {
      if (argc != 3) {
	Tcl_AppendResult(interp, "wrong # args: should be \"",
			 argv[0], " height pixmapName\"", (char *) NULL);
	return TCL_ERROR;
      }
      pixmap = Tk_PixmapOfName(Tk_GetUid(argv[2]), tkwin);
      if (pixmap != None) {
	Tk_SizeOfPixmap(Tk_Display(tkwin), pixmap, &width, &height);
      } else {
	Tcl_AppendResult(interp, "unknown pixmap: \"",
			 argv[2], "\"", (char *) NULL);
	return TCL_ERROR;
      }
      sprintf(interp->result, "%d", height);
    } else if ((c == 'n') && (strncmp(argv[1], "names", length) == 0)) {
      if (argc != 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"",
			 argv[0], "\"", (char *) NULL);
	return TCL_ERROR;
      }
      for (idHashPtr = Tcl_FirstHashEntry(&nameTable, &search);
	   idHashPtr != NULL; idHashPtr = Tcl_NextHashEntry(&search)) {
	ret = (NameKey *) Tcl_GetHashKey(&nameTable, idHashPtr);
	Tcl_AppendResult(interp, ret->name, " ", (char *) NULL);
      }
    } else if ((c == 's') && (strncmp(argv[1], "save", length) == 0)) {
      if (argc != 5) {
	Tcl_AppendResult(interp, "wrong # args: should be \"",
			 argv[0], " save pixmapName fileName format\"",
			 (char *) NULL);
	return TCL_ERROR;
      }
      pixmap = Tk_PixmapOfName(Tk_GetUid(argv[2]), tkwin);
      if (pixmap != None) {
	Tk_SizeOfPixmap(Tk_Display(tkwin), pixmap, &width, &height);
	if (Tk_GetPixmapClipMask(Tk_Display(tkwin), pixmap) != None) {
	  if (Tk_SavePixmap(interp, tkwin, argv[3], pixmap,
			    Tk_GetPixmapClipMask(Tk_Display(tkwin), pixmap),
			    width, height, argv[4]) != TCL_OK) {
	    Tcl_AppendResult(interp, "could not save pixmap: \"",
			     argv[2], "\" to \"", argv[3], "\"",
			     (char *) NULL);
	    return TCL_ERROR;
	  }
	} else {
	  if (Tk_SavePixmap(interp, tkwin, argv[3], pixmap,
			    (Pixmap) NULL, width, height, argv[4]) != TCL_OK) {
	    Tcl_AppendResult(interp, "could not save pixmap: \"",
			     argv[2], "\" to \"", argv[3], "\"",
			     (char *) NULL);
	    return TCL_ERROR;
	  }
	}
      } else {
	Tcl_AppendResult(interp, "unknown pixmap: \"",
			 argv[2], "\"", (char *) NULL);
	return TCL_ERROR;
      }
    } else if ((c == 'u') && (strncmp(argv[1], "undefine", length) == 0)) {
      if (argc != 3) {
	Tcl_AppendResult(interp, "wrong # args: should be \"",
			 argv[0], " undefine pixmapName\"", (char *) NULL);
	return TCL_ERROR;
      }
      pixmap = Tk_PixmapOfName(Tk_GetUid(argv[2]), tkwin);
      if (pixmap != None) {
	Tk_UndefinePixmap(interp, Tk_GetUid(argv[2]), tkwin);
      } else {
	Tcl_AppendResult(interp, "unknown pixmap: \"",
			 argv[2], "\"", (char *) NULL);
	return TCL_ERROR;
      }
    } else if ((c == 'w') && (strncmp(argv[1], "width", length) == 0)) {
      if (argc != 3) {
	Tcl_AppendResult(interp, "wrong # args: should be \"",
			 argv[0], " width pixmapName\"", (char *) NULL);
	return TCL_ERROR;
      }
      pixmap = Tk_PixmapOfName(Tk_GetUid(argv[2]), tkwin);
      if (pixmap != None) {
	Tk_SizeOfPixmap(Tk_Display(tkwin), pixmap, &width, &height);
      } else {
	Tcl_AppendResult(interp, "unknown pixmap: \"",
			 argv[2], "\"", (char *) NULL);
	return TCL_ERROR;
      }
      sprintf(interp->result, "%d", width);
    } else {
      Tcl_AppendResult(interp, "bad option \"", argv[1],
		       "\": must be cache, cacheconfig, clipmaskid, ",
		       "clipmaskdata, data, define, depth, formats, ",
		       "get, height, names, save, undefine, width",
                       (char *) NULL);
      return TCL_ERROR;
    }
    return TCL_OK;
}

int
Pinfo_Init(interp)
Tcl_Interp *interp;         /* Interpreter for application. */
{
#if (TK_MAJOR_VERSION >= 3) && (TK_MINOR_VERSION >= 3)
    Tk_Window main;

    main = Tk_MainWindow(interp);
    Tk_MakeWindowExist(main);
    Tcl_CreateCommand(interp, "pinfo", Tk_PinfoCmd,
		      (ClientData) main, (void (*)()) NULL);
#endif
    return TCL_OK;
}

#endif

/* eof */

