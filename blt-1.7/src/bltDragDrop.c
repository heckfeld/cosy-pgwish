/*
 * ------------------------------------------------------------------------
 *  PURPOSE:  drag&drop widget registration facility
 *
 *  Allows widgets to be registered as drag&drop sources and targets
 *  for handling "drag-and-drop" operations between Tcl/Tk applications.
 *
 *  USAGE:
 *    blt_drag&drop source
 *    blt_drag&drop source <pathName>
 *    blt_drag&drop source <pathName> config ?options...?
 *    blt_drag&drop source <pathName> handler ?<dataType> <cmd>...?
 *
 *    blt_drag&drop target
 *    blt_drag&drop target <pathName> handler
 *    blt_drag&drop target <pathName> handler ?<dataType> <cmd>...?
 *    blt_drag&drop target <pathName> handle <dataType>
 *
 *    blt_drag&drop drag <pathName> <x> <y>
 *    blt_drag&drop drop <pathName> <x> <y>
 *
 *    blt_drag&drop errors ?<proc>?
 *    blt_drag&drop active
 *    blt_drag&drop location ?<x> <y>?
 *
 * ------------------------------------------------------------------------
 *  AUTHOR:  Michael J. McLennan       Phone: (215)770-2842
 *           AT&T Bell Laboratories   E-mail: aluxpo!mmc@att.com
 *
 *     RCS:  bltDragDrop.c,v 1.10 1994/04/14 21:11:03 gah Exp
 * ========================================================================
 *                 Copyright (c) 1993-1994  AT&T Bell Laboratories
 * ========================================================================
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that the copyright notice and warranty disclaimer appear in
 * supporting documentation, and that the names of AT&T Bell Laboratories
 * any of their entities not be used in advertising or publicity
 * pertaining to distribution of the software without specific, written
 * prior permission.
 * 
 * AT&T disclaims all warranties with regard to this software, including
 * all implied warranties of merchantability and fitness.  In no event
 * shall AT&T be liable for any special, indirect or consequential
 * damages or any damages whatsoever resulting from loss of use, data or
 * profits, whether in an action of contract, negligence or other
 * tortuous action, arising out of or in connection with the use or
 * performance of this software.
 * ========================================================================
 */
#include "blt.h"
#include <X11/Xatom.h>

#ifndef DRAGDROP_VERSION
#define DRAGDROP_VERSION "2.1"
#endif

#define DRAGDROP_COMMAND	"blt_drag&drop"   /* Command name */
#define DRAGDROP_CLASS		"DragDrop"  	  /* CLASS NAME for token window */
#define DRAGDROP_PROPINFO	"BltDragDropInfo" /* Property name */
#define DRAGDROP_ERRORPROC	"tkerror"		  /* Error Proc used to report 
											   * drag&drop background errors */

#ifndef _BLT_H
/*
 *  -------------  The following color definitions are taken directly
 *  >> WARNING <<  from "default.h" in the Tk distribution, and should
 *  -------------  be kept up to date with those conventions.
 */
#define BLACK       "Black"
#define WHITE       "White"
#define GRAY        "#b0b0b0"

#define BISQUE1     "#ffe4c4"
#define BISQUE2     "#eed5b7"
#define BISQUE3     "#cdb79e"
#endif							/* _BLT_H */
#define DEF_BUTTON_ACTIVE_BG_COLOR  BISQUE2
#define DEF_BUTTON_ACTIVE_BG_MONO   BLACK
#define DEF_BUTTON_ACTIVE_FG_COLOR  BLACK
#define DEF_BUTTON_ACTIVE_FG_MONO   WHITE
#define DEF_BUTTON_BG_COLOR     BISQUE1
#define DEF_BUTTON_BG_MONO      WHITE
#define DEF_TOKEN_OUTLINE_COLOR     BLACK
#define DEF_TOKEN_OUTLINE_MONO      BLACK

/*
 *  DRAG&DROP ROOT WINDOW HIERARCHY (cached during "drag" operations)
 */
typedef struct DD_WinRep {
	Window win;               /* X window for this record */
	int initialized;          /* non-zero => rest of info is valid */
	int x0, y0;               /* upper-left corner of window */
	int x1, y1;               /* lower-right corner of window */
	char *ddprop;             /* drag&drop property info */
	char *ddinterp;           /* interp name within ddprop */
	char *ddwin;              /* target window name within ddprop */
	char *ddhandlers;         /* list of handlers within ddprop */
	struct DD_WinRep* parent; /* window containing this as a child */
	struct DD_WinRep* kids;   /* list of child windows */
	struct DD_WinRep* next;   /* next sibling */
} DD_WinRep;

/*
 *  DRAG&DROP REGISTRATION DATA
 */
typedef struct {
	Tcl_Interp *interp;       /* interpreter managing this drag&drop command */
	Tk_Window root;           /* main window for application */
	Tcl_HashTable srcList;    /* list of source widgets */
	Tcl_HashTable trgList;    /* list of target widgets */
	char *errorProc;          /* proc invoked for drag&drop errors */
	int numactive;            /* number of active drag&drop operations */
	int locx, locy;           /* last location point */
	DD_WinRep *pool;          /* pool of available DD_WinRep records */
} DD_RegList;

typedef struct {
	DD_RegList *ddlist;       /* parent registration list */
	Tk_Window tkwin;          /* registered window */
} DD_RegEntry;

/*
 *  DRAG&DROP SOURCE REGISTRATION RECORD
 */
typedef struct DD_SourceHndl {
	char *dataType;               /* name of data type */
	char *cmd;                    /* command used to send data */
	struct DD_SourceHndl* next;   /* next handler in linked list */
} DD_SourceHndl;

typedef struct {
	DD_RegList *ddlist;           /* registration list containing this */

	Display *display;             /* drag&drop source window display */
	Tk_Window tkwin;              /* drag&drop source window */
	Atom ddAtom;                  /* X atom referring to "DragDropInfo" */
	int button;                   /* button used to invoke drag for sources */

	Tk_Window tokenwin;           /* window representing drag item */
	Tk_Anchor tokenAnchor;        /* position of token win relative to mouse */
	Cursor tokenCursor;           /* cursor used when dragging token */
	Tk_3DBorder tokenOutline;     /* outline around token window */
	Tk_3DBorder tokenBorder;      /* border/background for token window */
	int tokenBorderWidth;         /* border width in pixels */
	XColor *rejectFg;             /* color used to draw rejection fg: (\) */
	XColor *rejectBg;             /* color used to draw rejection bg: (\) */
	Pixmap rejectSt;              /* stipple used to draw rejection: (\) */
	GC rejectFgGC;                /* GC used to draw rejection fg: (\) */
	GC rejectBgGC;                /* GC used to draw rejection bg: (\) */

	int pkgcmdInProg;             /* non-zero => executing pkgcmd */
	char *pkgcmd;                 /* cmd executed on start of drag op */
	char *pkgcmdResult;           /* result returned by recent pkgcmd */
	char *sitecmd;                /* cmd executed to update token win */

	DD_WinRep *allwins;           /* window info (used during "drag") */
	int selfTarget;               /* non-zero => source can drop onto itself */
	int overTargetWin;            /* non-zero => over target window */
	int tokenx, tokeny;           /* last position of token window */
	Tk_TimerToken hidetoken;      /* token for routine to hide tokenwin */
	Cursor normalCursor;          /* cursor restored after dragging */

	char *send;                   /* list of data handler names or "all" */
	DD_SourceHndl *handlers;      /* list of data handlers */
} DD_Source;

/*
 *  STACK INFO
 */
typedef struct DD_Stack {
    ClientData *values;     /* values on stack */
    int len;                /* number of values on stack */
    int max;                /* maximum size of stack */
    ClientData space[5];    /* initial space for stack data */
} DD_Stack;

/*
 *  CONFIG PARAMETERS
 */
static Tk_ConfigSpec SourceConfigSpecs[] = {

	{TK_CONFIG_INT,
		"-button", "buttonBinding", "ButtonBinding",
		"3", Tk_Offset(DD_Source, button),
		0},

	{TK_CONFIG_STRING,
		"-packagecmd", "packageCommand", "Command",
		NULL, Tk_Offset(DD_Source, pkgcmd),
		TK_CONFIG_NULL_OK},

	{TK_CONFIG_COLOR,
		"-rejectbg", "rejectBackground", "Background",
		DEF_BUTTON_BG_COLOR, Tk_Offset(DD_Source, rejectBg),
		TK_CONFIG_COLOR_ONLY},
	{TK_CONFIG_COLOR,
		"-rejectbg", "rejectBackground", "Background",
		"white", Tk_Offset(DD_Source, rejectBg),
		TK_CONFIG_MONO_ONLY},

	{TK_CONFIG_COLOR,
		"-rejectfg", "rejectForeground", "Foreground",
		"red", Tk_Offset(DD_Source, rejectFg),
		TK_CONFIG_COLOR_ONLY},
	{TK_CONFIG_COLOR,
		"-rejectfg", "rejectForeground", "Foreground",
		"black", Tk_Offset(DD_Source, rejectFg),
		TK_CONFIG_MONO_ONLY},

	{TK_CONFIG_BITMAP,
		"-rejectstipple", "rejectStipple", "Stipple",
		(char*)NULL, Tk_Offset(DD_Source, rejectSt),
		TK_CONFIG_COLOR_ONLY | TK_CONFIG_NULL_OK},
	{TK_CONFIG_BITMAP,
		"-rejectstipple", "rejectStipple", "Stipple",
		"gray50", Tk_Offset(DD_Source, rejectSt),
		TK_CONFIG_MONO_ONLY},

	{TK_CONFIG_BOOLEAN,
		"-selftarget", "selfTarget", "SelfTarget",
		"no", Tk_Offset(DD_Source, selfTarget),
		0},

	{TK_CONFIG_STRING,
		"-send", "send", "Send",
		"all", Tk_Offset(DD_Source, send),
		TK_CONFIG_NULL_OK},

	{TK_CONFIG_STRING,
		"-sitecmd", "siteCommand", "Command",
		NULL, Tk_Offset(DD_Source, sitecmd),
		TK_CONFIG_NULL_OK},

	{TK_CONFIG_ANCHOR,
		"-tokenanchor", "tokenAnchor", "Anchor",
		"center", Tk_Offset(DD_Source, tokenAnchor),
		0},

	{TK_CONFIG_BORDER,
		"-tokenbg", "tokenBackground", "Background",
		DEF_BUTTON_BG_COLOR, Tk_Offset(DD_Source, tokenBorder),
		TK_CONFIG_COLOR_ONLY},
	{TK_CONFIG_BORDER,
		"-tokenbg", "tokenBackground", "Background",
		DEF_BUTTON_BG_MONO, Tk_Offset(DD_Source, tokenBorder),
		TK_CONFIG_MONO_ONLY},

	{TK_CONFIG_BORDER,
		"-tokenoutline", "tokenOutline", "Outline",
		DEF_TOKEN_OUTLINE_COLOR, Tk_Offset(DD_Source, tokenOutline),
		TK_CONFIG_COLOR_ONLY},
	{TK_CONFIG_BORDER,
		"-tokenoutline", "tokenOutline", "Outline",
		DEF_TOKEN_OUTLINE_MONO, Tk_Offset(DD_Source, tokenOutline),
		TK_CONFIG_MONO_ONLY},

	{TK_CONFIG_PIXELS,
		"-tokenborderwidth", "tokenBorderWidth", "BorderWidth",
		"3", Tk_Offset(DD_Source, tokenBorderWidth),
		0},

	{TK_CONFIG_CURSOR,
		"-tokencursor", "tokenCursor", "Cursor",
		"center_ptr", Tk_Offset(DD_Source, tokenCursor),
		TK_CONFIG_NULL_OK},

	{TK_CONFIG_END,
		(char*)NULL, (char*)NULL, (char*)NULL,
		(char*)NULL, 0,
		0},
};


/*
 *  DRAG&DROP TARGET REGISTRATION RECORD
 */
typedef struct DD_TargetHndl {
	char *dataType;               /* Name of data type (malloc-ed) */
	char *command;                /* command to handle data type (malloc-ed) */
	struct DD_TargetHndl* next;   /* next handler in linked list */
} DD_TargetHndl;

typedef struct {
	DD_RegList *ddlist;           /* registration list containing this */

	Display *display;             /* drag&drop target window display */
	Tk_Window tkwin;              /* drag&drop target window */
	DD_TargetHndl *handlers;      /* list of data handlers */
} DD_Target;

/*
 * Each "drag&drop" widget window is tagged with a "DragDropInfo"
 * property in XA_STRING format.  This property identifies the
 * window as a "drag&drop" widget, and contains the following:
 *
 *     "<interp-name>]<drag&drop-path>]<handler-list>"
 *
 * The <drag&drop-path> is the window path name of the drag&drop
 * widget, <interp-name> is the name of the interpreter controlling
 * the widget (useful for the "send" command), and <handler-list>
 * is the list of handler types recognized by the widget.
 *
 * When the user invokes the "drag" operation, a snapshot of the
 * entire window hierarchy is made, and windows carrying a
 * "DragDropInfo" property are identified.  As the token window is
 * dragged around, * this snapshot can be queried to determine when
 * the token is over a valid target window.  When the token is
 * dropped over a valid site, the drop information is sent to the
 * application via the usual "send" command.  If communication fails,
 * the drag&drop facility automatically posts a rejection symbol on
 * the token window.
 */

/*
 *  Maximum size property that can be read at one time:
 */
#define MAX_PROP_SIZE 1000

/*
 *  FORWARD DECLARATIONS
 */
int Blt_DragDropInit _ANSI_ARGS_((Tcl_Interp* interp));

static void DragDrop_Delete _ANSI_ARGS_((ClientData clientData));
static int DragDrop_Cmd _ANSI_ARGS_((ClientData clientData,
	Tcl_Interp *interp, int argc, char **argv));

static DD_Source* GetSourceInfo _ANSI_ARGS_((DD_RegList *ddlist,
	char *pathname, int *newEntry));
static void DestroySourceInfo _ANSI_ARGS_((DD_RegList *ddlist,
	char *pathName));
static int ConfigSource _ANSI_ARGS_((Tcl_Interp *interp, DD_Source *dsPtr,
	int argc, char **argv, int flags));
static char* FindSourceHandler _ANSI_ARGS_((DD_Source* dsPtr, char *dtname));
static void PutSourceHandler _ANSI_ARGS_((DD_Source* dsPtr, char *dtname,
	char *cmd));
static DD_SourceHndl* CreateSourceHandler _ANSI_ARGS_((char *dtname,
	char *cmd));
static void DestroySourceHandler _ANSI_ARGS_((DD_SourceHndl *dsHndl));
static void UnregSource _ANSI_ARGS_((ClientData clientData, XEvent *eventPtr));

static DD_Target* GetTargetInfo _ANSI_ARGS_((DD_RegList *ddlist,
	char *pathname, int *newEntry));
static void DestroyTargetInfo _ANSI_ARGS_((DD_RegList *ddlist,
	char *pathName));
static char* FindTargetHandler _ANSI_ARGS_((DD_Target* dtPtr, char *dtname));
static void PutTargetHandler _ANSI_ARGS_((DD_Target* dtPtr, char *dtname,
	char *cmd));
static DD_TargetHndl* CreateTargetHandler _ANSI_ARGS_((char *dtname,
	char *cmd));
static void DestroyTargetHandler _ANSI_ARGS_((DD_TargetHndl *dtHndl));
static void UnregTarget _ANSI_ARGS_((ClientData clientData, XEvent *eventPtr));

static void DragDropSend _ANSI_ARGS_((DD_Source *dsPtr));
static char* DragDropSendHndlr _ANSI_ARGS_((DD_Source *dsPtr,
	char *interpName, char *ddName));

static DD_WinRep* GetWinRepInfo _ANSI_ARGS_((DD_Source *dsPtr,
	DD_RegList *ddlist));
static DD_WinRep* FindTargetWin _ANSI_ARGS_((DD_Source *dsPtr, int x, int y));
static DD_WinRep* WinRepAlloc _ANSI_ARGS_((DD_RegList* ddlist));
static void WinRepRelease _ANSI_ARGS_((DD_WinRep* wr, DD_RegList* ddlist));
static void WinRepInit _ANSI_ARGS_((DD_WinRep* wr, DD_Source *dsPtr));

static void AddDDProp _ANSI_ARGS_((DD_Target *dtPtr));
static void DDTokenEventProc _ANSI_ARGS_((ClientData, XEvent*));
static void MoveDDToken _ANSI_ARGS_((DD_Source *dsPtr));
static void UpdateDDToken _ANSI_ARGS_((ClientData clientData));
static void HideDDToken _ANSI_ARGS_((ClientData clientData));
static void RejectDDToken _ANSI_ARGS_((DD_Source* dsPtr));

static void StackInit _ANSI_ARGS_((DD_Stack *stack));
static void StackDelete _ANSI_ARGS_((DD_Stack *stack));
static void StackPush _ANSI_ARGS_((ClientData cdata, DD_Stack *stack));
static ClientData StackPop _ANSI_ARGS_((DD_Stack *stack));


/*
 * ------------------------------------------------------------------------
 *  Blt_DragDropInit()
 *
 *  Adds the drag&drop command to the given interpreter.  Should be
 *  invoked to properly install the command whenever a new interpreter
 *  is created.
 * ------------------------------------------------------------------------
 */
int
Blt_DragDropInit(interp)
	Tcl_Interp *interp;  /* interpreter to be updated */
{
	DD_RegList *ddlist;
	Tk_Window tkwin;
	Tcl_CmdInfo cmdInfo;

	/*
	 *  See if drag&drop is already installed.
	 */
	if (Tcl_GetCommandInfo(interp, DRAGDROP_COMMAND, &cmdInfo))
	{
		Tcl_ResetResult(interp);
		Tcl_AppendResult(interp, "already installed: ", DRAGDROP_COMMAND,
						 (char*)NULL);
		return TCL_ERROR;
	}

	/*
	 *  Make sure that this is a Tk application.
	 */
	tkwin = Tk_MainWindow(interp);
	if (tkwin == None)
	{
		Tcl_ResetResult(interp);
		Tcl_AppendResult(interp, "requires Tk facilities: ", DRAGDROP_COMMAND,
						 (char*)NULL);
		return TCL_ERROR;
	}

	/*
	 *  Install drag&drop facilities.
	 */
    ddlist = (DD_RegList*)malloc(sizeof(DD_RegList));
    ddlist->interp = interp;
    ddlist->root = tkwin;
    Tcl_InitHashTable(&ddlist->srcList,TCL_STRING_KEYS);
    Tcl_InitHashTable(&ddlist->trgList,TCL_STRING_KEYS);
    ddlist->errorProc = strdup(DRAGDROP_ERRORPROC);
    ddlist->numactive = 0;
    ddlist->locx = ddlist->locy = 0;
    ddlist->pool = NULL;

	Tcl_CreateCommand(interp, DRAGDROP_COMMAND, DragDrop_Cmd,
		(ClientData)ddlist, DragDrop_Delete);

	Tcl_SetVar2(interp, "blt_versions", DRAGDROP_COMMAND, DRAGDROP_VERSION,
		TCL_GLOBAL_ONLY);

	return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  DragDrop_Delete()
 *
 *  Invoked when the drag&drop command is removed from an interpreter
 *  to free up allocated memory.
 * ------------------------------------------------------------------------
 */
static void
DragDrop_Delete(cdata)
	ClientData cdata;    /* client data for drag&drop command */
{
	DD_RegList *ddlist = (DD_RegList*)cdata;
	DD_WinRep *wrpool, *wrnext;

    Tcl_DeleteHashTable(&ddlist->srcList);
    Tcl_DeleteHashTable(&ddlist->trgList);
	if (ddlist->errorProc != NULL) {
		free((char*)ddlist->errorProc);
	}
	for (wrpool=ddlist->pool; wrpool; wrpool=wrnext)
	{
		wrnext = wrpool->next;
		free((char*)wrpool);
	}
	free((char*)ddlist);
}

/*
 * ------------------------------------------------------------------------
 *  DragDrop_Cmd()
 *
 *  Invoked by TCL whenever the user issues a drag&drop command.
 *  Handles the following syntax:
 *
 *    blt_drag&drop source
 *    blt_drag&drop source <pathName>
 *    blt_drag&drop source <pathName> config ?options...?
 *    blt_drag&drop source <pathName> handler <dataType> <command> ?...?
 *
 *    blt_drag&drop target
 *    blt_drag&drop target <pathName> handler
 *    blt_drag&drop target <pathName> handler <dataType> <command> ?...?
 *    blt_drag&drop target <pathName> handle <dataType>
 *
 *    blt_drag&drop drag <pathName> <x> <y>
 *    blt_drag&drop drop <pathName> <x> <y>
 *
 *    blt_drag&drop errors ?<proc>?
 *    blt_drag&drop active
 *    blt_drag&drop location ?<x> <y>?
 *
 * ------------------------------------------------------------------------
 */
static int
DragDrop_Cmd(clientData, interp, argc, argv)
	ClientData clientData;   /* main window associated with interp */
	Tcl_Interp *interp;      /* current interpreter */
	int argc;                /* number of arguments */
	char **argv;             /* argument strings */
{
	DD_RegList *ddlist = (DD_RegList*)clientData;
	DD_RegEntry *ddentry;
	register DD_Source *dsPtr;
	register DD_Target *dtPtr;

	int status, length, x, y, newEntry;
	char c;

	Tk_Window tokenwin;
	XSetWindowAttributes atts;
	char buffer[1024];

	if (argc < 2)
	{
		Tcl_AppendResult(interp, "wrong # args: should be \"",
			argv[0], " option ?args?\"",
			(char*)NULL);
		return TCL_ERROR;
	}
	c = argv[1][0];
	length = strlen(argv[1]);

	/*
	 *  HANDLE:  blt_drag&drop source
	 *           blt_drag&drop source <pathName>
	 *           blt_drag&drop source <pathName> config ?options...?
	 *           blt_drag&drop source <pathName> handler <data> <scmd> ?...?
	 */
	if ((c == 's') && strncmp(argv[1], "source", length) == 0)
	{
		/*
		 *  HANDLE:  blt_drag&drop source
		 */
		if (argc == 2)
		{
			Tcl_HashSearch cursor;
			Tcl_HashEntry *entryPtr;
			char *name;
			
			for (entryPtr = Tcl_FirstHashEntry(&ddlist->srcList, &cursor);
				 entryPtr != NULL; entryPtr = Tcl_NextHashEntry(&cursor)) 
			{
				name = Tcl_GetHashKey(&ddlist->srcList, entryPtr);
				Tcl_AppendElement(interp, name);
			}
			return TCL_OK;
		}

		/*
		 *  Find or create source info...
		 */
		dsPtr = GetSourceInfo(ddlist, argv[2], &newEntry);
		dsPtr->tkwin = Tk_NameToWindow(interp, argv[2], ddlist->root);
		if (!dsPtr->tkwin)
		{
			Tcl_AppendResult(interp, "window does not exist: ", argv[2],
				(char*)NULL);
			DestroySourceInfo(ddlist,argv[2]);
			return TCL_ERROR;
		}
		dsPtr->display = Tk_Display(dsPtr->tkwin);
		dsPtr->ddAtom  = XInternAtom(dsPtr->display, DRAGDROP_PROPINFO, False);

		if (newEntry)
			ConfigSource(interp, dsPtr, 0, (char**)NULL, 0);

		/*
		 *  HANDLE:  blt_drag&drop source <pathName> config ?options...?
		 */
		if (argc > 3)
		{
			c = argv[3][0];
			length = strlen(argv[3]);

			if ((c == 'c') && strncmp(argv[3], "config", length) == 0)
			{
				if (argc == 4)
					status = Tk_ConfigureInfo(interp, dsPtr->tokenwin,
						SourceConfigSpecs, (char*)dsPtr, (char*)NULL, 0);

				else if (argc == 5)
					status = Tk_ConfigureInfo(interp, dsPtr->tokenwin,
						SourceConfigSpecs, (char*)dsPtr, argv[4], 0);

				else
					status = ConfigSource(interp, dsPtr, argc-4, argv+4,
						TK_CONFIG_ARGV_ONLY);
			}

			/*
			 *  HANDLE:  blt_drag&drop source <pathName> handler \
			 *             ?<data> <scmd>...?
			 */
			else if ((c == 'h') && strncmp(argv[3], "handler", length) == 0)
			{
				if (argc == 4)
				{
					DD_SourceHndl *dsHndl = dsPtr->handlers;
					while (dsHndl)
					{
						Tcl_AppendElement(interp, dsHndl->dataType);
						dsHndl = dsHndl->next;
					}
					return TCL_OK;
				}

				/*
				 *  Process handler definitions
				 */
				status = TCL_OK;
				for (x=4; (x < argc) && (status == TCL_OK); x+=2)
				{
					if (x+1 < argc)
					{
						char *p;
						for (p=argv[x]; *p != '\0'; p++)
							if (*p == ' ')
							{
								Tcl_AppendResult(interp,
									"bad source handler name \"",
									argv[x], "\" (should not contain spaces)",
									(char*)NULL);
								return TCL_ERROR;
							}

						PutSourceHandler(dsPtr, argv[x], argv[x+1]);
					}
					else
					{
						Tcl_AppendResult(interp,
							"missing command for source handler: should be \"",
							argv[x], " command\"");
						return TCL_ERROR;
					}
				}
				return TCL_OK;
			}
			else
			{
				Tcl_AppendResult(interp, "bad option \"", argv[3],
					"\": must be config or handler",
					(char*)NULL);
				return TCL_ERROR;
			}
		}

		if (newEntry)
		{
			/*
			 *  Create the window for the drag&drop token...
			 */
			sprintf(buffer, "dd-token%x", (int)dsPtr);
			tokenwin = Tk_CreateWindow(dsPtr->ddlist->interp, dsPtr->tkwin,
				buffer, "");

			if (!tokenwin)
			{
				Tcl_AppendResult(interp, "could not create token window",
					(char*)NULL);
				DestroySourceInfo(ddlist,argv[2]);
				return TCL_ERROR;
			}
			Tk_SetClass(tokenwin, DRAGDROP_CLASS);
			Tk_CreateEventHandler(tokenwin, ExposureMask|StructureNotifyMask,
				DDTokenEventProc, (ClientData)dsPtr);

			atts.override_redirect = True;
			atts.save_under = True;
			Tk_ChangeWindowAttributes(tokenwin,
				CWOverrideRedirect|CWSaveUnder, &atts);

			Tk_SetInternalBorder(tokenwin, 2*dsPtr->tokenBorderWidth);
			dsPtr->tokenwin = tokenwin;

			if (dsPtr->button > 0)
			{
				sprintf(buffer,
					"bind %.100s <ButtonPress-%d> {%s drag %.100s %%X %%Y}; \
					bind %.100s <B%d-Motion> {%s drag %.100s %%X %%Y}; \
					bind %.100s <ButtonRelease-%d> {%s drop %.100s %%X %%Y}",
					argv[2], dsPtr->button, DRAGDROP_COMMAND, argv[2],
					argv[2], dsPtr->button, DRAGDROP_COMMAND, argv[2],
					argv[2], dsPtr->button, DRAGDROP_COMMAND, argv[2]);

				if (Tcl_Eval(interp, buffer) != TCL_OK)
				{
					Tk_DestroyWindow(tokenwin);
					DestroySourceInfo(ddlist,argv[2]);
					return TCL_ERROR;
				}
			}

			/*
			 *  Arrange for the window to unregister itself when it
			 *  is destroyed.
			 */
			ddentry = (DD_RegEntry*)malloc(sizeof(DD_RegEntry));
			ddentry->ddlist = ddlist;
			ddentry->tkwin = dsPtr->tkwin;
			Tk_CreateEventHandler(dsPtr->tkwin, StructureNotifyMask,
				UnregSource, (ClientData)ddentry);
		}
	}

	/*
	 *  HANDLE:  blt_drag&drop target ?<pathName>? ?handling info...?
	 */
	else if ((c == 't') && strncmp(argv[1], "target", length) == 0)
	{
		/*
		 *  HANDLE:  blt_drag&drop target
		 */
		if (argc == 2)
		{
			Tcl_HashSearch pos;
			Tcl_HashEntry *entry = Tcl_FirstHashEntry(&ddlist->trgList,&pos);
			while (entry)
			{
				Tcl_AppendElement(interp,
					Tcl_GetHashKey(&ddlist->trgList,entry));
				entry = Tcl_NextHashEntry(&pos);
			}
			return TCL_OK;
		}

		dtPtr = GetTargetInfo(ddlist,argv[2],&newEntry);
		dtPtr->tkwin = Tk_NameToWindow(interp, argv[2], ddlist->root);

		if (!dtPtr->tkwin)
		{
			Tcl_AppendResult(interp, "window does not exist: ", argv[2],
				(char*)NULL);
			DestroyTargetInfo(ddlist,argv[2]);
			return TCL_ERROR;
		}
		dtPtr->display = Tk_Display(dtPtr->tkwin);

		/*
		 *  If this is a new target, attach a property to identify
		 *  window as "drag&drop" target, and arrange for the window
		 *  to un-register itself when it is destroyed.
		 */
		if (newEntry)
		{
			Tk_MakeWindowExist(dtPtr->tkwin);
			AddDDProp(dtPtr);

			/*
			 *  Arrange for the window to unregister itself when it
			 *  is destroyed.
			 */
			ddentry = (DD_RegEntry*)malloc(sizeof(DD_RegEntry));
			ddentry->ddlist = ddlist;
			ddentry->tkwin = dtPtr->tkwin;
			Tk_CreateEventHandler(dtPtr->tkwin, StructureNotifyMask,
				UnregTarget, (ClientData)ddentry);
		}

		/*
		 *  HANDLE:  blt_drag&drop target <pathName> handler
		 *           blt_drag&drop target <pathName> handler <data> <cmd> ?...?
		 */
		if ((argc >= 4) && (strcmp(argv[3], "handler") == 0))
		{
			if (argc == 4)
			{
				DD_TargetHndl *dtHndl = dtPtr->handlers;
				while (dtHndl)
				{
					Tcl_AppendElement(interp, dtHndl->dataType);
					dtHndl = dtHndl->next;
				}
				return TCL_OK;
			}

			/*
			 *  Process handler definitions
			 */
			status = TCL_OK;
			for (x=4; (x < argc) && (status == TCL_OK); x+=2)
			{
				if (x+1 < argc)
					PutTargetHandler(dtPtr, argv[x], argv[x+1]);
				else
				{
					Tcl_AppendResult(interp,
						"missing command for target handler: should be \"",
						argv[x], " command\"",
						(char*)NULL);
					return TCL_ERROR;
				}
			}
			return TCL_OK;
		}

		/*
		 *  HANDLE:  blt_drag&drop target <pathName> handle <data>
		 */
		else if ((argc == 5) && (strcmp(argv[3], "handle") == 0))
		{
			char *cmd;
			if ((cmd=FindTargetHandler(dtPtr, argv[4])) != NULL)
				return Tcl_Eval(interp, cmd);

			Tcl_AppendResult(interp, "target cannot handle datatype: ",
				argv[4], (char*)NULL);
			return TCL_ERROR;  /* no handler found */
		}
		else
		{
			Tcl_AppendResult(interp,"usage: ", argv[0], " target ", argv[2],
				" handler ?defns?\n   or: ", argv[0], " target ", argv[2],
				" handle <data>",
				(char*)NULL);
			return TCL_ERROR;
		}
	}

	/*
	 *  HANDLE:  blt_drag&drop drag <path> <x> <y>
	 */
	else if ((c == 'd') && strncmp(argv[1], "drag", length) == 0)
	{
		if (argc < 5)
		{
			Tcl_AppendResult(interp, "wrong # args: should be \"",
				argv[0], " drag pathname x y\"",
				(char*)NULL);
			return TCL_ERROR;
		}

		dsPtr = GetSourceInfo(ddlist,argv[2],&newEntry);
		if (newEntry)
		{
			Tcl_AppendResult(interp, "not a drag&drop source: ", argv[2],
				(char*)NULL);
			DestroySourceInfo(ddlist,argv[2]);
			return TCL_ERROR;
		}
		if ((Tcl_GetInt(interp, argv[3], &x) != TCL_OK) ||
			(Tcl_GetInt(interp, argv[4], &y) != TCL_OK))
			return TCL_ERROR;

		ddlist->locx = x;   /* save drag&drop location */
		ddlist->locy = y;
		dsPtr->tokenx = x;
		dsPtr->tokeny = y;

		/*
		 *  If HideDDToken() is pending, then do it now!
		 */
		if (dsPtr->hidetoken)
		{
			Tk_DeleteTimerHandler(dsPtr->hidetoken);
			HideDDToken((ClientData)dsPtr);
		}

		/*
		 *  If pkgcmd is in progress, then ignore subsequent calls
		 *  until it completes.  Only perform drag if pkgcmd
		 *  completed successfully and token window is mapped.
		 */
		if (!Tk_IsMapped(dsPtr->tokenwin) && !dsPtr->pkgcmdInProg)
		{
			/*
			 *  No list of send handlers?  Then source is disabled.
			 *  Abort drag quietly.
			 */
			if (dsPtr->send == NULL)
				return TCL_OK;

			/*
			 *  No token command?  Then cannot build token.
			 *  Signal error.
			 */
			if (!dsPtr->pkgcmd)
			{
				Tcl_AppendResult(interp, "missing -packagecmd: ", argv[2],
					(char*)NULL);
				return TCL_ERROR;
			}

			/*
			 *  Execute token command to initialize token window.
			 */
			dsPtr->pkgcmdInProg = ~0;
			status = Tcl_VarEval(dsPtr->ddlist->interp,
				dsPtr->pkgcmd, " ", Tk_PathName(dsPtr->tokenwin),
				(char*)NULL);
			dsPtr->pkgcmdInProg = 0;

			/*
			 *  Null string from the package command?
			 *  Then quietly abort the drag&drop operation.
			 */
			if (*interp->result == '\0')
				return TCL_OK;

			/*
			 *  Save result of token command for send command.
			 */
			if (dsPtr->pkgcmdResult)
				free(dsPtr->pkgcmdResult);
			dsPtr->pkgcmdResult = strdup(interp->result);

			/*
			 *  Token building failed?  If an error handler is defined,
			 *  then signal the error.  Otherwise, abort quietly.
			 */
			if (status != TCL_OK)
			{
				if (ddlist->errorProc && *ddlist->errorProc)
				{
					return Tcl_VarEval(ddlist->interp,
						ddlist->errorProc, " {", ddlist->interp->result, "}",
						(char*)NULL);
				}
				else
					return TCL_OK;
			}

			/*
			 *  Install token cursor...
			 */
			if (dsPtr->tokenCursor != None)
			{
				status = Tcl_VarEval(dsPtr->ddlist->interp,
					Tk_PathName(dsPtr->tkwin), " config -cursor",
					(char*)NULL);

				if (status == TCL_OK)
				{
					char *cname = interp->result;
					while (*cname != '\0')
						cname++;

					while ((cname > interp->result) && (*(cname-1) != ' '))
						cname--;

					if (dsPtr->normalCursor != None)
					{
						Tk_FreeCursor(dsPtr->display, dsPtr->normalCursor);
						dsPtr->normalCursor = None;
					}

					if (strcmp(cname,"{}") != 0)
						dsPtr->normalCursor = Tk_GetCursor(interp,
							dsPtr->tkwin, Tk_GetUid(cname));
				}
				Tk_DefineCursor(dsPtr->tkwin, dsPtr->tokenCursor);
			}

			/*
			 *  Get ready to drag token window...
			 *  1) Cache info for all windows on root
			 *  2) Map token window to begin drag operation
			 */
			if (dsPtr->allwins)
				WinRepRelease(dsPtr->allwins, ddlist);
			dsPtr->allwins = GetWinRepInfo(dsPtr, ddlist);

			ddlist->numactive++;   /* one more drag&drop window active */
			Tk_MapWindow(dsPtr->tokenwin);
			XRaiseWindow(Tk_Display(dsPtr->tokenwin),
				Tk_WindowId(dsPtr->tokenwin));
		}

		/*
		 *  Arrange to update status of token window...
		 */
		Tk_CancelIdleCall(UpdateDDToken, (ClientData)dsPtr);
		Tk_DoWhenIdle(UpdateDDToken, (ClientData)dsPtr);

		/*
		 *  Move the token window to the current drag point...
		 */
		MoveDDToken(dsPtr);
	}

	/*
	 *  HANDLE:  blt_drag&drop drop <path> <x> <y>
	 */
	else if ((c == 'd') && strncmp(argv[1], "drop", length) == 0)
	{
		if (argc < 5)
		{
			Tcl_AppendResult(interp, "wrong # args: should be \"",
				argv[0], " drop pathname x y\"",
				(char*)NULL);
			return TCL_ERROR;
		}

		dsPtr = GetSourceInfo(ddlist,argv[2],&newEntry);
		if (newEntry)
		{
			Tcl_AppendResult(interp, "not a drag&drop source: ", argv[2],
				(char*)NULL);
			DestroySourceInfo(ddlist,argv[2]);
			return TCL_ERROR;
		}
		if ((Tcl_GetInt(interp, argv[3], &x) != TCL_OK) ||
			(Tcl_GetInt(interp, argv[4], &y) != TCL_OK))
			return TCL_ERROR;

		ddlist->locx = x;  /* save drag&drop location */
		ddlist->locy = y;
		dsPtr->tokenx = x;
		dsPtr->tokeny = y;

		/*
		 *  Put the cursor back to its usual state.
		 */
		if (dsPtr->normalCursor == None)
			Tk_UndefineCursor(dsPtr->tkwin);
		else
			Tk_DefineCursor(dsPtr->tkwin, dsPtr->normalCursor);

		Tk_CancelIdleCall(UpdateDDToken, (ClientData)dsPtr);

		/*
		 *  Make sure that token window was not dropped before it
		 *  was either mapped or packed with info.
		 */
		if (Tk_IsMapped(dsPtr->tokenwin) && !dsPtr->pkgcmdInProg)
		{
			UpdateDDToken((ClientData)dsPtr);

			if (dsPtr->send)
			{
				if (dsPtr->overTargetWin)
					DragDropSend(dsPtr);
				else
					HideDDToken((ClientData)dsPtr);
			}
			ddlist->numactive--;  /* one fewer active token window */
		}
	}

	/*
	 *  HANDLE:  blt_drag&drop errors ?<proc>?
	 */
	else if ((c == 'e') && strncmp(argv[1], "errors", length) == 0)
	{
		if (argc == 3)
		{
			if (ddlist->errorProc) {
				free(ddlist->errorProc);
			}
    		ddlist->errorProc = strdup(argv[2]);
		}
		else if (argc != 2)
		{
			Tcl_AppendResult(interp, "wrong # args: should be \"",
				argv[0], " errors ?proc?\"",
				(char*)NULL);
			return TCL_ERROR;
		}
		Tcl_SetResult(interp, ddlist->errorProc, TCL_VOLATILE);
		return TCL_OK;
	}

	/*
	 *  HANDLE:  blt_drag&drop active
	 */
	else if ((c == 'a') && strncmp(argv[1], "active", length) == 0)
	{
		if (argc != 2)
		{
			Tcl_AppendResult(interp, "wrong # args: should be \"",
				argv[0], " active\"",
				(char*)NULL);
			return TCL_ERROR;
		}
		Tcl_SetResult(interp, (ddlist->numactive > 0) ? "1" : "0", TCL_STATIC);
		return TCL_OK;
	}

	/*
	 *  HANDLE:  blt_drag&drop location ?<x> <y>?
	 */
	else if ((c == 'l') && strncmp(argv[1], "location", length) == 0)
	{
		if (argc == 2)
		{
			sprintf(buffer, "%d %d", ddlist->locx, ddlist->locy);
			Tcl_SetResult(interp, buffer, TCL_VOLATILE);
			return TCL_OK;
		}
		else if ((argc == 4) &&
			(Tcl_GetInt(interp, argv[2], &x) == TCL_OK) &&
			(Tcl_GetInt(interp, argv[3], &y) == TCL_OK))
		{
			ddlist->locx = x;
			ddlist->locy = y;
			sprintf(buffer, "%d %d", ddlist->locx, ddlist->locy);
			Tcl_SetResult(interp, buffer, TCL_VOLATILE);
			return TCL_OK;
		}
		else
		{
			Tcl_AppendResult(interp, "wrong # args: should be \"",
				argv[0], " location ?x y?\"",
				(char*)NULL);
			return TCL_ERROR;
		}
	}

	/*
	 *  Report improper command arguments
	 */
	else
	{
		Tcl_AppendResult(interp, "bad option \"", argv[1], "\": must be source, target, drag, drop, errors, active or location",
			(char*)NULL);
		return TCL_ERROR;
	}
	return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  GetSourceInfo()
 *
 *  Looks for a DD_Source record in the hash table for drag&drop source
 *  widgets.  Creates a new record if the widget name is not already
 *  registered.  Returns a pointer to the desired record.
 * ------------------------------------------------------------------------
 */
static DD_Source*
GetSourceInfo(ddlist,pathname,newEntry)
	DD_RegList* ddlist;  /* drag&drop records for all registered widgets */
	char* pathname;      /* widget pathname for desired record */
	int* newEntry;       /* returns non-zero => new record created */
{
	DD_Source *dsPtr;
	Tcl_HashEntry *ddEntry;

	ddEntry = Tcl_CreateHashEntry(&ddlist->srcList, pathname, newEntry);
	if (*newEntry)
	{
		/*
		 *  Initialize a data structure for the widget...
		 */
		dsPtr = (DD_Source*)malloc(sizeof(DD_Source));
		dsPtr->ddlist = ddlist;
		dsPtr->display = NULL;
		dsPtr->tkwin = NULL;
		dsPtr->ddAtom = None;
		dsPtr->button = 0;

		dsPtr->tokenwin = NULL;
		dsPtr->tokenAnchor = TK_ANCHOR_CENTER;
		dsPtr->tokenCursor = None;
		dsPtr->tokenOutline = NULL;
		dsPtr->tokenBorder = NULL;
		dsPtr->tokenBorderWidth = 0;
		dsPtr->rejectFg = NULL;
		dsPtr->rejectBg = NULL;
		dsPtr->rejectSt = None;
		dsPtr->rejectFgGC = None;
		dsPtr->rejectBgGC = None;

		dsPtr->pkgcmdInProg = 0;
		dsPtr->pkgcmd = NULL;
		dsPtr->pkgcmdResult = NULL;
		dsPtr->sitecmd = NULL;

		dsPtr->allwins = NULL;
		dsPtr->selfTarget = 0;
		dsPtr->overTargetWin = 0;
		dsPtr->tokenx = dsPtr->tokeny = 0;
		dsPtr->hidetoken = NULL;
		dsPtr->normalCursor = None;

		dsPtr->send = NULL;
		dsPtr->handlers = NULL;

		Tcl_SetHashValue(ddEntry, (ClientData)dsPtr);
	}
	return (DD_Source*)Tcl_GetHashValue(ddEntry);
}

/*
 * ------------------------------------------------------------------------
 *  DestroySourceInfo()
 *
 *  Looks for a DD_Source record in the hash table for drag&drop source
 *  widgets.  Destroys the record if found.
 * ------------------------------------------------------------------------
 */
static void
DestroySourceInfo(ddlist,pathname)
	DD_RegList* ddlist;	 /* drag&drop records for all registered widgets */
	char* pathname;		 /* widget pathname for desired record */
{
	DD_Source *dsPtr;
	DD_SourceHndl *dsHndl, *next;
	Tcl_HashEntry *ddEntry;

	ddEntry = Tcl_FindHashEntry(&ddlist->srcList, pathname);
	if (ddEntry == NULL) {
		return;
	}
	dsPtr = (DD_Source*)Tcl_GetHashValue(ddEntry);
	if (dsPtr)
	{
		Tk_CancelIdleCall(UpdateDDToken, (ClientData)dsPtr);
		if (dsPtr->hidetoken)		   
			Tk_DeleteTimerHandler(dsPtr->hidetoken);

		Tk_FreeOptions(SourceConfigSpecs, (char *)dsPtr, dsPtr->display, 0);

		if (dsPtr->rejectFgGC != None) 
			Tk_FreeGC(dsPtr->display, dsPtr->rejectFgGC);
		if (dsPtr->rejectBgGC != None) 
			Tk_FreeGC(dsPtr->display, dsPtr->rejectBgGC);
		if (dsPtr->pkgcmdResult)	   
			free(dsPtr->pkgcmdResult);

		if (dsPtr->allwins)			   
			WinRepRelease(dsPtr->allwins, ddlist);

		if (dsPtr->normalCursor != None)
			Tk_FreeCursor(dsPtr->display, dsPtr->normalCursor);

		dsHndl = dsPtr->handlers;
		while (dsHndl)
		{
			next = dsHndl->next;
			DestroySourceHandler(dsHndl);
			dsHndl = next;
		}
		free((char*)dsPtr);
	}
	Tcl_DeleteHashEntry(ddEntry);
}

/*
 * ------------------------------------------------------------------------
 *  ConfigSource()
 *
 *  Called to process an (argc,argv) list to configure (or reconfigure)
 *  a drag&drop source widget.
 * ------------------------------------------------------------------------
 */
static int
ConfigSource(interp, dsPtr, argc, argv, flags)
	Tcl_Interp *interp;        /* current interpreter */
	register DD_Source *dsPtr; /* drag&drop source widget record */
	int argc;                  /* number of arguments */
	char **argv;               /* argument strings */
	int flags;                 /* flags controlling interpretation */
{
	unsigned long gcMask;
	XGCValues gcValues;
	GC newGC;

	/*
	 *  Handle the bulk of the options...
	 */
	if (Tk_ConfigureWidget(interp, dsPtr->tkwin, SourceConfigSpecs,
		argc, argv, (char*)dsPtr, flags) != TCL_OK)
		return TCL_ERROR;

	/*
	 *  Check the button binding for valid range (0 or 1-5)
	 */
	if (dsPtr->button < 0 || dsPtr->button > 5)
	{
		Tcl_SetResult(interp,
			"invalid button binding: should be 1-5 or 0 for no bindings",
			TCL_STATIC);
		return TCL_ERROR;
	}
	/*
	 *  Set up the rejection foreground GC for the token window...
	 */
	gcValues.foreground = dsPtr->rejectFg->pixel;
	gcValues.subwindow_mode = IncludeInferiors;
	gcValues.graphics_exposures = False;
	gcMask = GCForeground|GCSubwindowMode|GCGraphicsExposures;

	if (dsPtr->rejectSt != None)
	{
		gcValues.stipple = dsPtr->rejectSt;
		gcValues.fill_style = FillStippled;
		gcMask |= GCForeground|GCStipple|GCFillStyle;
	}
	newGC = Tk_GetGC(dsPtr->tkwin, gcMask, &gcValues);

	if (dsPtr->rejectFgGC != None)
		Tk_FreeGC(dsPtr->display, dsPtr->rejectFgGC);
	dsPtr->rejectFgGC = newGC;

	/*
	 *  Set up the rejection background GC for the token window...
	 */
	gcValues.foreground = dsPtr->rejectBg->pixel;
	gcValues.subwindow_mode = IncludeInferiors;
	gcValues.graphics_exposures = False;
	gcMask = GCForeground|GCSubwindowMode|GCGraphicsExposures;

	newGC = Tk_GetGC(dsPtr->tkwin, gcMask, &gcValues);

	if (dsPtr->rejectBgGC != None)
		Tk_FreeGC(dsPtr->display, dsPtr->rejectBgGC);
	dsPtr->rejectBgGC = newGC;

	/*
	 *  Reset the border width in case it has changed...
	 */
	if (dsPtr->tokenwin)
		Tk_SetInternalBorder(dsPtr->tokenwin, 2*dsPtr->tokenBorderWidth);

	return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  FindSourceHandler()
 *
 *  Looks for the requested data type of the list of handlers for the
 *  given drag&drop source.
 * ------------------------------------------------------------------------
 */
static char*
FindSourceHandler(dsPtr, dtname)
	register DD_Source *dsPtr; /* drag&drop source widget record */
	char *dtname;              /* name of requested data type */
{
	DD_SourceHndl *dsHndl;

	for (dsHndl=dsPtr->handlers; dsHndl; dsHndl=dsHndl->next)
		if (strcmp(dsHndl->dataType,dtname) == 0)
			return dsHndl->cmd;

	return NULL;
}

/*
 * ------------------------------------------------------------------------
 *  PutSourceHandler()
 *
 *  Looks for the requested data type of the list of handlers for the
 *  given drag&drop source.  If found, then its associated commands are
 *  changed to the given commands.  If not found, then a new handler
 *  is created.
 * ------------------------------------------------------------------------
 */
static void
PutSourceHandler(dsPtr, dtname, cmd)
	register DD_Source *dsPtr; /* drag&drop target widget record */
	char *dtname;              /* name of data type */
	char *cmd;                 /* command used to send data */
{
	DD_SourceHndl *tail = NULL;
	DD_SourceHndl *dsHndl;

	for (dsHndl=dsPtr->handlers; dsHndl; tail=dsHndl,dsHndl=dsHndl->next)
		if (strcmp(dsHndl->dataType,dtname) == 0)
		{
			if (*cmd == '\0')
			{
				if (tail)
					tail->next = dsHndl->next;
				else
					dsPtr->handlers = dsHndl->next;

				DestroySourceHandler(dsHndl);
				return;
			}
			else
			{
				if (dsHndl->cmd != NULL) {
					free(dsHndl->cmd);
				}
				dsHndl->cmd = strdup(cmd);
				return;
			}
		}

	if (tail)
		tail->next = CreateSourceHandler(dtname,cmd);
	else
		dsPtr->handlers = CreateSourceHandler(dtname,cmd);
}

/*
 * ------------------------------------------------------------------------
 *  CreateSourceHandler()
 *
 *  Creates a new source handler record and returns a pointer to it.
 * ------------------------------------------------------------------------
 */
static DD_SourceHndl*
CreateSourceHandler(dtname, cmd)
	char *dtname;              /* name of data type */
	char *cmd;                 /* command used to send data */
{
	DD_SourceHndl *retn;
	retn = (DD_SourceHndl*)malloc(sizeof(DD_SourceHndl));

	retn->dataType = strdup(dtname);
	retn->cmd = strdup(cmd);
	retn->next = NULL;
	return retn;
}

/*
 * ------------------------------------------------------------------------
 *  DestroySourceHandler()
 *
 *  Destroys a source handler record.
 * ------------------------------------------------------------------------
 */
static void
DestroySourceHandler(dsHndl)
	DD_SourceHndl *dsHndl;
{
	if (dsHndl->dataType != NULL) {
		free(dsHndl->dataType);
	}
	if (dsHndl->cmd != NULL) {
		free(dsHndl->cmd);
	}
	free((char*)dsHndl);
}

/*
 * ------------------------------------------------------------------------
 *  UnregSource()
 *
 *  Invoked by Tk_HandleEvent whenever a DestroyNotify event is received
 *  on a registered drag&drop source widget.
 * ------------------------------------------------------------------------
 */
static void
UnregSource(cdata, eventPtr)
	ClientData cdata;   /* drag&drop registration list */
	XEvent *eventPtr;   /* event description */
{
	DD_RegEntry *ddentry = (DD_RegEntry*)cdata;
	DD_RegList *ddlist = ddentry->ddlist;
	char *ddname = Tk_PathName(ddentry->tkwin);

	if (eventPtr->type == DestroyNotify)
	{
		DestroySourceInfo(ddlist,ddname);
		free((char*)ddentry);
	}
}

/*
 * ------------------------------------------------------------------------
 *  GetTargetInfo()
 *
 *  Looks for a DD_Target record in the hash table for drag&drop target
 *  widgets.  Creates a new record if the widget name is not already
 *  registered.  Returns a pointer to the desired record.
 * ------------------------------------------------------------------------
 */
static DD_Target*
GetTargetInfo(ddlist,pathname,newEntry)
	DD_RegList* ddlist;  /* drag&drop records for all registered widgets */
	char* pathname;      /* widget pathname for desired record */
	int* newEntry;       /* returns non-zero => new record created */
{
	DD_Target *dtPtr;
	Tcl_HashEntry *ddEntry;

	ddEntry = Tcl_CreateHashEntry(&ddlist->trgList, pathname, newEntry);
	if (*newEntry)
	{
		/*
		 *  Initialize a data structure for the widget...
		 */
		dtPtr = (DD_Target*)malloc(sizeof(DD_Target));
		dtPtr->ddlist = ddlist;
		dtPtr->display = NULL;
		dtPtr->tkwin = NULL;
		dtPtr->handlers = NULL;

		Tcl_SetHashValue(ddEntry, (ClientData)dtPtr);
	}
	return (DD_Target*)Tcl_GetHashValue(ddEntry);
}

/*
 * ------------------------------------------------------------------------
 *  DestroyTargetInfo()
 *
 *  Looks for a DD_Target record in the hash table for drag&drop target
 *  widgets.  Destroys the record if found.
 * ------------------------------------------------------------------------
 */
static void
DestroyTargetInfo(ddlist,pathname)
	DD_RegList* ddlist;  /* drag&drop records for all registered widgets */
	char* pathname;      /* widget pathname for desired record */
{
	DD_Target *dtPtr;
	DD_TargetHndl *dtHndl, *next;
	Tcl_HashEntry *ddEntry;

	ddEntry = Tcl_FindHashEntry(&ddlist->trgList, pathname);
	dtPtr = (ddEntry) ? (DD_Target*)Tcl_GetHashValue(ddEntry) : NULL;

	if (dtPtr)
	{
		dtHndl = dtPtr->handlers;
		while (dtHndl)
		{
			next = dtHndl->next;
			DestroyTargetHandler(dtHndl);
			dtHndl = next;
		}
		free((char*)dtPtr);
	}
	if (ddEntry) Tcl_DeleteHashEntry(ddEntry);
}

/*
 * ------------------------------------------------------------------------
 *  FindTargetHandler()
 *
 *  Looks for the requested data type of the list of handlers for the
 *  given drag&drop target.
 * ------------------------------------------------------------------------
 */
static char*
FindTargetHandler(dtPtr, dtname)
	register DD_Target *dtPtr; /* drag&drop target widget record */
	char *dtname;              /* name of requested data type */
{
	DD_TargetHndl *dtHndl;

	for (dtHndl=dtPtr->handlers; dtHndl; dtHndl=dtHndl->next)
		if (strcmp(dtHndl->dataType,dtname) == 0)
			return dtHndl->command;

	return NULL;
}

/*
 * ------------------------------------------------------------------------
 *  PutTargetHandler()
 *
 *  Looks for the requested data type of the list of handlers for the
 *  given drag&drop target.  If found, then its associated command is
 *  changed to the given command.  If not found, then a new handler
 *  is created.
 * ------------------------------------------------------------------------
 */
static void
PutTargetHandler(dtPtr, dtname, cmd)
	register DD_Target *dtPtr; /* drag&drop target widget record */
	char *dtname;              /* name of data type */
	char *cmd;                 /* command string for data type */
{
	DD_TargetHndl *tail = NULL;
	DD_TargetHndl *dtHndl;

	for (dtHndl=dtPtr->handlers; dtHndl; tail=dtHndl,dtHndl=dtHndl->next)
		if (strcmp(dtHndl->dataType,dtname) == 0)
		{
			if (*cmd == '\0')
			{
				if (tail)
					tail->next = dtHndl->next;
				else
					dtPtr->handlers = dtHndl->next;

				DestroyTargetHandler(dtHndl);
				return;
			}
			else
			{
				if (dtHndl->command != NULL) {
					free(dtHndl->command);
				}
				dtHndl->command = strdup(cmd);
				return;
			}
		}

	if (tail)
		tail->next = CreateTargetHandler(dtname,cmd);
	else
		dtPtr->handlers = CreateTargetHandler(dtname,cmd);

	/*
	 *  Update handler list stored in target window property.
	 */
	AddDDProp(dtPtr);
}

/*
 * ------------------------------------------------------------------------
 *  CreateTargetHandler()
 *
 *  Creates a new target handler record and returns a pointer to it.
 * ------------------------------------------------------------------------
 */
static DD_TargetHndl*
CreateTargetHandler(dtname, cmd)
	char *dtname;              /* name of data type */
	char *cmd;                 /* command string for data type */
{
	DD_TargetHndl *retn;
	retn = (DD_TargetHndl*)malloc(sizeof(DD_TargetHndl));

	retn->dataType = strdup(dtname);
	retn->command = strdup(cmd);

	retn->next = NULL;
	return retn;
}

/*
 * ------------------------------------------------------------------------
 *  DestroyTargetHandler()
 *
 *  Destroys a target handler record.
 * ------------------------------------------------------------------------
 */
static void
DestroyTargetHandler(dtHndl)
	DD_TargetHndl *dtHndl;
{
	if (dtHndl->dataType != NULL) {
		free(dtHndl->dataType);
	}
	if (dtHndl->command != NULL) {
		free(dtHndl->command);
	}
	free((char*)dtHndl);
}

/*
 * ------------------------------------------------------------------------
 *  UnregTarget()
 *
 *  Invoked by Tk_HandleEvent whenever a DestroyNotify event is received
 *  on a registered drag&drop target widget.
 * ------------------------------------------------------------------------
 */
static void
UnregTarget(cdata, eventPtr)
	ClientData cdata;   /* drag&drop registration list */
	XEvent *eventPtr;   /* event description */
{
	DD_RegEntry *ddentry = (DD_RegEntry*)cdata;
	DD_RegList *ddlist = ddentry->ddlist;
	char *ddname = Tk_PathName(ddentry->tkwin);

	if (eventPtr->type == DestroyNotify)
	{
		DestroyTargetInfo(ddlist,ddname);
		free((char*)ddentry);
	}
}

/*
 * ------------------------------------------------------------------------
 *  DragDropSend()
 *
 *  Invoked after a drop operation to send data to the drop application.
 * ------------------------------------------------------------------------
 */
static void
DragDropSend(dsPtr)
	register DD_Source *dsPtr;  /* drag&drop source record */
{
	DD_RegList *ddlist = dsPtr->ddlist;

	int status;
	char *sendcmd;
	DD_WinRep *target;

	/*
	 *  See if current position is over drop point...
	 */
	target = FindTargetWin(dsPtr, dsPtr->tokenx,dsPtr->tokeny);

	if (target)
	{
		char buffer[256];

		sprintf(buffer, "%d %d", dsPtr->tokenx, dsPtr->tokeny);
		status = Tcl_VarEval(ddlist->interp,
			"send {",target->ddinterp,"} ", DRAGDROP_COMMAND," location ",buffer,
			(char*)NULL);

		if (status == TCL_OK)
		{
			sendcmd = DragDropSendHndlr(dsPtr, target->ddinterp, target->ddwin);
			if (sendcmd)
			{
				status = Tcl_VarEval(ddlist->interp,
					sendcmd, " {",target->ddinterp,"} {",target->ddwin,
					"} {",dsPtr->pkgcmdResult,"}",
					(char*)NULL);
			}
			else
			{
				Tcl_AppendResult(ddlist->interp, "target \"",
					target->ddwin,
					"\" does not recognize handlers for source \"",
					Tk_PathName(dsPtr->tkwin), "\"",
					(char*)NULL);
				status = TCL_ERROR;
			}
		}

		/*
		 *  Give success/failure feedback to user.
		 *  If an error occurred and an error proc is defined,
		 *  then use it to handle the error.
		 */
		if (status == TCL_OK)
			HideDDToken((ClientData)dsPtr);
		else
		{
			RejectDDToken(dsPtr);

			if (ddlist->errorProc && *ddlist->errorProc)
				(void) Tcl_VarEval(ddlist->interp,
					ddlist->errorProc, " {", ddlist->interp->result, "}",
					(char*)NULL);
		}
	}
}

/*
 * ------------------------------------------------------------------------
 *  DragDropSendHndlr()
 *
 *  Queries the drag&drop target under the specified interpreter for a
 *  handler that is compatible with one of the handlers defined for the
 *  source.  Returns a pointer to the appropriate send command, or
 *  NULL if none is found.
 * ------------------------------------------------------------------------
 */
static char*
DragDropSendHndlr(dsPtr,interpName,ddName)
	register DD_Source *dsPtr;  /* drag&drop source record */
	char *interpName;           /* interpreter containing drag&drop target */
	char *ddName;               /* drag&drop target pathname */
{
	char *retn = NULL;  /* no handler found yet */
	Tcl_Interp *interp = dsPtr->ddlist->interp;

	int hndlc, hi, ei;
	char **hndlv, *hlist;
	DD_SourceHndl *dsHndl;
	char buffer[1024];

	/*
	 *  Query the drag&drop target for its list of known handlers.
	 */
	Tcl_ResetResult(interp); /* for Tcl_AppendResult() below */
	if (Tcl_VarEval(interp,
		"send {",interpName,"} ", DRAGDROP_COMMAND," target {",ddName,"} handler",
		(char*)NULL) != TCL_OK)
		return NULL;

	hlist = strdup(interp->result);
	if (Tcl_SplitList(interp, hlist, &hndlc, &hndlv) == TCL_OK)
	{
		/*
		 *  If the list of send handlers is specified as "all", then
		 *  search through the handlers in order.
		 */
		if (strcmp(dsPtr->send,"all") == 0)
			for (dsHndl=dsPtr->handlers; dsHndl && !retn; dsHndl=dsHndl->next)
			{
				for (hi=0; (hi < hndlc) && !retn; hi++)
					if (strcmp(dsHndl->dataType, hndlv[hi]) == 0)
						retn = dsHndl->cmd;
			}

		/*
		 *  Otherwise, search through the specified send handlers.
		 */
		else
		{
			int elemc;
			char **elemv;
			if (Tcl_SplitList(interp,dsPtr->send,&elemc,&elemv)==TCL_OK)
			{
				for (ei=0; (ei < elemc) && !retn; ei++)
					for (hi=0; (hi < hndlc) && !retn; hi++)
						if (strcmp(elemv[ei], hndlv[hi]) == 0)
						{
							retn = FindSourceHandler(dsPtr, elemv[ei]);
							if (!retn)
							{
								sprintf(buffer, "unknown handler \"%.50s\" requested for drag&drop source \"%.200s\"", elemv[ei], Tk_PathName(dsPtr->tkwin));
								Tcl_ResetResult(interp);
								Tcl_AddErrorInfo(interp, buffer);
								Tk_BackgroundError(interp);
							}
						}

				free((char*)elemv);
			}
			else
			{
				sprintf(buffer, "drag&drop source has invalid -send: %.200s",
					dsPtr->send);
				Tcl_ResetResult(interp);
				Tcl_AddErrorInfo(interp, buffer);
				Tk_BackgroundError(interp);
			}
		}
		free((char*)hndlv);
	}
	free(hlist);

	return retn;
}


/*
 * ------------------------------------------------------------------------
 *  GetWinRepInfo()
 *
 *  Invoked at the start of a "drag" operation to capture the positions
 *  of all windows on the current root.  Queries the entire window
 *  hierarchy and determines the placement of each window.  Queries
 *  "DragDropInfo" property info where appropriate.  This information
 *  is used during the drag operation to determine when the drag&drop
 *  token is over a valid drag&drop target.
 *
 *  Returns the record for the root window, which contains records for
 *  all other windows as children.
 * ------------------------------------------------------------------------
 */
static DD_WinRep*
GetWinRepInfo(dsPtr,ddlist)
	DD_Source *dsPtr;     /* drag&drop source window */
	DD_RegList *ddlist;   /* drag&drop registration info */
{
	DD_WinRep *wr;

	wr = WinRepAlloc(ddlist);
	wr->win = DefaultRootWindow(dsPtr->display);
	WinRepInit(wr, dsPtr);

	return wr;
}

/*
 * ------------------------------------------------------------------------
 *  FindTargetWin()
 *
 *  Checks to see if a compatible drag&drop target exists at the given
 *  position.  A target is "compatible" if it is a drag&drop window,
 *  and if it has a handler that is compatible with the current source
 *  window.
 *
 *  Returns a pointer to a structure describing the target, or NULL
 *  if no compatible target is found.
 * ------------------------------------------------------------------------
 */
static DD_WinRep*
FindTargetWin(dsPtr,x,y)
	DD_Source *dsPtr;     /* drag&drop source window */
	int x,y;              /* current drag&drop location (virtual coords) */
{
	int vx, vy;
	unsigned int vw, vh;
	register char *type;

	DD_WinRep *wr, *wrkid;
	DD_Stack stack;
	DD_SourceHndl *shandl;

	/*
	 *  If window representations have not yet been built, then
	 *  abort this call.  This probably means that the token is being
	 *  moved before it has been properly built.
	 */
	if (!dsPtr->allwins)
		return NULL;

	/*
	 *  Adjust current location for virtual root windows.
	 */
	Tk_GetVRootGeometry(dsPtr->tkwin, &vx, &vy, &vw, &vh);
	x += vx;
	y += vy;

	/*
	 *  Build a stack of all windows containing the given point,
	 *  in order from least to most specific.
	 */
	StackInit(&stack);

	wr = dsPtr->allwins;
	if ((x >= wr->x0) && (x <= wr->x1) &&
        (y >= wr->y0) && (y <= wr->y1))
		StackPush((ClientData)wr, &stack);

	while (wr)
	{
		for (wrkid=wr->kids; wrkid; wrkid=wrkid->next)
		{
			if (!wrkid->initialized)
				WinRepInit(wrkid, dsPtr);

			if ((x >= wrkid->x0) && (x <= wrkid->x1) &&
		        (y >= wrkid->y0) && (y <= wrkid->y1))
			{
				StackPush((ClientData)wrkid, &stack);
				break;
			}
		}
		wr = wrkid;  /* continue search */
	}

	/*
	 *  Pop windows from the stack until one containing a
	 *  "DragDropInfo" property is found.  See if the handlers
	 *  listed in this property are compatible with the
	 *  given source.
	 */
	while ((wr=(DD_WinRep*)StackPop(&stack)) != NULL)
		if (wr->ddprop)
			break;

	if (wr && wr->ddhandlers)
	{
		type = wr->ddhandlers;
		while (*type != '\0')
		{
			for (shandl=dsPtr->handlers; shandl; shandl=shandl->next)
				if (strcmp(shandl->dataType, type) == 0)
					break;

			if (shandl)   /* found a match? */
				break;    /* then stop searching */
			else          /* otherwise, move to next handler type */
			{
				while (*type++ != '\0')
					;
			}
		}
		if (*type == '\0')  /* no handler match? */
			wr = NULL;      /* then return NULL */
	}
	StackDelete(&stack);

	return wr;
}


/*
 * ------------------------------------------------------------------------
 *  WinRepAlloc()
 *
 *  Returns a new structure for representing X window position info.
 *  Such structures are typically allocated at the start of a drag&drop
 *  operation to capture the placement of all windows on the root
 *  window.  The drag&drop registration list keeps a pool of such
 *  structures so that they can be allocated quickly when needed.
 *  Returns a pointer to an empty structure.
 * ------------------------------------------------------------------------
 */
static DD_WinRep*
WinRepAlloc(ddlist)
	DD_RegList *ddlist;  /* drag&drop registration list */
{
	DD_WinRep *wr;

	/*
	 *  Return the top-most structure in the pool.
	 *  If the pool is empty, add a new structure to it.
	 */
	if (!ddlist->pool)
	{
		wr = (DD_WinRep*)malloc(sizeof(DD_WinRep));
		wr->next = NULL;
		ddlist->pool = wr;
	}
	wr = ddlist->pool;
	ddlist->pool = wr->next;

	wr->initialized = 0;
	wr->ddprop = NULL;
	wr->ddinterp = wr->ddwin = wr->ddhandlers = NULL;
	wr->parent = wr->kids = wr->next = NULL;
	return wr;
}

/*
 * ------------------------------------------------------------------------
 *  WinRepRelease()
 *
 *  Puts a window representation structure back into the global pool,
 *  making it available for future calls to WinRepAlloc().  Any
 *  associated resources (within the structure) are automatically freed.
 * ------------------------------------------------------------------------
 */
static void
WinRepRelease(wr,ddlist)
	DD_WinRep *wr;       /* window rep to be freed */
	DD_RegList *ddlist;  /* drag&drop registration list */
{
	DD_WinRep *wrkid, *wrnext;

	for (wrkid=wr->kids; wrkid; wrkid=wrnext)
	{
		wrnext = wrkid->next;
		WinRepRelease(wrkid,ddlist);
	}

	if (wr->ddprop)
		XFree(wr->ddprop);

	wr->next = ddlist->pool;  /* put back into pool */
	ddlist->pool = wr;
}


/*
 * ------------------------------------------------------------------------
 *  WinRepInit()
 *
 *  Invoked during "drag" operations to dig a little deeper into the
 *  root window hierarchy and cache the resulting information.  If a
 *  point coordinate lies within an uninitialized DD_WinRep, this
 *  routine is called to query window coordinates and drag&drop info.
 *  If this particular window has any children, more uninitialized
 *  DD_WinRep structures are allocated.  Further queries will cause
 *  these structures to be initialized in turn.
 * ------------------------------------------------------------------------
 */
static void
WinRepInit(wr,dsPtr)
	DD_WinRep *wr;         /* window rep to be initialized */
	DD_Source *dsPtr;      /* drag&drop source managing win rep */
{
	Window ignoreSource = Tk_WindowId(dsPtr->tkwin);
	Window ignoreToken  = Tk_WindowId(dsPtr->tokenwin);

	DD_WinRep *wrkid, *wrtail;

	Window root, parent, *kids;
	unsigned int nkids;
	XWindowAttributes winInfo;

	char *propInfo;
	int i, result, actualFormat;
	Atom actualType;
	unsigned long numItems, bytesAfter;

	/*
	 *  If the self-target flag is set, allow the source window to
	 *  drop onto itself.  Do not ignore source window during search.
	 */
	if (dsPtr->selfTarget)
		ignoreSource = None;

	if (!wr->initialized)
	{
		/*
		 *  Query for the window coordinates.
		 */
		if (XGetWindowAttributes(dsPtr->display, wr->win, &winInfo) &&
		    (winInfo.map_state == IsViewable) &&
			(wr->win != ignoreToken) &&
			(wr->win != ignoreSource))
		{
			wr->x0  = winInfo.x;
			wr->y0  = winInfo.y;
			wr->x1  = winInfo.x + winInfo.width;
			wr->y1  = winInfo.y + winInfo.height;

			if (wr->parent)  /* offset by parent coords */
			{
				wr->x0 += wr->parent->x0;
				wr->y0 += wr->parent->y0;
				wr->x1 += wr->parent->x0;
				wr->y1 += wr->parent->y0;
			}
		}
		else
		{
			wr->x0 = wr->y0 = -1;
			wr->x1 = wr->y1 = -1;
		}

		/*
		 *  See if this window has a "DragDropInfo" property.
		 */
		result = XGetWindowProperty(dsPtr->display, wr->win,
			dsPtr->ddAtom, 0, MAX_PROP_SIZE, False, XA_STRING,
			&actualType, &actualFormat,
			&numItems, &bytesAfter, (unsigned char**)&propInfo);

		if ((result != Success) ||
		    (actualFormat != 8) ||
		    (actualType != XA_STRING))
		{
			if (propInfo != NULL) {
				XFree((caddr_t)propInfo);
			}
			propInfo = NULL;
		}

		wr->ddprop = propInfo;
		if (wr->ddprop)
		{
			char *p = wr->ddprop;
			wr->ddinterp = wr->ddprop;

			while ((*p != '\0') && (*p != ']'))
				p++;

			if (*p != '\0')
			{
				*p++ = '\0';         /* terminate interp name */
				wr->ddwin = p;       /* get start of window name */
			}

			while ((*p != '\0') && (*p != ']'))
				p++;

			if (*p != '\0')
			{
				*p++ = '\0';         /* terminate window name */
				wr->ddhandlers = p;  /* get start of handler list */

				/*
				 *  Handler strings are of the form:
				 *  "<type> <type> ... <type> "
				 */
				while (*p != '\0')
				{
					while ((*p != ' ') && (*p != '\0'))
						p++;

					*p++ = '\0';  /* null terminate handler type */
				}
			}
		}

		/*
		 *  If this window has any children, then create DD_WinReps
		 *  for them as well.
		 */
		if (XQueryTree(dsPtr->display, wr->win, &root, &parent, &kids, &nkids))
		{
			wrtail = NULL;
			for (i=nkids-1; i >= 0; i--)
			{
				wrkid = WinRepAlloc(dsPtr->ddlist);
				wrkid->win = kids[i];
				wrkid->parent = wr;

				if (wrtail)
					wrtail->next = wrkid;
				else
					wr->kids = wrkid;

				wrtail = wrkid;
			}
			if (kids != NULL) {
				XFree((caddr_t)kids);   /* done with list of kids */
			}
		}
	}
	wr->initialized = ~0;
}


/*
 * ------------------------------------------------------------------------
 *  AddDDProp()
 *
 *  Attaches a "DragDropInfo" property to the given target window.  This
 *  property allows the drag&drop mechanism to recognize the window as
 *  a valid target, and stores important information including the
 *  interpreter managing the target and the pathname for the target
 *  window.  Usually invoked when the target is first registered or
 *  first exposed (so that the X-window really exists).
 * ------------------------------------------------------------------------
 */
static void
AddDDProp(dtPtr)
	DD_Target* dtPtr;  /* drag&drop target window data */
{
	Tcl_Interp *interp = dtPtr->ddlist->interp;

	Atom ddProperty;
	char buffer[MAX_PROP_SIZE], *path, *info;
	DD_TargetHndl *thandl;

	if (dtPtr->tkwin != None)
	{
		static char command[] = { "winfo name ." };

		path = Tk_PathName(dtPtr->tkwin);
		if (Tcl_Eval(interp, command)==TCL_OK)
			sprintf(buffer, "%s]%s]", interp->result, path);
		else
			sprintf(buffer, "]%s]", path);

		Tcl_SetResult(interp, buffer, TCL_VOLATILE);
		for (thandl=dtPtr->handlers; thandl; thandl=thandl->next)
			Tcl_AppendResult(interp, thandl->dataType, " ", (char*)NULL);

		ddProperty = XInternAtom(dtPtr->display, DRAGDROP_PROPINFO, False);
		info = interp->result;

		XChangeProperty(dtPtr->display, Tk_WindowId(dtPtr->tkwin),
			ddProperty, XA_STRING, 8, PropModeReplace,
			(unsigned char*)info, strlen(info)+1);
	}
}

/*
 * ------------------------------------------------------------------------
 *  DDTokenEventProc()
 *
 *  Invoked by the Tk dispatcher to handle widget events.
 *  Manages redraws for the drag&drop token window.
 * ------------------------------------------------------------------------
 */
static void
DDTokenEventProc(clientData, eventPtr)
	ClientData clientData;     /* data associated with widget */
	XEvent *eventPtr;          /* information about event */
{
	register DD_Source *dsPtr = (DD_Source*)clientData;

	if ((eventPtr->type == Expose) && (eventPtr->xexpose.count == 0))
	{
		if (dsPtr->tokenwin)
		{
			Tk_Window tkwin = dsPtr->tokenwin;
			int bd;

			bd = dsPtr->tokenBorderWidth;
			Tk_Fill3DRectangle(dsPtr->display, Tk_WindowId(tkwin),
				dsPtr->tokenOutline,
				0, 0, Tk_Width(tkwin), Tk_Height(tkwin),
				0, TK_RELIEF_FLAT);
			Tk_Fill3DRectangle(Tk_Display(tkwin), Tk_WindowId(tkwin),
				dsPtr->tokenBorder,
				bd, bd, Tk_Width(tkwin)-2*bd, Tk_Height(tkwin)-2*bd,
				bd, (dsPtr->overTargetWin) ? TK_RELIEF_RAISED : TK_RELIEF_FLAT);
		}
	}
	else if (eventPtr->type == DestroyNotify)
		dsPtr->tokenwin = NULL;
}

/*
 * ------------------------------------------------------------------------
 *  MoveDDToken()
 *
 *  Invoked during "drag" operations to move a token window to its
 *  current "drag" coordinate.
 * ------------------------------------------------------------------------
 */
static void
MoveDDToken(dsPtr)
	DD_Source *dsPtr;  /* drag&drop source window data */
{
	int x = dsPtr->tokenx;
	int y = dsPtr->tokeny;
	Tk_Window tokenwin = dsPtr->tokenwin;

	int max;
	int vx, vy;
	unsigned int vw, vh;

	/*
	 *  Adjust current location for virtual root windows.
	 */
	Tk_GetVRootGeometry(dsPtr->tkwin, &vx, &vy, &vw, &vh);
	x += vx;
	y += vy;

	max = WidthOfScreen(Tk_Screen(dsPtr->tkwin)) - Tk_Width(tokenwin);
	switch (dsPtr->tokenAnchor)
	{
	case TK_ANCHOR_NW:
	case TK_ANCHOR_W:
	case TK_ANCHOR_SW:
		break;

	case TK_ANCHOR_N:
	case TK_ANCHOR_CENTER:
	case TK_ANCHOR_S:
		x -= Tk_Width(tokenwin)/2;
		break;

	case TK_ANCHOR_NE:
	case TK_ANCHOR_E:
	case TK_ANCHOR_SE:
		x -= Tk_Width(tokenwin);
		break;
	}
	if (x > max) {
		x = max;
	} else if (x < 0) {
		x = 0;
	}
	max = HeightOfScreen(Tk_Screen(dsPtr->tkwin)) - Tk_Height(tokenwin);
	switch (dsPtr->tokenAnchor)
	{
	case TK_ANCHOR_NW:
	case TK_ANCHOR_N:
	case TK_ANCHOR_NE:
		break;

	case TK_ANCHOR_W:
	case TK_ANCHOR_CENTER:
	case TK_ANCHOR_E:
		y -= Tk_Height(tokenwin)/2;
		break;

	case TK_ANCHOR_SW:
	case TK_ANCHOR_S:
	case TK_ANCHOR_SE:
		y -= Tk_Height(tokenwin);
		break;
	}
	if (y > max) {
		y = max;
	} else if (y < 0) {
		y = 0;
	}
	if ((x != Tk_X(tokenwin)) || (y != Tk_Y(tokenwin)))
		Tk_MoveWindow(tokenwin, x, y);
}

/*
 * ------------------------------------------------------------------------
 *  UpdateDDToken()
 *
 *  Invoked when the event loop is idle to determine whether or not
 *  the current drag&drop token position is over another drag&drop
 *  target.
 * ------------------------------------------------------------------------
 */
static void
UpdateDDToken(clientData)
	ClientData clientData;  /* widget data */
{
	register DD_Source *dsPtr = (DD_Source*)clientData;
	Tk_Window tkwin = dsPtr->tokenwin;

	int status, bd;

	status = (FindTargetWin(dsPtr, dsPtr->tokenx, dsPtr->tokeny) != NULL);

	bd = dsPtr->tokenBorderWidth;
	if (dsPtr->overTargetWin ^ status)
	{
		Tk_Fill3DRectangle(dsPtr->display, Tk_WindowId(tkwin),
			dsPtr->tokenOutline,
			0, 0, Tk_Width(tkwin), Tk_Height(tkwin),
			0, TK_RELIEF_FLAT);
		Tk_Fill3DRectangle(dsPtr->display, Tk_WindowId(tkwin),
			dsPtr->tokenBorder,
			bd, bd, Tk_Width(tkwin)-2*bd, Tk_Height(tkwin)-2*bd,
			bd, (status) ? TK_RELIEF_RAISED : TK_RELIEF_FLAT);

		/*
		 *  If the source has a site command, then invoke it to
		 *  modify the appearance of the token window.  Pass any
		 *  errors onto the drag&drop error handler.
		 */
		if (dsPtr->sitecmd)
		{
			char buffer[200];

			sprintf(buffer, "%d", status);
			if ((Tcl_VarEval(dsPtr->ddlist->interp,
					dsPtr->sitecmd," ",buffer," ",Tk_PathName(dsPtr->tokenwin),
					(char*)NULL) != TCL_OK) &&
				dsPtr->ddlist->errorProc && *dsPtr->ddlist->errorProc)

				(void) Tcl_VarEval(dsPtr->ddlist->interp,
					dsPtr->ddlist->errorProc, " {",
					dsPtr->ddlist->interp->result, "}",
					(char*)NULL);
		}
	}
	dsPtr->overTargetWin = status;
}

/*
 * ------------------------------------------------------------------------
 *  HideDDToken()
 *
 *  Unmaps the drag&drop token.  Invoked directly at the end of a
 *  successful communication, or after a delay if the communication
 *  fails (allowing the user to see a graphical picture of failure).
 * ------------------------------------------------------------------------
 */
static void
HideDDToken(clientData)
	ClientData clientData;  /* widget data */
{
	register DD_Source *dsPtr = (DD_Source*)clientData;

	if (dsPtr->tokenwin)
		Tk_UnmapWindow(dsPtr->tokenwin);

	dsPtr->hidetoken = NULL;
}

/*
 * ------------------------------------------------------------------------
 *  RejectDDToken()
 *
 *  Draws a rejection mark on the current drag&drop token, and arranges
 *  for the token to be unmapped after a small delay.
 * ------------------------------------------------------------------------
 */
static void
RejectDDToken(dsPtr)
	DD_Source* dsPtr;  /* widget data */
{
	int div = 6;      /* controls size of rejection symbol */
	int w,h,lwid,x,y,margin;

	Tk_Window tkwin = dsPtr->tokenwin;

	margin = 2*dsPtr->tokenBorderWidth;
	w = Tk_Width(tkwin) - 2*margin;
	h = Tk_Height(tkwin) - 2*margin;

	lwid = (w < h) ? w/div : h/div;
	lwid = (lwid < 1) ? 1 : lwid;

	w = h = lwid*(div-1);
	x = (Tk_Width(tkwin) - w)/2;
	y = (Tk_Height(tkwin) - h)/2;

	/*
	 *  Draw the rejection symbol background (\) on the token window...
	 */
	XSetLineAttributes(Tk_Display(tkwin), dsPtr->rejectBgGC,
		lwid+4, LineSolid, CapButt, JoinBevel);

	XDrawArc(Tk_Display(tkwin), Tk_WindowId(tkwin), dsPtr->rejectBgGC,
		x, y, w, h, 0, 23040);

	XDrawLine(Tk_Display(tkwin), Tk_WindowId(tkwin), dsPtr->rejectBgGC,
		x+lwid, y+lwid, x+w-lwid, y+h-lwid);

	/*
	 *  Draw the rejection symbol foreground (\) on the token window...
	 */
	XSetLineAttributes(Tk_Display(tkwin), dsPtr->rejectFgGC,
		lwid, LineSolid, CapButt, JoinBevel);

	XDrawArc(Tk_Display(tkwin), Tk_WindowId(tkwin), dsPtr->rejectFgGC,
		x, y, w, h, 0, 23040);

	XDrawLine(Tk_Display(tkwin), Tk_WindowId(tkwin), dsPtr->rejectFgGC,
		x+lwid, y+lwid, x+w-lwid, y+h-lwid);

	/*
	 *  Arrange for token window to disappear eventually.
	 */
	dsPtr->hidetoken
		= Tk_CreateTimerHandler(1000, HideDDToken, (ClientData)dsPtr);
}


/*
 * ------------------------------------------------------------------------
 *  StackInit()
 *
 *  Initializes a stack structure, allocating a certain amount of memory
 *  for the stack and setting the stack length to zero.
 * ------------------------------------------------------------------------
 */
static void
StackInit(stack)
	DD_Stack *stack;     /* stack to be initialized */
{
	stack->values = stack->space;
	stack->max = sizeof(stack->space)/sizeof(ClientData);
	stack->len = 0;
}

/*
 * ------------------------------------------------------------------------
 *  StackDelete()
 *
 *  Destroys a stack structure, freeing any memory that may have been
 *  allocated to represent it.
 * ------------------------------------------------------------------------
 */
static void
StackDelete(stack)
	DD_Stack *stack;     /* stack to be deleted */
{
	if (stack->values != stack->space)  /* allocated extra memory? */
		free((char*)stack->values);   /* then free it */

	stack->values = NULL;
	stack->len = stack->max = 0;
}

/*
 * ------------------------------------------------------------------------
 *  StackPush()
 *
 *  Pushes a piece of client data onto the top of the given stack.
 * ------------------------------------------------------------------------
 */
static void
StackPush(cdata,stack)
	ClientData cdata;    /* data to be pushed onto stack */
	DD_Stack *stack;     /* stack */
{
	ClientData *newStack;

	if (stack->len+1 >= stack->max)
	{
		stack->max = (stack->max == 0) ? 5 : 2*stack->max;
		newStack = (ClientData*)
			malloc((unsigned)(stack->max*sizeof(ClientData)));

		if (stack->values)
		{
			memcpy((char *)newStack, (char *)stack->values,
				stack->len*sizeof(ClientData));

			if (stack->values != stack->space)
				free((char*)stack->values);
		}
		stack->values = newStack;
	}
	stack->values[stack->len++] = cdata;
}

/*
 * ------------------------------------------------------------------------
 *  StackPop()
 *
 *  Pops a bit of client data from the top of the given stack.
 * ------------------------------------------------------------------------
 */
static ClientData
StackPop(stack)
	DD_Stack *stack;  /* stack to be manipulated */
{
	if ((stack->values != NULL) && (stack->len > 0)) {
		return (stack->values[--stack->len]);
	}
	return (ClientData) 0;
}
