/*-------------------------------------------------------------------------
 *
 * pg_parg.h--
 *	  definition of the system "parg" relation (pg_parg)
 *	  along with the relation's initial contents.
 *
 * [whatever this relation was, it doesn't seem to be used anymore --djm]
 *
 * Copyright (c) 1994, Regents of the University of California
 *
 * $Id: pg_parg.h,v 1.5 1997/09/08 02:35:21 momjian Exp $
 *
 * NOTES
 *	  the genbki.sh script reads this file and generates .bki
 *	  information from the DATA() statements.
 *
 *-------------------------------------------------------------------------
 */
#ifndef PG_PARG_H
#define PG_PARG_H

/* ----------------
 *		postgres.h contains the system type definintions and the
 *		CATALOG(), BOOTSTRAP and DATA() sugar words so this file
 *		can be read by both genbki.sh and the C compiler.
 * ----------------
 */

/* ----------------
 *		pg_parg definition.  cpp turns this into
 *		typedef struct FormData_pg_parg
 * ----------------
 */
CATALOG(pg_parg)
{
	Oid			parproid;
	int2		parnum;
	char		parbound;
	Oid			partype;
} FormData_pg_parg;

/* ----------------
 *		Form_pg_parg corresponds to a pointer to a tuple with
 *		the format of pg_parg relation.
 * ----------------
 */
typedef FormData_pg_parg *Form_pg_parg;

/* ----------------
 *		compiler constants for pg_parg
 * ----------------
 */
#define Natts_pg_parg					4
#define Anum_pg_parg_parproid			1
#define Anum_pg_parg_parnum				2
#define Anum_pg_parg_parbound			3
#define Anum_pg_parg_partype			4

/* ----------------
 *		initial contents of pg_parg
 * ----------------
 */

DATA(insert OID = 0 (  1242 1 - 23 ));
DATA(insert OID = 0 (  1243 1 - 16 ));
DATA(insert OID = 0 (  1244 1 - 23 ));
DATA(insert OID = 0 (  31 1 - 17 ));
DATA(insert OID = 0 (  1245 1 - 23 ));
DATA(insert OID = 0 (  33 1 - 18 ));
DATA(insert OID = 0 (  34 1 - 23 ));
DATA(insert OID = 0 (  35 1 - 19 ));
DATA(insert OID = 0 (  36 1 - 23 ));
DATA(insert OID = 0 (  37 1 - 20 ));
DATA(insert OID = 0 (  38 1 - 23 ));
DATA(insert OID = 0 (  39 1 - 21 ));
DATA(insert OID = 0 (  40 1 - 23 ));
DATA(insert OID = 0 (  41 1 - 22 ));
DATA(insert OID = 0 (  42 1 - 23 ));
DATA(insert OID = 0 (  43 1 - 23 ));
DATA(insert OID = 0 (  44 1 - 23 ));
DATA(insert OID = 0 (  45 1 - 24 ));
DATA(insert OID = 0 (  46 1 - 23 ));
DATA(insert OID = 0 (  47 1 - 25 ));
DATA(insert OID = 0 (  50 1 - 23 ));
DATA(insert OID = 0 (  50 2 - 23 ));
DATA(insert OID = 0 (  50 3 - 23 ));
DATA(insert OID = 0 (  51 1 - 23 ));
DATA(insert OID = 0 (  52 1 - 23 ));
DATA(insert OID = 0 (  52 2 - 23 ));
DATA(insert OID = 0 (  52 3 - 23 ));
DATA(insert OID = 0 (  52 4 - 23 ));
DATA(insert OID = 0 (  53 1 - 23 ));
DATA(insert OID = 0 (  54 1 - 23 ));
DATA(insert OID = 0 (  54 2 - 23 ));
DATA(insert OID = 0 (  55 1 - 23 ));
DATA(insert OID = 0 (  55 2 - 23 ));
DATA(insert OID = 0 (  56 1 - 23 ));
DATA(insert OID = 0 (  56 2 - 23 ));
DATA(insert OID = 0 (  57 1 - 23 ));
DATA(insert OID = 0 (  57 2 - 23 ));
DATA(insert OID = 0 (  57 3 - 23 ));
DATA(insert OID = 0 (  60 1 - 16 ));
DATA(insert OID = 0 (  60 2 - 16 ));
DATA(insert OID = 0 (  61 1 - 18 ));
DATA(insert OID = 0 (  61 2 - 18 ));
DATA(insert OID = 0 (  63 1 - 21 ));
DATA(insert OID = 0 (  63 2 - 21 ));
DATA(insert OID = 0 (  64 1 - 21 ));
DATA(insert OID = 0 (  64 2 - 21 ));
DATA(insert OID = 0 (  65 1 - 23 ));
DATA(insert OID = 0 (  65 2 - 23 ));
DATA(insert OID = 0 (  66 1 - 23 ));
DATA(insert OID = 0 (  66 2 - 23 ));
DATA(insert OID = 0 (  67 1 - 25 ));
DATA(insert OID = 0 (  67 2 - 25 ));

#endif							/* PG_PARG_H */
