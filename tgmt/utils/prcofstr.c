/*
 *
 * Copyright (c) 1991 Forschungszentrum Juelich
 * Author : Michael Simon (COSY Control)
 * All rights reserved.
 *
 * %Z%%M%	%I% (mike) %H%
 *
 */

/*
 * prconfstr - print configuration string to stdout
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*#include <h/tgmt.h>*/
#define OSTGMTCFGFMT "%64s %15s %64s %15s %12.12d %6.6s %2.2d %256s"

int
main( int argc, char *argv[])
{
    if( argc < 8)
	exit(1);

    printf(OSTGMTCFGFMT, argv[1], argv[2], argv[3], argv[4], (int)time(0), argv[5], atoi(argv[6]), argv[7]);
    if( argc == 9)
	printf( " %s",argv[8]);
    putchar('\0');
    exit(0);
}
