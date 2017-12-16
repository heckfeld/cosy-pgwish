/*
 * $Header$
 */
/******************************************************************************/
/*                                                                            */
/* int tgtmgt (const char *name, char **erg);                                 */
/*                                                                            */
/* 'name' enthaelt aufzuloesende Majornummer oder Geraetenamen.               */
/* In 'erg' wird das Ergebnis geschrieben, oder eine Fehlermeldung.           */
/* Der Speicherbereich fuer 'erg' wird allokiert, und muss von dem rufenden   */
/* Programm wieder freigegeben werden.                                        */
/*                                                                            */
/* Rueckgabewert:                                                             */
/*    0  alles ok                                                             */
/*    1  Fehler                                                               */
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

#include <tgt_tgmtp.h>
#include "libpq-fe.h"


/* Namen der Tabellen in der Datenbank */
char *param = "param";
char *dev = "dev";


/*
 * Globale Variable, damit die Interrupt-Funktion auch darauf
 * zugreifen kann.
 */
PGconn *conn = (PGconn *) 0;


int tgtmgt (const char *name, char **erg)
{
   int retval = 0,
       devnr,
       i, j, len,
       tuples,
       status;
   char cmd[1024],
        string[1024];

   char *pgport = NULL,
        *pgoptions = NULL,
        *pgtty = NULL;

   char *ptr;
   char namecopy[1024],
        gruppe[256],
        geraet[256],
        parameter[256],
        badr[128],
        record[128],
        ddname[128];

   PGresult *res = (PGresult *) 0;

   if (erg == (char **) 0)
      return (1);

   /* Damit in 'name' keine Leerzeichen am Ende stehen !!! */
   memset (namecopy, '\0', sizeof(namecopy));
   strncpy (namecopy, name, sizeof(namecopy)-1);
   for (ptr=&namecopy[strlen(namecopy)-1]; isspace(*ptr); ptr--)
      ;
   *(++ptr) = '\0';

   /* Speicherplatz fuer das Ergebnis */
   *erg = (char *) malloc (1024);

   if (isdigit (*namecopy)) {
      devnr = strtol (namecopy, &ptr, 10);
      if (*ptr != '\0') {
         sprintf (*erg, "Ungueltige Majornummer.");
         return (1);
      }
      len = sprintf (cmd, "SELECT name from %s WHERE major='%d'",
                        dev, devnr);
/*
 * Verbindung zur Datenbank aufbauen
 */
      DATENBANK_OEFFNEN(conn,pghost,pgport,pgoptions,pgtty,dbName);
#ifdef TEST
      fprintf (stdout, "Connection to database '%s' established.\n", dbName);
#endif
#ifdef TEST
      fprintf (stdout, "cmd: %s\n", cmd);
#endif
      res = PQexec (conn, cmd);
      if (res == (PGresult *) 0
       || (status = PQresultStatus (res)) != PGRES_TUPLES_OK) {
#ifdef TEST
         fprintf (stderr, "Fehler beim Lesen aus der "
                          "Datenbank (%d).\n", status);
         fprintf (stderr, "%s\n", PQerrorMessage (conn));
#endif
         PQreset (conn);

      } else {
         if ((tuples = PQntuples (res)) > 0) {
            sprintf (*erg, "%s", (char *) PQgetvalue (res, 0, 0));
         } else {
            sprintf (*erg, "Treibername %s existiert nicht.", name);
            retval = 1;
         }
      }
      if (res != (PGresult *) 0) {
         PQclear (res);
         res = (PGresult *) 0;
      }

/*
 * Datenbank muss wieder geschlossen werden.
 */
      /* close the connection to the database and cleanup */
      PQfinish (conn);

   } else {
      if (sscanf (namecopy, "%[^.].%[^.].%[^.]",
       gruppe, geraet, parameter) != 3) {
         sprintf (*erg, "Ungueltiger Geraetename.");
         return (1);
      }
      len = sprintf (cmd, "SELECT data from %s WHERE "
                          "gruppe='%s' AND geraet='%s' AND param='%s'",
                          param, gruppe, geraet, parameter);

/*
 * Verbindung zur Datenbank aufbauen
 */
      DATENBANK_OEFFNEN(conn,pghost,pgport,pgoptions,pgtty,dbName);
#ifdef TEST
      fprintf (stdout, "Connection to database '%s' established.\n", dbName);
#endif
#ifdef TEST
      fprintf (stdout, "cmd: %s\n", cmd);
#endif
      res = PQexec (conn, cmd);
      if (res == (PGresult *) 0
       || (status = PQresultStatus (res)) != PGRES_TUPLES_OK) {
#ifdef TEST
         fprintf (stderr, "Fehler beim Lesen aus der "
                          "Datenbank (%d).\n", status);
         fprintf (stderr, "%s\n", PQerrorMessage (conn));
#endif
         PQreset (conn);

      } else {
         if ((tuples = PQntuples (res)) > 0) {
            strcpy (string, (char *) PQgetvalue (res, 0, 0));
            PQclear (res);
            res = (PGresult *) 0;

            /* Damit in 'string' keine Leerzeichen am Ende stehen !!! */
            for (ptr=&string[strlen(string)-1]; isspace(*ptr); ptr--)
               ;
            *(++ptr) = '\0';

          /* Die Daten muessen auseinandergepflueckt werden, da die    */
          /* Majornummer des Device-Treibers auch mit ausgegeben wird. */
            /* 1.Token: wird nicht gebraucht */
            (void) strtok (string, ".");
            /* 2. Token: Basisadresse */
            strcpy (badr, strtok ((char *) 0, "."));
            /* 3. - 10. Token */
            record[0] = '\0';
            for (i=3; i<=10; ++i) {
               strcat (record, strtok ((char *) 0, "."));
               strcat (record, ".");
            }
            /* 11. Token: Treibername */
            strcpy (ddname, strtok ((char *) 0, "."));

            len = sprintf (cmd, "SELECT major from %s WHERE name='%s'",
                              dev, ddname);
#ifdef TEST
            fprintf (stdout, "cmd: %s\n", cmd);
#endif
            res = PQexec (conn, cmd);
            if (res == (PGresult *) 0
             || (status = PQresultStatus (res)) != PGRES_TUPLES_OK) {
#ifdef TEST
               fprintf (stderr, "Fehler beim Lesen aus der "
                                "Datenbank (%d).\n", status);
               fprintf (stderr, "%s\n", PQerrorMessage (conn));
#endif
               PQreset (conn);

            } else {
               if ((tuples = PQntuples (res)) > 0) {
                  devnr = atoi ((char *) PQgetvalue (res, 0, 0));
               } else {
                  sprintf (*erg, "Treibername %s existiert nicht.", name);
                  retval = 1;
               }
            }
            if (res != (PGresult *) 0) {
               PQclear (res);
               res = (PGresult *) 0;
            }

            sprintf (*erg, "%s.%d.%s%s", badr, devnr, record, ddname);

         } else {
            sprintf (*erg, "Geraet/Parameter nicht in Datenbank enthalten");
            retval = 1;
         }
      }
      if (res != (PGresult *) 0) {
         PQclear (res);
         res = (PGresult *) 0;
      }

/*
 * Datenbank muss wieder geschlossen werden.
 */
      /* close the connection to the database and cleanup */
      PQfinish (conn);

   }

   return (retval);
}
