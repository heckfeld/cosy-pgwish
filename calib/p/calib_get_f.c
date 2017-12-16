
/* $Header$ */

/******************************************************************************/
/*                                                                            */
/* Aufruf       : calib_get_f (double a[6], double x);                        */
/*                                                                            */
/* Funktion     : Berechnung des Funktionswertes eines Polynoms 5. Grades     */
/*                an der Stelle x. Die Koeffizienten sind in a[0] - a[5]      */
/*                abgelegt.                                                   */
/*                                                                            */
/* Rueckgabewert: Funktionswert an der Stelle x                               */
/*                                                                            */
/******************************************************************************/

double calib_get_f (a, x)
double a[6];
double x;

{
   int i;
   double y;

   for (y=a[5], i=4; i>=0; --i)
      y = a[i] + (x * y);

   return (y);
}
