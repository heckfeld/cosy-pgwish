//
// $Header$
//

# include <string.h>
# include <ctype.h>
# include <stdlib.h>

# include <fgetstr.h>         // fgetstr-Macro
# include <pgcalib.h>
# include <calibglobals.h>


extern "C" double calib_get_f (double [6], double);
extern "C" double calib_get_u (double [6], double, double, double);


fehler_class fehler_liste[] = {
   {0,  0},       // kein Fehler
   {7,  0},       // aus select_all
   {12, 0},       // aus delete_data

   {-1, 1},       // aus select_key

   {1,  2},       // aus exec_cmd

   {2,  7},       // aus exec_cmd
   {6,  7},       // aus select_all
   {10, 7},       // aus delete_data
   {18, 7},       // aus select_entry

   {3,  4},       // aus get_key_data
   {4,  4},       // aus get_key_data
   {5,  4},       // aus get_key_data
   {8,  4},       // aus get_key_data (char *)
   {9,  4},       // aus get_all_data
   {13, 4},       // aus find_entry (char *, double, int)
   {14, 4},       // aus find_entry (char *, char *, char *, double, int)
   {15, 4},       // aus find_entry (char *, char *, char *, double, int)
   {16, 4},       // aus berechne_wert
   {19, 4},       // aus find_entry (char *, char *, char *, char *, int)
   {20, 4},       // aus find_entry (char *, char *, char *, char *, int)
   {21, 4},       // aus find_entry (char *, int &)
   {22, 4},       // aus find_entry (char *, int &)

   {11, 3},       // aus delete_data
   {17, 3},       // aus select_entry

   -999999
};


////////////////////////////////////////////////////////////////////////////////
//
// PRIVATE FUNCTIONS
//
////////////////////////////////////////////////////////////////////////////////

// Find Typ
//---------
int pgcalib::find_typ (char *typ) {
   int i,
       found;

   for (found=0, i=0; i<(sizeof(ng_typ)/sizeof(ng_typ[0])); i++)
   {
      if (strncmp (typ, ng_typ[i], TYP_COMP_LEN) == 0)
      {
         strcpy (typ, ng_typ[i]);   // Den Typ richtig schreiben
         found = 1;
         break;
      }
   }

   return (found);

} // int pgcalib::check_typ


// Find Fkt
//---------
int pgcalib::find_fkt (char *fkt) {
   int i,
       found;

   for (found=0, i=0; i<(sizeof(ng_fkt)/sizeof(ng_fkt[0])); i++)
   {
      if (strncmp (fkt, ng_fkt[i], FKT_COMP_LEN) == 0)
      {
         strcpy (fkt, ng_fkt[i]);      // Die Funktion richtig schreiben
         found = 1;
         break;
      }
   }

   return (found);

} // int pgcalib::find_fkt


// Select Entry
//
int pgcalib::select_entry (int umkehr) {

   int i, len;

   if (fehler == 0) {
      // Den passenden Eintrag in der Datenbank suchen
      len = sprintf (cmd, "%s FROM %s WHERE ", select, tabName);
      len += sprintf (&cmd[len], "name='%s' AND typ='%s' AND "
                                 "einheit='%s' ",
                                 data.netzg, data.typ, data.einheit);
      if (umkehr) {
         len += sprintf (&cmd[len], "AND yanfang<='%.10g' "
                                    "AND yende>='%.10g'",
                                    data.x, data.x);
      } else {
         len += sprintf (&cmd[len], "AND xanfang<='%.2lf' "
                                    "AND xende>='%.2lf'",
                                    data.x, data.x);
      }
#ifdef TEST
fprintf (stdout, "pgcalib::select_entry: cmd=\"%s\"\n", cmd);
#endif

      if ((fehler = exec_cmd (cmd, PGRES_TUPLES_OK)) == 0) {
         switch (get_tuples ()) {
         case -1:
            fehler = 18;    // Fehler beim Abfragen der Tuples
            break;
         case 0:
            fehler = 17;    // Die Datenbank-Tabelle ist leer
            break;
         default:
            fehler = 0;    // Es koennen Tuples gelesen werden

            // Es wird nur das erste Tupel verwendet
            // Es werden nur die unbekannten Daten gelesen
            data.xanfang = strtod (get_erg_field (0, 3), (char **) 0);
            data.xende = strtod (get_erg_field (0, 4), (char **) 0);
//          strcpy (data.fkt, get_erg_field (0, 5));
            for (i=0; i<6; ++i)                            // a[0] - a[5]
               data.a[i] = strtod (get_erg_field (0, 6+i), (char **) 0);
//          data.yanfang = strtod (get_erg_field (0, 12), (char **) 0);
//          data.yende = strtod (get_erg_field (0, 13), (char **) 0);

            break;

         } // switch (get_tuples ())

      } // if ((fehler = exec_cmd (...) ...)

   } // if (fehler == 0)

   return (fehler ? 1: 0);

} // int pgcalib::select_entry


////////////////////////////////////////////////////////////////////////////////
//
// PUBLIC FUNCTIONS
//
////////////////////////////////////////////////////////////////////////////////

//
// Constructor
// -----------
pgcalib::pgcalib (FILE *fp) : pgdbif (CALIB_DBNAME, fp) {
#ifdef TEST
printf ("&v2Spgcalib constructor\n");
#endif

   tabName = CALIB_TABNAME;

   select = "SELECT rtrim(name,' '), rtrim(typ,' '), "
            "rtrim(einheit,' '), xanfang, xende, "
            "rtrim(funktion,' '), a0, a1, a2, a3, a4, a5";

   select_spalten = "name:typ:einheit:xanfang:xende:"
                    "funktion:a0:a1:a2:a3:a4:a5";

   set_delim (':');    // Trennzeichen innerhalb der Argumente
   set_separ (":");    // Separator fuer die Ausgabe

   fehler = 0;

}


//
// Destructor
// ----------
pgcalib::~pgcalib () {

#ifdef TEST
printf ("&v2Spgcalib destructor\n");
#endif
}

// Get DbName
// ----------
char *pgcalib::get_calib_dbname () {
#ifdef TEST
printf ("&v2Spgcalib get_calib_dbname\n");
#endif

   return (CALIB_DBNAME);

} // char *pgcalib::get_calib_dbname


// Select Key
// ----------
int pgcalib::select_key (char *key_string) {
#ifdef TEST
printf ("&v2Spgcalib select_key\n");
#endif

   char key[MAX_KEY_NUM][80], del[2], *token;
   int len, i;

   if (fehler == 0) {

      if (key_string == (char *) 0 || !key_string[0]) {
         fehler = -1;
      } else {
         // Weil strtok() einen String als Delimiter erwartet.
         del[0] = delim;
         del[1] = '\0';

         len = sprintf (cmd, "%s FROM %s WHERE ", select, tabName);
         i = 0;   // zaehlt die Anzahl der angegebenen Schluessel-Spalten
         token = strtok (key_string, del);
         while (token != (char *) 0 && i < MAX_KEY_NUM) {
            strncpy (key[i++], token, sizeof(key[0])-1);
            token = strtok ((char *) 0, del);
         }
         switch (i) {
         case 1:
            len += sprintf (&cmd[len], "name~'^%s'", key[0]);
            break;
         case 2:
            len += sprintf (&cmd[len], "name='%s' AND typ~'^%s'",
                                       key[0], key[1]);
            break;
         case 3:
            len += sprintf (&cmd[len], "name='%s' AND typ='%s' "
                                       "AND einheit~'^%s'",
                            key[0], key[1], key[2]);
            break;
         case 4:
            len += sprintf (&cmd[len], "name='%s' AND typ='%s' "
                                       "AND einheit='%s' "
                                       "AND xanfang='%s'",
                            key[0], key[1], key[2], key[3]);
            break;
         case 5:
            len += sprintf (&cmd[len], "name='%s' AND typ='%s' "
                                       "AND einheit='%s' "
                                       "AND xanfang='%s' AND xende='%s'",
                            key[0], key[1], key[2], key[3], key[4]);
            break;
         }
#ifdef TEST
printf ("pgcalib::select_key: cmd=\"%s\"\n", cmd);
#endif

         fehler = exec_cmd (cmd, PGRES_TUPLES_OK);
      }

   } // if (fehler == 0)

   return (fehler ? 1 : 0);

} // int pgcalib::select_key


// Select All
// ----------
int pgcalib::select_all () {
#ifdef TEST
printf ("&v2Spgcalib select_all\n");
#endif

   if (fehler == 0) {

      (void) sprintf (cmd, "%s FROM %s", select, tabName);

      if ((fehler = exec_cmd (cmd, PGRES_TUPLES_OK)) == 0) {
         switch (get_tuples ()) {
         case -1:
            fehler = 6;    // Fehler beim Abfragen der Tuples
            break;
         case 0:
            fehler = 7;    // Die Datenbank-Tabelle ist leer
            break;
         default:
            fehler = 0;    // Es koennen Tuples gelesen werden
            break;
         }
      }

   } // if (fehler == 0)

   return (fehler ? 1 : 0);

} // int pgcalib::select_all


// Find Entry
// ----------
// Suchen eines Datenbank-Eintrags fuer die Berechnung des Funktionswertes bzw.
// des Wertes der Umkehrfunktion an der Stelle x
// Rueckgabewert 0: alles ok
//               1: Fehler
//
int pgcalib::find_entry (char *netzg, char *typ, char *einheit,
                         double x, int umkehr) {
#ifdef TEST
printf ("&v2Spgcalib find_entry (char *, char *, char *, double, int)\n");
#endif

   if (fehler == 0) {
      if (netzg == (char *) 0 || !netzg[0] ||
          typ == (char *) 0 || !typ[0] ||
          einheit == (char *) 0 || !einheit[0]) {
         fehler = 14;
      } else {

// Den Netzgeraete-Typ ueberpruefen.
         strcpy (data.typ, typ);
         if (find_typ (data.typ) == 0 ||
             (umkehr && strcmp (data.typ, "const") == 0)) {
            fehler = 15;
         } else {

            strcpy (data.netzg, netzg);
            strcpy (data.einheit, einheit);
            data.x = x;

            (void) select_entry (umkehr);

         } // else von Netzgeraete-Typ ok?

      } // else von Argumentenueberpruefung ok?

   } // if (fehler == 0)

   return (fehler ? 1: 0);

} // int pgcalib::find_entry (char *, char *, char *, double, int)


// Find Entry
// ----------
// Suchen eines Datenbank-Eintrags fuer die Berechnung des Funktionswertes bzw.
// des Wertes der Umkehrfunktion an der Stelle x
// Rueckgabewert 0: alles ok
//               1: Fehler
//
int pgcalib::find_entry (char *netzg, char *typ, char *einheit,
                         char *data_string, int umkehr) {
#ifdef TEST
printf ("&v2Spgcalib find_entry (char *, char *, char *, char *, int)\n");
#endif

   if (fehler == 0) {
      if (netzg == (char *) 0 || !netzg[0] ||
          typ == (char *) 0 || !typ[0] ||
          einheit == (char *) 0 || !einheit[0] ||
          data_string == (char *) 0 || !data_string[0]) {
         fehler = 19;
      } else {

// Den Netzgeraete-Typ ueberpruefen.
         strcpy (data.typ, typ);
         if (find_typ (data.typ) == 0 ||
             (umkehr && strcmp (data.typ, "const") == 0)) {
            fehler = 20;
         } else {

            strcpy (data.netzg, netzg);
            strcpy (data.einheit, einheit);

            strcpy (data.name, strtok (data_string, ":"));   // Name fuer Output
            data.x = strtod (strtok ((char *) 0, ":"), (char **) 0);

            (void) select_entry (umkehr);

         } // else von Netzgeraete-Typ ok?

      } // else von Argumentenueberpruefung ok?

   } // if (fehler == 0)

   return (fehler ? 1: 0);

} // int pgcalib::find_entry (char *, char *, char *, char *, int)


// Find Entry
// ----------
// Suchen eines Datenbank-Eintrags fuer die Berechnung des Funktionswertes bzw.
// des Wertes der Umkehrfunktion an der Stelle x
// Rueckgabewert 0: alles ok
//               1: Fehler
//
int pgcalib::find_entry (char *data_string, double x, int umkehr) {
#ifdef TEST
printf ("&v2Spgcalib find_entry (char *, double, int)\n");
#endif

   char *cp;
   char *token;   // unter Linux gibt's Segmentation-Fault, wenn bei
                  // strcpy ein Null-Pointer Argument uebergeben wird.

   if (fehler == 0) {
      if ((token = strtok (data_string, ":")) != (char *) 0)
         strcpy (data.name, token);                          // Name fuer Output
      else
         data.name[0] = '\0';

      if ((token = strtok ((char *) 0, ":")) != (char *) 0)
         strcpy (data.netzg, token);                         // Netzgeraet
      else
         data.netzg[0] = '\0';

      if ((token = strtok ((char *) 0, ":")) != (char *) 0)
         strcpy (data.typ, token);                           // Typ
      else
         data.typ[0] = '\0';
      for (cp=data.typ; *cp; cp++)
         *cp = tolower (*cp);

      if ((token = strtok ((char *) 0, ":")) != (char *) 0)
         strcpy (data.einheit, token);                       // Einheit
      else
         data.einheit[0] = '\0';

      if (!data.netzg[0] || !data.typ[0] || !data.einheit[0]
       || find_typ (data.typ) == 0
       || (umkehr && strcmp (data.typ, "const") == 0)) {
         fehler = 13;
      } else {
         data.x = x;

         (void) select_entry (umkehr);
      }

   } // if (fehler == 0)

   return (fehler ? 1: 0);

} // int pgcalib::find_entry (char *, double, int)


// Find Entry
// ----------
// Suchen eines Datenbank-Eintrags fuer die Berechnung des Funktionswertes bzw.
// des Wertes der Umkehrfunktion an der Stelle x, x und das
// Umkehrfunktions-Flag sind in data_string enthalten.
// Das Umkehrfunktions-Flag wird zur Information zurueckgegeben.
// Rueckgabewert 0: alles ok
//               1: Fehler
//
int pgcalib::find_entry (char *data_string, int &umkehr) {
#ifdef TEST
printf ("&v2Spgcalib find_entry (char *, int &)\n");
#endif

   double x;
   char *cp;
   char *token;   // unter Linux gibt's Segmentation-Fault, wenn bei
                  // strcpy ein Null-Pointer Argument uebergeben wird.

   if (fehler == 0) {
      if ((token = strtok (data_string, ":")) != (char *) 0)
         strcpy (data.name, token);                          // Name fuer Output
      else
         data.name[0] = '\0';

      if ((token = strtok ((char *) 0, ":")) != (char *) 0)
         strcpy (data.netzg, token);                         // Netzgeraet
      else
         data.netzg[0] = '\0';

      if ((token = strtok ((char *) 0, ":")) != (char *) 0)
         strcpy (data.typ, token);                           // Typ
      else
         data.typ[0] = '\0';
      for (cp=data.typ; *cp; cp++)
         *cp = tolower (*cp);

      if ((token = strtok ((char *) 0, ":")) != (char *) 0)
         strcpy (data.einheit, token);                       // Einheit
      else
         data.einheit[0] = '\0';

      if ((token = strtok ((char *) 0, ":")) != (char *) 0) {
         x = strtod (token, (char **) 0);                    // x

         if ((token = strtok ((char *) 0, ":")) != (char *) 0) {
            umkehr = (strcmp (token, "-u") == 0) ? 1 : 0;
         }
         else
            umkehr = 0;

         if (!data.netzg[0] || !data.typ[0] || !data.einheit[0]
          || find_typ (data.typ) == 0
          || (umkehr && strcmp (data.typ, "const") == 0)) {
            fehler = 21;
         } else {
            data.x = x;

            (void) select_entry (umkehr);
         }
     } else {
        fehler = 22;
     }

   } // if (fehler == 0)

   return (fehler ? 1: 0);

} // int pgcalib::find_entry (char *, int &)


// Berechne Wert
// -------------
// Funktion zum Berechnen des Funktionswertes bzw. des Wertes der
// Umkehrfunktion an der Stelle x
// Rueckgabewert 0: alles ok
//               1: Fehler
//
double pgcalib::berechne_wert (int umkehr) {
#ifdef TEST
printf ("&v2Spgcalib berechne_wert\n");
#endif

   if (fehler == 0) {

      data.y = umkehr ? calib_get_u (data.a, data.x, data.xanfang, data.xende)
                      : calib_get_f (data.a, data.x);

   } // if (fehler == 0)

   return (fehler ? 0.0 : data.y);

} // double pgcalib::berechne_wert


//
// Funktion, um den Namen zu einem Eintrag aus der Datei zu bekommen
// 
char *pgcalib::return_name () {

   if (fehler == 0)
      return (data.name);

   return ((char *) 0);

} // char *pgcalib::return_name


// Get Key Data
// ------------
// Funktion zum Einlesen der Schluessel-Daten
// Rueckgabewert 0: alles ok
//               1: Fehler
//
int pgcalib::get_key_data () {
#ifdef TEST
printf ("&v2Spgcalib get_key_data\n");
#endif

   char *cp,
        double_str[256],
        help[1024];

   if (fehler == 0) {

      fprintf (stdout, "Netzgeraet     : ");
      fflush (stdout);
      fgetstr (stdin, data.netzg, help, sizeof(help));

      fprintf (stdout, "Typ            : ");
      fflush (stdout);
      fgetstr (stdin, data.typ, help, sizeof(help));
      for (cp=data.typ; *cp; cp++)
         *cp = tolower (*cp);

// Den Netzgeraete-Typ ueberpruefen.
      if (find_typ (data.typ) == 0) {
         fehler = 3;
      }
      else {

         fprintf (stdout, "Einheit        : ");
         fflush (stdout);
         fgetstr (stdin, data.einheit, help, sizeof(help));

         fprintf (stdout, "Bereichsanfang : ");
         fflush (stdout);
         fgetstr (stdin, double_str, help, sizeof(help));
         data.xanfang = strtod (double_str, (char **) 0);

         fprintf (stdout, "Bereichsende   : ");
         fflush (stdout);
         fgetstr (stdin, double_str, help, sizeof(help));
         data.xende = strtod (double_str, (char **) 0);

      // Eingabe ueberpruefen
         if (data.xanfang >= data.xende) {
            fehler = 4;
         }
      }

   } // if (fehler == 0)

   return (fehler ? 1 : 0);

} // int pgcalib::get_key_data


// Get Key Data
// ------------
// Funktion zum Einlesen der Schluessel-Daten aus dem uebergebenen
// Datenstring
// Rueckgabewert 0: alles ok
//               1: Fehler
//
int pgcalib::get_key_data (char *data_string) {
#ifdef TEST
printf ("&v2Spgcalib get_key_data (char *, int)\n");
#endif

   char *cp,
        double_str[256];

   if (fehler == 0) {
      strcpy (data.netzg, strtok (data_string, ":"));        // Netzgeraet
      strcpy (data.typ, strtok ((char *) 0, ":"));           // Typ
      for (cp=data.typ; *cp; cp++)
         *cp = tolower (*cp);
      strcpy (data.einheit, strtok ((char *) 0, ":"));       // Einheit
      strcpy (double_str, strtok ((char *) 0, ":"));         // Bereichsanfang
      data.xanfang = strtod (double_str, (char **) 0);
      strcpy (double_str, strtok ((char *) 0, ":"));         // Bereichsende
      data.xende = strtod (double_str, (char **) 0);

// Vollstaendigkeit/Korrektheit der Schluessel-Daten ueberpruefen.
      if ( data.netzg[0] == '\0' || find_typ (data.typ) == 0 ||
           data.einheit[0] == '\0' || data.xanfang >= data.xende )
         fehler = 8;

   } // if (fehler == 0)

   return (fehler ? 1 : 0);

} // int pgcalib::get_key_data (char *data_string)


// Get Data
// --------
// Funktion zum Einlesen der Funktionsdaten
// Rueckgabewert 0: alles ok
//               1: Fehler
//
int pgcalib::get_data () {
#ifdef TEST
printf ("&v2Spgcalib get_data\n");
#endif

   char *cp,
        double_str[256],
        help[1024];

   if (fehler == 0) {

      fprintf (stdout, "Funktion       : ");
      fflush (stdout);
      fgetstr (stdin, data.fkt, help, sizeof(help));
      for (cp=data.fkt; *cp; cp++)
         *cp = tolower (*cp);

    // Eingabe ueberpruefen
      if (find_fkt (data.fkt) == 0) {
         fehler = 5;

      } else {
         fprintf (stdout, "Koeffizient a0 : ");
         fflush (stdout);
         fgetstr (stdin, double_str, help, sizeof(help));
         data.a[0] = strtod (double_str, (char **) 0);

         fprintf (stdout, "Koeffizient a1 : ");
         fflush (stdout);
         fgetstr (stdin, double_str, help, sizeof(help));
         data.a[1] = strtod (double_str, (char **) 0);

         fprintf (stdout, "Koeffizient a2 : ");
         fflush (stdout);
         fgetstr (stdin, double_str, help, sizeof(help));
         data.a[2] = strtod (double_str, (char **) 0);

         fprintf (stdout, "Koeffizient a3 : ");
         fflush (stdout);
         fgetstr (stdin, double_str, help, sizeof(help));
         data.a[3] = strtod (double_str, (char **) 0);

         fprintf (stdout, "Koeffizient a4 : ");
         fflush (stdout);
         fgetstr (stdin, double_str, help, sizeof(help));
         data.a[4] = strtod (double_str, (char **) 0);

         fprintf (stdout, "Koeffizient a5 : ");
         fflush (stdout);
         fgetstr (stdin, double_str, help, sizeof(help));
         data.a[5] = strtod (double_str, (char **) 0);

         data.yanfang = calib_get_f (data.a, data.xanfang);
         data.yende = calib_get_f (data.a, data.xende);
      }

   } // if (fehler == 0)

   return (fehler ? 1 : 0);

} // int pgcalib::get_data


// Get All Data
// ------------
// Funktion zum Einlesen der Schluessel- und Funktionsdaten aus 
// dem uebergebenen String
// Rueckgabewert 0: alles ok
//               1: Fehler
//
int pgcalib::get_all_data (char *data_string) {
#ifdef TEST
printf ("&v2Spgcalib get_all_data\n");
#endif

   char *cp,
        double_str[256];
   int i;

   if (fehler == 0) {

      if (get_key_data (data_string) == 0) {
         strcpy (data.fkt, strtok ((char *) 0, ":"));  // Funktion
         for (cp=data.fkt; *cp; cp++)
            *cp = tolower (*cp);
         for (i=0; i<6; ++i)                           // Koeffizient a0 - a5
         {
            strcpy (double_str, strtok ((char *) 0, ":"));
            data.a[i] = strtod (double_str, (char **) 0);
         }

// Vollstaendigkeit/Korrektheit der Daten ueberpruefen.
         if (find_fkt (data.fkt) == 0) {
            fehler = 9;
         } else {
            data.yanfang = calib_get_f (data.a, data.xanfang);
            data.yende = calib_get_f (data.a, data.xende);
         }
      }

   } // if (fehler == 0)

   return (fehler ? 1 : 0);

} // int pgcalib::get_all_data


// Insert Data
// -----------
// Funktion, um einen Eintrag in die Datenbank vorzunehmen
//
// Rueckgabewert 0: alles ok
//               1: Fehler
//
int pgcalib::insert_data (int line) {
#ifdef TEST
printf ("&v2Spgcalib insert_data\n");
#endif

   char help[1024];

   if (fehler == 0) {

      if (data.yanfang < data.yende)
         sprintf (help, "'%.10g', '%.10g'", data.yanfang, data.yende);
      else
         sprintf (help, "'%.10g', '%.10g'", data.yende, data.yanfang);
      sprintf (cmd, "INSERT into %s VALUES ("
                    "'%s', '%s', '%s', "
                    "'%.2lf', '%.2lf', '%s', "
                    "'%.10g', '%.10g', '%.10g', "
                    "'%.10g', '%.10g', '%.10g', "
                    "%s)",
                    tabName,
                    data.netzg, data.typ, data.einheit,
                    data.xanfang, data.xende, data.fkt,
                    data.a[0], data.a[1], data.a[2],
                    data.a[3], data.a[4], data.a[5],
                    help);
#ifdef TEST
fprintf (stdout, "pgcalib::insert_data: cmd=\"%s\"\n", cmd);
#endif

      switch ((fehler = exec_cmd (cmd, PGRES_COMMAND_OK, line))) {
      case 1:
         // Datenbank nicht geoeffnet
         break;
      case 0:
         // Daten sind eingefuegt
         break;

      case 2:
#ifdef TEST
fprintf (stderr, "%s\n", get_error ());
#endif
         // UPDATE-Kommando zusammenstellen
         if (data.yanfang < data.yende)
            sprintf (help, "yanfang='%.10g', yende='%.10g'",
                           data.yanfang, data.yende);
         else
            sprintf (help, "yanfang='%.10g', yende='%.10g'",
                           data.yende, data.yanfang);
         sprintf (cmd, "UPDATE %s SET "
                       "funktion='%s', "
                       "a0='%.10g', a1='%.10g', a2='%.10g', "
                       "a3='%.10g', a4='%.10g', a5='%.10g', "
                       "%s WHERE "
                       "name='%s' AND typ='%s' AND einheit='%s' "
                       "AND xanfang='%.2lf' AND xende='%.2lf'",
                       tabName,
                       data.fkt,
                       data.a[0], data.a[1], data.a[2],
                       data.a[3], data.a[4], data.a[5],
                       help,
                       data.netzg, data.typ, data.einheit,
                       data.xanfang, data.xende);

#ifdef TEST
fprintf (stdout, "pgcalib::insert_data: cmd=\"%s\"\n", cmd);
#endif

         switch ((fehler = exec_cmd (cmd, PGRES_COMMAND_OK, line))) {
         case 0:
            // Daten sind upgedatet
            break;
         case 1:
            // Datenbank nicht geoeffnet
            break;
         case 2:
            // Fehler beim PQexec-AUfruf
            break;
         } // UPDATE: switch (exec_cmd (cmd, PGRES_COMMAND_OK, line))

         break;

      } // INSERT: switch (exec_cmd (cmd, PGRES_COMMAND_OK, line))

   } // if (fehler == 0)

   return (fehler ? 1 : 0);

} // int pgcalib::insert_data


// Delete Data
// -----------
// Funktion, um einen Eintrag aus der Datenbank zu loeschen
//
// Rueckgabewert 0: alles ok
//               1: Fehler
//
int pgcalib::delete_data (int line) {
#ifdef TEST
printf ("&v2Spgcalib delete_data\n");
#endif

   if (fehler == 0) {

      sprintf (cmd, "DELETE from %s WHERE "
                    "name='%s' AND typ='%s' AND einheit='%s' AND "
                    "xanfang='%.2lf' AND xende='%.2lf'",
                    tabName,
                    data.netzg, data.typ, data.einheit,
                    data.xanfang, data.xende);

      if ((fehler = exec_cmd (cmd, PGRES_COMMAND_OK, line)) == 0) {
         fehler = 11 + get_delstat ();
      }

   } // if (fehler == 0)

   return (fehler ? 1 : 0);

} // int pgcalib::delete_data


// Reset Fehler
// ------------
void pgcalib::reset_fehler () {
#ifdef TEST
printf ("&v2Spgcalib reset_fehler\n");
#endif

   fehler = 0;

   return;

} // void pgcalib::reset_fehler


// Print Wert
// ----------
int pgcalib::print_wert () {
#ifdef TEST
printf ("&v2Spgcalib print_wert\n");
#endif

   if (fehler == 0) {
      fprintf (fpout, "%.10g\n", data.y);
   }

   return (fehler ? 1 : 0);

} // int pgcalib::print_wert


// Print Tuples
// ------------
int pgcalib::print_tuples () {
#ifdef TEST
printf ("&v2Spgcalib print_tuples\n");
#endif

   char *ptr;

   if (fehler == 0) {

      if (isopen ()) {
         ptr = get_next_tuple ();
         while (ptr[0]) {
            fprintf (fpout, "%s\n", ptr);
            ptr = get_next_tuple ();
         }
      } else
         fehler = 1;

   } // if (fehler == 0)

   return (fehler ? 1 : 0);

} // int pgcalib::print_tuples


// Print Fehler
// ------------
int pgcalib::print_fehler (int line) {
#ifdef TEST
printf ("&v2Spgcalib print_fehler\n");
#endif

   int retval = 0;

   switch (fehler) {
   case 0:
      break;

   case -1: // aus select_key
      fprintf (fpout, "Unzulaessiger Uebergabeparameter bei select_key.\n");
      break;

   case 1: // aus exec_cmd
      fprintf (fpout, "Datenbank ist nicht geoeffnet.\n");
      break;

   case 2: // aus exec_cmd
      fprintf (fpout, "Fehler beim PQexec-Aufruf:--> %s\n", get_error ());
      break;

   case 6:  // aus select_all
   case 18: // select_entry
      fprintf (fpout, "Fehler beim Abfragen der Tupels.\n");
      break;

   case 7: // aus select_all
      fprintf (fpout, "Die Datenbank %s ist leer.\n", dbName);
      break;

   case 3:  // aus get_key_data
   case 4:  // aus get_key_data
   case 5:  // aus get_key_data
   case 8:  // aus get_key_data (char *)
   case 9:  // aus get_all_data
   case 13: // aus find_entry
   case 14: // aus find_entry
   case 15: // aus find_entry
   case 16: // aus berechne_wert
   case 19: // aus find_entry
   case 20: // aus find_entry
   case 21: // aus find_entry
   case 22: // aus find_entry
      if (line >= 0)
         fprintf (fpout, "Zeile %3d: ", line);
      fprintf (fpout, "Falsche Eingabe.\n");
      break;

   case 10: // aus delete_data
      if (line >= 0)
         fprintf (fpout, "Zeile %3d: ", line);
      fprintf (fpout, "Kommando-Status zum Delete kann nicht "
                      "abgefragt werden.\n");
      break;

   case 11: // aus delete_data
   case 17: // aus select_entry
      if (line >= 0)
         fprintf (fpout, "Zeile %3d: ", line);
      fprintf (fpout, "Kein Eintrag in der Datenbank.\n");
      break;

   case 12: // aus delete_data
      if (line >= 0)
         fprintf (fpout, "Zeile %3d: ", line);
      fprintf (fpout, "Daten geloescht.\n");
      break;

   }

   if (line < 0)
      retval = set_retval ();

   return (retval);

} // int pgcalib::print_fehler


// Set retval
// ----------
int pgcalib::set_retval () {
#ifdef TEST
printf ("&v2Spgcalib set_retval\n");
#endif

   int i;

   for (i=0; fehler_liste[i].num!=-999999; ++i)
   {
      if (fehler_liste[i].num == fehler)
         return (fehler_liste[i].retval);
   }

   return (fehler);

} // int pgcalib::set_retval

