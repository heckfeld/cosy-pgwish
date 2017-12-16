
/* $Header: /mnt/cc-x4/ccp/database/src/coob/tx/rcs/coob_tx.c,v 1.1 2003/02/04 13:17:54 tine Exp $ */

/******************************************************************************/
/*                                                                            */
/* Aufruf       : coob_tx -i                                                  */
/*                coob_tx -d<name>                                            */
/*                coob_tx -n<name> [parameter]                                */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* Funktion     : Zugriff auf die Datenbank fuer den Timing-Sender            */
/*                -i : Der Inhalt der Datenbank fuer alle Geraete             */
/*                     wird ausgelesen.                                       */
/*                -d : Der Datenbankeintrag fuer das spezifizierte            */
/*                     Geraet wird geloescht.                                 */
/*                -n : Der Datenbankeintrag fuer das spezifizierte            */
/*                     Geraet wird geaendert oder ausgelesen.                 */
/*                                                                            */
/*                parameter:                                                  */
/*                   -    (==> Default-Werte, falls noch kein Eintrag)        */
/*                                                                            */
/*                   -v<events>   ("$COOBDB/tx_ev_list.dat")                  */
/*                   -z<zyklus>   ("-1,-1,-1")                                */
/*                   -c<cyco>     ("-1,-1,-1")                                */
/*                   -mo<mode>    ("continuous")                              */
/*                   -e<expmode>  ("-1")                                      */
/*                   -s<sfolge>   ("-1")                                      */
/*                   -mi<micro>   ("2000 20 extern on")                       */
/*                   -ma<macro>   ("2000 20 gated off")                       */
/*                                                                            */
/* Rueckgabewert: 0 alles ok                                                  */
/*                1 falsche usage                                             */
/*                2 fehler beim zugriff auf die datenbank                     */
/*                3 kein eintrag in der datenbank                             */
/*                4 abbruch durch ctrl-c                                      */
/*                                                                            */
/******************************************************************************/

#include <stdio.h>           /* fuer argc, argv, fprintf ... */
#include <stdlib.h>          /* fuer getenv() */
#include <string.h>          /* fuer strncpy() */
#include <signal.h>          /* fuer signal() */

#include "libpq-fe.h"
#include "coobp.h"           /* 'basename'-Macro, DATENBANK_OEFFNEN */
#include "txp.h"             /* fuer Parameter-Definitionen */

#define PRINT_USAGE { \
   fprintf (stdout, "usage: %s -i\n", basename(argv[0], ptr)); \
   fprintf (stdout, "       %s -d<name>\n", basename(argv[0], ptr)); \
   fprintf (stdout, "       %s -n<name> [parameter]\n", \
                                               basename(argv[0], ptr)); \
   exit (1); \
}

void signal_handler (int);

/*
 * Globale Variable, damit die Interrupt-Funktion auch darauf
 * zugreifen kann.
 */
PGconn *conn = (PGconn *) 0;


int main (int argc, char *argv[])
{
   int tuples,
       status;
   char cmd[1024];

   char *pgport = NULL,
        *pgoptions = NULL,
        *pgtty = NULL;

   PGresult *res = (PGresult *) 0;

  /* Groessen dieser Felder sind an die Tabelle in der Datenbank angepasst */
   char name[32],
        events[256],
        zyklus[64],
        cyco[64],
        mode[64],
        expmode[32],
        sfolge[32],
        micro[64],
        macro[64];
   char data_string[512];

   int exval = 0;             /* Exit-Wert fuer das Programm                  */
   int put = 0;               /* Flag, ob Eintrag in Datenbank mit INSERT     */
   char *ptr;                 /* fuer 'basename'-Macro (s.o.)                 */
   int iarg, i;               /* Schleifenindizes                             */

/*
 * Aufsetzen des Signal-Handlings fuer 'CTRL-C' etc.
 */
   (void) signal (SIGINT, signal_handler);    /* interrupt */
   (void) signal (SIGKILL, signal_handler);   /* kill */
   (void) signal (SIGTERM, signal_handler);   /* software termination signal */
   (void) signal (SIGPIPE, signal_handler);   /* write on a pipe with no one */
                                              /* to read it */

   if (argc <= 1)
      PRINT_USAGE;
   for (iarg=1; iarg<argc; ++iarg)
      if (argv[iarg][0] != '-')
         PRINT_USAGE;

/*
 * Initialisieren der Variablen ...
 */
   memset (cmd, '\0', sizeof (cmd));
   memset (data_string, '\0', sizeof (data_string));
   memset (name, '\0', sizeof (name));
   memset (events, '\0', sizeof (events));
   memset (zyklus, '\0', sizeof (zyklus));
   memset (cyco, '\0', sizeof (cyco));
   memset (mode, '\0', sizeof (mode));
   memset (expmode, '\0', sizeof (expmode));
   memset (sfolge, '\0', sizeof (sfolge));
   memset (micro, '\0', sizeof (micro));
   memset (macro, '\0', sizeof (macro));

/*
 * Die einzelnen Argumente auswerten.
 * Es wird davon ausgegangen, dass die moeglichen Optionen richtig
 * aufgefuehrt sind.
 */
   switch (argv[1][1])
   {
   case 'i': /* Inhalt der kompletten Datenbank auslesen */
      DATENBANK_OEFFNEN(conn,pghost,pgport,pgoptions,pgtty,dbName);

/*
 * Alle Eintraege aus der Datenbank suchen.
 */
      sprintf (cmd, "SELECT rtrim(name,' '), rtrim(data,' ') "
                          "from %s", tabName);
      res = PQexec (conn, cmd);
      if (res == (PGresult *) 0
       || (status = PQresultStatus (res)) != PGRES_TUPLES_OK) {
         PQreset (conn);
         exval = 2;
      } else {
         if ((tuples = PQntuples (res)) > 0) {
            for (i=0; i<tuples; ++i) {
               strcpy (name, (char *) PQgetvalue (res, i, 0));
               strcpy (data_string, (char *) PQgetvalue (res, i, 1));
               fprintf (stdout, "%s:%s\n", name, data_string);
            }
         }
      }
      if (res != (PGresult *) 0)
         PQclear (res);

      break;

   case 'd': /* einzelnen Eintrag loeschen */
/*
 * Verbindung zur Datenbank aufbauen
 */
      DATENBANK_OEFFNEN(conn,pghost,pgport,pgoptions,pgtty,dbName);

/*
 * Aus der Datenbank loeschen.
 */
      sprintf (cmd, "DELETE from %s WHERE name='%s'",
                    tabName, &argv[1][2]);
      res = PQexec (conn, cmd);
      if (res == (PGresult *) 0 || (status = PQresultStatus (res)) != 1) {
         PQreset (conn);
         exval = 2;
      } else {
         strcpy (cmd, PQcmdStatus (res));

         /* Antwort auswerten ("DELETE 1" oder "DELETE 0") */
         if ((ptr = strchr (cmd, ' ')) == (char *) 0 || *(++ptr) == '0') {
            fprintf (stdout, "Es existiert kein Eintrag in der Datenbank.\n");
            exval = 3;
         }
      }
      if (res != (PGresult *) 0)
         PQclear (res);

      break;

   case 'n': /* Inhalt der Datenbank fuer ein Geraet */
             /* auslesen oder aendern (hinzufuegen)  */

/*
 * Verbindung zur Datenbank aufbauen
 */
      DATENBANK_OEFFNEN(conn,pghost,pgport,pgoptions,pgtty,dbName);

      sprintf (cmd, "SELECT rtrim(name,' '), rtrim(data,' ') "
                          "from %s WHERE name='%s'",
                          tabName, &argv[1][2]);
      res = PQexec (conn, cmd);
      if (res == (PGresult *) 0
       || (status = PQresultStatus (res)) != PGRES_TUPLES_OK) {
         PQreset (conn);
         exval = 2;
      } else {
         if ((tuples = PQntuples (res)) > 0) {
            strcpy (name, (char *) PQgetvalue (res, 0, 0));
            strcpy (data_string, (char *) PQgetvalue (res, 0, 1));
         }
      }
      if (res != (PGresult *) 0)
         PQclear (res);

      switch (argc)
      {
      case 2:   /* Inhalt auslesen */

         if (tuples > 0)
            fprintf (stdout, "%s", data_string);
         else {
            fprintf (stdout, "Es existiert kein Eintrag in der Datenbank.\n");
            exval = 3;
         }
         break;

      default:   /* spezifizierte[n] Parameter aendern */

         if (tuples <= 0)
         {
/*
 * Default-Werte fuer die einzelnen Komponenten setzen
 */
            strcpy (events, "$COOBDB/tx_ev_list.dat");
            strcpy (zyklus, "-1,-1,-1");
            strcpy (cyco, "-1,-1,-1");
            strcpy (mode, "continuous");
            strcpy (expmode, "-1");
            strcpy (sfolge, "-1");
            strcpy (micro, "2000 20 extern on");
            strcpy (macro, "2000 20 gated off");

            put = 1;

         } /* if (tuples <= 0) */
         else {
/*
 * Alle Komponenten aus dem Datenstring extrahieren
 */
            strcpy (events, strtok (data_string, ":"));
            strcpy (zyklus, strtok ((char *) 0, ":"));
            strcpy (cyco, strtok ((char *) 0, ":"));
            strcpy (mode, strtok ((char *) 0, ":"));
            strcpy (expmode, strtok ((char *) 0, ":"));
            strcpy (sfolge, strtok ((char *) 0, ":"));
            strcpy (micro, strtok ((char *) 0, ":"));
            strcpy (macro, strtok ((char *) 0, ":"));

         } /* else von if (tuples <= 0) */

         for (iarg=2; iarg<argc; ++iarg)
         {

            switch (argv[iarg][1]) {
            case 'v':  /* Events */
               strncpy (events, &argv[iarg][2], sizeof(events)-1);
               break;

            case 'z':  /* Zyklus */
               strncpy (zyklus, &argv[iarg][2], sizeof(zyklus)-1);
               break;

            case 'c':  /* Cycount */
               strncpy (cyco, &argv[iarg][2], sizeof(cyco)-1);
               break;

            case 's':  /* Sfolge */
               strncpy (sfolge, &argv[iarg][2], sizeof(sfolge)-1);
               break;

            case 'e':  /* Expmode */
               strncpy (expmode, &argv[iarg][2], sizeof(expmode)-1);
               break;

            case 'm':
               switch (argv[iarg][2]) {
               case 'o':  /* Mode */
                  strncpy (mode, &argv[iarg][3], sizeof(mode)-1);
                  break;

               case 'i':  /* Micro */
                  strncpy (micro, &argv[iarg][3], sizeof(micro)-1);
                  break;

               case 'a':  /* Macro */
                  strncpy (macro, &argv[iarg][3], sizeof(macro)-1);
                  break;
               }
               break;

            default:
               /* PRINT_USAGE; */
               /* Es sollen nur die Default-Werte gesetzt werden. */
               break;

            } /* switch (argv[iarg][1]) */

         } /* for (iarg=2; iarg<argc; ++iarg) */

/* Datenbank-Eintrag machen */
         sprintf (data_string, "%s:%s:%s:%s:%s:%s:%s:%s",
                  events, zyklus, cyco, mode, expmode,
                  sfolge, micro, macro);

         /* Entscheiden, ob INSERT oder UPDATE */
         if (put) {
            sprintf (cmd, "INSERT into %s VALUES ('%s', '%s')",
                       tabName, &argv[1][2], data_string);
         } else {
            sprintf (cmd, "UPDATE %s SET data='%s' "
                          "WHERE name='%s'",
                          tabName, data_string, &argv[1][2]);
         }
         res = PQexec (conn, cmd);
         if (res == (PGresult *) 0 || (status = PQresultStatus (res)) != 1) {
            PQreset (conn);
            exval = 2;
         }
         if (res != (PGresult *) 0)
            PQclear (res);

         break;  /* default */

      } /* switch (argc) */

      break;

   default:
      PRINT_USAGE;

   } /* switch (argv[1][1]) */

/*
 * Die Datenbank muss wieder geschlossen werden.
 */
   /* close the connection to the database and cleanup */
   PQfinish (conn);
   exit (exval);
}

void signal_handler (int num)
{
   if (conn != (PGconn *) 0)
      PQfinish (conn);

   exit (4);
}
