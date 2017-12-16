//
// $Header: cosmocalib.h,v 1.1 97/08/18 14:44:02 tine Exp $
//

# ifndef PGCOSMOCALIB_INCLUDED
#  define PGCOSMOCALIB_INCLUDED

# include <pgcalib.h>


//
// Class pgcosmocalib
// ------------------
//
   class pgcosmocalib : public pgcalib {
   protected:
      double Pi;        // PI
      double Ep;        // Ruhemasse des Protons [GeV]
      double Ed;        // Ruhemasse des Deuterons [GeV]
      double Ee;        // Ruhemasse des Elektrons [GeV]
      double Edminus;   // Ruhemasse von D- [GeV]
      double Ehminus;   // Ruhemasse von H- [GeV]
      double E;         // Ruhemasse fuer die Berechnungen [GeV]
      double c;         // Lichtgeschwindigkeit im Vakuum

   public:
      pgcosmocalib (FILE * = stdout);
      ~pgcosmocalib ();

      void set_E (double);

      double get_Pi ();
      double get_Ep ();
      double get_Ed ();
      double get_Ee ();
      double get_Edminus ();
      double get_Ehminus ();
      double get_E ();
      double get_c ();

      void print_Pi ();
      void print_Ep ();
      void print_Ed ();
      void print_Ee ();
      void print_Edminus ();
      void print_Ehminus ();
      void print_E ();
      void print_c ();
   };

# endif

