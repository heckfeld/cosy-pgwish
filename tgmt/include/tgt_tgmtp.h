/* $Header$ */

#ifndef TGT_TGMTP_INCLUDED
# define TGT_TGMTP_INCLUDED


/* Name der Datenbank */
char *dbName = "tgmt";

/* Hier solldie Environment-Variable verwendet werden */
/* Postgres-Host */
char *pghost = (char *) 0;


#ifndef mit_makro_DATENBANK_NAME
/*
 * Macro zur Bestimmung des absoluten Pfadnamen fuer eine Datenbank.
 */

# define DATENBANK_NAME(path,name) \
{ \
/* Directory ist $COSMODB oder $PWD, falls COSMODB nicht gesetzt. */ \
   strcpy (path, getenv ("COSMODB")); \
   if (! path[0]) \
      strcpy (path, getenv ("PWD")); \
   strcat (path, "/"); \
   strcat (path, name); \
}
#endif

/*
 * Macro zum Oeffen einer Postgres-Datenbank.
 */

#define DATENBANK_OEFFNEN(conn,pghost,pgport,pgoptions,pgtty,dbName) { \
   conn = PQsetdb (pghost, pgport, pgoptions, pgtty, dbName); \
   if (PQstatus (conn) == CONNECTION_BAD) { \
      fprintf (stderr, "Connection to database '%s' failed.\n", dbName); \
      PQfinish (conn); \
      exit (2); \
   } \
}

/*
 * Macro zum Lesen eines Strings, als leere Zeile bzw. ohne fuehrende,
 * abschliessende White-Space-Zeichen.
 */

#define fgetstr(fp,str,hlp,n) \
{  \
   memset ((void *) hlp, '\0', (size_t) n); \
   *str = '\0'; \
   fgets (hlp, n, fp); \
   sscanf (hlp, "%s", str); \
}

#endif
