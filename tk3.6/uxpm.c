/* 
**	Martin L. Smith  NER, Inc
*/
#if defined(USE_UXPM)

#include <stdlib.h>
#include <tk.h>
#if (TK_MAJOR_VERSION >= 3) && (TK_MINOR_VERSION < 3)
#include <tclInt.h>
#endif
#include <ctype.h>
#include <string.h>
/*#include <focus_std.h>*/

typedef struct {
    int		good;
    Tk_Uid	pixname;
    Pixmap	pixmap;
    GC		pixmapgc;
    Tk_Window	pW;
    XGCValues	gcvalues;
} PixmapInfo;

static Tcl_HashTable pxminfo;
static int hash_ready = 0;

/*
**	return pointer to PixmapInfo for named pixmap
*/
static PixmapInfo* getPixmapInfo(dummy, pname, crt)
ClientData dummy;
char* pname;
int crt;
{
    PixmapInfo* pmi;
    int isnew;
    Tcl_HashEntry* he;

    if(hash_ready == 0) {
	++hash_ready;
	Tcl_InitHashTable(&pxminfo, TCL_STRING_KEYS);
    }
    he = Tcl_CreateHashEntry(&pxminfo, pname, &isnew);
/*
**	if it's already in the table, just return its clientdata
*/
    if(isnew == 0) {
	pmi = (PixmapInfo *) he->clientData;
	return pmi;
    }
/*
**	not in the table yet.
*/
    pmi = (PixmapInfo *) malloc(sizeof(PixmapInfo));
    pmi->good = 1;
    he->clientData = pmi;
    pmi->pixname = Tk_GetUid(pname);
    pmi->pW = (Tk_Window) dummy;
/*
**	if pixmap already exists, just get its id.
**	if pixmap is new, leave the id slot blank for the
**		caller to fill in.
*/
    if(crt == 0) {
	pmi->pixmap = Tk_PixmapOfName(pmi->pixname, pmi->pW);
	if(pmi->pixmap == None) {
	    pmi->good = 0;
	    return pmi;
	}
    }
    pmi->pixmapgc = XCreateGC(Tk_Display(pmi->pW),
			      XDefaultRootWindow(Tk_Display(pmi->pW)),
			      0, &pmi->gcvalues);
    return pmi;
}
/*
**	return pointer to PixmapInfo and zap the entry if it exists.
*/
static PixmapInfo* NCZgetPixmapInfo(dummy, pname)
ClientData dummy;
char* pname;
{
    PixmapInfo* pmi;
    int isnew;
    Tcl_HashEntry* he;

    if(hash_ready == 0)
	return 0;
    he = Tcl_FindHashEntry(&pxminfo, pname);
    if(he == 0)
	return 0;
    pmi = (PixmapInfo *) he->clientData;
    Tcl_DeleteHashEntry(he);
    return pmi;
}

static int badPixmapArgs(interp, argc, argv)
Tcl_Interp* interp;
int argc;
char** argv;
{
    Tcl_AppendResult(interp, "bad command: should be ",
		     " upx_pixmap {create name wide high [fmt]}",
		     " | {clear name}",
		     (char *) NULL);
    return TCL_ERROR;
}

/*
**	create name width height depth format
*/
int createSubCmd(dummy, interp, argc, argv)
ClientData dummy;
Tcl_Interp *interp;
int argc;
char **argv;
{
    int	lenx, leny;
    int depth;
    char* format;
    char pstring[32];

    PixmapInfo* pmi;

    if (argc != 7) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" name x y depth format\"", (char *) NULL);
	return TCL_ERROR;
    }

    pmi = getPixmapInfo(dummy, argv[2], 1);

    lenx = strtol(argv[3], 0, 0);
    leny = strtol(argv[4], 0, 0);
    depth = strtol(argv[5], 0, 0);
    format = argv[6];

    if (Tk_DefinePixmap(interp, pmi->pixname, pmi->pW, 0, 0, 0,
			 lenx, leny, depth, format) != TCL_OK) {
	fprintf(stderr, "Tk_DefinePixmap: %s\n", interp->result);
	exit(1);
    }
    if((pmi->pixmap = Tk_GetPixmap(interp, pmi->pW, pmi->pixname)) == None) {
	fprintf(stderr, "Tk_GetPixmap: %s\n", interp->result);
	exit(1);
    }
    sprintf(pstring, "%ld", pmi->pixmap);
    Tcl_AppendResult(interp, pstring, 0);
    return TCL_OK;
}
/*
**	destroy name
*/
int destroySubCmd(dummy, interp, argc, argv)
ClientData dummy;
Tcl_Interp *interp;
int argc;
char **argv;
{
    PixmapInfo* pmi;

    if (argc != 3) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" name\"", (char *) NULL);
	return TCL_ERROR;
    }

    pmi = NCZgetPixmapInfo(dummy, argv[2]);
    if(pmi == 0) {
	Tcl_AppendResult(interp, "destroy: not a pixmap: ", argv[2], 0);
	return TCL_ERROR;
    }
    if (Tk_UndefinePixmap(interp, pmi->pixname, pmi->pW) != TCL_OK ) {
	fprintf(stderr, "Tk_UndefinePixmap: %s\n", interp->result);
	exit(1);
    }
    Tk_FreePixmap(Tk_Display(pmi->pW), pmi->pixmap);
    free(pmi);
    return TCL_OK;
}

int PixresizeCmd(dummy, interp, argc, argv)
ClientData dummy;
Tcl_Interp *interp;
int argc;
char **argv;
{
    int	lenx, leny;

    if (argc != 3) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" x y\"", (char *) NULL);
	return TCL_ERROR;
    }
    lenx = strtol(argv[1], (char **) NULL, 0);
    leny = strtol(argv[2], (char **) NULL, 0);
/*
    UndefineAndFreePixmap(interp);
    DefineAndGetPixmap(interp, lenx, leny);
*/

    return TCL_OK;
}
/*
**	return the index of the first trailing option argument
**	(not a number) or -1 if there are none.  Arguments checked
**	are at first, first + incr, ...  Non-arguments are any numeric
**	strings
*/
static int firstOptionArg(argc, argv, first, incr)
int argc;
char* argv[];
int first;
int incr;
{
    int i;
    char* a;

    for(i = first; i < argc; i += incr) {
	a = argv[i];
	if(isdigit(*a))
	    continue;
	if((*a == '-') && (isdigit(*a)))
	    continue;
	return i;
    }
    return -1;
}
/*
**	process arguments that alter the gc
*/
static void fixGC(interp, argc, argv, firstoptarg, pmi)
Tcl_Interp* interp;
int argc;
char* argv[];
int firstoptarg;
PixmapInfo* pmi;
{
    static XColor* color = 0;

    int i;
    for(i = firstoptarg; i < argc; ) {
	if(strcmp(argv[i], "-fill") == 0) {
	    i++;
	    if (color != NULL) {
		Tk_FreeColor(color);
		color = NULL;
	    }
	    if((color = Tk_GetColor(interp, pmi->pW, 0, 
				    Tk_GetUid(argv[i]))) == 0) {
		pmi->good = 0;
		return;
	    }
	    pmi->gcvalues.foreground = color->pixel;
	    XChangeGC(Tk_Display(pmi->pW), pmi->pixmapgc, GCForeground,
		      &pmi->gcvalues);
	    i++;
	    continue;
	} else {
	    fprintf(stderr, "fixGC: bad arg \"%s\"\n", argv[i]);
	    exit(3);
	}
    }
    return;
}
/*
**	line name x0 y0 x1 y1 ... ?options?
*/
static int lineSubCmd(dummy, interp, argc, argv)
ClientData dummy;
Tcl_Interp *interp;
int argc;
char **argv;
{
    PixmapInfo* pmi;
    int newX, newY;
    int oldX, oldY;
    int lastarg;
    int lastvaluearg;

    if (argc < 6) {
	Tcl_AppendResult(interp, "not enough args: should be \"", argv[0],
		" name x0 y0 x1 y1 ...\"", (char *) NULL);
	return TCL_ERROR;
    }
    pmi = getPixmapInfo(dummy, argv[2], 0);
    if(!pmi->good) {
	Tcl_AppendResult(interp, "not a pixmap: ", argv[2], 0);
	return TCL_ERROR;
    }
    lastvaluearg = firstOptionArg(argc, argv, 3, 2);
    if(lastvaluearg < 0) {
	lastvaluearg = argc - 1;
    } else {
	fixGC(interp, argc, argv, lastvaluearg, pmi);
	--lastvaluearg;
	if(!pmi->good)
	    return TCL_ERROR;
    }
    if(lastvaluearg < 6) {
	Tcl_AppendResult(interp, "no complete line segments: ", argv[2], 0);
	return TCL_ERROR;
    }
    oldX = atoi(argv[3]);
    oldY = atoi(argv[4]);
    lastarg = 6;
    for(;;) {
	if(lastarg > lastvaluearg)
	    break;
	newX = atoi(argv[lastarg - 1]);
	newY = atoi(argv[lastarg]);
	XDrawLine(Tk_Display(pmi->pW), pmi->pixmap, pmi->pixmapgc,	\
		  oldX, oldY, newX, newY);
	lastarg += 2;
	oldX = newX;
	oldY = newY;
    }
    return TCL_OK;
}
/*
**	dot name x0 y0 ... ?options?
*/
static int dotSubCmd(dummy, interp, argc, argv)
ClientData dummy;
Tcl_Interp *interp;
int argc;
char **argv;
{
    int newX, newY;
    int lastarg;
    int lastvaluearg;
    PixmapInfo* pmi;

    if (argc < 5) {
	Tcl_AppendResult(interp, "not enough args: should be \"", argv[0],
		" name x0 y0 ...\"", (char *) NULL);
	return TCL_ERROR;
    }
    pmi = getPixmapInfo(dummy, argv[2], 0);
    if(!pmi->good) {
	Tcl_AppendResult(interp, "not a pixmap: ", argv[2], 0);
	return TCL_ERROR;
    }
    lastvaluearg = firstOptionArg(argc, argv, 3, 2);
    if(lastvaluearg < 0) {
	lastvaluearg = argc - 1;
    } else {
	fixGC(interp, argc, argv, lastvaluearg, pmi);
	--lastvaluearg;
	if(!pmi->good)
	    return TCL_ERROR;
    }
    if(lastvaluearg < 4) {
	Tcl_AppendResult(interp, "no complete dots: ", argv[2], 0);
	return TCL_ERROR;
    }
    lastarg = 4;
    for(;;) {
	if(lastarg > lastvaluearg)
	    break;
	newX = atoi(argv[lastarg - 1]);
	newY = atoi(argv[lastarg]);
	XDrawPoint(Tk_Display(pmi->pW), pmi->pixmap,
		   pmi->pixmapgc, newX, newY);
	lastarg += 2;
    }
    return TCL_OK;
}
/*
**	clear name ?options?
*/
static int clearSubCmd(dummy, interp, argc, argv)
ClientData dummy;
Tcl_Interp *interp;
int argc;
char **argv;
{
    PixmapInfo* pmi;
    unsigned int lenx, leny;

    if (argc < 3) {
	Tcl_AppendResult(interp, "not enough args: should be \"", argv[0],
		" name ?options?\"", (char *) NULL);
	return TCL_ERROR;
    }
    pmi = getPixmapInfo(dummy, argv[2], 0);
    if(!pmi->good) {
	Tcl_AppendResult(interp, "not a pixmap: ", argv[1], 0);
	return TCL_ERROR;
    }
    if(argc > 3) {
	fixGC(interp, argc, argv, 3, pmi);
	if(!pmi->good)
	    return TCL_ERROR;
    }
    Tk_SizeOfPixmap( Tk_Display(pmi->pW), pmi->pixmap, &lenx, &leny);
    XFillRectangle(Tk_Display(pmi->pW), pmi->pixmap, pmi->pixmapgc,
		   0, 0, lenx, leny);
    return TCL_OK;
}
/*
**	rectangle name xl yb ht wd ?options?
*/
static int rectangleSubCmd(dummy, interp, argc, argv)
ClientData dummy;
Tcl_Interp *interp;
int argc;
char **argv;
{
    PixmapInfo* pmi;
    unsigned int xz, yz, dx, dy;

    if (argc < 7) {
	Tcl_AppendResult(interp, "not enough args: should be \"", argv[0],
		" name xl yb ht wd ?options?\"", (char *) NULL);
	return TCL_ERROR;
    }
    xz = atoi(argv[3]);
    yz = atoi(argv[4]);
    dx = atoi(argv[5]);
    dy = atoi(argv[6]);
    pmi = getPixmapInfo(dummy, argv[2], 0);
    if(!pmi->good) {
	Tcl_AppendResult(interp, "not a pixmap: ", argv[1], 0);
	return TCL_ERROR;
    }
    if(argc > 7) {
	fixGC(interp, argc, argv, 7, pmi);
	if(!pmi->good)
	    return TCL_ERROR;
    }
    XFillRectangle(Tk_Display(pmi->pW), pmi->pixmap, pmi->pixmapgc,
		   xz, yz, dx, dy);
    return TCL_OK;
}
/*
**	copy src xs ys dx dy dest xd yd ?options?
*/
static int copySubCmd(dummy, interp, argc, argv)
ClientData dummy;
Tcl_Interp *interp;
int argc;
char **argv;
{
    PixmapInfo* pmi;
    PixmapInfo* dmi;
    int xs, ys, dx, dy, xd, yd;

    if (argc < 10) {
	Tcl_AppendResult(interp, "not enough args: should be \"", argv[0],
			 " name src xs ys ht wd dest xd yd ?options?\"",
			 (char *) NULL);
	return TCL_ERROR;
    }
    xs = atoi(argv[3]);
    ys = atoi(argv[4]);
    dx = atoi(argv[5]);
    dy = atoi(argv[6]);
    xd = atoi(argv[8]);
    yd = atoi(argv[9]);
    pmi = getPixmapInfo(dummy, argv[2], 0);
    if(!pmi->good) {
	Tcl_AppendResult(interp, "source not a pixmap: ", argv[1], 0);
	return TCL_ERROR;
    }
    dmi = getPixmapInfo(dummy, argv[7], 0);
    if(!dmi->good) {
	Tcl_AppendResult(interp, "destination not a pixmap: ", argv[1], 0);
	return TCL_ERROR;
    }
    if(argc > 10) {
	fixGC(interp, argc, argv, 10, pmi);
	if(!pmi->good)
	    return TCL_ERROR;
    }
    XCopyArea(Tk_Display(pmi->pW), pmi->pixmap, dmi->pixmap,
		   dmi->pixmapgc, xs, ys, dx, dy, xd, yd);
    return TCL_OK;
}

typedef int ((*subProc)(ClientData, Tcl_Interp*, int, char**));
typedef struct {
    char*	subname;
    subProc	f;
} subEntry;

static subEntry subTable[] = {
    {"create", createSubCmd},
    {"destroy", destroySubCmd},
    {"clear", clearSubCmd},
    {"rectangle", rectangleSubCmd},
    {"copy", copySubCmd},
    {"dot", dotSubCmd},
    {"line", lineSubCmd},
    { 0, 0}
};

/*
**	upx_uxpm master command
*/
int uxpmCmd(cd, interp, argc, argv)
ClientData cd;
Tcl_Interp* interp;
int argc;
char *argv[];
{
    int i;

    if (argc < 2)
	return badPixmapArgs(interp, argc, argv);

    for(i = 0; subTable[i].subname != 0; i++)
	if(strcmp(argv[1], subTable[i].subname) == 0)
	   return subTable[i].f(cd, interp, argc, argv);

    return badPixmapArgs(interp, argc, argv);
}

int
Uxpm_Init(interp)
Tcl_Interp *interp;         /* Interpreter for application. */
{
#if (TK_MAJOR_VERSION >= 3) && (TK_MINOR_VERSION >= 3)
    Tk_Window main;

    main = Tk_MainWindow(interp);
    Tk_MakeWindowExist(main);
    Tcl_CreateCommand(interp, "uxpm", uxpmCmd,
		      (ClientData) main, (void (*)()) NULL);
#endif
    return TCL_OK;
}

#endif

/* eof */
