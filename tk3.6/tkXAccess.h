/*
 * Copyright (c) 1993 by Sven Delmas
 * All rights reserved.
 * See the file COPYRIGHT for the copyright notes.
 *
 */

#if !defined(TK_XACCESS_H)
#define TK_XACCESS_H 1

EXTERN int TkXAccess_Init _ANSI_ARGS_((Tcl_Interp *interp));
EXTERN int TkXAccessCmd _ANSI_ARGS_((ClientData dummy,
  Tcl_Interp *interp, int argc, char **argv));

#endif

/* eof */
