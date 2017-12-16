/*
 * tkXmain.c
 *
 * Main for wishx.  This is usable for C or C++.
 *-----------------------------------------------------------------------------
 * Copyright 1991-1994 Karl Lehenbauer and Mark Diekhans.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies.  Karl Lehenbauer and
 * Mark Diekhans make no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without express or
 * implied warranty.
 *-----------------------------------------------------------------------------
 * $Id: tkXmain.c,v 4.0 1994/07/16 05:30:56 markd Rel $
 *-----------------------------------------------------------------------------
 */

#include "tclExtend.h"


/*
 *----------------------------------------------------------------------
 *
 * main --
 *
 *	Main program for Wishx.
 *
 *----------------------------------------------------------------------
 */

#ifdef __cplusplus
int
main (int     argc,
      char  **argv)
#else
int
main(argc, argv)
    int argc;				/* Number of arguments. */
    char **argv;			/* Array of argument strings. */
#endif
{
    TkX_Wish (argc, argv);

    return 0;  /* Never exits here: make the compiler happy */
}
