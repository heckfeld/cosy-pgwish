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
      double Ep;        // Ruhemasse des Protons
      double c;         // Lichtgeschwindigkeit im Vakuum

   public:
      pgcosmocalib (FILE * = stdout);
      ~pgcosmocalib ();

      double get_Pi ();
      double get_Ep ();
      double get_c ();

      void print_Pi ();
      void print_Ep ();
      void print_c ();
   };

# endif

