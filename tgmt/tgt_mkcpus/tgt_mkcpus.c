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
#include <tgt_cpusp.h>
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
   int i,
       tuples,
       status;
   char cmd[1024];

   char *pgport = NULL,
        *pgoptions = NULL,
        *pgtty = NULL;

   PGresult *res = (PGresult *) 0;

   FILE *fp;
   char help[256], data_string[256];
   char cput[128], la[128], kbin[256], abin[256], savefilename[256];

   int save = 0;
   int wert, zaehler, line, len;
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

   memset (data_string, '\0', sizeof (data_string));
   memset (cput, '\0', sizeof (cput));
   memset (la, '\0', sizeof (la));
   memset (kbin, '\0', sizeof (kbin));
   memset (abin, '\0', sizeof (abin));

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

      fprintf (stdout, "CPU-Typ: ");
      fflush (stdout);
      fgetstr (stdin, cput, help, sizeof(help));
      if (!cput[0]) {
         fprintf (stderr, "Ungueltiger CPU-Typ.\n");
         exit (2);
      }

      fprintf (stdout, "Ladeadresse: ");
      fflush (stdout);
      fgetstr (stdin, la, help, sizeof(help));
      if (!isla (la)) {
         fprintf (stderr, "Ungueltige Ladeadresse.\n");
         exit (2);
      }

      fprintf (stdout, "Kernelbinary: ");
      fflush (stdout);
      fgetstr (stdin, kbin, help, sizeof(help));
      if (!kbin[0]) {
         fprintf (stderr, "Ungueltiges Kernelbinary.\n");
         exit (2);
      }

      fprintf (stdout, "Applikationsbinary: ");
      fflush (stdout);
      fgetstr (stdin, abin, help, sizeof(help));
      if (!abin[0]) {
         fprintf (stderr, "Ungueltiges Applikationsbinary.\n");
         exit (2);
      }

      sprintf (data_string, "%s %s", kbin, abin);
      fprintf (stdout, "\n%s %s %s\n", cput, la, data_string);

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

      sprintf (cmd, "INSERT into %s VALUES ('%s', '%s', '%s')",
                    tabName, cput, la, data_string);
      res = PQexec (conn, cmd);
      if (res == (PGresult *) 0 || PQresultStatus (res) != 1) {
         PQreset (conn);
         if (res != (PGresult *) 0) {
            PQclear (res);
            res = (PGresult *) 0;
         }

         sprintf (cmd, "UPDATE %s SET la='%s', data='%s' WHERE name='%s'",
                       tabName, la, data_string, cput);
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

         memset (cput, '\0', sizeof (cput));
         memset (la, '\0', sizeof (la));
         memset (kbin, '\0', sizeof (kbin));
         memset (abin, '\0', sizeof (abin));

         len = sscanf (help, "%s%s%s%s", cput, la, kbin, abin);

/* Vollstaendigkeit/Korrektheit der Daten ueberpruefen. */
         if (len > 0) {
            if (len != 4) {
               fprintf (stderr, "- %s (Zeile %d): Fehlerhafte Eingabe.\n",
                                 help, line);
            } else {
               if (!cput[0]) {
                  fprintf (stderr, "- %s (Zeile %d): Ungueltiger "
                                   "CPU-Typ.\n",
                                   help, line);
               } else {
                  if (!isla (la)) {
                     fprintf (stderr, "- %s (Zeile %d): Ungueltige "
                                      "Ladeadresse.\n",
                                      help, line);
                  } else {
                     if (!kbin[0]) {
                        fprintf (stderr, "- %s (Zeile %d): Ungueltiges "
                                         "Kernelbinary.\n",
                                         help, line);
                     } else {
                        if (!abin[0]) {
                           fprintf (stderr, "- %s (Zeile %d): Ungueltiges "
                                            "Applikationsbinary.\n",
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
                           sprintf (data_string, "%s %s", kbin, abin);
                           fprintf (stdout, "%s %s %s\n",
                                            cput, la, data_string);

                           sprintf (cmd, "INSERT into %s VALUES "
                                         "('%s', '%s', '%s')",
                                         tabName, cput, la, data_string);
                           res = PQexec (conn, cmd);
                           if (res == (PGresult *) 0
                            || PQresultStatus (res) != 1) {
                              PQreset (conn);
                              if (res != (PGresult *) 0) {
                                 PQclear (res);
                                 res = (PGresult *) 0;
                              }

                              sprintf (cmd, "UPDATE %s SET la='%s', data='%s' "
                                            "WHERE name='%s'",
                                            tabName, la, data_string, cput);
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

                        } /* else von if (!abin[0]) */

                     } /* else von if (!kbin[0]) */

                  } /* else von if (!isla (la)) */

               } /* else von if (!cput[0]) */

            } /* else von if (len != 4) */

         } /* if (len > 0) */

         line++;

      } /* while (fgets (...)) */

      fclose (fp);
      fprintf (stderr, "\n%d CPUs eingetragen.\n", zaehler);

      break;

   case 'd': /* einen Eintrag loeschen */
      if (argc > 2)
         PRINT_USAGE;

      fprintf (stdout, "CPU-Typ: ");
      fflush (stdout);
      fgetstr (stdin, cput, help, sizeof(help));

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
      sprintf (cmd, "DELETE from %s WHERE name='%s'",
                    tabName, cput);

      res = PQexec (conn, cmd);
      if (res == (PGresult *) 0 || PQresultStatus (res) != 1) {
         fprintf (stderr, "FEHLER: exec von %s failed\n%s\n------\n",
                          cmd, PQerrorMessage (conn));
         PQreset (conn);
         exval = 2;
      } else {
         strcpy (cmd, PQcmdStatus (res));

         /* Antwort auswerten ("DELETE 1" oder "DELETE 0") */
         if ((ptr = strchr (cmd, ' ')) != (char *) 0 && *(++ptr) == '1')
            fprintf (stdout, "CPU %s geloescht.\n", cput);
         else {
            fprintf (stderr, "Fehler: %s konnte nicht geloescht werden.\n",
                             cput);
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

      fprintf (stdout, "CPU-Typ: ");
      fflush (stdout);
      fgetstr (stdin, cput, help, sizeof(help));

/*
 * Verbindung zur Datenbank aufbauen
 */
      DATENBANK_OEFFNEN(conn,pghost,pgport,pgoptions,pgtty,dbName);
#ifdef TEST
      fprintf (stdout, "Connection to database '%s' established.\n", dbName);
#endif

     len = sprintf (cmd, "SELECT rtrim(la,' '), rtrim(data,' ') "
                         "from %s WHERE name='%s'",
                         tabName, cput);
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
            strcpy (la, (char *) PQgetvalue (res, 0, 0));
            strcpy (data_string, (char *) PQgetvalue (res, 0, 1));
            fprintf (stderr, "CPU-Typ Ladeadresse "
                             "Kernelbinary Appl.binary\n");
            fprintf (stdout, "%s %s %s\n", cput, la, data_string);

            if (save)     /* Wenn gewuenscht, an savefile anhaengen */
            {
               if ((fp = fopen (savefilename, "a")) == (FILE *) 0)
               {
                  fprintf (stderr, "Fehler: Kann File %s nicht oeffnen\n",
                                   savefilename);
                  exval = 2;
               } else {
                  fprintf (fp, "%s %s %s\n", cput, la, data_string);
                  fclose (fp);
               }
            }
         } else {
            fprintf (stderr, "Fehler: %s konnte nicht gelesen werden\n", cput);
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
      len = sprintf (cmd, "SELECT rtrim(name,' '), rtrim(la,' '), "
                          "rtrim(data,' ') from %s", tabName);
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
            fprintf (stderr, "CPU-Typ Ladeadresse "
                             "Kernelbinary Appl.binary\n");
            for (i=0; i<tuples; ++i) {
               strcpy (cput, (char *) PQgetvalue (res, i, 0));
               strcpy (la, (char *) PQgetvalue (res, i, 1));
               strcpy (data_string, (char *) PQgetvalue (res, i, 2));
               fprintf (stdout, "%s %s %s\n", cput, la, data_string);
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
