//
// $Header$
//

# ifndef PGDBIF_INCLUDED
#  define PGDBIF_INCLUDED

#  include <stdio.h>
#  include <stdarg.h>

#  include <pgpatch.h>
#  include "libpq-fe.h"


#  define DELIM_DEFAULT ':'
#  define SEPAR_DEFAULT " "


//
// Class pgdbif
// ------------
//
   class pgdbif {
   protected:
      int delim;           // Delimiter zwischen einzelnen Spalte bzw.
                           // den Spaltennamen und ihren Werten

      char *separ;         // Field-Separator (zwischen den einzelnen Spalten)
                           // fuer die Ausgabe

      char *dbName;        // Name der Postgres-Datenbank

      PGconn *conn;        // fuer Verbindung mit der Datenbank
      PGresult *res;       // fuer Ergebnis eines Kommandos

      FILE *fpout;         // wohin bei print geschrieben wird
      int   openflag;

      char cmd[1024];      // statisches Memory fuer ein Kommando
      char erg[1024];      // statisches Memory fuer ein Ergebnis
      char erg_field[1024];// statisches Memory fuer eine Spalte eines Tupels
      char error[1024];    // statisches Memory fuer Fehlermeldung in exec_cmd

      int verbose;         // Flag, ob Fehlermeldung auf stderr geschrieben
                           // werden soll

      int fields;          // Anzahl der Felder nach SELECT
      int tuples;          // Anzahl der Tupels nach SELECT
      int akt_tuple_ind;   // Index des zuletzt gelesenen Tupels

   public:
      pgdbif ();
      pgdbif (char *, FILE * = stdout);
      ~pgdbif ();

      PGconn *open (char *, FILE * = stdout);
      PGconn *get_conn ();
      int isopen ();
      void close ();

      char *get_dbName();
      char *get_error();

      int exec_cmd (char *, int, int = -1);
      PGresult *get_res ();
      int get_resstat ();
      int get_delstat ();
      int set_tuples ();
      int get_tuples ();
      int get_fields ();

      void set_delim (int);
      int get_delim ();

      void set_separ (char *);
      char *get_separ ();

      void set_verbose ();
      void set_nonverbose ();
      int isverbose ();

      char *dbget (char *, char *, int, ...);
      char *get_erg_field (int, int);
      char *get_next_tuple ();
      int dbput (char *, char *, int, ...);
      int dbdel (char *, int, ...);

      virtual void dbprint (char *, int, ...);

   private:
      char *pghost;        // Hier soll der Inhalt der PGHOST
                           // Environment-Variablen verwendet werden

      char *pgport;
      char *pgoptions;
      char *pgtty;

      int status;          // PQresultStatus
      int delstat;         // PQcmdStatus fuer DELETE

      void init ();
      char *rtrim (char *);
      char *find_first_field (char *, int, char **);
   };


# endif

