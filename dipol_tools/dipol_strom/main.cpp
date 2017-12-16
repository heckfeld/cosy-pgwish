//
//  $Header: main.cpp,v 1.1 2001/11/19 15:39:47 tine Exp $
//
//  Berechung der Stromstaerke an
//  den Dipolen der Extraktionsbeamlines
//  bei gegebenen Impuls, Ablenkwinkel
//
//  Andreas Steinhaus
//  Tel: 3719
//  letzte Aenderung: 8.Maerz.2000
//
//  neue Anpassung von C. Deliege (ab 19.11.2001)
//  alter/neuer Aufruf: dipol_angle <impuls[MeV/c]>
//
// In der Cosmo-Calib-Datenbank sind Eintraege fuer die Umrechnungsfaktoren
// etc. enthalten.
// z.B. unter msep_B, msep_L, msep_Imax, msep_Angle, msep_BmalL
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Anbindung an die Cosmo-Calib-Datenbank
#include <pgcosmocalib.h>


//
// Umrechnung des Amperewertes in Prozent
//
double gui_strom (double strom, double Imax)
{
  return (strom / Imax * 100.0);
}


//
// Aufzaehlzyp fuer die Indizierung des Feldes der Elemente, letzter Eintrag
// spezifiziert die Anzahl der Elemente, fuer die Dimensionierung des Feldes.
//
enum name_list { msep=0, d27_30, d31_33, d32, d34_35, anzahl };


//
// Klasse fuer die Daten und Operationen eines Elementes
//
class element {
private:
   char *_gui;       // Name des Elements in der Oberflaeche
   char *_db;        // Name des Elements in der Datenbank
   char _einh;       // Einheit des uebergebenen Wertes, bewirkt u.U. Umrechnung
public:
   double Imax;      // Imax zum Element aus der Datenbank
   double BmalL;     // BmalL zum Element aus der Datenbank
   double B;         // B (ehem. feld) zum Element aus der Datenbank
   double gui_strom; // Strom fuer die Oberflaeche, in A|%
   double strom;     // Strom in Ampere
   double soll_angle;// fester Ablenkwinkel aus der Datenbank

   void init (char *gui_name, char *db_name, char einh) {
      _gui = (gui_name != (char *) 0) ? gui_name : (char *)"";
      _db = (db_name != (char *) 0) ? db_name : (char *)"";
      _einh = einh;
   }
   char *db () {
      return (_db);
   }
   char *gui () {
      return (_gui);
   }
   char einheit () {
      return (_einh);
   }
};
  

//
// Hauptprogramm
//
int main (int argc, char** argv) {

   pgcosmocalib calib_db;   // fuer den Zugriff auf die Cosmo-Calib-Datenbank
   element elem[anzahl];    // Liste der Elemente, fuer die Berechnung erfolgt

   double p;                // Impuls: Uebergabeparameter: argv[1]
   double brho;             // brho(p) aus der Cosmo-Calib-Datenbank

   int i;                   // Schleifenindex
   char db_entry[16];       // String fuer Uebergabe an Datenbank-Funktion
  

   if (argc != 2) {
      printf ("Falscher Aufruf!\n%s <impuls[MeV/c]>\n", argv[0]);
      exit (EXIT_FAILURE);
   }

   // Initialisierung der Oberflaechen- und Datenbank-Namen der Elemente
   elem[msep].init ("msep", "msep", 'A');
   elem[d27_30].init ("D27-30", "d27_30", '%');
   elem[d31_33].init ("D31-33", "d31_33", '%');
   elem[d32].init ("D32", "d32tof", '%');
   elem[d34_35].init ("D34/35", "d34_35", '%');

   // Impuls in GeV/c fuer Berechnung von brho aus der Datenbank
   p = strtod (argv[1], (char **) 0) / 1000.0;

   // brho(p) aus der Datenbank
   if (calib_db.find_entry ("brho", "calc", "GeV/c2Tm", p) == 0)
      brho = calib_db.berechne_wert();

 
   // Ausgabe des Ergebnisses als Tabelle
   printf ("\n");
   printf (" Name   | Sollwinkel|   Feld|   Strom|   Strom\n");
   printf ("        |     [Grad]|    [T]|     [A]|     [%%]\n");
   printf ("*-------|-----------|-------|--------|--------*\n");
  
   for (i=0; i<anzahl; i++) {
      // Falls beim Zugriff auf die Datenbank ein Fehler aufgetaucht ist,
      // diesen sicherheitshalber zuruecksetzen, damit dadurch keine weiteren
      // Berechnungen verhindert werden.
      calib_db.reset_fehler ();

      // Soll-Winkel aus der Datenbank
      sprintf (db_entry, "%s_Angle", elem[i].db());
      if (calib_db.find_entry (db_entry, "const", "Grad", elem[i].strom) == 0)
         elem[i].soll_angle = calib_db.berechne_wert();

      elem[i].BmalL = elem[i].soll_angle * brho / (180.0 / M_PI);

      // Ueber die Umkehrfunktion den Amperewert aus der Datenbank berechnen
      sprintf (db_entry, "%s_BmalL", elem[i].db());
      if (calib_db.find_entry (db_entry, "dipol", "A2Tm", elem[i].BmalL, 1)
       == 0)
         elem[i].strom = calib_db.berechne_wert(1);

      // B mit Umrechnungspolynom aus der Datenbank berechnen
      sprintf (db_entry, "%s_B", elem[i].db());
      if (calib_db.find_entry (db_entry, "dipol", "A2T", elem[i].strom) == 0) {
         elem[i].B = calib_db.berechne_wert();
      }

      // Umrechnung des Stroms in die Einheit der GUI
      if (elem[i].einheit() == '%') {
         sprintf (db_entry, "%s_Imax", elem[i].db());
         if (calib_db.find_entry (db_entry, "const", "A", 0.0) == 0)
            elem[i].Imax = calib_db.berechne_wert();
         elem[i].gui_strom = gui_strom (elem[i].strom, elem[i].Imax);
      } else {
         // (elem[i].einheit() == 'A')
         elem[i].gui_strom = elem[i].strom;
      }

      // Ausgabe des Ergebnisses
      printf (" %-7s %11.2lf %7.4lf %8.2lf %8.2lf\n",
             elem[i].gui(), elem[i].soll_angle, elem[i].B,
             elem[i].strom, elem[i].gui_strom);

   } // for-Schleife (0 .. anzahl-1)
}
