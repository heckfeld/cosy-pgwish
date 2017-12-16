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


/* Funktion fuer die Ueberpruefung, ob der uebergebene tname ein */
/* zugelassenes Target ist                                       */

int istarget (char *tname, target_info *tinfo)
{
   struct hostent *hp;
   struct in_addr addr;
   int i;

   if ((hp = gethostbyname (tname)) == (struct hostent *) 0) {
      return (0);
   }

   if (tinfo != (target_info *) 0) {
      i = 0;
      while (hp->h_addr_list[i] != (char *) 0) {
         addr.s_addr = * (int *) hp->h_addr_list[i];
         strcpy (tinfo->iaddr[i], inet_ntoa (addr));
         tinfo->subn[i] = ((* (int *) hp->h_addr_list[i]) >> 8) & 0xff;
         ++i;
      }
      tinfo->n = i;
   }

   return (1);
}
