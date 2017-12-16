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


/* Funktion fuer die Ueberpruefung, ob die uebergebenene ea eine */
/* gueltige Ethernetadresse ist                                  */

int isea (char *ea)
{
   char *cp;

   cp = ea;
   if (strlen (cp) < EALEN || *cp++ != '0' || tolower(*cp++) != 'x')
      return (0);
   while (*cp) {
      if (!isxdigit(*cp))
         return (0);
      cp++;
   }
   return (1);
}
