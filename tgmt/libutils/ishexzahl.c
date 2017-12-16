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


/* Funktion fuer die Ueberpruefung, ob der uebergebene str eine */
/* gueltige hexadezimale Zahl ist                               */

int ishexzahl (char *str)
{
   char *cp;

   cp = str;
   if (strlen (cp) == 1 && *cp == '0')
      strcpy (cp, "0x0");
   /* Hinter "0x" muss mindestens eine Hex-Ziffer stehen! */
   if (strlen (cp) < 3 || *cp++ != '0' || tolower(*cp++) != 'x')
      return (0);
   while (*cp) {
      if (!isxdigit(*cp))
         return (0);
      cp++;
   }

   return (1);
}
