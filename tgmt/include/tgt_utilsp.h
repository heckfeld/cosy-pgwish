/* $Header$ */

#ifndef TGT_UTILSP_INCLUDED
# define TGT_UTILSP_INCLUDED

/* Definitionen fuer Adressen und Subnetze eines Targets */

# define EALEN   14          /* '0' + 'x' + 12 Hex-Ziffern */
# define SUBN    10          /* max. Anzahl einzutragender Subnetze */
# define IADDR   10          /* max. Anzahl einzutragender Internetadressen */
# define LEN     16          /* max. Laenge einer Internetadresse */

typedef struct {
   int n;                   /* Index in untenstehenden Feldern */
   int subn[SUBN];          /* Subnetze aus den Internetadressen */
   char iaddr[IADDR][LEN];  /* alle Internetadressen */
} target_info;


/* Funktionen */

extern int istarget (char *, target_info *);
extern int isea (char *);
extern int iswc (char *, target_info *);
extern int isla (char *);
extern int isposint (char *);
extern int isposint0 (char *);
extern int ismask (char [5][64], int);
extern int ishexzahl (char *);

#endif
