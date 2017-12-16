/*
 * Copyright (c) 1993 by Sven Delmas
 * All rights reserved.
 * See the file COPYRIGHT for the copyright notes.
 *
 */

#if !defined(TCL_RAW_TCP_H)
#define TCL_RAW_TCP_H 1

EXTERN int              Tcp_AcceptCmd _ANSI_ARGS_((ClientData clientData,
                            Tcl_Interp *interp, int argc, char **argv));
EXTERN int              Tcp_ConnectCmd _ANSI_ARGS_((ClientData clientData,
                            Tcl_Interp *interp, int argc, char **argv));
EXTERN int              Tcp_ShutdownCmd _ANSI_ARGS_((ClientData clientData,
                            Tcl_Interp *interp, int argc, char **argv));
EXTERN int              Tcp_FileHandlerCmd _ANSI_ARGS_((ClientData clientData,
                            Tcl_Interp *interp, int argc, char **argv));
EXTERN int              Tcp_RecvCmd _ANSI_ARGS_((ClientData clientData,
                            Tcl_Interp *interp, int argc, char **argv));

#define create_tclRawTCP()\
	{ Tcl_CreateCommand(interp, "accept", Tcp_AcceptCmd,\
		(ClientData) NULL, (Tcl_CmdDeleteProc*) NULL);\
	  Tcl_CreateCommand(interp, "shutdown", Tcp_ShutdownCmd,\
		(ClientData) NULL, (Tcl_CmdDeleteProc*) NULL);\
	  Tcl_CreateCommand(interp, "receive", Tcp_RecvCmd,\
		(ClientData) NULL, (Tcl_CmdDeleteProc*) NULL);\
	  Tcl_CreateCommand(interp, "connect", Tcp_ConnectCmd,\
		(ClientData) NULL, (Tcl_CmdDeleteProc*) NULL);\
	  Tcl_CreateCommand(interp, "filehandler", Tcp_FileHandlerCmd,\
		(ClientData) NULL, (Tcl_CmdDeleteProc*) NULL); }
#endif

/* eof */
