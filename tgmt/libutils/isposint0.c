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


/* Funktion fuer die Ueberpruefung, ob der uebergebenene str eine */
/* gueltige positive dezimale Integerzahl ist (oder 0)            */

int isposint0 (char *str)
{
   char *ptr;
   int wert;

   wert = strtol (str, &ptr, 10);
   if (ptr != &str[strlen (str)] || wert < 0)
      return (0);

   sprintf (str, "%d", wert);

   return (1);
}
