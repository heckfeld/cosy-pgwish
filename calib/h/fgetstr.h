/* $Header$ */

#ifndef FGETSTR_INCLUDED
# define FGETSTR_INCLUDED

# include <stdio.h>
# include <string.h>

/*
 * Macro zum Lesen eines Strings, als leere Zeile bzw. ohne fuehrende,
 * abschliessende White-Space-Zeichen.
 */

# define fgetstr(fp,str,hlp,n) {  \
     memset ((void *) hlp, '\0', (size_t) n); \
     *str = '\0'; \
     fgets (hlp, n, fp); \
     sscanf (hlp, "%s", str); \
  }

#endif
