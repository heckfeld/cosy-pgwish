/*
 * extrude.c - Polygon extrusion.  Uses lib to output to
 * many different raytracer formats.
 *
 * Author:  Michael Simon
 *
 * input file parameter...
 */

#include <stdio.h>
#include <math.h>
#include <string.h>	/* strchr */
#include <stdlib.h>	/* atoi */
#include "def.h"
#include "drv.h"	/* display_close() */
#include "lib.h"

/* These may be read from the command line */
static int raytracer_format = OUTPUT_RT_DEFAULT;
static int output_format    = OUTPUT_CURVES;


#ifdef OUTPUT_TO_FILE
static FILE * stdout_file = NULL;
#else
#define stdout_file stdout
#endif /* OUTPUT_TO_FILE */

static COORD3 dir;


/*----------------------------------------------------------------------
Polygon.  A polygon is defined by a set of vertices.  With these databases,
    a polygon is defined to have all points coplanar.  A polygon has only
    one side, with the order of the vertices being counterclockwise as you
    face the polygon (right-handed coordinate system).  The first two edges
    must form a non-zero convex angle, so that the normal and side visibility
    can be determined.  Description:

    "p" total_vertices
    vert1.x vert1.y vert1.z
    [etc. for total_vertices vertices]

Format:
    p %d
    [ %g %g %g ] <-- for total_vertices vertices
----------------------------------------------------------------------
Polygonal patch.  A patch is defined by a set of vertices and their normals.
    With these databases, a patch is defined to have all points coplanar.
    A patch has only one side, with the order of the vertices being
    counterclockwise as you face the patch (right-handed coordinate system).
    The first two edges must form a non-zero convex angle, so that the normal
    and side visibility can be determined.  Description:

    "pp" total_vertices
    vert1.x vert1.y vert1.z norm1.x norm1.y norm1.z
    [etc. for total_vertices vertices]

Format:
    pp %d
    [ %g %g %g %g %g %g ] <-- for total_vertices vertices
----------------------------------------------------------------------*/
static void
do_poly(fp, nverts)
FILE *fp;
int nverts;
{
	int	vertcount;
	COORD3 *verts;
	float	x, y, z;

	verts = (COORD3*)malloc(nverts*sizeof(COORD3));
	if (verts == NULL)
		goto memerr;

	/* read all the vertices into temp array */
	for (vertcount = 0; vertcount < nverts; vertcount++) {
		if (fscanf(fp, " %f %f %f", &x, &y, &z) != 3)
			goto fmterr;
		SET_COORD3(verts[vertcount],x,y,z);
	}

	/* write output */
	lib_output_polygon_extrusion(nverts, verts, dir);

	free(verts);

	return;
fmterr:
	fprintf(stderr, "polygon or patch syntax error\n");
	exit(1);
memerr:
	fprintf(stderr, "can't allocate memory for polygon or patch (%d)\n", nverts);
	exit(1);
}


/*----------------------------------------------------------------------
----------------------------------------------------------------------*/
static void
parse_nff(fp)
FILE *fp;
{
	char	line[256];
	float	x,y,z;

	while ( fgets( line, 256, fp) != NULL )
		switch( line[0]) {
		case 'a':
			if (sscanf(line, " at %f %f %f", &x, &y, &z) == 3)
				SET_COORD3(dir, x,y,z);
			break;
		case 'p':
			do_poly(fp, atoi( line + 2));
			break;
		default:
			fputs(line, stdout);
			break;
		}
	
} /* parse_nff */


/*----------------------------------------------------------------------
----------------------------------------------------------------------*/
#ifdef _Windows
void
#else
int
#endif
main(argc,argv)
    int argc ;
    char *argv[] ;
{
    char file_name[64];
    FILE *fp;

    PLATFORM_INIT(SPD_READNFF);

    SET_COORD3(dir,0.0,1.0,0.0);

    file_name[0] = '\0';

    /* Start by defining which raytracer we will be using */
    if ( lib_read_get_opts( argc, argv,
			&raytracer_format, &output_format, file_name ) ) {
	return EXIT_FAIL;
    }

    if ( lib_open( raytracer_format, "ReadNFF.out" ) ) {
	return EXIT_FAIL;
    }

    if (file_name[0] == 0)
        fp = stdin;
    else {
        fp = fopen(file_name, "r");
        if (fp == NULL) {
	    fprintf(stderr, "Cannot open nff file: '%s'\n", file_name);
	    return EXIT_FAIL;
	}
    }

    lib_set_polygonalization(3, 3);

    parse_nff(fp);

    fclose(fp);

    lib_close();

    PLATFORM_SHUTDOWN();
    return EXIT_SUCCESS;
}
