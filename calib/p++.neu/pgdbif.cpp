//
// $Header$
//

# include <pgdbif.h>

# include <string.h>
# include <stdlib.h>
# include <ctype.h>


////////////////////////////////////////////////////////////////////////////////
//
// PRIVATE FUNCTIONS
//
////////////////////////////////////////////////////////////////////////////////

//
// Gemeinsame Initialisierung fuer alle Construktoren
//
void pgdbif::init () {
   char *env;

   // Den Postgres-Host explizit entsprechend der
   // Environment-Variablen PGHOST setzen
   env = getenv ("PGHOST");

#ifdef TEST
if (env != (char *) 0)
 printf ("PGHOST='%s'\n", env);
else
 printf ("PGHOST ist nicht gesetzt\n");
#endif

   if (env == (char *) 0)
      pghost = (char *) 0;
   else {
      // getenv() gibt Zeiger auf statisches Memory zurueck.
      // Der aktuelle Wert kann durch weitere Aufrufe ueberschrieben werden.
      // Deshalb muss das Ergebnis kopiert werden.
      pghost = (char *) malloc (strlen (env) + 1);
      strcpy (pghost, env);
   }

   pgport = NULL;
   pgoptions = NULL;
   pgtty = NULL;

   res = (PGresult *) 0;

   delim = DELIM_DEFAULT;
   separ = SEPAR_DEFAULT;

   conn = (PGconn *) 0;
   dbName = (char *) 0;
   openflag = 0;

   verbose = 0;

   fields = -1;
   tuples = -1;
   akt_tuple_ind = -1;

   status = -1;

   return;

} // void pgdbif::init


//
// Bei dem uebergebenen String werden abschliessende White-Spaces entfernt.
// Der uebergebene String wird zurueckgegeben.
//
char *pgdbif::rtrim (char *s) {
   char *ptr;

   ptr = &s[strlen(s) - 1];
   while (isspace (*ptr))
      ptr--;
   *(++ptr) = '\0';

   return (s);
}


//
// Nach dem ersten Feld wird '\0' in den uebergebenen String geschrieben.
// Der Zeiger auf den Rest hinter dem '\0' wird in *e zurueckgegeben.
//
char *pgdbif::find_first_field (char *s, int del, char **e) {
   char *ptr;

   if (e == (char **) 0)
      return (char *) 0;

   ptr = s;
   while (*ptr && *ptr != del)
      ptr++;
   if (*ptr) {
      *ptr = '\0';
      *e = ++ptr;
   } else {
      *e = ptr;
   }

   return (s);
}

////////////////////////////////////////////////////////////////////////////////
//
// PUBLIC FUNCTIONS
//
////////////////////////////////////////////////////////////////////////////////

// Constructor
// -----------
pgdbif::pgdbif () {

#ifdef TEST
printf ("&v4Spgdbif constructor ohne\n");
#endif

   init ();
}

pgdbif::pgdbif (char *n, FILE *fp) {

#ifdef TEST
printf ("&v4Spgdbif constructor\n");
#endif

   init ();

   (void) open (n, fp);

} // pgdbif::pgdbif (char *n, FILE *fp)


// Destructor
// ----------
pgdbif::~pgdbif () {
   free (pghost);

   if (isopen ())
   {
      close ();
   }

#ifdef TEST
printf ("&v4Spgdbif destructor\n");
#endif

}


// Open Database
// -------------
PGconn *pgdbif::open (char *n, FILE *fp) {

#ifdef TEST
printf ("&v4Spgdbif open (%s)\n", n);
#endif

   fpout = fp;

   // Wenn genau diese Datenbank bereits geoeffnet ist, den entsprechenden
   // Pointer zurueckgeben.
   // Ist die Datenbank zwar geoeffnet, der Name stimmt aber nicht ueberein,
   // wird ein Null-Pointer zurueckgegeben.
   if (isopen ()) {
      if (strcmp (n, dbName) != 0)
         return ((PGconn *) 0);

   } else {

      dbName = (char *) malloc (strlen (n) + 1);  // zzgl. abschliessendes '\0'
      strcpy (dbName, n);

      // Konnte die Datenbank geoeffnet werden?
      conn = PQsetdb (pghost, pgport, pgoptions, pgtty, dbName); 
      if (PQstatus (conn) != CONNECTION_OK)
      {
#ifdef TEST
printf ("&v4Spgdbif constructor: Connection to database '%s' failed\n",
        dbName);
#endif
         conn = (PGconn *) 0;
         free (dbName);
         dbName = (char *) 0;
      }
      else
         openflag = 1;
   }

   return (conn);

} // PGconn *pgdbif::open


// Get_Conn
// --------
PGconn *pgdbif::get_conn () {

   return (conn);

} // PGconn *pgdbif::get_conn


// Is_Open
// -------
int pgdbif::isopen () {

   return (openflag);

} // int pgdbif::isopen


// Close
// ------
void pgdbif::close () {

#ifdef TEST
printf ("&v4Spgdbif close\n");
#endif

   if (isopen ())
   {
      PQfinish (conn);
      conn = (PGconn *) 0;
      free (dbName);
      dbName = (char *) 0;
      openflag = 0;

      res = (PGresult *) 0;

      fields = -1;
      tuples = -1;
      akt_tuple_ind = -1;

      status = -1;
   }

} // void pgdbif::close


// Get DbName
// ----------
char *pgdbif::get_dbName () {

   return (dbName);

} // char *pgdbif::get_dbName


// Get Error
// ---------
char *pgdbif::get_error () {

   return (error);

} // char *pgdbif::get_error


// Execute Command
// ---------------
// Rueckgabewert 0: alles ok
//               1: Datenbank nicht geoeffnet
//               2: Fehler bei PQexec-Aufruf
int pgdbif::exec_cmd (char *cmd_str, int status_flag, int line) {

#ifdef TEST
printf ("&v4SIn Funktion exec_cmd ...\n");
#endif

   int retval = 0;
   int len = 0;

   if (isopen ()) {
      res = PQexec (conn, cmd_str);
      if (get_resstat () != status_flag) {
         if (line >= 0)
            len = sprintf (error, "Zeile %3d: ", line);
         sprintf (&error[len], "FEHLER: exec von \"%s\" failed (%d)\n"
                          "-->Postgres: %s",
                          cmd_str, status, PQerrorMessage (conn));
         PQreset (conn);
         retval = 2;
      }
      else {
         if (status_flag == PGRES_TUPLES_OK)
            set_tuples ();
      }
   }
   else
      retval = 1;

   return (retval);

} // int pgdbif::exec_cmd


// Get_Res
// -------
PGresult *pgdbif::get_res () {

   return (res);

} // PGresult *pgdbif::get_res


// Get_Result_Status
// -----------------
int pgdbif::get_resstat () {

   status = -1;
   if (res != (PGresult *) 0)
       status = PQresultStatus (res);

   return (status);

} // int pgdbif::get_resstat


// Get_Delete_Status
// -----------------
int pgdbif::get_delstat () {
   char *statptr, *ptr;

   delstat = -1;
   if (res != (PGresult *) 0) {
      statptr = PQcmdStatus (res);
      if (strncmp (statptr, "DELETE", strlen ("DELETE")) == 0) {
         delstat = ((ptr = strchr (statptr, ' ')) != (char *) 0
                   && *(++ptr) == '1');
      }
   }

   return (delstat);

} // int pgdbif::get_delstat


// Set Tuples
// ----------
int pgdbif::set_tuples () {
   fields = -1;
   tuples = -1;
   akt_tuple_ind = -1;

   if (res != (PGresult *) 0) {
       tuples = PQntuples (res);
       fields = PQnfields (res);
   }

   return (tuples);

} // int pgdbif::set_tuples


// Get Tuples
// ----------
int pgdbif::get_tuples () {

   return (tuples);

} // int pgdbif::get_tuples


// Get Fields
// ----------
int pgdbif::get_fields () {

   return (fields);

} // int pgdbif::get_fields


// Set Delimiter
// -------------
void pgdbif::set_delim (int d) {

   delim = d;

} // void pgdbif::set_delim


// Get Delimiter
// -------------
int pgdbif::get_delim () {

   return (delim);

} // int pgdbif::get_delim


// Set Separator
// -------------
void pgdbif::set_separ (char *s) {

   separ = s;

} // void pgdbif::set_separ


// Get Separator
// -------------
char *pgdbif::get_separ () {

   return (separ);

} // char *pgdbif::get_separ


// Set Verbose
// -----------
void pgdbif::set_verbose () {

   verbose = 1;

} // void pgdbif::set_verbose


// Set Non Verbose
// ---------------
void pgdbif::set_nonverbose () {

   verbose = 0;

} // void pgdbif::set_nonverbose


// Is Verbose
// ----------
int pgdbif::isverbose () {

   return (verbose);

} // int pgdbif::isverbose


// Dbget
// -----
// tab : Name der Tabelle
// s   : String mit Namen der auszulesenden Spalten, durch delim getrennt
// nk  : Anzahl der nachfolgenden Schluessel-Daten-Paare
//       (Schluessel und Daten sind durch delim getrennt)
// Es werden nur die Daten des ersten Tupels zurueckgegeben !!!
char *pgdbif::dbget (char *tab, char *s, int nk, ...) {

#ifdef TEST
printf ("&v4SIn Funktion dbget ...\n");
#endif

// Da auch konstante Character-Strings uebergeben werden koennen, aber
// in s und in die Schluessel-Daten-Paare geschrieben wird, erst eine Kopie
// der Strings machen und damit weiterarbeiten.

   va_list argptr;
   char *kptr, *token, del[2];
   char help[1024];
   char spalte[1024],         // fuer Kopie von s
        keypair[1024];        // fuer Kopie der Schluessel-Daten-Paare
   int  lenvgl, len, i;

   if (isopen ())
   {
      if (tab != (char *) 0 && *tab
       && s != (char *) 0 && *s
       && nk > 0) {
#ifdef TEST
printf ("&v4SDaten aus Spalte %s fuer %d Schluessel lesen\n", s, nk);
#endif
         strcpy (spalte, s);

         // Weil strtok() einen String als Delimiter erwartet.
         del[0] = delim;
         del[1] = '\0';

         // Welche Spalten sollen gelesen werden?
         if (spalte[0] == '*') {
            len = sprintf (cmd, "SELECT *");
         } else {
            lenvgl = strlen (spalte);
            len = sprintf (cmd, "SELECT %s", strtok (spalte, del));

         // Ist vielleicht nicht mehr notwendig, da mit Kopie gearbeitet wird.
         // Aus Sicherheitsgruenden, da noch nicht getestet, erstmal drinlassen.
            spalte[strlen(spalte)] = del[0];   // alten Zustand
                                               // wiederherstellen !!!

            while ((token = strtok ((char *) 0, del)) != (char *) 0) {
               len += sprintf (&cmd[len], ", %s", token);

            // Vielleicht nicht mehr noetig (s.o.).
               token[strlen(token)] = del[0];
            }
         // Vielleicht nicht mehr noetig (s.o.).
            spalte[lenvgl] = '\0';
         }

         // Die Schluessel-Argumente abarbeiten
         va_start (argptr, nk);
         kptr = va_arg (argptr, char *);
         strcpy (keypair, kptr);
         lenvgl = strlen (keypair);
         (void) find_first_field (keypair, delim, &token);
         len += sprintf (&cmd[len], " FROM %s WHERE %s='%s'",
                                    tab, keypair, token);

         // Ist vielleicht nicht mehr notwendig, da mit Kopie gearbeitet wird.
         // Aus Sicherheitsgruenden, da noch nicht getestet, erstmal drinlassen.
         // den urspruenglichen Zustand des Arguments wiederherstellen
         if (strlen (keypair) < lenvgl)
            keypair[strlen(keypair)] = delim;

         for (i=1; i<nk; ++i) {
            kptr = va_arg (argptr, char *);
            strcpy (keypair, kptr);
            lenvgl = strlen (keypair);
            (void) find_first_field (keypair, delim, &token);
            len += sprintf (&cmd[len], " AND %s='%s'", keypair, token);

            // Vielleicht nicht mehr noetig (s.o.).
            // den urspruenglichen Zustand des Arguments wiederherstellen
            if (strlen (keypair) < lenvgl)
               keypair[strlen(keypair)] = delim;
         }
         va_end (argptr);
#ifdef TEST
printf ("cmd: %s\n", cmd);
#endif

         res = PQexec (conn, cmd);
         if (get_resstat () != PGRES_TUPLES_OK) {
            PQreset (conn);
            return ((char *) 0);
         }

         // Das erste Tupel in Ergebnis schreiben und zurueckgeben
         if (set_tuples () > 0) {
            akt_tuple_ind = 0;
            strcpy (help, (char *) PQgetvalue (res, 0, 0));
            strcpy (erg, rtrim (help));
            for (i=1; i<fields; ++i) {
               strcpy (help, (char *) PQgetvalue (res, 0, i));
               strcat (erg, separ);
               strcat (erg, rtrim (help));
            }
         } else
            erg[0] = '\0';

         return (erg);
      }
#ifdef TEST
      else
         printf ("Unzulaessige Uebergabeparmeter!!!\n");
#endif
   }
#ifdef TEST
   else
      printf ("Datenbank ist nicht geoeffnet!!!\n");
#endif

   return ((char *) 0);

} // char *pgdbif::dbget


char *pgdbif::get_erg_field (int t, int f) {
   if (!isopen () || t < 0 || t >= tuples || f < 0 || f >= fields)
      return ((char *) 0);

   strcpy (erg_field, (char *) PQgetvalue (res, t, f));

   return (rtrim (erg_field));

} // char *pgdbif::get_field


char *pgdbif::get_next_tuple () {
   int i;
   char help[1024];

   if (isopen ())
   {
      if (tuples > 0) {
         if ((++akt_tuple_ind) < tuples) {
            strcpy (help, (char *) PQgetvalue (res, akt_tuple_ind, 0));
            strcpy (erg, rtrim (help));
            for (i=1; i<fields; ++i) {
               strcpy (help, (char *) PQgetvalue (res, akt_tuple_ind, i));
               strcat (erg, separ);
               strcat (erg, rtrim (help));
            }
         } else
            erg[0] = '\0';

         return (erg);
      }
   }

   erg[0] = '\0';
   return (erg);
   // return ((char *) 0);

} // char *pgdbif::get_next_tuple


// Dbput
// -----
// tab: Name der Tabelle
// d  : String mit den Daten fuer die einzelnen Spalten, durch delim getrennt
// nk : Anzahl der nachfolgenden Schluessel-Daten-Paare
int pgdbif::dbput (char *tab, char *d, int nk, ...) {
   return (0);
}


// Dbdel
// -----
// tab: Name der Tabelle
// nk : Anzahl der nachfolgenden Schluessel-Daten-Paare
int pgdbif::dbdel (char *tab, int nk, ...) {
   return (-1);
}


// Dbprint (alle Daten oder nur zu einem vorgegebenen Schluessel der Datenbank)
// ----------------------------------------------------------------------------
// tab: Name der Tabelle
// n  : Anzahl der nachfolgenden Argumente
void pgdbif::dbprint (char *tab, int n, ...) {
   return;
}

