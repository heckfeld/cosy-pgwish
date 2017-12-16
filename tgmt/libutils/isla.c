/*
 * $Header$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <tgt_utilsp.h>


/* Funktion fuer die Ueberpruefung, ob das uebergebene Argument */
/* eine gueltige Ladeadresse ist                                */

int isla (char *la)
{
   char *cp;

   switch (strlen (la)) {
   case 1:
      if (*la != '0')
         return (0);
   case 0:
      sprintf (la, "0x0");
      return (1);
   }

   cp = la;
   if (*cp++ != '0' || tolower(*cp++) != 'x')
      return (0);

   while (*cp) {
      if (!isxdigit(*cp))
         return (0);
      cp++;
   }
   return (1);
}
