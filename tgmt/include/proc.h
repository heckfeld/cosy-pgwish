/*
 *
 * Copyright (c) 1991 Forschungszentrum Juelich
 * Author : Michael Simon (AP)
 * All rights reserved.
 *
 * %Z%%M%	%I% (mike) %H%
 *
 */
#ifndef _PROC_INCLUDED
#define _PROC_INCLUDED

struct proc {
	caddr_t p_wchan;
};

#ifdef KERNEL
extern struct proc * pfind();
#endif
#endif
