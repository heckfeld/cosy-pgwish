//
//  $Header: main.cpp,v 1.5 2001/11/19 09:47:06 tine Exp $
//
//  Berechung des Ablenkwinkels  
//  an den Dipolen der Extraktionsbeamlines 
//  bei gegebenen Impuls, Stromstaerke
//
//  Andreas Steinhaus
//  Tel: 3719
//  letzte Aenderung: 8.Maerz.2000
//
//  neue Anpassung von C. Deliege (ab 07.11.2001)
//  neuer Aufruf: dipol_angle <impuls[MeV/c]> <soll(msep)[A]> \
//                            <soll(D27-30)[%]> <soll(D31-33)[%]> \
//                            <soll(D32)[%]> <soll(D34/35)[%]>
//
// In der Cosmo-Calib-Datenbank sind Eintraege fuer die Umrechnungsfaktoren
// etc. enthalten.
// z.B. unter msep_B, msep_L, msep_Imax
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Anbindung an die Cosmo-Calib-Datenbank
#include <pgcosmocalib.h>


//
// Berechnung des Ablenkwinkels
//
double getAngle (double sollfeld, double leff, double brho) {
   return (sollfeld * (180 / M_PI) * leff / brho);
}


//
// Umrechnung des Prozentwertes in Ampere
//
double strom (double gui, double Imax)
{
  return (gui * Imax / 100.0);
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
   int rechnen;      // Flag, ob Berechnungen und Ausgabe erfolgen sollen
   double Imax;      // Imax zum Element aus der Datenbank
   double L;         // L (ehem. Leff) zum Element aus der Datenbank
   double B;         // B (ehem. feld) zum Element aus der Datenbank
   double gui_strom; // Strom aus der Oberflaeche (Uebergabeparameter), in A|%
   double strom;     // Strom in Ampere
   double angle;     // berechneter Ablenkwinkel
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
  

   if (argc != 7) {
      printf ("Falscher Aufruf!\n%s <impuls[MeV/c]> "
              "<soll(msep)[A]> <soll(D27-30)[\%]> <soll(D31-33)[\%]> "
              "<soll(D32)[\%]> <soll(D34/35)[\%]>\n", argv[0]);
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

   // Die restlichen Argumente auswerten/umwandeln
   for (i=0; i<argc-2; ++i) {

      // Wenn -1 uebergeben wurde, soll weder Berechnung noch Ausgabe erfolgen.
      elem[i].rechnen =
       ((elem[i].gui_strom = strtod (argv[i+2], (char **) 0)) == -1) ? 0 : 1;

      if (elem[i].rechnen) {
         if (elem[i].einheit() == '%') {
            sprintf (db_entry, "%s_Imax", elem[i].db());
            if (calib_db.find_entry (db_entry, "const", "A", 0.0) == 0)
               elem[i].Imax = calib_db.berechne_wert();
            elem[i].strom = strom (elem[i].gui_strom, elem[i].Imax);
         } else {
            // (elem[i].einheit() == 'A')
            elem[i].strom = elem[i].gui_strom;
         }
      }
   }

   // brho(p) aus der Datenbank
   if (calib_db.find_entry ("brho", "calc", "GeV/c2Tm", p) == 0)
      brho = calib_db.berechne_wert();

 
   // Ausgabe des Ergebnisses als Tabelle
   printf ("\n");
 //printf (" Berechnung fuer Impuls = %.3lf MeV/c\n", (p * 1000.0));
 //printf ("*-----------------------------------------------------------*\n");
   printf (" Name   |   Strom|   Strom|   Feld| Ablenkwinkel| Sollwinkel\n");
   printf ("        |     [%%]|     [A]|    [T]|       [Grad]|     [Grad]\n");
   printf ("*-------|--------|--------|-------|-------------|-----------*\n");
  
   for (i=0; i<anzahl; i++) {
      if (elem[i].rechnen) {
         // Falls beim Zugriff auf die Datenbank ein Fehler aufgetaucht ist,
         // diesen sicherheitshalber zuruecksetzen, damit dadurch keine weiteren
         // Berechnungen verhindert werden.
         calib_db.reset_fehler ();

         // Soll-Winkel aus der Datenbank
         sprintf (db_entry, "%s_Angle", elem[i].db());
         if (calib_db.find_entry (db_entry, "const", "Grad", elem[i].strom)
          == 0)
            elem[i].soll_angle = calib_db.berechne_wert();

         // feld mit Umrechnungspolynom aus der Datenbank berechnen
         sprintf (db_entry, "%s_B", elem[i].db());
         if (calib_db.find_entry (db_entry, "dipol", "A2T", elem[i].strom) == 0)
            elem[i].B = calib_db.berechne_wert();

         // Leff mit Umrechnungspolynom aus der Datenbank berechnen
         sprintf (db_entry, "%s_L", elem[i].db());
         if (calib_db.find_entry (db_entry, "dipol", "A2m", elem[i].strom)
          == 0) {
            elem[i].L = calib_db.berechne_wert();
         }

         elem[i].angle = getAngle (elem[i].B, elem[i].L, brho);

         // Ausgabe des Ergebnisses
         printf (" %-7s %8.2lf %8.2lf %7.4lf %13.2lf %11.2lf\n",
                elem[i].gui(), elem[i].gui_strom, elem[i].strom,
                elem[i].B, elem[i].angle, elem[i].soll_angle);
      }

   } // for-Schleife (0 .. anzahl-1)
}
