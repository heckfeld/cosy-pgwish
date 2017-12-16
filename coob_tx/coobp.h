/* $Header: /mnt/cc-x4/ccp/database/src/coob/h/rcs/coobp.h,v 1.2 2003/02/04 15:35:56 tine Exp $ */

#ifndef COOBP_INCLUDED
# define COOBP_INCLUDED

# include "pgdefs.h"


/* Hier soll die Environment-Variable verwendet werden */
/* Postgres-Host */
char *pghost = (char *) 0;


/*
 * Macro zum Lesen eines Strings, als leere Zeile bzw. ohne fuehrende,
 * abschliessende White-Space-Zeichen.
 */

#define fgetstr(fp,str,hlp,n) \
{  \
   memset ((void *) hlp, '\0', (size_t) n); \
   *str = '\0'; \
   fgets (hlp, n, fp); \
   sscanf (hlp, "%s", str); \
}


/*
 * Um vom uebergebenen (vielleicht absoluten) Namen
 * den Namen ohne Pfad zu erhalten.
 * Eingabeparameter:
 *    char *name : zeigt auf absoluten Namen
 *    char *ptr  : Hilfspointer fuer Bearbeitung
 */

# define basename(name,ptr) (((ptr = strrchr (name, '/')) != (char *) 0) \
                        ? ptr + 1 : name)

#endif
