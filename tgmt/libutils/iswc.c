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


/* Funktion fuer die Ueberpruefung, ob die uebergebene wc eine gultige */
/* Workcell fuer das mit tinfo beschriebene Target ist                 */

int iswc (char *wc, target_info *tinfo)
{
   struct hostent *hp;
   int i, subn, found;

   if ((hp = gethostbyname (wc)) == (struct hostent *) 0) {
      return (0);
   }
   if (tinfo != (target_info *) 0 && tinfo->n > 0) {
      i = 0;
      found = 0;
      while (!found && hp->h_addr_list[i] != (char *) 0) {
         subn = ((* (int *) hp->h_addr_list[i]) >> 8) & 0xff;
         if (tinfo->subn[0] == subn)
            found = 1;
         ++i;
      }
   }

   return (found);

}
