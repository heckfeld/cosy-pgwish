/*
 * tkX.h --
 *
 * Tk C interfaces supplied by Extended Tcl.  These are used by wishx startup
 * and signal handling.
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
 * $Id: tkX.h,v 4.0 1994/07/16 05:31:05 markd Rel $
 *-----------------------------------------------------------------------------
 */

#ifndef TKX_H
#define TKX_H

typedef void (TkX_ShellSignalProc) _ANSI_ARGS_((int signalNum));

extern void
TkX_Startup _ANSI_ARGS_((Tcl_Interp          *interp,
                         int                  interactive,
                         TkX_ShellSignalProc *errorSignalProc));

void
TkX_WishInit _ANSI_ARGS_((Tcl_Interp *interp));

#endif /* TKX_H */
