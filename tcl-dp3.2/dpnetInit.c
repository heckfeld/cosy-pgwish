/*
 * dpnetInit.c --
 *
 *	Initialize an interpreter with Tcl-DP.
 *	This mostly just makes a bunch of calls to Tcl_CreateCommand to
 *	create tcl commands for the various modules.
 *
 * Copyright (c) 1993 The Regents of the University of California.
 * All rights reserved.
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 * 
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
 * CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

#include <tk.h>
#include "dpInt.h"

/*
 * The data structure below is used by the "dp_whenidle" command to remember
 * the command to be executed later.
 */

typedef struct {
    Tcl_Interp *interp;		/* Interpreter in which to execute command. */
    char *command;		/* Command to execute.  Malloc'ed, so must
				 * be freed when structure is deallocated. */
} IdleInfo;


/*
 *--------------------------------------------------------------
 *
 * IdleProc --
 *
 *	Idle callback to execute commands registered with the
 *	dp_whenidle command.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      Executes whatever command was specified.  If the command
 *      returns an error, then the command "tkerror" is invoked
 *      to process the error;  if tkerror fails then information
 *      about the error is output on stderr.
 *
 *
 *--------------------------------------------------------------
 */
static void
IdleProc  (clientData)
    ClientData clientData;

{
    IdleInfo *idlePtr = (IdleInfo *) clientData;
    int result;

    if (idlePtr->command != NULL) {
	result = Tcl_GlobalEval(idlePtr->interp, idlePtr->command);
	if (result != TCL_OK) {
	    Tcl_AddErrorInfo(idlePtr->interp,
			     "\n    within dp_whenidle callback");
	    Tk_BackgroundError(idlePtr->interp);
	}
	ckfree(idlePtr->command);
    }
    ckfree((char *) idlePtr);
}

/*
 *--------------------------------------------------------------
 *
 * Tdp_WhenIdleCmd --
 *
 *	This function implements the "dp_whenidle" tcl command.  See
 *	the user documentation for what it does.
 *
 * Results:
 *	A standard tcl result.
 *
 * Side effects:
 *	Arrange for the tcl command specified to be evaluated as
 *	an idle event
 *
 *--------------------------------------------------------------
 */
            /* ARGSUSED */
int
Tdp_WhenIdleCmd (clientData, interp, argc, argv)
    ClientData clientData;          /* Often ignored */
    Tcl_Interp *interp;             /* tcl interpreter */
    int argc;                       /* Number of arguments */
    char *argv[];                   /* Arg list */
{
    IdleInfo *idlePtr;

    if (argc < 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"",
		argv[0], " command ?arg arg ...?\"",
		(char *) NULL);
	return TCL_ERROR;
    }

    idlePtr = (IdleInfo *) ckalloc((unsigned) (sizeof(IdleInfo)));
    idlePtr->interp = interp;
    if (argc == 2) {
	idlePtr->command = (char *) ckalloc((unsigned) (strlen(argv[1]) + 1));
	strcpy(idlePtr->command, argv[1]);
    } else {
	idlePtr->command = Tcl_Concat(argc-1, argv+1);
    }
    Tk_DoWhenIdle(IdleProc, (ClientData) idlePtr);
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Tdp_UpdateCmd --
 *
 *	This procedure is invoked to process the "dp_update" Tcl-DP
 *	command.  This is exactly the same as Tk's "update" command,
 *	execpt it doesn't require a connection to the X server.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
int
Tdp_UpdateCmd(clientData, interp, argc, argv)
    ClientData clientData;	/* Ignored */
    Tcl_Interp *interp;		/* Current interpreter. */
    int argc;			/* Number of arguments. */
    char **argv;		/* Argument strings. */
{
    int flags;

    if (argc == 1) {
	flags = TK_DONT_WAIT;
    } else if (argc == 2) {
	if (strncmp(argv[1], "idletasks", strlen(argv[1])) != 0) {
	    Tcl_AppendResult(interp, "bad argument \"", argv[1],
		    "\": must be idletasks", (char *) NULL);
	    return TCL_ERROR;
	}
	flags = TK_IDLE_EVENTS;
    } else {
	Tcl_AppendResult(interp, "wrong # args: should be \"",
		argv[0], " ?idletasks?\"", (char *) NULL);
	return TCL_ERROR;
    }

    /*
     * Handle all pending events.
     */

    while (Tk_DoOneEvent(flags) != 0) {
	/* Empty loop body */
    }

    /*
     * Must clear the interpreter's result because event handlers could
     * have executed commands.
     */

    Tcl_ResetResult(interp);
    return TCL_OK;
}

/*
 *--------------------------------------------------------------
 *
 * Tdp_WaitVariable --
 *
 *	Implements the "dp_wait" tcl command -- the function of
 *	this command is the same as "tkwait -variable"
 *
 * Results:
 *	A standard tcl result
 *
 * Side effects:
 *	Execution is 
 *
 *--------------------------------------------------------------
 */
/* ARGSUSED */
int
Tdp_WaitVariable(clientData, interp, argc, argv)
    ClientData clientData;	/* unused */
    Tcl_Interp *interp;		/* Current interpreter. */
    int argc;			/* Number of arguments. */
    char **argv;		/* Argument strings. */
{
    char *nargv[4];

    if (argc != 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"",
		argv[0], " variable", (char *) NULL);
	return TCL_ERROR;
    }
    nargv[0] = argv[0];
    nargv[1] = "variable";
    nargv[2] = argv[1];
    nargv[3] = (char *)0;
    return(Tk_TkwaitCmd(clientData, interp, 3, nargv));
}

/*
 *--------------------------------------------------------------
 *
 * Tdp_Init --
 *
 *	Initialize the full Tcl-DP package.
 *
 * Results:
 *	None
 *
 * Side effects:
 *	network related commands are bound to the interpreter.
 *
 *--------------------------------------------------------------
 */
int
Tdp_Init (interp)
     Tcl_Interp *interp;
{
    static char initCmd[] =
	"if [file exists $dp_library/init.tcl] {\
	    source $dp_library/init.tcl\n\
	} else {\n\
	    set msg \"can't find $dp_library/init.tcl; perhaps you \"\n\
	    append msg \"need to\\ninstall Tcl-DP or set your DP_LIBRARY \"\n\
	    append msg \"environment variable?\"\n\
	    error $msg\n\
	}";
    char *libDir;

    libDir = getenv("DP_LIBRARY");
    if (libDir == NULL) {
	libDir = DP_LIBRARY;
    }
    Tcl_SetVar(interp, "dp_library", libDir, TCL_GLOBAL_ONLY);
    Tcl_SetVar(interp, "dp_version", DP_VERSION, TCL_GLOBAL_ONLY);

    Tcl_CreateCommand (interp, "dp_update", Tdp_UpdateCmd,
	   (ClientData) NULL, (void (*) _ANSI_ARGS_((ClientData))) NULL);
    Tcl_CreateCommand (interp, "dp_after", Tk_AfterCmd,
	   (ClientData) NULL, (void (*) _ANSI_ARGS_((ClientData))) NULL);
    Tcl_CreateCommand (interp, "dp_whenidle", Tdp_WhenIdleCmd,
	   (ClientData) NULL, (void (*) _ANSI_ARGS_((ClientData))) NULL);
    Tcl_CreateCommand (interp, "dp_address", Tdp_AddressCmd,
	   (ClientData) NULL, (void (*) _ANSI_ARGS_((ClientData))) NULL);
    Tcl_CreateCommand(interp, "dp_waitvariable", Tdp_WaitVariable,
		(ClientData)0, (void (*) _ANSI_ARGS_((ClientData))) NULL);

    Tdp_Tcp_Init(interp);
    Tdp_RPCInit(interp);

    /*
     * Execute the tcl routine provided with Tcl-DP to preload all
     * library routines into the interpreter.  This is necessary
     * because some of the library routines shadow built in commands
     * (e.g., the close command)
     */
    return Tcl_GlobalEval (interp, initCmd);
}
