/*
 *
 * Copyright (c) 1991 Forschungszentrum Juelich
 * Author : Michael Simon (AP)
 * All rights reserved.
 *
 * %Z%%M%	%I% (mike) %H%
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXBUF	BUFSIZ

char input[MAXBUF];
char output[MAXBUF*4];


int reccount;

/* output of a check record */

void reccheck()
{
        unsigned char chck;

        chck = ~(reccount + (reccount >> 8 & 0xff) + 3);
        printf("S503%4.4X%2.2X\n",reccount,chck);
}

/*
 * tsrd: read bytes from stdin and dump to stdout in MOTOROLA S-Record Format
 */
int
main( int argc, char *argv[])
{
    unsigned long adr = 0;
    int           rec = 0;
    int           cnt = 0;
    int           len;
    extern void get_srec();

    switch( argc) {
    case 3:
	adr = strtoul( argv[2], 0, 16);
    case 2:
	adr += strtoul( argv[1], 0, 16);
    }

    reccount = 0;
    while((len = read( 0, input, MAXBUF)) > 0) {
	rec = 0;
        get_srec( output, input, adr, len, & rec);
	puts( output);
	adr += len;
	cnt += len;
	reccount += rec;
    }
    reccheck();
    exit(0);
}
