/*-------------------------------------------------------------------------
 *
 * utility.h--
 *	  prototypes for utility.c.
 *
 *
 * Copyright (c) 1994, Regents of the University of California
 *
 * $Id: utility.h,v 1.5 1997/09/08 21:54:45 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef UTILITY_H
#define UTILITY_H

#include <executor/execdesc.h>

extern void ProcessUtility(Node *parsetree, CommandDest dest);

#endif							/* UTILITY_H */
