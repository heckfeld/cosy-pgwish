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


/* Funktion fuer die Ueberpruefung, ob die uebergebene mask eine gultige */
/* Maske enthaelt, und ggfs. Aufteilen und Ueberpruefen der Maskenteile  */

int ismask (char mask[5][64], int flg)
{
   int i;
   char *token;

   if (flg == 0) {

      if (mask[0][0] == '\0') {
        strcpy (mask[1], "0x0");
        strcpy (mask[2], "0x0");
        strcpy (mask[3], "0x0");
        strcpy (mask[4], "0x0");

        /* Keine weitere Ueberpruefung notwendig */
        return (1);
      }

      strcpy (mask[1], strtok (mask[0], "."));

      /* Ist naechster Wert neu, oder vorigen Wert uebernehmen? */
      if ((token = strtok ((char *) 0, ".")) == (char *) 0)
         strcpy (mask[2], mask[1]);
      else
         strcpy (mask[2], token);

      /* Ist naechster Wert neu, oder vorigen Wert uebernehmen? */
      if ((token = strtok ((char *) 0, ".")) == (char *) 0)
         strcpy (mask[3], mask[2]);
      else
         strcpy (mask[3], token);

      /* Ist naechster Wert neu, oder vorigen Wert uebernehmen? */
      if ((token = strtok ((char *) 0, ".")) == (char *) 0)
         strcpy (mask[4], mask[3]);
      else
         strcpy (mask[4], token);

   }
   /* die einzelnen Hex-Zahlen ueberpruefen */
   for (i=1; i<=4; ++i) {
      if (!ishexzahl (mask[i]))
         return (0);
   }

   return (1);
}
