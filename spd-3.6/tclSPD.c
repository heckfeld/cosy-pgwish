#include <stdio.h>

#include "tclInt.h"
#include "tclUnix.h"

# include "def.h"
# include "drv.h"
# include "lib.h"

static int SPDNormalizeVectorCmd _ANSI_ARGS_((ClientData clientData,
                            Tcl_Interp *interp, int argc, char **argv));
static int SPDZeroMatrixCmd _ANSI_ARGS_((ClientData clientData,
                            Tcl_Interp *interp, int argc, char **argv));
static int SPDIdentityMatrixCmd _ANSI_ARGS_((ClientData clientData,
                            Tcl_Interp *interp, int argc, char **argv));
static int SPDRotateMatrixCmd _ANSI_ARGS_((ClientData clientData,
                            Tcl_Interp *interp, int argc, char **argv));
static int SPDTranslateMatrixCmd _ANSI_ARGS_((ClientData clientData,
                            Tcl_Interp *interp, int argc, char **argv));
static int SPDScaleMatrixCmd _ANSI_ARGS_((ClientData clientData,
                            Tcl_Interp *interp, int argc, char **argv));
static int SPDViewMatrixCmd _ANSI_ARGS_((ClientData clientData,
                            Tcl_Interp *interp, int argc, char **argv));
static int SPDTransVectorCmd _ANSI_ARGS_((ClientData clientData,
                            Tcl_Interp *interp, int argc, char **argv));
static int SPDTransPointCmd _ANSI_ARGS_((ClientData clientData,
                            Tcl_Interp *interp, int argc, char **argv));
static int SPDScalePointCmd _ANSI_ARGS_((ClientData clientData,
                            Tcl_Interp *interp, int argc, char **argv));
static int SPDRotatePointCmd _ANSI_ARGS_((ClientData clientData,
                            Tcl_Interp *interp, int argc, char **argv));
static int SPDMulMatrixCmd _ANSI_ARGS_((ClientData clientData,
                            Tcl_Interp *interp, int argc, char **argv));


int Spd_Init( interp)
	Tcl_Interp *interp;
{
    Tcl_CreateCommand( interp, "normalizevector", SPDNormalizeVectorCmd, NULL, NULL);
    Tcl_CreateCommand( interp, "zeromatrix", SPDZeroMatrixCmd, NULL, NULL);
    Tcl_CreateCommand( interp, "identitymatrix", SPDIdentityMatrixCmd, NULL, NULL);
    Tcl_CreateCommand( interp, "rotatematrix", SPDRotateMatrixCmd, NULL, NULL);
    Tcl_CreateCommand( interp, "scalematrix", SPDScaleMatrixCmd, NULL, NULL);
    Tcl_CreateCommand( interp, "viewmatrix", SPDViewMatrixCmd, NULL, NULL);
    Tcl_CreateCommand( interp, "transformvector", SPDTransVectorCmd, NULL, NULL);
    Tcl_CreateCommand( interp, "transformpoint", SPDTransPointCmd, NULL, NULL);
    Tcl_CreateCommand( interp, "scalepoint", SPDScalePointCmd, NULL, NULL);
    Tcl_CreateCommand( interp, "rotatepoint", SPDRotatePointCmd, NULL, NULL);
    Tcl_CreateCommand( interp, "matrixmult", SPDMulMatrixCmd, NULL, NULL);
}

/*
 * Fetch a vector from argv
 */
static int
GetVector( interp, string, vec)
    Tcl_Interp *interp;
    char *string;
    COORD3 vec;
{
    int result, i;
    int argc = 0;
    char **argv = NULL;

    result = Tcl_SplitList( interp, string, &argc, &argv);
    if( result == TCL_ERROR)
	goto cleanup;

    if (argc != 3) {
	interp->result = "Bad vector length";
	result = TCL_ERROR;
	goto cleanup;
    }
    for( i = 0; i < argc; i++)
	if(Tcl_GetDouble( interp, argv[i], &vec[i]) == TCL_ERROR)
	    goto cleanup;

    result = TCL_OK;

cleanup:
    if(argv)
	ckfree((char *) argv);
    return result;
}

/*
 * Fetch matrix from list
 */
static int
GetMatrix( interp, list, mx)
    Tcl_Interp *interp;
    char *list;
    MATRIX mx;
{
    int result, i, j, n;
    int argc = 0;
    char **argv = NULL;

    result = Tcl_SplitList( interp, list, &argc, &argv);
    if( result == TCL_ERROR)
	goto cleanup;

    if (argc != 16) {
	interp->result = "Bad matrix length";
	result = TCL_ERROR;
	goto cleanup;
    }

    n = 0;
    for( i = 0; i < 4; i++)
      for( j = 0; j < 4; j++)
	if(Tcl_GetDouble( interp, argv[n++], &mx[i][j]) == TCL_ERROR)
	    goto cleanup;

    result = TCL_OK;

cleanup:
    if(argv)
	ckfree((char *) argv);
    return result;
}

static char *
PrintVector( interp, vec)
    Tcl_Interp * interp;
    COORD3 vec;
{
    int i, x;
    /*static char buffer[3 * TCL_DOUBLE_SPACE + 4];*/
    char *buffer = ckalloc( 3 * TCL_DOUBLE_SPACE + 4);

    x = 0;
    for( i = 0; i < 3; i++) {
	Tcl_PrintDouble( interp, vec[i], &buffer[x]);
	x = strlen( buffer);
	buffer[x++] = ' ';
    }
    buffer[x] = '\0';
    return buffer;
}

static char *
PrintMatrix( interp, mx)
    Tcl_Interp * interp;
    MATRIX mx;
{
    int i, j, x;
    /*static char buffer[16 * TCL_DOUBLE_SPACE + 17];*/
    char * buffer = ckalloc(16 * TCL_DOUBLE_SPACE + 17);

    x = 0;
    for( i = 0; i < 4; i++)
      for( j = 0; j < 4; j++) {
	Tcl_PrintDouble( interp, mx[i][j], &buffer[x]);
	x = strlen( buffer);
	buffer[x++] = ' ';
      }
    buffer[x-1] = '\0';
    return buffer;
}

/*
 * Multiply a point by a matrix.
 */
static int
SPDTransPointCmd(clientData, interp, argc, argv)
    ClientData clientData;
    Tcl_Interp *interp;
    int argc;
    char **argv;
{
    COORD3 vres, vec;
    MATRIX mx;

    if (argc != 3) {
	Tcl_AppendResult(interp, "wrong # args:  should be \"",
		argv[0], " point matrix\"", (char *) NULL);
	return TCL_ERROR;
    }

    if( GetVector( interp, argv[1], vec) == TCL_ERROR)
	return TCL_ERROR;

    if( GetMatrix( interp, argv[2], mx) == TCL_ERROR)
	return TCL_ERROR;

    lib_transform_point(vres, vec, mx);

    Tcl_SetResult( interp, PrintVector( interp, vres), TCL_DYNAMIC);
    return TCL_OK;
}

/*
 * Scale a point by a vector.
 */
static int
SPDScalePointCmd(clientData, interp, argc, argv)
    ClientData clientData;
    Tcl_Interp *interp;
    int argc;
    char **argv;
{
    COORD3 vres, vec, scale;
    MATRIX mx;

    if (argc != 3) {
	Tcl_AppendResult(interp, "wrong # args:  should be \"",
		argv[0], " point vector\"", (char *) NULL);
	return TCL_ERROR;
    }

    if( GetVector( interp, argv[1], vec) == TCL_ERROR)
	return TCL_ERROR;

    if( GetVector( interp, argv[2], scale) == TCL_ERROR)
	return TCL_ERROR;

    lib_create_scale_matrix(mx, scale);

    lib_transform_point(vres, vec, mx);

    Tcl_SetResult( interp, PrintVector( interp, vres), TCL_DYNAMIC);
    return TCL_OK;
}

/*
 * Scale a point by a vector.
 */
static int
SPDRotatePointCmd(clientData, interp, argc, argv)
    ClientData clientData;
    Tcl_Interp *interp;
    int argc;
    char **argv;
{
    double angle;
    COORD3 vres, vec, rot;
    MATRIX mx;

    if (argc != 4) {
	Tcl_AppendResult(interp, "wrong # args:  should be \"",
		argv[0], " point vector angle\"", (char *) NULL);
	return TCL_ERROR;
    }

    if( GetVector( interp, argv[1], vec) == TCL_ERROR)
	return TCL_ERROR;

    if( GetVector( interp, argv[2], rot) == TCL_ERROR)
	return TCL_ERROR;

    if( Tcl_GetDouble( interp, argv[3], &angle) == TCL_ERROR)
	return TCL_ERROR;

    lib_create_axis_rotate_matrix( mx, rot, angle);

    lib_transform_point(vres, vec, mx);

    Tcl_SetResult( interp, PrintVector( interp, vres), TCL_DYNAMIC);
    return TCL_OK;
}

static int
SPDNormalizeVectorCmd (clientData, interp, argc, argv)
    ClientData clientData;
    Tcl_Interp *interp;
    int argc;
    char **argv;
{
    double divisor;
    char *list;
    COORD3 vec;
    static char buffer[TCL_DOUBLE_SPACE+1];

    if (argc != 2) {
	Tcl_AppendResult(interp, "wrong # args:  should be \"",
		argv[0], " vector\"", (char *) NULL);
	return TCL_ERROR;
    }

    if((list = Tcl_GetVar( interp, argv[1], TCL_LEAVE_ERR_MSG)) == NULL)
	return TCL_ERROR;

    if( GetVector( interp, list, vec) == TCL_ERROR)
	return TCL_ERROR;

    divisor = lib_normalize_vector( vec);
    Tcl_SetVar( interp, argv[1], list = PrintVector( interp, vec), TCL_LEAVE_ERR_MSG);
    free( list);

    Tcl_PrintDouble( interp, divisor, buffer);
    Tcl_SetResult( interp, buffer, TCL_STATIC);
    return TCL_OK;
}

static int
SPDTransVectorCmd (clientData, interp, argc, argv)
    ClientData clientData;
    Tcl_Interp *interp;
    int argc;
    char **argv;
{
    COORD3 vres, vec;
    MATRIX mx;

    if (argc != 3) {
	Tcl_AppendResult(interp, "wrong # args:  should be \"",
		argv[0], " vector matrix\"", (char *) NULL);
	return TCL_ERROR;
    }

    if( GetVector( interp, argv[1], vec) == TCL_ERROR)
	return TCL_ERROR;

    if( GetMatrix( interp, argv[2], mx) == TCL_ERROR)
	return TCL_ERROR;

    lib_transform_vector(vres, vec, mx);

    Tcl_SetResult( interp, PrintVector( interp, vres), TCL_DYNAMIC);
    return TCL_OK;
}
static int
SPDZeroMatrixCmd (clientData, interp, argc, argv)
    ClientData clientData;
    Tcl_Interp *interp;
    int argc;
    char **argv;
{
    MATRIX zero;

    lib_zero_matrix( zero);

    Tcl_SetResult( interp, PrintMatrix( interp, zero), TCL_DYNAMIC);
    return TCL_OK;
}
static int
SPDIdentityMatrixCmd (clientData, interp, argc, argv)
    ClientData clientData;
    Tcl_Interp *interp;
    int argc;
    char **argv;
{
    MATRIX ident;

    lib_create_identity_matrix( ident);

    Tcl_SetResult( interp, PrintMatrix( interp, ident), TCL_DYNAMIC);
    return TCL_OK;
}

static int
SPDTranslateMatrixCmd (clientData, interp, argc, argv)
    ClientData clientData;
    Tcl_Interp *interp;
    int argc;
    char **argv;
{
}
static int
SPDRotateMatrixCmd (clientData, interp, argc, argv)
    ClientData clientData;
    Tcl_Interp *interp;
    int argc;
    char **argv;
{
    double angle, divisor;
    COORD3 vec;
    MATRIX mres;

    if (argc != 3) {
	Tcl_AppendResult(interp, "wrong # args:  should be \"",
		argv[0], " angle axis\"", (char *) NULL);
	return TCL_ERROR;
    }

    if( Tcl_GetDouble( interp, argv[1], &angle) == TCL_ERROR)
	return TCL_ERROR;

    if( GetVector( interp, argv[2], vec) == TCL_ERROR)
	return TCL_ERROR;

    divisor = lib_normalize_vector( vec);
    if( divisor == 0.0) {
	Tcl_AppendResult(interp, argv[0], ": degenerate angle", (char *) NULL);
	return TCL_ERROR;
    }

    lib_create_axis_rotate_matrix( mres, vec, angle);

    Tcl_SetResult( interp, PrintMatrix(interp, mres), TCL_DYNAMIC);
    return TCL_OK;
}

static int
SPDScaleMatrixCmd (clientData, interp, argc, argv)
    ClientData clientData;
    Tcl_Interp *interp;
    int argc;
    char **argv;
{
    COORD3 vec;
    MATRIX mres;

    if (argc != 2) {
	Tcl_AppendResult(interp, "wrong # args:  should be \"",
		argv[0], " vector\"", (char *) NULL);
	return TCL_ERROR;
    }

    if( GetVector( interp, argv[1], vec) == TCL_ERROR)
	return TCL_ERROR;

    lib_create_scale_matrix( mres, vec);

    Tcl_SetResult( interp, PrintMatrix(interp, mres), TCL_DYNAMIC);
    return TCL_OK;
}

static int
SPDViewMatrixCmd (clientData, interp, argc, argv)
    ClientData clientData;
    Tcl_Interp *interp;
    int argc;
    char **argv;
{
    int xres, yres;
    double angle, aspect;
    COORD3 from, at, up;
    MATRIX mres;

    if (argc != 8) {
	Tcl_AppendResult(interp, "wrong # args:  should be \"",
		argv[0], " from at up xres yres angle aspect\"", (char *) NULL);
	return TCL_ERROR;
    }

    if( GetVector( interp, argv[1], from) == TCL_ERROR)
	return TCL_ERROR;

    if( GetVector( interp, argv[2], at) == TCL_ERROR)
	return TCL_ERROR;

    if( GetVector( interp, argv[3], up) == TCL_ERROR)
	return TCL_ERROR;

    if( Tcl_GetInt( interp, argv[4], &xres) == TCL_ERROR)
	return TCL_ERROR;

    if( Tcl_GetInt( interp, argv[5], &yres) == TCL_ERROR)
	return TCL_ERROR;

    if( Tcl_GetDouble( interp, argv[6], &angle) == TCL_ERROR)
	return TCL_ERROR;

    if( Tcl_GetDouble( interp, argv[7], &aspect) == TCL_ERROR)
	return TCL_ERROR;

    lib_create_view_matrix(mres, from, at, up, xres, yres, angle, aspect);

    Tcl_SetResult( interp, PrintMatrix(interp, mres), TCL_DYNAMIC);
    return TCL_OK;
}

static int
SPDMulMatrixCmd (clientData, interp, argc, argv)
    ClientData clientData;
    Tcl_Interp *interp;
    int argc;
    char **argv;
{
    MATRIX mx1, mx2, mres;

    if (argc != 3) {
	Tcl_AppendResult(interp, "wrong # args:  should be \"",
		argv[0], " matrix1 matrix2\"", (char *) NULL);
	return TCL_ERROR;
    }

    if( GetMatrix( interp, argv[1], mx1) == TCL_ERROR)
	return TCL_ERROR;

    if( GetMatrix( interp, argv[2], mx2) == TCL_ERROR)
	return TCL_ERROR;

    lib_matrix_multiply(mres, mx1, mx2);

    Tcl_SetResult( interp, PrintMatrix( interp, mres), TCL_DYNAMIC);
    return TCL_OK;
}
