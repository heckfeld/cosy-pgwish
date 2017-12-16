//
// $Header$
//
//
// Aufruf       : cosmo_calib {<key> | -a | -i | -f <file> | -d [<file>]}
//
// Funktion     : Zugriff auf die Datenbank fuer die Calibrierungsdaten.
//                <key> ist der Anfang des Schluessel eines Datenbankeintrag.
//                -a : Der komplette Inhalt der Datenbank wird ausgeben.
//                -i : Interaktive Eingabe der Daten fuer neuen Eintrag.
//                -f : Daten fuer neuen Eintrag werden aus <file> gelesen.
//                -d : Daten fuer Eintrag werden geloescht, interakt. Fragen.
//
// Rueckgabewert: 0 alles ok
//                1 falsche usage
//                2 fehler beim oeffnen der datenbank
//                3 kein eintrag in der datenbank
//                4 ungueltiger wert
//                5 fehler beim oeffnen des datenfiles
//                6 abbruch durch ctrl-c
//                7 fehler beim zugriff auf die datenbank
//
//

#include <stdio.h>           // fuer argc, argv, fprintf ...
#include <stdlib.h>          // fuer exit()
#include <signal.h>          // fuer signal()

#include <basename.h>        // basename-Macro
#include <fgetstr.h>         // fgetstr-Macro

#include <pgpatch.h>
#include "libpq-fe.h"

#include <pgcalib.h>         // fuer pgcalib-Klasse


#define PRINT_USAGE { \
   fprintf (stdout, "usage: %s {<key> | -a | -i | " \
     "-f <file> | -d [<file>]}\n", basename(argv[0],ptr)); \
   return (1); \
}


void signal_handler (int);


//
// Globale Variable, damit die Interrupt-Funktion auch darauf
// zugreifen kann.
//
pgcalib *calib_db_ptr = (pgcalib *) 0;


int main (int argc, char *argv[])
{
   FILE *fp;
   char help[1024], data_string[1024];
   int line;
   int exval = 0;
   char *ptr;      // wird fuer das 'basename'-Macro (s.o.) gebraucht.

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
   if (argc < 2 || argv[1][0] == '\0' || (argv[1][0] != '-' && argc > 2))
      PRINT_USAGE;


//
// Datenbank oeffnen
//
   if (!calib_db.isopen ()) {
      fprintf (stdout, "Datenbank \"%s\" konnte nicht geoeffnet werden\n",
                       calib_db.get_calib_dbname ());
      return (2);
   }


   if (argv[1][0] != '-') {
//
// <key>: alle Eintraege zu diesem Namen aus der Datenbank suchen.
//
     if (calib_db.select_key (argv[1]) == 0) {
        (void) calib_db.print_tuples ();
     }
   }
   else
   {
//
// Es ist mindestens eine Option angegeben.
// Entsprechend abarbeiten.
//
      switch (argv[1][1])
      {
      case 'a':  // Ausgeben aller Daten
         if (argc > 2)
            PRINT_USAGE;

         if (calib_db.select_all () == 0) {
            (void) calib_db.print_tuples ();
         }

         break;

      case 'i':  // interaktives Einlesen der Daten
         if (argc > 2)
            PRINT_USAGE;

         if (calib_db.get_key_data () == 0) {
            if (calib_db.get_data () == 0) {
               (void) calib_db.insert_data ();
            }
         }

         break;

      case 'f':  // Einlesen der Daten vom File
         if (argc != 3)
            PRINT_USAGE;

         if ((fp = fopen (argv[2], "r")) == (FILE *) 0)
         {
            fprintf (stdout, "File \"%s\" kann nicht geoeffnet werden.\n",
                     argv[2]);
            return (5);
         }

         fgetstr (fp, data_string, help, sizeof(help));
         line = 1;
         while (data_string[0])
         {
            if (calib_db.get_all_data (data_string) == 0) {
               (void) calib_db.insert_data (line);
            }
            (void) calib_db.print_fehler (line);
            calib_db.reset_fehler ();

            fgetstr (fp, data_string, help, sizeof(help));
            line++;

         } // while (data_string[0])

         fclose (fp);

         break;

      case 'd':  // Loeschen von Daten

         switch (argc)
         {
         case 3:   // Schluessel-Daten aus File lesen.

            if ((fp = fopen (argv[2], "r")) == (FILE *) 0)
            {
               fprintf (stdout, "File \"%s\" kann nicht geoeffnet werden.\n",
                        argv[2]);
               return (5);
            }

            fgetstr (fp, data_string, help, sizeof(help));
            line = 1;
            while (data_string[0])
            {
               if (calib_db.get_key_data (data_string) == 0) {
                  calib_db.delete_data (line);
               }
               (void) calib_db.print_fehler (line);
               calib_db.reset_fehler ();

               fgetstr (fp, data_string, help, sizeof(help));
               line++;

            } // while (data_string[0])

            fclose (fp);

            break;

         case 2:    // Schluessel-Daten interaktiv lesen.

            if (calib_db.get_key_data () == 0) {
               calib_db.delete_data ();
            }

            break;

         default:
            PRINT_USAGE;

         } // switch (argc)

         break;

      default:
         PRINT_USAGE;

      } // switch (argv[1][1])

   } // else von if (argv[1][0] != '-')

   exval = calib_db.print_fehler ();

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

   exit (6);
}

