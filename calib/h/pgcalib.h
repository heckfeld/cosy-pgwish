//
// $Header$
//

# ifndef PGCALIB_INCLUDED
#  define PGCALIB_INCLUDED

#  include <pgdbif.h>


#  define CALIB_DBNAME  "calib"
#  define CALIB_TABNAME "calib"
#  define MAX_KEY_NUM   5

//
// Klasse fuer Zuordnung von Fehlernummern zu Rueckgabewerten
// ----------------------------------------------------------
//
    class fehler_class {
    public:
       int num;        // Nummer des Fehlers
       int retval;     // zugehoeriger Rueckgabewert fuer weitere Verwendung
    };


//
// Class pgcalib
// -------------
//
   class pgcalib : public pgdbif {
   private:
      char *tabName;

      const char *select;               // Anfang des SELECT-Kommandos mit
                                        // den auszulesenden Spalten

      const char *select_spalten;       // Spalten, die beim SELECT-Kommando
                                        // ausgelesen werden sollen, fuer den
                                        // Aufruf von dbget ()

      int fehler;                       // Fehler bei dem Aufruf einer
                                        // Memberfunktion (auch der Basisklasse)

      int find_typ (char *);            // Fuer Ueberpruefung des Typs
      int find_fkt (char *);            // Fuer Ueberpruefung der Funktion
      int select_entry (int = 0);       // Wird in find_entry verwendet

   protected:
      struct {
         char name[80],
              netzg[80],
              typ[80],
              einheit[80],
              fkt[80];
         double xanfang,
                xende,
                yanfang,
                yende,
                a[6];
         double x,
                y;
      } data;

   public:
      pgcalib (FILE * = stdout);
      ~pgcalib ();

      char *get_calib_dbname ();
      int select_key (char *);
      int select_all ();

      int find_entry (char *, char *, char *, double, int = 0);
      int find_entry (char *, char *, char *, char *, int = 0);
      int find_entry (char *, double, int = 0);
      int find_entry (char *, int &);
      double berechne_wert (int = 0);
      char *return_name ();

      int get_key_data ();
      int get_key_data (char *);

      int get_data ();
      int get_all_data (char *);

      int insert_data (int = -1);
      int delete_data (int = -1);

      void reset_fehler ();

      int print_wert ();
      int print_tuples ();
      int print_fehler (int = -1);

      int set_retval ();
   };

# endif

