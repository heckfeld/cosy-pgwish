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

#include "libpq-fe.h"

#include "tgt_tgmtp.h"
#include "tgt_utilsp.h"
#include "tgt_paramp.h"


/*
 * Nur der Name des Programms soll bei der Usage mit ausgegeben werden.
 */
#define basename(name) (((ptr = strrchr (name, '/')) != (char *) 0) \
                        ? ptr + 1 : name)

#define PRINT_USAGE { \
   fprintf (stdout, "usage: %s -{i|f <file>|d|l[ -s <file>]|a} <gruppe>\n", \
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
   char help[256], string[256], data_string[256];
   char gruppe[128],        /* Gruppe */
        geraet[128],        /* Geraet */
        param[128];         /* Parameter */
   char tname[128],         /* Targetname */
        na[64],             /* Nodeadresse der Karte (hexadezimal) */
        fbi[64],            /* FBi (dezimal), Fieldbus-Interface */
        station[64],        /* Stationsnummer (hexadezimal) */
        baseadr[64],        /* Baseadresse im G64-Crate (hexadezimal) */
        subadr[64],         /* Subadresse (hexadezimal) */
        mask[5][64],        /* Maske (hexadezimal) */
        dd[64];             /* Treibername */
   char savefilename[256];

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

   memset (gruppe, '\0', sizeof (gruppe));
   memset (geraet, '\0', sizeof (geraet));
   memset (param, '\0', sizeof (param));
   memset (tname, '\0', sizeof (tname));
   memset (na, '\0', sizeof (na));
   memset (fbi, '\0', sizeof (fbi));
   memset (station, '\0', sizeof (station));
   memset (baseadr, '\0', sizeof (baseadr));
   memset (subadr, '\0', sizeof (subadr));
   memset (mask, '\0', sizeof (mask));
   memset (dd, '\0', sizeof (dd));
   memset (savefilename, '\0', sizeof (savefilename));

/*
 * Die Argumente auswerten.
 */
   if (argc < 3 || argv[1][0] != '-' || argv[argc-1][0] == '-')
      PRINT_USAGE;

   /* Letztes Argument ist der Name der Gruppe */
   strcpy (gruppe, argv[argc-1]);

/* Es ist eine Option angegeben. */
/* Entsprechend abarbeiten. */

   switch (argv[1][1])
   {
   case 'i': /* ein Target interaktiv eintragen */
      if (argc > 3)
         PRINT_USAGE;

      fprintf (stdout, "Geraet: ");
      fflush (stdout);
      fgetstr (stdin, geraet, help, sizeof(help));
      if (!geraet[0]) {
         fprintf (stderr, "Ungueltiges Geraet.\n");
         exit (2);
      }

      fprintf (stdout, "Parameter: ");
      fflush (stdout);
      fgetstr (stdin, param, help, sizeof(help));
      if (!param[0]) {
         fprintf (stderr, "Ungueltiger Parameter.\n");
         exit (2);
      }

      fprintf (stdout, "Targetname: ");
      fflush (stdout);
      fgetstr (stdin, tname, help, sizeof(help));
      if (!tname[0]) {
         fprintf (stderr, "Ungueltiges Target.\n");
         exit (2);
      }

      fprintf (stdout, "Nodeadresse (hexadezimal): ");
      fflush (stdout);
      fgetstr (stdin, na, help, sizeof(help));
      if (!ishexzahl (na)) {
         fprintf (stderr, "Ungueltige Nodeadresse.\n");
         exit (2);
      }

      fprintf (stdout, "FBi (dezimal): ");
      fflush (stdout);
      fgetstr (stdin, fbi, help, sizeof(help));
      if (!isposint0 (fbi)) {
         fprintf (stderr, "Ungueltiges FBi.\n");
         exit (2);
      }

      fprintf (stdout, "Stationsnummer (hexadezimal): ");
      fflush (stdout);
      fgetstr (stdin, station, help, sizeof(help));
      if (!ishexzahl (station)) {
         fprintf (stderr, "Ungueltige Stationsnummer.\n");
         exit (2);
      }

      fprintf (stdout, "Baseadresse im G64-Crate (hexadezimal): ");
      fflush (stdout);
      fgetstr (stdin, baseadr, help, sizeof(help));
      if (!ishexzahl (baseadr)) {
         fprintf (stderr, "Ungueltige Baseadresse.\n");
         exit (2);
      }

      fprintf (stdout, "Subadresse (physikalisch, hexadezimal): ");
      fflush (stdout);
      fgetstr (stdin, subadr, help, sizeof(help));
      if (!ishexzahl (subadr)) {
         fprintf (stderr, "Ungueltige Subadresse.\n");
         exit (2);
      }

      fprintf (stdout, "Maske (nr.nr.nr.nr, nr hexadezimal): ");
      fflush (stdout);
      fgetstr (stdin, mask[0], help, sizeof(help));
      if (!ismask (mask, 0)) {
         fprintf (stderr, "Ungueltige Maske.\n");
         exit (2);
      }

      fprintf (stdout, "Treibername: ");
      fflush (stdout);
      fgetstr (stdin, dd, help, sizeof(help));
      if (!dd[0]) {
         fprintf (stderr, "Ungueltiger Treibername.\n");
         exit (2);
      }

      sprintf (data_string, "%s.%s.%s.%s.%s.%s.%s.%s.%s.%s.%s",
                       tname, na, fbi, station, baseadr, subadr,
                       mask[1], mask[2], mask[3], mask[4], dd);
      fprintf (stdout, "\n%s.%s %s\n",
                       geraet, param, data_string);

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
                    tabName, gruppe, geraet, param, data_string);
      res = PQexec (conn, cmd);
      if (res == (PGresult *) 0 || PQresultStatus (res) != 1) {
         PQreset (conn);
         if (res != (PGresult *) 0) {
            PQclear (res);
            res = (PGresult *) 0;
         }

         sprintf (cmd, "UPDATE %s SET data='%s' "
                       "WHERE gruppe='%s' AND geraet='%s' AND param='%s'",
                       tabName, data_string, gruppe, geraet, param);
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

   case 'f': /* mehrere Geraete aus einem File eintragen */
      if (argc != 4)
         PRINT_USAGE;

      if ((fp = fopen (argv[2], "r")) == (FILE *) 0)
      {
         fprintf (stdout, "Datei \"%s\" konnte nicht geoeffnet werden.\n",
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

         memset (gruppe, '\0', sizeof (gruppe));
         memset (geraet, '\0', sizeof (geraet));
         memset (param, '\0', sizeof (param));
         memset (data_string, '\0', sizeof (data_string));

         len = sscanf (help, "%s%s%s%s", gruppe, geraet, param, string);

/* Vollstaendigkeit/Korrektheit der Daten ueberpruefen. */
         if (len > 0) {
            if (len != 4) {
               fprintf (stderr, "- %s (Zeile %d): Fehlerhafte Eingabe.\n",
                                 help, line);
            } else {
               if (!gruppe[0]) {
                  fprintf (stderr, "- %s (Zeile %d): Ungueltige Gruppe.\n",
                                   help, line);
               } else {

                  if (!geraet[0]) {
                     fprintf (stderr, "- %s (Zeile %d): Ungueltiges "
                                      "Geraet.\n",
                                      help, line);
                  } else {

                     if (!param[0]) {
                        fprintf (stderr, "- %s (Zeile %d): Ungueltiger "
                                         "Parameter.\n",
                                         help, line);
                     } else {

                        strcpy (data_string, string);
                        /*  data_string auf die Variablen verteilen */

                        strcpy (tname, strtok (string, "."));
                        strcpy (na, strtok ((char *) 0, "."));
                        strcpy (fbi, strtok ((char *) 0, "."));
                        strcpy (station, strtok ((char *) 0, "."));
                        strcpy (baseadr, strtok ((char *) 0, "."));
                        strcpy (subadr, strtok ((char *) 0, "."));
                        strcpy (mask[1], strtok ((char *) 0, "."));
                        strcpy (mask[2], strtok ((char *) 0, "."));
                        strcpy (mask[3], strtok ((char *) 0, "."));
                        strcpy (mask[4], strtok ((char *) 0, "."));
                        strcpy (dd, strtok ((char *) 0, "."));

                        if (!tname[0] || !ishexzahl (na) ||
                            !isposint0 (fbi) ||
                            !ishexzahl (station) || !ishexzahl (baseadr) ||
                            !ishexzahl (subadr) || !ismask (mask, 1) ||
                            !dd[0]) {
                           fprintf (stderr, "- %s (Zeile %d): Ungueltiger "
                                            "Eintrag.\n",
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
                           fprintf (stdout, "%s.%s %s\n", geraet, param,
                                                          data_string);

                           sprintf (cmd,
                                   "INSERT into %s VALUES "
                                   "('%s', '%s', '%s', '%s')",
                                   tabName, gruppe, geraet, param, data_string);
                           res = PQexec (conn, cmd);
                           if (res == (PGresult *) 0
                            || PQresultStatus (res) != 1) {
                              PQreset (conn);
                              if (res != (PGresult *) 0) {
                                 PQclear (res);
                                 res = (PGresult *) 0;
                              }

                              sprintf (cmd, "UPDATE %s "
                                            "SET data='%s' "
                                            "WHERE gruppe='%s' AND "
                                            "geraet='%s' AND param='%s'",
                                            tabName, data_string,
                                            gruppe, geraet, param);
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

                        } /* else von if (!tname[0]) || ...) */

                     } /* else von if (!param[0]) */

                  } /* else von if (!geraet[0]) */

               } /* else von if (!gruppe[0]) */

            } /* else von if (len != 4) */

         } /* if (len > 0) */

         line++;

      } /* while (fgets (...)) */

      fclose (fp);
      fprintf (stdout, "\n%d Geraete eingetragen.\n", zaehler);

      break;

   case 'd': /* einen Eintrag loeschen */
      if (argc > 3)
         PRINT_USAGE;

      fprintf (stdout, "Geraet: ");
      fflush (stdout);
      fgetstr (stdin, geraet, help, sizeof(help));

      fprintf (stdout, "Parameter: ");
      fflush (stdout);
      fgetstr (stdin, param, help, sizeof(help));

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
      sprintf (cmd, "DELETE from %s WHERE "
                    "gruppe='%s' AND geraet='%s' AND param='%s'",
                    tabName, gruppe, geraet, param);

      res = PQexec (conn, cmd);
      if (res == (PGresult *) 0 || PQresultStatus (res) != 1) {
         fprintf (stderr, "FEHLER: exec von %s failed\n%s\n------\n",
                          cmd, PQerrorMessage (conn));
         PQreset (conn);
         exval = 2;
      } else {
         strcpy (cmd, PQcmdStatus (res));
         if ((ptr = strchr (cmd, ' ')) != (char *) 0 && *(++ptr) == '1')
            fprintf (stdout, "Geraet %s.%s geloescht\n", geraet, param);
         else {
            fprintf (stderr, "Fehler: %s.%s konnte nicht geloescht werden\n",
                             geraet, param);
            exval = 2;
         }
      }
      if (res != (PGresult *) 0) {
         PQclear (res);
         res = (PGresult *) 0;
      }

      break;

   case 'l': /* einen Eintrag anzeigen */
      if (argc > 3) {
         if (argc != 5 || strcmp (argv[2], "-s") != 0)
            PRINT_USAGE;
         save = 1;
         strcpy (savefilename, argv[3]);
      }

      fprintf (stdout, "Geraet: ");
      fflush (stdout);
      fgetstr (stdin, geraet, help, sizeof(help));

      fprintf (stdout, "Parameter: ");
      fflush (stdout);
      fgetstr (stdin, param, help, sizeof(help));

/*
 * Verbindung zur Datenbank aufbauen
 */
      DATENBANK_OEFFNEN(conn,pghost,pgport,pgoptions,pgtty,dbName);
#ifdef TEST
      fprintf (stdout, "Connection to database '%s' established.\n", dbName);
#endif

     len = sprintf (cmd, "SELECT rtrim(data,' ') from %s "
                         "WHERE gruppe='%s' AND geraet='%s' AND param='%s'",
                         tabName, gruppe, geraet, param);
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
            fprintf (stderr, "Zu %s.%s gehoerender Eintrag:\n", geraet, param);
            fprintf (stdout, "%s %s %s %s\n",
                             gruppe, geraet, param, data_string);

            if (save)     /* Wenn gewuenscht, an savefile anhaengen */
            {
               if ((fp = fopen (savefilename, "a")) == (FILE *) 0)
               {
                  fprintf (stderr, "Fehler: Kann File %s nicht oeffnen\n",
                                   savefilename);
                  exval = 2;
               } else {
                  fprintf (fp, "%s %s %s %s\n",
                               gruppe, geraet, param, data_string);
                  fclose (fp);
               }
            }
         } else {
            fprintf (stderr, "Datenbank fuer Gruppe %s:\n", gruppe);
            fprintf (stderr, "Fehler: %s.%s konnte nicht gelesen werden\n",
                              geraet, param);
            exval = 2;
         }
      }
      if (res != (PGresult *) 0) {
         PQclear (res);
         res = (PGresult *) 0;
      }

      break;

   case 'a': /* gesamten Datenbankinhalt ausgeben */
      if (argc > 3)
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
      len = sprintf (cmd, "SELECT rtrim(gruppe,' '), rtrim(geraet,' '), "
                          "rtrim(param,' '), rtrim(data,' ') from %s "
                          "WHERE gruppe='%s'",
                          tabName, gruppe);
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
            for (i=0; i<tuples; ++i) {
               strcpy (gruppe, (char *) PQgetvalue (res, i, 0));
               strcpy (geraet, (char *) PQgetvalue (res, i, 1));
               strcpy (param, (char *) PQgetvalue (res, i, 2));
               strcpy (data_string, (char *) PQgetvalue (res, i, 3));
               fprintf (stdout, "%s %s %s %s\n",
                          gruppe, geraet, param, data_string);
            }
         }
         else 
            fprintf (stderr, "Die Datenbank fuer Gruppe %s ist leer.\n",
                         gruppe);
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
