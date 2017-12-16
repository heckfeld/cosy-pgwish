static char rcsid[]="$Id: tclUtil.c,v 1.3 94/02/14 14:55:19 mangin Exp $";
#include <tclExtend.h>
#include <string.h>

/**
 **  ctailCmd :
 **  
 **    Returns the portion of a string following
 **    the first occurence of a given character.
 **    Tcl equivalent : string range [expr 1 + [string first <c> <s>] end]
 **/

int ctailCmd(clientData, interp, argc, argv)
     ClientData clientData;              /* Inutilise. */
     Tcl_Interp *interp;                 /* Interpreteur tcl. */
     int argc;                           /* Nombre d'arguments. */
     char **argv;                        /* Arguments passes a la commande tcl. */
{
  char *s;
  /* syntax : ctail char string */

  if (argc != 3) {
    Tcl_AppendResult (interp, "Wrong # of arguments: should be \"",
		      argv[0], " char string", (char*)0);
    return(TCL_ERROR);
  }

  if ((*argv[1] == '\0') || (*(argv[1]+1) != '\0')) {
    Tcl_AppendResult (interp, "expected single char, got \"",
		      argv[1], "\"", (char *)0);
    return(TCL_ERROR);
  }

  s = strchr(argv[2], *argv[1]);
  if (s == NULL)
    Tcl_SetResult(interp, "", TCL_STATIC);
  else
    Tcl_SetResult(interp, s+1, TCL_VOLATILE);

  return(TCL_OK);
}

/**
 **  lvarrmCmd :
 **  
 **    Removes from a list elements matching the
 **    specified pattern (glob style matching),
 **    and returns the list of removed elements.
 **    Tcl equivalent:
 **    while {[set i [lsearch -glob $list pattern]] >= 0} {
 **      set list [lreplace $list $i $i]
 **      }
 **/

int lvarrmCmd(clientData, interp, argc, argv)
     ClientData clientData;              /* Inutilise. */
     Tcl_Interp *interp;                 /* Interpreteur tcl. */
     int argc;                           /* Nombre d'arguments. */
     char **argv;                        /* Arguments passes a la commande tcl. */
{
  /* syntax : lvarrm var pattern */

  int listArgc, i;
  char **listArgv;
  int newArgc;
  char **newArgv;
  char *list;

  if (argc != 3) {
    Tcl_AppendResult (interp, "Wrong # of arguments: should be \"",
		      argv[0], " var pattern", (char*)0);
    return(TCL_ERROR);
  }
  
  list = Tcl_GetVar(interp, argv[1], TCL_LEAVE_ERR_MSG);
  if (list == NULL)
    return(TCL_ERROR);
  if (*list == '\0')
    return(TCL_OK);
  
  if (Tcl_SplitList(interp, list, &listArgc, &listArgv) != TCL_OK) {
    return TCL_ERROR;
  }

  newArgv = (char **)malloc(listArgc*sizeof(char *));
  newArgc = 0;

  for (i = 0; i < listArgc; i++) {
    if (Tcl_StringMatch(listArgv[i], argv[2])) {
      Tcl_AppendElement(interp, listArgv[i]);
    } else {
      newArgv[newArgc++] = listArgv[i];
    }      
  }

  if (Tcl_SetVar(interp, argv[1], Tcl_Merge(newArgc, newArgv),
		 TCL_LEAVE_ERR_MSG) == NULL) {
    free(newArgv);
    return(TCL_ERROR);
  }

  free(newArgv);
  return(TCL_OK);
}

/**
 **  lfind ?-all? list pattern ?retvar?
 **
 **    Returns first (or all with -all switch) element
 **      of a list matching the specified pattern.
 **    If retvar is specified, returns 1 if an element has
 **      been found, 0 otherwise.
 **/

int lfindCmd(clientData, interp, argc, argv)
     ClientData clientData;              /* Inutilise. */
     Tcl_Interp *interp;                 /* Interpreteur tcl. */
     int argc;                           /* Nombre d'arguments. */
     char **argv;                        /* Arguments passes a la commande tcl. */
{
  int listArgc, retArgc, i;
  char **listArgv, **retArgv;
  int first;
  
  if ((argc < 3) || (argc > 5)) {
    Tcl_AppendResult (interp, "Wrong # of arguments: should be \"",
		      argv[0], " ?-all? list pattern ?retvar?", (char*)0);
    return(TCL_ERROR);
  }

  if ((*argv[1]=='-') && (!strcmp(argv[1], "-all")))
    first = 2;
  else
    first = 1;

  if (Tcl_SplitList(interp, argv[first], &listArgc, &listArgv) != TCL_OK) {
    return TCL_ERROR;
  }

  retArgv = (char **)malloc(listArgc*sizeof(char *));
  retArgc = 0;

  for (i = 0; i < listArgc; i++) {
    if (Tcl_StringMatch(listArgv[i], argv[first+1])) {
      retArgv[retArgc++] = listArgv[i];
      if (first == 1)
	break;
    }
  }

  if (argc == first+3) {
    /*  retvar specified  */
    if (retArgc > 0) {
      Tcl_SetResult(interp, "1", TCL_STATIC);
      if (Tcl_SetVar(interp, argv[first+2],
		     Tcl_Merge(retArgc, retArgv),
		     TCL_LEAVE_ERR_MSG) == NULL) {
	free(retArgv);
	return(TCL_ERROR);
      }
    } else {
      Tcl_SetResult(interp, "0", TCL_STATIC);
      if (Tcl_SetVar(interp, argv[first+2], "",
		     TCL_LEAVE_ERR_MSG) == NULL) {
	free(retArgv);
	return(TCL_ERROR);
      }
    }
  } else {
    /*  no retvar specified  */
    Tcl_SetResult(interp, Tcl_Merge(retArgc, retArgv),
		  TCL_VOLATILE);
  }

  free(retArgv);
  return(TCL_OK);
}

/*
 *  Checks if a box is included in another box
 */

int boxinCmd(clientData, interp, argc, argv)
     ClientData clientData;              /* Inutilise. */
     Tcl_Interp *interp;                 /* Interpreteur tcl. */
     int argc;                           /* Nombre d'arguments. */
     char **argv;                        /* Arguments passes a la commande tcl. */
{
  double x1, y1, x2, y2;
  double X1, Y1, X2, Y2;
  
  if (argc != 9) {
    Tcl_AppendResult (interp, "Wrong # of arguments: should be \"",
		      argv[0], " x1 y1 x2 y2 X1 Y1 X2 Y2\"", (char *)0);
    return(TCL_ERROR);
  }

  if ((Tcl_GetDouble(interp, argv[1], &x1) != TCL_OK) ||
      (Tcl_GetDouble(interp, argv[2], &y1) != TCL_OK) ||
      (Tcl_GetDouble(interp, argv[3], &x2) != TCL_OK) ||
      (Tcl_GetDouble(interp, argv[4], &y2) != TCL_OK) ||
      (Tcl_GetDouble(interp, argv[5], &X1) != TCL_OK) ||
      (Tcl_GetDouble(interp, argv[6], &Y1) != TCL_OK) ||
      (Tcl_GetDouble(interp, argv[7], &X2) != TCL_OK) ||
      (Tcl_GetDouble(interp, argv[8], &Y2) != TCL_OK)) {
    return(TCL_ERROR);
  }

  if ((x1 >= X1) && (x2 <= X2) &&
      (y1 >= Y1) && ( y2 <= Y2)) {
    Tcl_SetResult(interp, "1", TCL_STATIC);
  } else {
    Tcl_SetResult(interp, "0", TCL_STATIC);
  }

  return(TCL_OK);
}
