/*-------------------------------------------------------------------------
 *
 * geqo_mutation.h--
 *	  prototypes for mutation functions in optimizer/geqo
 *
 * Copyright (c) 1994, Regents of the University of California
 *
 * $Id: geqo_mutation.h,v 1.5 1997/11/26 01:13:29 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */

/* contributed by:
   =*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
   *  Martin Utesch				 * Institute of Automatic Control	   *
   =							 = University of Mining and Technology =
   *  utesch@aut.tu-freiberg.de  * Freiberg, Germany				   *
   =*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
 */

#ifndef GEQO_MUTATION_H
#define GEQO_MUTATION_H

#include "optimizer/geqo_gene.h"

extern void geqo_mutation(Gene *tour, int num_gene);

#endif							/* GEQO_MUTATION_H */
