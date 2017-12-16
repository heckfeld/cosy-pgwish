/*
 * libextrude.c - a library of extrude object output routines.
 *
 * Author:  Michael Simon
 *
 */


/*-----------------------------------------------------------------*/
/* include section */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "lib.h"
#include "drv.h"


/*-----------------------------------------------------------------*/
/* defines/constants section */


/*-----------------------------------------------------------------*/
/* Globals to determine which axes can be used to split the polygon */


/*-----------------------------------------------------------------*/
void
lib_output_polygon_extrusion(tot_vert, tvert, dir)
    int tot_vert;
    COORD3 tvert[];
    COORD3 dir;
{
    int i, j;
    COORD3 x, *vert, pl[4];
    MATRIX mx;

    vert = (COORD3 *) calloc( tot_vert, sizeof(COORD3));
    if( vert == NULL)
	return;

    memcpy( vert, tvert, sizeof(COORD3) * tot_vert);

    /* First lets do a couple of checks to see if this is a valid polygon */
    for (i=0;i<tot_vert;) {
	/* If there are two adjacent coordinates that degenerate then
	   collapse them down to one */
	SUB3_COORD3(x, vert[i], vert[(i+1)%tot_vert]);
	if (lib_normalize_vector(x) < EPSILON2) {
	    for (j=i;j<tot_vert-1;j++)
		memcpy(&vert[j], &vert[j+1], sizeof(COORD4));
	    tot_vert--;
	}
	else {
	    i++;
	}
    }

    if (tot_vert < 3) {
	free( vert);
	/* No such thing as a poly that only has two sides */
	return;
    }

    lib_create_translate_matrix(mx, dir);

    /* the polygon itself, bottom face */
    lib_output_polypatch( tot_vert, vert, NULL);

    /* the sides */
    for (i=1;i<=tot_vert;i++) {
	if( i == tot_vert) j = 0;
	else j = i;
	COPY_COORD3(pl[0], vert[i-1]);
	COPY_COORD3(pl[1], vert[j]);
	lib_transform_point(pl[2], vert[j], mx);
	lib_transform_point(pl[3], vert[i-1], mx);

	lib_output_polypatch( 4, pl, NULL);	/* force triangles */
    }

    /* the polygon itself, top face */
    lib_tx_push();
    if( lib_tx_active) {
	MATRIX current, new;
	lib_get_current_tx( current);
	lib_matrix_multiply(new, mx, current);
	lib_set_current_tx(new);
    }
	lib_set_current_tx(mx);
    lib_output_polypatch( tot_vert, vert, NULL);	/* force triangles */
    lib_tx_push();
}
