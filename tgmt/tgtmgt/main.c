/*
 * $Header$
 */
/******************************************************************************/
/*                                                                            */
/* usage: tgtmgt {<Majornummer>|<Geraetename>}                                */
/*                                                                            */
/* Rueckgabewert:                                                             */
/*    0  alles ok                                                             */
/*    1  ungueltige Majornummer                                               */
/*                                                                            */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <tgtmgt.h>


/*
 * Nur der Name des Programms soll bei der Usage mit ausgegeben werden.
 */
#define basename(name) (((ptr = strrchr (name, '/')) != (char *) 0) \
                        ? ptr + 1 : name)

#define PRINT_USAGE { \
   fprintf (stderr, "usage: %s {<Majornummer>|<Geraetename>}\n", \
     basename(argv[0])); \
   exit (1); \
}


void signal_handler (int);


int main (int argc, char *argv[])
{
   char *ergebnis;

   int exval = 0;
   char *ptr;      /* wird fuer das 'basename'-Macro (s.o.) gebraucht. */

/*
 * Aufsetzen des Signal-Handlings fuer 'CTRL-C' etc.
 */
   (void) signal (SIGINT, signal_handler);    /* interrupt */
   (void) signal (SIGKILL, signal_handler);   /* kill */
   (void) signal (SIGTERM, signal_handler);   /* software termination signal */
   (void) signal (SIGPIPE, signal_handler);   /* write on a pipe with no one */
                                              /* to read it */

/*
 * Die Argumente auswerten.
 */
   if (argc != 2)
      PRINT_USAGE;

   if (exval = tgtmgt (argv[1], &ergebnis))
      fprintf (stderr, "%s\n", ergebnis);
   else
      fprintf (stdout, "%s\n", ergebnis);

   free ((void *) ergebnis);
   exit (exval);
}


void signal_handler (int num)
{
   fprintf (stderr, "Abbruch von tgtmgt\n");

   exit (3);
}
