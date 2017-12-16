/*
 * $Header$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <tgt_tgmtp.h>
#include <tgt_utilsp.h>
#include <tgt_fcp.h>
#include "libpq-fe.h"


/*
 * Nur der Name des Programms soll bei der Usage mit ausgegeben werden.
 */
#define basename(name) (((ptr = strrchr (name, '/')) != (char *) 0) \
                        ? ptr + 1 : name)

#define PRINT_USAGE { \
   fprintf (stdout, "usage: %s -{i|f <file>|d|l[ -s <file>]|a}\n", \
     basename(argv[0])); \
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
   target_info tinfo;     /* fuer Target (Index 0) und Workcell (Index 1) */
   int i,
       tuples,
       status;
   char cmd[1024];

   char *pgport = NULL,
        *pgoptions = NULL,
        *pgtty = NULL;

   PGresult *res = (PGresult *) 0;

   FILE *fp;
   char help[256];
   char tname[128], na[64], slot[64], cput[128], savefilename[256];

   int save = 0;
   int zaehler, line, len;
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

   memset ((char *) &tinfo, '\0', sizeof (tinfo));
   memset (tname, '\0', sizeof (tname));
   memset (na, '\0', sizeof (na));
   memset (cput, '\0', sizeof (cput));
   memset (slot, '\0', sizeof (slot));

/*
 * Die Argumente auswerten.
 */
   if (argc < 2 || argv[1][0] != '-')
      PRINT_USAGE;

/* Es ist mindestens eine Option angegeben. */
/* Entsprechend abarbeiten.                 */

   switch (argv[1][1])
   {
   case 'i': /* ein Target interaktiv eintragen */
      if (argc > 2)
         PRINT_USAGE;

      fprintf (stdout, "Targetname: ");
      fflush (stdout);
      fgetstr (stdin, tname, help, sizeof(help));
      if (!istarget (tname, &tinfo) || tinfo.n < 1) {
         fprintf (stderr, "Target \"%s\" hat keine Internetadresse.\n", tname);
         exit (2);
      }

      fprintf (stdout, "Nodeadresse (dezimal): ");
      fflush (stdout);
      fgetstr (stdin, na, help, sizeof(help));
      if (!isposint (na)) {
         fprintf (stderr, "Ungueltige Nodeadresse.\n");
         exit (2);
      }

      fprintf (stdout, "CPU-Typ: ");
      fflush (stdout);
      fgetstr (stdin, cput, help, sizeof(help));
      if (!cput[0]) {
         fprintf (stderr, "Ungueltiger CPU-Typ.\n");
         exit (2);
      }

      fprintf (stdout, "Slotnummer (dezimal): ");
      fflush (stdout);
      fgetstr (stdin, slot, help, sizeof(help));
      if (!isposint (slot)) {
         fprintf (stderr, "Ungueltige Slotnummer.\n");
         exit (2);
      }
      fprintf (stdout, "\n%s.%s %s %s\n", tname, na, cput, slot);

/*
 * Verbindung zur Datenbank aufbauen
 */
      DATENBANK_OEFFNEN(conn,pghost,pgport,pgoptions,pgtty,dbName);
#ifdef TEST
      fprintf (stdout, "Connection to database '%s' established.\n", dbName);
#endif

/*
 * In die Datenbank eintragen.
 * Erst INSERT versuchen, wenn gescheitert, dann UPDATE.
 */
#ifdef TEST
      fprintf (stdout, "Werte in die Datenbank eintragen.\n");
#endif

      sprintf (cmd, "INSERT into %s VALUES ('%s', '%s', '%s', '%s')",
                    tabName, tname, na, cput, slot);
      res = PQexec (conn, cmd);
      if (res == (PGresult *) 0 || PQresultStatus (res) != 1) {
         PQreset (conn);
         if (res != (PGresult *) 0) {
            PQclear (res);
            res = (PGresult *) 0;
         }

         sprintf (cmd, "UPDATE %s SET cput='%s', slot='%s' "
                       "WHERE name='%s' AND node='%s'",
                       tabName, cput, slot, tname, na);
         res = PQexec (conn, cmd);
         if (res == (PGresult *) 0 || PQresultStatus (res) != 1) {
            fprintf (stderr, "FEHLER: exec von %s failed\n%s\n------\n",
                             cmd, PQerrorMessage (conn));
            PQreset (conn);
            exval = 2;
         }
      }
      if (res != (PGresult *) 0) {
         PQclear (res);
         res = (PGresult *) 0;
      }

      break;

   case 'f': /* mehrere Targets aus einem File eintragen */
      if (argc != 3)
         PRINT_USAGE;

      if ((fp = fopen (argv[2], "r")) == (FILE *) 0)
      {
         fprintf (stdout, "File \"%s\" kann nicht geoeffnet werden.\n",
                  argv[2]);
         exit (2);
      }

/*
 * Verbindung zur Datenbank aufbauen
 */
      DATENBANK_OEFFNEN(conn,pghost,pgport,pgoptions,pgtty,dbName);
#ifdef TEST
      fprintf (stdout, "Connection to database '%s' established.\n", dbName);
#endif

      line = 1;
      zaehler = 0;
      while (fgets (help, sizeof(help), fp) != (char *) 0)
      {
         if ((ptr = strchr (help, '#')) != (char *) 0)
            *ptr = '\0';            /* Kommentar weg*/

         memset (tname, '\0', sizeof (tname));
         memset (na, '\0', sizeof (na));
         memset (cput, '\0', sizeof (cput));
         memset (slot, '\0', sizeof (slot));

         len = sscanf (help, "%s%s%s%s", tname, na, cput, slot);

/* Vollstaendigkeit/Korrektheit der Daten ueberpruefen. */
         if (len > 0) {
            if (len != 4) {
               fprintf (stderr, "- %s (Zeile %d): Fehlerhafte Eingabe.\n",
                                 help, line);
            } else {
               if (!istarget (tname, &tinfo) || tinfo.n < 1) {
                  fprintf (stderr, "- %s (Zeile %d): Target \"%s\" "
                                   "hat keine Internetadresse.\n",
                                   help, line, tname);
               } else {
                  tinfo.n = 1;

                  if (!isposint (na)) {
                     fprintf (stderr, "- %s (Zeile %d): Ungueltige "
                                      "Nodeadresse.\n",
                                      help, line);
                  } else {

                     if (!cput[0]) {
                        fprintf (stderr, "- %s (Zeile %d): Ungueltiger "
                                         "CPU-Typ.\n",
                                         help, line);
                     } else {
                        if (!isposint (slot)) {
                           fprintf (stderr, "- %s (Zeile %d): Ungueltige "
                                            "Slotnummer.\n",
                                            help, line);
                        } else {
/*
 * In die Datenbank eintragen.
 * Erst INSERT versuchen, wenn gescheitert, dann UPDATE.
 */
#ifdef TEST
                           fprintf (stdout, "Werte in die Datenbank "
                                            "eintragen.\n");
#endif
                           fprintf (stdout, "%s.%s %s %s\n", tname, na,
                                                          cput, slot);

                           sprintf (cmd,
                                   "INSERT into %s VALUES "
                                   "('%s', '%s', '%s', '%s')",
                                   tabName, tname, na, cput, slot);
                           res = PQexec (conn, cmd);
                           if (res == (PGresult *) 0
                            || PQresultStatus (res) != 1) {
                              PQreset (conn);
                              if (res != (PGresult *) 0) {
                                 PQclear (res);
                                 res = (PGresult *) 0;
                              }

                              sprintf (cmd, "UPDATE %s "
                                            "SET cput='%s', slot='%s' "
                                            "WHERE name='%s' AND node='%s'",
                                            tabName, cput, slot, tname, na);
                              res = PQexec (conn, cmd);
                              if (res == (PGresult *) 0
                               || PQresultStatus (res) != 1) {
                                 fprintf (stderr, "FEHLER: exec von %s "
                                                  "failed\n%s\n------\n",
                                                  cmd, PQerrorMessage (conn));
                                 PQreset (conn);
                              } else
                                 zaehler++;
                           } else
                              zaehler++;

                           if (res != (PGresult *) 0) {
                              PQclear (res);
                              res = (PGresult *) 0;
                           }
                           fflush (stdout);

                        } /* else von if (!isposint (slot)) */

                     } /* else von if (!cput[0]) */

                  } /* else von if (!isposint (na)) */

               } /* else von if (!istarget(tname, &tinfo) || ... ) */

            } /* else von if (len != 4) */

         } /* if (len > 0) */

         line++;

      } /* while (fgets (...)) */

      fclose (fp);
      fprintf (stderr, "\n%d Karten eingetragen.\n", zaehler);

      break;

   case 'd': /* einen Eintrag loeschen */
      if (argc > 2)
         PRINT_USAGE;

      fprintf (stdout, "Targetname: ");
      fflush (stdout);
      fgetstr (stdin, tname, help, sizeof(help));

      fprintf (stdout, "Nodeadresse (dezimal): ");
      fflush (stdout);
      fgetstr (stdin, na, help, sizeof(help));

/*
 * Verbindung zur Datenbank aufbauen
 */
      DATENBANK_OEFFNEN(conn,pghost,pgport,pgoptions,pgtty,dbName);
#ifdef TEST
      fprintf (stdout, "Connection to database '%s' established.\n", dbName);
#endif

/*
 * Aus der Datenbank loeschen.
 */
#ifdef TEST
      fprintf (stdout, "Werte aus der Datenbank loeschen.\n");
#endif
      sprintf (cmd, "DELETE from %s WHERE name='%s' AND node='%s'",
                    tabName, tname, na);

      res = PQexec (conn, cmd);
      if (res == (PGresult *) 0 || PQresultStatus (res) != 1) {
         fprintf (stderr, "FEHLER: exec von %s failed\n%s\n------\n",
                          cmd, PQerrorMessage (conn));
         PQreset (conn);
         exval = 2;
      } else {
         strcpy (cmd, PQcmdStatus (res));
         if ((ptr = strchr (cmd, ' ')) != (char *) 0 && *(++ptr) == '1')
            fprintf (stdout, "Karte %s.%s geloescht.\n", tname, na);
         else {
            fprintf (stderr, "Fehler: %s.%s konnte nicht geloescht werden.\n",
                             tname, na);
            exval = 2;
         }
      }
      if (res != (PGresult *) 0) {
         PQclear (res);
         res = (PGresult *) 0;
      }

      break;

   case 'l': /* einen Eintrag anzeigen */
      if (argc > 2) {
         if (argc != 4 || strcmp (argv[2], "-s") != 0)
            PRINT_USAGE;
         save = 1;
         strcpy (savefilename, argv[3]);
      }

      fprintf (stdout, "Targetname: ");
      fflush (stdout);
      fgetstr (stdin, tname, help, sizeof(help));

      fprintf (stdout, "Nodeadresse (dezimal): ");
      fflush (stdout);
      fgetstr (stdin, na, help, sizeof(help));

/*
 * Verbindung zur Datenbank aufbauen
 */
      DATENBANK_OEFFNEN(conn,pghost,pgport,pgoptions,pgtty,dbName);
#ifdef TEST
      fprintf (stdout, "Connection to database '%s' established.\n", dbName);
#endif

     len = sprintf (cmd, "SELECT rtrim(cput,' '), slot from %s "
                         "WHERE name='%s' AND node='%s'",
                         tabName, tname, na);
#ifdef TEST
      fprintf (stdout, "cmd: %s\n", cmd);
#endif
      res = PQexec (conn, cmd);
      if (res == (PGresult *) 0
       || (status = PQresultStatus (res)) != PGRES_TUPLES_OK) {
#ifdef TEST
         fprintf (stderr, "Fehler beim Lesen aus der Datenbank (%d).\n",
                          status);
         fprintf (stderr, "%s\n", PQerrorMessage (conn));
#endif
         PQreset (conn);
         exval = 2;

      } else {

         if ((tuples = PQntuples (res)) > 0) {
            strcpy (cput, (char *) PQgetvalue (res, 0, 0));
            strcpy (slot, (char *) PQgetvalue (res, 0, 1));
            fprintf (stderr, "Targetname Nodeadresse CPU-Typ Slotnummer\n");
            fprintf (stdout, "%s %s %s %s\n", tname, na, cput, slot);

            if (save)     /* Wenn gewuenscht, an savefile anhaengen */
            {
               if ((fp = fopen (savefilename, "a")) == (FILE *) 0)
               {
                  fprintf (stderr, "Fehler: Kann File %s nicht oeffnen\n",
                                   savefilename);
                  exval = 2;
               }
               fprintf (fp, "%s %s %s %s\n", tname, na, cput, slot);
               fclose (fp);
            }
         } else {
            fprintf (stderr, "Fehler: %s.%s konnte nicht gelesen werden\n",
                              tname, na);
            exval = 2;
         }
      }
      if (res != (PGresult *) 0) {
         PQclear (res);
         res = (PGresult *) 0;
      }

      break;

   case 'a': /* gesamten Datenbankinhalt ausgeben */
      if (argc > 2)
         PRINT_USAGE;

/*
 * Verbindung zur Datenbank aufbauen
 */
      DATENBANK_OEFFNEN(conn,pghost,pgport,pgoptions,pgtty,dbName);
#ifdef TEST
      fprintf (stdout, "Connection to database '%s' established.\n", dbName);
#endif

/*
 * Alle Eintraege aus der Datenbank suchen.
 */
      len = sprintf (cmd, "SELECT rtrim(name,' '), node, "
                          "rtrim(cput,' '), slot from %s", tabName);
#ifdef TEST
      fprintf (stdout, "cmd: %s\n", cmd);
#endif
      res = PQexec (conn, cmd);
      if (res == (PGresult *) 0
       || (status = PQresultStatus (res)) != PGRES_TUPLES_OK) {
#ifdef TEST
         fprintf (stderr, "Fehler beim Lesen aus der Datenbank (%d).\n",
                          status);
         fprintf (stderr, "%s\n", PQerrorMessage (conn));
#endif
         PQreset (conn);
         exval = 2;
      } else {

#ifndef PQPRINTTUPLES
         if ((tuples = PQntuples (res)) > 0) {
            fprintf (stderr, "Targetname Nodeadresse CPU-Typ Slotnummer\n");
            for (i=0; i<tuples; ++i) {
               strcpy (tname, (char *) PQgetvalue (res, i, 0));
               strcpy (na, (char *) PQgetvalue (res, i, 1));
               strcpy (cput, (char *) PQgetvalue (res, i, 2));
               strcpy (slot, (char *) PQgetvalue (res, i, 3));
               fprintf (stdout, "%s %s %s %s\n", tname, na, cput, slot);
            }
         }
         else 
            fprintf (stderr, "Die DB %s ist leer.\n", tabName);
#else
         PQprintTuples (res, stdout, 0, 1, 0);
#endif
      }
      if (res != (PGresult *) 0) {
         PQclear (res);
         res = (PGresult *) 0;
      }

      break;

   default:
      PRINT_USAGE;

   } /* switch (argv[1][1]) */

/*
 * Datenbank muss wieder geschlossen werden.
 */
   /* close the connection to the database and cleanup */
   PQfinish (conn);
   exit (exval);
}


void signal_handler (int num)
{
   if (conn != (PGconn *) 0)
      PQfinish (conn);

   exit (3);
}
