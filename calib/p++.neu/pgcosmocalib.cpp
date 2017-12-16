//
// $Header$
//

# include <pgcosmocalib.neu.h>
# include <math.h>


//
// Constructor
// -----------
pgcosmocalib::pgcosmocalib (FILE *fp) : pgcalib (fp) {

   // PI
   Pi = M_PI;

   // Ruhemasse des Protons [GeV]
   if (find_entry ("mp", "const", "MeV", 0.0) == 0)
      Ep = berechne_wert ();
   Ep /= 1000.0;

   // Ruhemasse des Deuterons [GeV]
   if (find_entry ("md", "const", "MeV", 0.0) == 0)
      Ed = berechne_wert ();
   Ed /= 1000.0;

   // Ruhemasse des Elektrons [GeV]
   if (find_entry ("me", "const", "MeV", 0.0) == 0)
      Ee = berechne_wert ();
   Ee /= 1000.0;

   // Ruhemasse von D- [GeV]
   if (find_entry ("mdminus", "const", "MeV", 0.0) == 0)
      Edminus = berechne_wert ();
   Edminus /= 1000.0;

   // Ruhemasse von H- [GeV]
   if (find_entry ("mhminus", "const", "MeV", 0.0) == 0)
      Ehminus = berechne_wert ();
   Ehminus /= 1000.0;

   // allgemeine Ruhemasse mit der des Protons intialisieren
   E = Ep;

   // Lichtgeschwindigkeit im Vakuum
   if (find_entry ("c", "const", "m/s", 0.0) == 0)
      c = berechne_wert ();

#ifdef TEST
printf ("&v3Spgcosmocalib constructor\n");
#endif
}


//
// Destructor
// ----------
pgcosmocalib::~pgcosmocalib () {

   close ();

#ifdef TEST
printf ("&v3Spgcosmocalib destructor\n");
#endif
}


//
// Set E
// -----
void pgcosmocalib::set_E (double wert) {
   E = wert;
}


//
// Get Pi
// ------
double pgcosmocalib::get_Pi () {
   return (Pi);
}


//
// Get Ep
// ------
double pgcosmocalib::get_Ep () {
   return (Ep);
}


//
// Get Ed
// ------
double pgcosmocalib::get_Ed () {
   return (Ed);
}


//
// Get Ee
// ------
double pgcosmocalib::get_Ee () {
   return (Ee);
}


//
// Get Edminus
// -----------
double pgcosmocalib::get_Edminus () {
   return (Edminus);
}


//
// Get Ehminus
// -----------
double pgcosmocalib::get_Ehminus () {
   return (Ehminus);
}


//
// Get E
// -----
double pgcosmocalib::get_E () {
   return (E);
}


//
// Get C
// -----
double pgcosmocalib::get_c () {
   return (c);
}


//
// Print Pi
// --------
void pgcosmocalib::print_Pi () {
   fprintf (fpout, "%.10g\n", Pi);
   return;
}


//
// Print Ep
// --------
void pgcosmocalib::print_Ep () {
   fprintf (fpout, "%.10g\n", Ep);
   return;
}


//
// Print Ed
// --------
void pgcosmocalib::print_Ed () {
   fprintf (fpout, "%.10g\n", Ed);
   return;
}


//
// Print Ee
// --------
void pgcosmocalib::print_Ee () {
   fprintf (fpout, "%.10g\n", Ee);
   return;
}


//
// Print Edminus
// -------------
void pgcosmocalib::print_Edminus () {
   fprintf (fpout, "%.10g\n", Edminus);
   return;
}


//
// Print Ehminus
// -------------
void pgcosmocalib::print_Ehminus () {
   fprintf (fpout, "%.10g\n", Ehminus);
   return;
}


//
// Print E
// -------
void pgcosmocalib::print_E () {
   fprintf (fpout, "%.10g\n", E);
   return;
}


//
// Print C
// -------
void pgcosmocalib::print_c () {
   fprintf (fpout, "%.10g\n", c);
   return;
}

