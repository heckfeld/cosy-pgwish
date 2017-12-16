/* $Header: /mnt/cc-x4/ccp/database/h/rcs/pgdefs.h,v 1.1 2003/02/03 15:13:37 tine Exp $ */

#ifndef PGDEFS_INCLUDED
# define PGDEFS_INCLUDED

/*
 * Macro zum Oeffen einer Postgres-Datenbank.
 * Das Macro zum Oeffnen einer PMF-Datenbank hat denselben Namen !!!
 */

# define DATENBANK_OEFFNEN(conn,pghost,pgport,pgoptions,pgtty,dbName) { \
   conn = PQsetdb (pghost, pgport, pgoptions, pgtty, dbName); \
   if (PQstatus (conn) == CONNECTION_BAD) { \
      fprintf (stderr, "Connection to database '%s' failed.\n", dbName); \
      PQfinish (conn); \
      exit (2); \
   } \
}

#endif
