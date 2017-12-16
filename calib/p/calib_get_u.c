
/* $Header$ */

/******************************************************************************/
/*                                                                            */
/* Aufruf       : calib_get_u (double a[6], double x);                        */
/*                                                                            */
/* Funktion     : Berechnung des des Wertes der Umkehrfunktion eines Polynoms */
/*                5. Grades an der Stelle x.                                  */
/*                Die Koeffizienten sind in a[0] - a[5] abgelegt.             */
/*                                                                            */
/* Rueckgabewert: Wert der Umkehrfunktion an der Stelle x                     */
/*                                                                            */
/******************************************************************************/

/******************************************************************************
*****************************  calib_get_u.c   ********************************
***                                                                         ***
*** Errechnet den x-Wert einer Funktion bei gegebenen y-Wert (Umkehrfkt.)   ***
***                                                                         ***
*** Benutztes Verfahren : Pegasus                                           ***
***                                                                         ***
*** Funktionsname, Aufrufparameter und Rueckgabewert :                      ***
***                                                                         ***
*** double calib_get_u (double a[6], double y, double x_min, double x_max)  *** 
***                                                                         ***
**************************  Carsten Horn  02/96  ******************************
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>

#define sign(X) ( (X) < (0) ? (-1) : ((X) > (0) ? (1) : (0)) )
#define dabs(X) ( (X) * sign(X) )                    /* double abs function */

#define MAX_ITER 10000     /* zur Sicherheit falls der Alg. sich verfranst ! */
                           /* oder die Genauigkeit nicht erreicht wird.      */

#define GENAU 0.0000001


double calib_get_u(a,y,u,o)
double *a;                                        /* Array mit Koeffizienten */
double y;                                         /* y-Wert                  */
double u;                                         /* Untere Intervallgrenze  */ 
double o;                                         /* Obere Intervallgrenze   */
{
 double yu,yo,                                    /*   y-Werte zu x_min&max  */
        yhelp,xhelp;                              /*   Hilfs-'punkt'         */
 int schritte=0;                                  /* Algorithmus Schritte    */

/* Falls das Plynom genau eine Gerade ist, a[1] != 0.0 braucht nicht */
/* abgefragt werden, da Gerade ohne Steigung nicht uebergeben wird.  */
 if (a[2] == 0.0 && a[3] == 0.0 && a[4] == 0.0 && a[5] == 0.0)
  return ( (y - a[0]) / a[1] );

 a[0] -= y;                                       /* verschieben der y-Achse */

 yu    = calib_get_f(a,u);                        /* Start y-Wert fuer x_min */
 yo    = calib_get_f(a,o);                        /* Start y-Wert fuer x_max */

 if (dabs(yu) <= GENAU)                           /* das Pegasus verfahren   */
  return (u);                                     /* arbeitet nicht korrekt  */
                                                  /* wenn die Nullstelle     */
 if (dabs(yo) <= GENAU)                           /* gleich einer Intervall- */
  return (o);                                     /* grenze ist, deswegen    */
                                                  /* diese Abfragen !!!      */

 while ((dabs(u-o)>0) && (dabs(yu-yo)>GENAU) && (schritte<MAX_ITER))
 {
  schritte++;
  xhelp = o - ( yo * (u-o)/(yu-yo) );
  yhelp = calib_get_f(a,xhelp);

  if (yhelp == 0)
  {
     return xhelp;
  }
  if (sign(yhelp)*sign(yo) < 0)
  {
     u  = o;
     o  = xhelp;
     yu = yo;
     yo = yhelp;
  }
  else
  {
     yu *= ( yo/(yo+yhelp) );
     o   = xhelp;
     yo  = yhelp;
   }
  } /* while */

  return ( schritte != 0 ? dabs(u) < dabs(o) ? u : o  : 0);

} /* calib_get_u */
