/*	Copyright (c) 1988 Stollmann GmbH, Hamburg
 *	author: am
 *	%Z%%M%	%I% (mike) %H%
 */

/*	general host - server interface via mailbox
	the mailbox mb_box is placed in the server ram
 */


/*	data buffers.
 *	there are two buffers,
 *		the server sends data to the host via the serverbuf
 *		the host sends data to the server via the hostbuf
 *		see below
 *	begin is a relativ address, the buffer itself
 *	is located at mb_buf + begin;
 *	thus problems with inhomogenous address space are avoided
 *	operations on the buffer need no semaphores
 *	there can be from 0 to size-1 bytes in the buffer
 */

typedef unsigned short	ushort;
typedef unsigned long	ulong;
typedef ulong		raddr;

struct mb_buf {
	unsigned char *	begin, * end;
	int	count;
	unsigned char *	rdptr;
	unsigned char *	wrptr;
};

#define mb_incadr(buf, ptr) \
	( ptr+1 >= (buf)->end? (buf)->begin: ptr+1 )

#define mb_isdata(buf) 			/* is any data in the buffer */ \
	( (buf)->rdptr != (buf)->wrptr )

#define mb_datacnt(buf)			/* number of data in buffer */ \
	((buf)->count)

#define mb_isfree(buf)			/* is there space in the buffer */  \
	( (buf)->rdptr != mb_incadr((buf), (buf)->wrptr) )

#define mb_putchar(buf, c)		/* put a char into the buffer	*/ \
     {	*(buf)->wrptr = c; \
	(buf)->wrptr = mb_incadr((buf), (buf)->wrptr); \
     }

#define mb_getchar(buf, c)		/* get a char from the buffer	*/ \
     {	c = *(buf)->rdptr; \
	(buf)->rdptr = mb_incadr((buf), (buf)->rdptr); \
     }

#define mb_init(buf, start, size)	/* initialize the mbuf struct */ \
     {  (buf)->begin = (unsigned char *)(start); \
	(buf)->end = (unsigned char *)((start) + (size)); \
	(buf)->count = 0; \
	(buf)->rdptr = (buf)->wrptr = (unsigned char *)(start); \
     }

/*	the structure mb_info
 */

typedef enum intby { none, byint, bymsgrd, bymsgwr } intby;

struct mb_info	{
	ushort	mb_open	:  1,		/* 00: is open ?		*/
		mb_canint: 1;		/*     flag: can interrupt	*/
	intby	mb_intby:  2;		/*     interrupt class		*/
	ushort	mb_ilvl:   3,		/*     interrupt level		*/
		mb_fill:   9;
	ushort	mb_cmd;			/* 02: command to send		*/
	ulong	mb_ivec;		/* 04: intvec or message int address*/
	ulong	mb_private;		/* 08: private data		*/
		};			/* 0C:				*/

/*	the mailbox itself
 */

struct mb_box	{
	ulong		mb_magic;	/* 00:				*/
	ulong		unused;		/* 04:				*/
	struct mb_info	hostinfo,	/* 08: info supplied by host	*/
			serverinfo;	/* 14:             by server	*/
	struct mb_buf	hostbuf,	/* 20: data from host to server */
			serverbuf;	/* 30:           server to host */
		};			/* 40:				*/

#define MBMAGIC	0x070588

/*	After reset:
 *	the server initialises mb_box:
 *		serverinfo.mb_intby, mb_ilvl, mb_ivec
 *		mb_magic
 *	host open:
 *		check mb_magic
 *		hostinfo.mb_intby, mb_ilvl, mb_ivec
 *		if host can respond to server int request:
 *			hostinfo.mb_canint = 1;
 *		hostinfo.mb_open = 1
 *	server respond:
 *		setup hostbuf, serverbuf,
 *		if server can respond to host int request:
 *			serverinfo.mb_canint = 1;
 *		serverinfo.mb_open = 1
 */
