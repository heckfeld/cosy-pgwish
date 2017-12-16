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
#include <tgt_thostsp.h>
#include "libpq-fe.h"


/*
 * Nur der Name des Programms soll bei der Usage mit ausgegeben werden.
 */
#define basename(name) (((ptr = strrchr (name, '/')) != (char *) 0) \
                        ? ptr + 1 : name)

#define PRINT_USAGE { \
   fprintf (stdout, "usage: %s -{i|f <file>|d|l[ -s <file>]|a|e}\n", \
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
   char help[256], string[256], data_string[256];
   char tname[128], ia[128], ea[128], wc[128], savefilename[256];

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

   memset ((char *) &tinfo, '\0', sizeof (tinfo));
   memset (data_string, '\0', sizeof (data_string));
   memset (tname, '\0', sizeof (tname));
   memset (ia, '\0', sizeof (ia));
   memset (ea, '\0', sizeof (ea));
   memset (wc, '\0', sizeof (wc));

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
      if (tinfo.n > 1) {
         for (i=0; i<tinfo.n; ++i) {
            fprintf (stdout, "%d: Internetaddresse: %s\n",
                              i+1, tinfo.iaddr[i]);
         }
         fprintf (stdout, "Auswahl der Internetadresse (1 - %d): ", tinfo.n);
         fflush (stdout);
         fgetstr (stdin, string, help, sizeof(help));
         if (!(*string))
            strcpy (ia, tinfo.iaddr[0]);
         else {
            wert = strtod (string, &ptr);
            if (ptr < &string[strlen(string)]
             || wert < 1
             || wert > tinfo.n) {
               fprintf (stderr, "Falsche Auswahl.\n");
               exit (2);
            } else {
               strcpy (ia, tinfo.iaddr[wert-1]);
               if (wert > 1) {
                  tinfo.subn[0] = tinfo.subn[wert-1];
                  strcpy (tinfo.iaddr[0], tinfo.iaddr[wert-1]);
               }
            }
         }
      } else {
         strcpy (ia, tinfo.iaddr[0]);
      }
      /* An der ersten Stelle steht nun die gewaehlte Internetadresse. */
      tinfo.n = 1;
      fprintf (stdout, "Internetaddresse: %s\n", ia);

      fprintf (stdout, "Ethernetadresse: ");
      fflush (stdout);
      fgetstr (stdin, ea, help, sizeof(help));
      if (!isea (ea)) {
         fprintf (stderr, "Ungueltige Ethernetadresse.\n");
         exit (2);
      }

      fprintf (stdout, "Workcell: ");
      fflush (stdout);
      fgetstr (stdin, wc, help, sizeof(help));
      if (!iswc (wc, &tinfo)) {
         fprintf (stderr, "Workcell kann das Target nicht erreichen.\n");
         exit (2);
      }

      sprintf (data_string, "%s %s %s", ia, ea, wc);
      fprintf (stdout, "\n%s %s\n", tname, data_string);

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

      sprintf (cmd, "INSERT into %s VALUES ('%s', '%s')",
                    tabName, tname, data_string);
      res = PQexec (conn, cmd);
      if (res == (PGresult *) 0 || PQresultStatus (res) != 1) {
         PQreset (conn);

         if (res != (PGresult *) 0) {
            PQclear (res);
            res = (PGresult *) 0;
         }

         sprintf (cmd, "UPDATE %s SET data='%s' WHERE name='%s'",
                       tabName, data_string, tname);
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
         memset (ia, '\0', sizeof (ia));
         memset (ea, '\0', sizeof (ea));
         memset (wc, '\0', sizeof (wc));

         /* Internetadresse aus Datei ignorieren. */
         /* Ist nur aus Kompatibilitaetsgruenden noch enthalten. */
         len = sscanf (help, "%s%s%s%s", tname, ia, ea, wc);

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
                  /* Es wird immer die erste Internetadresse genommen. */
                  strcpy (ia, tinfo.iaddr[0]);
                  if (tinfo.n > 1) {
                     fprintf (stderr, "- %s (Zeile %d): Mehrere "
                                      "Internetadresen "
                                      "(erste wird genommen)\n",
                                      help, line);
                     tinfo.n = 1;
                  }

                  if (!isea (ea)) {
                     fprintf (stderr, "- %s (Zeile %d): Ungueltige "
                                      "Ethernetadresse.\n",
                                      help, line);
                  } else {

                     if (!iswc (wc, &tinfo)) {
                        fprintf (stderr, "- %s (Zeile %d): Workcell kann das "
                                         "Target nicht erreichen.\n",
                                         help, line);
                     } else {
/*
 * In die Datenbank eintragen.
 * Erst INSERT versuchen, wenn gescheitert, dann UPDATE.
 */
#ifdef TEST
                        fprintf (stdout, "Werte in die Datenbank eintragen.\n");
#endif
                        sprintf (data_string, "%s %s %s", ia, ea, wc);
                        fprintf (stdout, "%s %s\n", tname, data_string);

                        sprintf (cmd, "INSERT into %s VALUES ('%s', '%s')",
                                      tabName, tname, data_string);
                        res = PQexec (conn, cmd);
                        if (res == (PGresult *) 0
                         || PQresultStatus (res) != 1) {
                           PQreset (conn);

                           if (res != (PGresult *) 0) {
                              PQclear (res);
                              res = (PGresult *) 0;
                           }

                           sprintf (cmd, "UPDATE %s SET data='%s' WHERE "
                                         "name='%s'",
                                         tabName, data_string, tname);
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

                     } /* else von if (!iswc (wc, &tinfo)) */

                  } /* else von if (!isea (ea)) */

               } /* else von if (!istarget(tname, &tinfo) || ... ) */

            } /* else von if (len != 4) */

         } /* if (len > 0) */

         line++;

      } /* while (fgets (...)) */

      fclose (fp);
      fprintf (stderr, "\n%d Targets eingetragen.\n", zaehler);

      break;

   case 'd': /* einen Eintrag loeschen */
      if (argc > 2)
         PRINT_USAGE;

      fprintf (stdout, "Targetname: ");
      fflush (stdout);
      fgetstr (stdin, tname, help, sizeof(help));

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
                    tabName, tname);

      res = PQexec (conn, cmd);
      if (res == (PGresult *) 0 || PQresultStatus (res) != 1) {
         fprintf (stderr, "FEHLER: exec von %s failed\n%s\n------\n",
                          cmd, PQerrorMessage (conn));
         PQreset (conn);
         exval = 2;
      } else {
         strcpy (cmd, PQcmdStatus (res));
         if ((ptr = strchr (cmd, ' ')) != (char *) 0 && *(++ptr) == '1')
            fprintf (stdout, "Target %s geloescht.\n", tname);
         else {
            fprintf (stderr, "Fehler: %s konnte nicht geloescht werden.\n",
                             tname);
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

/*
 * Verbindung zur Datenbank aufbauen
 */
      DATENBANK_OEFFNEN(conn,pghost,pgport,pgoptions,pgtty,dbName);
#ifdef TEST
      fprintf (stdout, "Connection to database '%s' established.\n", dbName);
#endif

     len = sprintf (cmd, "SELECT rtrim(data,' ') from %s WHERE name='%s'",
                         tabName, tname);
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
            strcpy (data_string, (char *) PQgetvalue (res, 0, 0));
            fprintf (stderr, "Targetname IA EA WC\n");
            fprintf (stdout, "%s %s\n", tname, data_string);

            if (save)     /* Wenn gewuenscht, an savefile anhaengen */
            {
               if ((fp = fopen (savefilename, "a")) == (FILE *) 0)
               {
                  fprintf (stderr, "Fehler: Kann File %s nicht oeffnen\n",
                                   savefilename);
                  exval = 2;
               } else {
                  fprintf (fp, "%s %s\n", tname, data_string);
                  fclose (fp);
               }
            }
         } else {
            fprintf (stderr, "Fehler: %s konnte nicht gelesen werden\n", tname);
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
      len = sprintf (cmd, "SELECT rtrim(name,' '), rtrim(data,' ') "
                          "from %s", tabName);
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
            fprintf (stderr, "Targetname IA EA WC\n");
            for (i=0; i<tuples; ++i) {
               strcpy (tname, (char *) PQgetvalue (res, i, 0));
               strcpy (data_string, (char *) PQgetvalue (res, i, 1));
               fprintf (stdout, "%s %s\n", tname, data_string);
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

   case 'e': /* Ethernetadresse eines Eintrags veraendern */
      if (argc > 2)
         PRINT_USAGE;

      fprintf (stdout, "Targetname: ");
      fflush (stdout);
      fgetstr (stdin, tname, help, sizeof(help));

/*
 * Verbindung zur Datenbank aufbauen
 */
      DATENBANK_OEFFNEN(conn,pghost,pgport,pgoptions,pgtty,dbName);
#ifdef TEST
      fprintf (stdout, "Connection to database '%s' established.\n", dbName);
#endif

      len = sprintf (cmd, "SELECT rtrim(data,' ') from %s WHERE name='%s'",
                          tabName, tname);
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
            strcpy (data_string, (char *) PQgetvalue (res, 0, 0));
            PQclear (res);

            fprintf (stdout, "%s\n", data_string);
            sscanf (data_string, "%s%s%s", ia, ea, wc);

            fprintf (stdout, "Neue Ethernetadresse: ");
            fflush (stdout);
            fgetstr (stdin, string, help, sizeof(help));
            if (isea (string)) {
               sprintf (data_string, "%s %s %s", ia, string, wc);
               sprintf (cmd, "UPDATE %s SET data='%s' WHERE name='%s'",
                             tabName, data_string, tname);

               res = PQexec (conn, cmd);
               if (res == (PGresult *) 0 || PQresultStatus (res) != 1) {
                  fprintf (stderr, "FEHLER: exec von %s failed\n%s\n------\n",
                                   cmd, PQerrorMessage (conn));
                  PQreset (conn);
                  exval = 2;
               }
            } else {
               fprintf (stderr, "Fehlerhafte Eingabe.\n");
               exval = 2;
            }
         } else {
            fprintf (stderr, "Fehler: %s konnte nicht gelesen werden\n", tname);
            exval = 2;
         }
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
