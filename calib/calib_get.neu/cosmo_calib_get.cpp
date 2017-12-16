//
// $Header$
//
//
// Aufruf       : cosmo_calib_get <netzgeraet> <typ> <einheit> <x> [-u]
//                cosmo_calib_get -f <datei> [<x> [-u]]
//
// Funktion     : Berechnung des Funktionswertes bzw. des Wertes der Umkehr-
//                funktion an der Stelle <x>.
//                <netzgeraet>, <typ> und <einheit> spezifizieren die
//                Funktion. Die zugehoerigen Daten, Koeffizienten, sind in
//                der Datenbank 'calib' abgelegt.
//                Der berechnete Wert wird ausgegeben.
//
//                -f: Berechnenen mehrerer Funktionswerte an der Stelle <x>.
//                    Ist <x> als Uebergabeparameter angebeben, wird dieser
//                    Wert und das zugehoerige Umkehrfunktions-Flag fuer alle
//                    Berechnungen verwendet, ansonsten die entsprechenden
//                    Werte in <datei>.
//                    <datei> enthaelt zeilenweise Eintraege fuer
//                    die Berechnung.
//                    Format der Datei:
//                      <Name fuer Ausgabe>:<netzgeraet>:<typ>:\
//                                          <einheit>[:<x>[:-u]]
//                    Die Ausgabe erfolgt zeilenweise mit folgendem Format:
//                      <Name fuer Ausgabe> <berechneter Wert>
//
//                -u : Umkehrfunktionsflag: Es soll der Wert der
//                     Umkehrfunktion berechnet werden.
//
// Rueckgabewert: 0 alles ok
//                1 falsche usage
//                2 fehler beim oeffnen der datenbank
//                3 kein eintrag in der datenbank
//                4 ungueltiger wert
//                5 abbruch durch ctrl-c
//                7 fehler beim zugriff auf die datenbank
//
//

#include <stdio.h>           // fuer argc, argv, fprintf ...
#include <stdlib.h>          // fuer strtod()
#include <string.h>          // fuer strcmp()
#include <signal.h>          // fuer signal()

#include <basename.h>        // basename-Macro
#include <fgetstr.h>         // fgetstr-Macro

#include <pgpatch.h>
#include "libpq-fe.h"

#include <pgcalib.h>         // fuer pgcalib-Klasse


#define PRINT_USAGE { \
   fprintf (stdout, "usage: %s <netzgeraet> <typ> <einheit> <x> [-u]\n", \
                            basename(argv[0],ptr)); \
   fprintf (stdout, "       %s -f <datei> <x> [-u]\n", \
                            basename(argv[0],ptr)); \
   return (1); \
}

void signal_handler (int);


//
// Globale Variable, damit die Interrupt-Funktion auch darauf
// zugreifen kann.
//
pgcalib *calib_db_ptr = (pgcalib *) 0;


main (int argc, char *argv[])
{
   FILE *fp;
   char help[1024], data_string[1024];
   int line, umkehr, von_datei, max_argc, min_argc;
   double x;
   int exval = 0;
   char *ptr;   // wird fuer das 'basename'-Makro fuer die Usage gebraucht.

   pgcalib calib_db;
   calib_db_ptr = &calib_db;

//
// Aufsetzen des Signal-Handlings fuer 'CTRL-C' etc.
//
   (void) signal (SIGINT, signal_handler);    // interrupt
   (void) signal (SIGKILL, signal_handler);   // kill
   (void) signal (SIGTERM, signal_handler);   // software termination signal
   (void) signal (SIGPIPE, signal_handler);   // write on a pipe with no one
                                              // to read it

//
// Die Argumente auswerten.
//
   if (argc < 3)
      PRINT_USAGE;

   if (strcmp (argv[1], "-f") == 0) {
      von_datei = 1;
      max_argc = 5;
      min_argc = 3;
   } else {
      von_datei = 0;
      max_argc = 6;
      min_argc = 5;
   }

   if (argc < min_argc || argc > max_argc
    || argc == max_argc && strcmp (argv[max_argc-1], "-u") != 0)
      PRINT_USAGE;

   umkehr = (argc == max_argc) ? 1 : 0;


//
// Datenbank oeffnen
//
   if (!calib_db.isopen ()) {
      fprintf (stdout, "Datenbank \"%s\" konnte nicht geoeffnet werden\n",
                       calib_db.get_calib_dbname ());
      return (2);
   }


   if (von_datei) {
      if (strcmp (argv[2], "-") == 0)
         fp = stdin;
      else {
         if ((fp = fopen (argv[2], "r")) == (FILE *) 0)
         {
            fprintf (stdout, "File \"%s\" kann nicht geoeffnet werden.\n",
                     argv[2]);
            return (5);
         }
      }
      if (argc > min_argc) {
         // ein <x> und ggfs. -u fuer alle Zeilen in <datei>
         x = strtod (argv[3], (char **) 0);

         fgetstr (fp, data_string, help, sizeof(help));
         line = 1;
         while (data_string[0])
         {
            if (calib_db.find_entry (data_string, x, umkehr) == 0) {
               fprintf (stdout, "%s %.10g\n", calib_db.return_name (),
                                              calib_db.berechne_wert (umkehr));
            }
            else {
               fprintf (stdout, "???\n");
               calib_db.reset_fehler ();
            }

            fgetstr (fp, data_string, help, sizeof(help));
            line++;

         } // while (data_string[0])

      } // if (argc > min_argc)
      else {
         // jede Zeile in <datei> enthaelt ihr eigenes <x> und ggfs. -u
         fgetstr (fp, data_string, help, sizeof(help));
         line = 1;
         while (data_string[0])
         {
            // umkehr ist hier Referenz !!!
            if (calib_db.find_entry (data_string, umkehr) == 0) {
               fprintf (stdout, "%s %.10g\n", calib_db.return_name (),
                                              calib_db.berechne_wert (umkehr));
            }
            else {
               fprintf (stdout, "???\n");
               calib_db.reset_fehler ();
            }

            fgetstr (fp, data_string, help, sizeof(help));
            line++;

         } // while (data_string[0])

      } // else von if (argc > min_argc)

      fclose (fp);

   } else {
      if (calib_db.find_entry (argv[1], argv[2], argv[3],
                               strtod (argv[4], (char **) 0), umkehr) == 0) {
         fprintf (stdout, "%.10g\n", calib_db.berechne_wert (umkehr));
      }

      exval = calib_db.print_fehler ();
   }

//
// Datenbank muss wieder geschlossen werden.
// Im Destructor fuer die Datenbank-Klasse wird ein Close
// auf die Datenbank gemacht !!!
//

   return (exval);

} // int main

void signal_handler (int num)
{
   if (calib_db_ptr != (pgcalib *) 0)
      calib_db_ptr->close ();

   exit (5);
}

