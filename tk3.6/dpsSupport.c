/* 
 * dpsSupport.c -- Miscellaneous support routines to access Display Postscript(DPS)
 * written by: Henry R. Tumblin (tumblin@crl.dec.com)
 */

#if defined(USE_DPS)

#include <stdio.h>
#include <X11/X.h>
#include <X11/Intrinsic.h>
#include <DPS/dpsfriends.h>
#include <DPS/dpsXpreview.h>

#include "tkConfig.h"
#include "tk.h"

/*
 *----------------------------------------------------------------------
 *
 * InternalEpsGet --
 *
 *      This procedure is invoked to read an encapsulated Postscript file
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
InternalEpsGet(interp, tkwin, filename, width, height, depth,
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
  int status,t;
  XRectangle bbox, pixelSize;
  FILE *file;
  Bool doneFlag;
  float pixelsPerPoint = XDPSPixelsPerPoint ( Tk_Screen(tkwin) );

  /*
   * Initial test to insure that it's and eps file
   */
  t = strlen (filename);
  if (strcmp (&filename[t-4],".eps"))
    return TCL_ERROR;

  file = fopen (filename,"r");
  if (file == NULL) {
    Tcl_AppendResult (interp, "error reading eps file \"",
			filename, "\"", (char *) NULL);
    return TCL_ERROR;
  }

  *depth = Tk_Depth(tkwin); 

  status = XDPSCreatePixmapForEPSF ( (DPSContext) NULL,
		Tk_Screen(tkwin), file, *depth, pixelsPerPoint,
		pixmap, &pixelSize, &bbox);

  switch (status) {
    case dps_status_success:
      break;
    case dps_status_failure:
      Tcl_AppendResult (interp, "error creating pixmap for eps file \"",
			filename, "\"", ", file is not EPSF\n", (char *) NULL);
      fclose (file);
      return TCL_ERROR;
    case dps_status_no_extension:
      Tcl_AppendResult (interp, "XServer doesn't support DPS\n", (char *) NULL);
      fclose (file);
      return TCL_ERROR;
    default:
      Tcl_AppendResult (interp, "Internal error while creating pixmap for \"",
			filename, "\"\n", (char *) NULL);
      fclose (file);
      return TCL_ERROR;
  }

  *height = pixelSize.height;
  *width = pixelSize.width;

  status = XDPSImageFileIntoDrawable ((DPSContext) NULL,
		Tk_Screen(tkwin), *pixmap, file, pixelSize.height,
		*depth, &bbox, -bbox.x, -bbox.y, pixelsPerPoint,
		True, False, True, &doneFlag);

  switch (status) {
    case dps_status_success:
      break;
    case dps_status_no_extension:
      Tcl_AppendResult (interp, "XServer doesn't support DPS", (char *) NULL);
      fclose (file);
      return TCL_ERROR;
    case dps_status_postscript_error:
      Tcl_AppendResult (interp, "error creating image from eps file \"",
			filename, "\"", ", execution error in EPSF file\n", (char *) NULL);
      fclose (file);
      return TCL_ERROR;
    default:
      Tcl_AppendResult (interp, "Internal error while creating image from eps file \"",
			filename, "\"\n", (char *) NULL);
      fclose (file);
      return TCL_ERROR;
  }

  *clipMask = None;
  fclose (file);

  return TCL_OK;
}

#endif

/* eof */
