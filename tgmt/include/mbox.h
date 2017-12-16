/*
 *	Copyright (c) 1989 KFA Juelich GmbH
 *	Author Michael Simon
 */
#include 	<types.h>

/*
 *	Die Mailbox ist als einfaches Feld von Bytes jeweils fuer Read und
 *	Write angelegt.
 *	Die Bezeichnung Read und Write beziehen sich auf die Sichtweise der
 *	Applikationen auf weiteren VMEbus-Targtes.
 */

#define 	MBLEN		128
#define 	MBCNT		8
#define 	READBUF		0
#define 	WRITEBUF	(MBLEN*MBCNT)

/*
 *
 */

struct fifo_msg {
	unsigned adr;
	char data[MBLEN];
} ;
