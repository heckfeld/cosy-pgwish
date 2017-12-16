/*
 * tclXdata.c --
 *
 * This file supplies the tcl_RcFileName global variable.  Its stored in
 * a seperate file so that both the TclX and TkX libraries can have a copy.
 * Otherwise, TkX will bring in the UCB Tk main containing this variable rather
 * than the TclX version.
 *-----------------------------------------------------------------------------
 * Copyright 1993-1994 Karl Lehenbauer and Mark Diekhans.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies.  Karl Lehenbauer and
 * Mark Diekhans make no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without express or
 * implied warranty.
 *-----------------------------------------------------------------------------
 * $Id: tclXdata.c,v 4.0 1994/07/16 05:28:48 markd Rel $
 *-----------------------------------------------------------------------------
 */

#include "tclExtdInt.h"

/*
 * Name of a user-specific startup script to source if the application is
 * being run interactively (e.g. "~/.tclrc").  Set by Tcl_AppInit.
 *  NULL means don't source anything ever.
 */
char *tcl_RcFileName = NULL;
