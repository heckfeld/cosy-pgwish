/* Syslog interface for tcl

*/
#include <tcl.h>
#include <syslog.h>
#include <string.h>
typedef struct {
                int logOpened;
                int facility,options;
                char ident[32];
                Tcl_HashTable *priorities;
                Tcl_HashTable *facilities;
               } SyslogInfo;

void Syslog_ListHash(Tcl_Interp *interp,Tcl_HashTable *table);	       
/* SyslogHelp - puts usage message into interp->result
 * 
 *
 */

void SyslogHelp(Tcl_Interp *interp,char *cmdname)
{  Tcl_AppendResult(interp,"Wrong # of args. should be ",cmdname, 
           " ?option value? priority message",NULL);
}

/* Syslog_Log -
 * implements syslog tcl command. General format: syslog ?options? level text
 * options -facility -ident -options
 * 
 */


int Syslog_Log(ClientData data, Tcl_Interp *interp, int argc, char **argv)
{    SyslogInfo *info=(SyslogInfo *)data;
    char *message = NULL;
    int priority;
    int i=1;
    if (argc<=1) {
        SyslogHelp(interp,argv[0]);
        return TCL_ERROR;
    }
  while (i<argc-1) {
    if (!strcmp(argv[i],"-facility")) {
        Tcl_HashEntry * entry=Tcl_FindHashEntry(info->facilities,argv[i+1]);
        if (!entry) {
           Tcl_AppendResult(interp,"Invalid facility name: \"",argv[i+1],
		   "\" available facilities:",
               NULL);
	   Syslog_ListHash(interp,info->facilities);
           return TCL_ERROR;
        }
        info->facility=(int)Tcl_GetHashValue(entry);
        if (info-> logOpened) {
            closelog();
            info-> logOpened=0;
        }
     } else if (!strcmp(argv[i],"-options")) {
         int tmp;
        if (Tcl_GetInt(interp,argv[i+1],&tmp)==TCL_ERROR)
             return TCL_ERROR;
        info->options=tmp;
        if (info->logOpened) {
            closelog();
            info->logOpened=0;
        }
     } else if (!strcmp(argv[i],"-ident")) {
        strncpy(info->ident, argv[i+1],32);
        info->ident[31]=0;
        if (info->logOpened) {
            closelog();
            info->logOpened=0;
        }
     } else {
       Tcl_HashEntry *entry=Tcl_FindHashEntry(info->priorities,argv[i]);
       if (!entry) {
          Tcl_AppendResult(interp,"Invalid syslog level \"",argv[i],
		  "\" available levels:",
               NULL);
	  Syslog_ListHash(interp,info->priorities); 
          return TCL_ERROR;
       }
       priority=(int)Tcl_GetHashValue(entry);
       message=argv[i+1];
       i+=2;
       if (i<argc-1) {
           SyslogHelp(interp,argv[0]);
           return TCL_ERROR;
       }
     }
     i+=2;
  }
  if (i<argc-1) {
     SyslogHelp(interp,argv[0]);
     return TCL_ERROR;
  }
  if (message) {
      if (!info->logOpened) {
	  openlog(info->ident,info->options,info->facility);
	  info->logOpened=1;
      }
      syslog(priority,"%s",message);
  }
  return TCL_OK;
}
/*
 * Syslog_ListHash - appends to interp result all the values of given
 * hash table
 */
void Syslog_ListHash(Tcl_Interp *interp,Tcl_HashTable *table) 
{
    Tcl_HashSearch *searchPtr=(Tcl_HashSearch *)
	  ckalloc(sizeof(Tcl_HashSearch));
    Tcl_HashEntry *entry;
    char separator[3]={' ',' ',0};   
    entry=Tcl_FirstHashEntry(table,searchPtr);
    while (entry) {
        Tcl_AppendResult(interp,separator,Tcl_GetHashKey(table,entry),NULL);
        separator[0]=',';
        entry=Tcl_NextHashEntry(searchPtr);
    }   
    ckfree((char *)searchPtr);
} 
/* 
 *  Syslog_Delete - Tcl_CmdDeleteProc for syslog command.
 *  Frees all hash tables and closes log if it was opened.
 */
void Syslog_Delete(ClientData data)
{ SyslogInfo *info=(SyslogInfo *)data;
  Tcl_DeleteHashTable(info->facilities);
  ckfree((char *)info->facilities);
  Tcl_DeleteHashTable(info->priorities);
  ckfree((char *)info->priorities);
  if (info->logOpened) {
     closelog();
  }
  ckfree((char *)info);
}
/*
 * My simplified wrapper for add values into hash
 *
 */
void AddEntry(Tcl_HashTable *table,char *key,int value)
{ int new;
  Tcl_HashEntry *entry=Tcl_CreateHashEntry(table,key,&new);
  Tcl_SetHashValue(entry,(ClientData)value);
}
/*
 * Syslog_Init 
 * Package initialization procedure for Syslog package. 
 * Creates command 'syslog', fills hash tables to map symbolic prioriry 
 * and facility names to system constants.
 */
int Syslog_Init(Tcl_Interp *interp)
{  char *argv0;
   SyslogInfo *info=(SyslogInfo *)ckalloc(sizeof(SyslogInfo));
   info->logOpened=0;
   info->options=0;
   info->facility=LOG_USER;
   argv0=Tcl_GetVar(interp,"argv0",TCL_GLOBAL_ONLY);
   if (argv0) {
       strncpy(info->ident,argv0,32);
   } else {
       strcpy(info->ident,"Tcl script");
   }
   info->ident[31]=0;
   info->facilities =(Tcl_HashTable *) ckalloc(sizeof(Tcl_HashTable));
   Tcl_InitHashTable(info->facilities,TCL_STRING_KEYS);
   AddEntry(info->facilities,"auth",LOG_AUTH);  
#ifndef LOG_AUTHPRIV
# define LOG_AUTHPRIV LOG_AUTH
#endif
   AddEntry(info->facilities,"authpriv",LOG_AUTHPRIV);  
   AddEntry(info->facilities,"cron",LOG_CRON);  
   AddEntry(info->facilities,"daemon",LOG_DAEMON);  
   AddEntry(info->facilities,"kernel",LOG_KERN);
   AddEntry(info->facilities,"lpr",LOG_LPR);
   AddEntry(info->facilities,"mail",LOG_MAIL);
   AddEntry(info->facilities,"news",LOG_NEWS);
   AddEntry(info->facilities,"syslog",LOG_SYSLOG);
   AddEntry(info->facilities,"user",LOG_USER);
   AddEntry(info->facilities,"uucp",LOG_UUCP);
   AddEntry(info->facilities,"local0",LOG_LOCAL0);
   AddEntry(info->facilities,"local1",LOG_LOCAL1);
   AddEntry(info->facilities,"local2",LOG_LOCAL2);
   AddEntry(info->facilities,"local3",LOG_LOCAL3);
   AddEntry(info->facilities,"local4",LOG_LOCAL4);
   AddEntry(info->facilities,"local5",LOG_LOCAL5);
   AddEntry(info->facilities,"local6",LOG_LOCAL6);
   AddEntry(info->facilities,"local7",LOG_LOCAL7);
   info->priorities = (Tcl_HashTable *) ckalloc(sizeof(Tcl_HashTable));
   Tcl_InitHashTable(info->priorities,TCL_STRING_KEYS);
   AddEntry(info->priorities,"emerg",LOG_EMERG);
   AddEntry(info->priorities,"alert",LOG_ALERT);
   AddEntry(info->priorities,"crit",LOG_CRIT);
   AddEntry(info->priorities,"err",LOG_ERR);
   AddEntry(info->priorities,"error",LOG_ERR);
   AddEntry(info->priorities,"warning",LOG_WARNING);
   AddEntry(info->priorities,"notice",LOG_NOTICE);
   AddEntry(info->priorities,"info",LOG_INFO);
   AddEntry(info->priorities,"debug",LOG_DEBUG);
   Tcl_CreateCommand(interp,"syslog",Syslog_Log,(ClientData) info,
            Syslog_Delete); 
   // return Tcl_PkgProvide(interp,"Syslog",VERSION);
}
