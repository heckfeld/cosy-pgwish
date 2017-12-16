/*
 *	%Z% %M%		%I%	(mike)		%H%
 */

#include <sys/types.h>

#define 	FORMAT_MASK	0x0f
#define 	S_RECORD	0x01
#define 	HP_ABS_FMT	0x02
#define 	MUFOM_FMT	0x03
#define 	BINARY_FMT	0x04

#define 	MODE_MASK	0xf0
#define 	DOWNLOAD	0x10
#define 	START		0x20
#define 	UPLOAD		0x30
#define 	MAILREAD	0x40
#define 	MAILWRITE	0x50
#define 	MAILBOX		0x60
#define 	TRANSFER	0x70
#define 	DEBUG		0x80

/*
 *	Definition der Offsets in einem Char-Array aehnlich der struct packet.
 */

#define 	ML_FUNC	0	/* 1 Byte u_char */
#define 	ML_LENGTH	1	/* 2 Byte */
#define 	ML_ADDR	3	/* 4 Byte */
#define 	ML_DATA	7	/* up to 1400 Byte */

#define 	DATA_SIZE	1400
#define 	PACKET_SIZE	(DATA_SIZE+ML_DATA)
#define 	OVHEAD_SIZE	ML_DATA

struct packet {
	unsigned func;
	unsigned char * adr;
	unsigned len;
	unsigned char data[DATA_SIZE];
} ;

#define 	SER_SAP		0x12	/* LSAP des mikro-Loaders */
#define 	REQ_SAP		0x14
#define 	MBX_SAP		0x16

#define UI_CONTROL      0x03
#define TEST_CONTROL    0xF3
#define XID_CONTROL     0xBF


extern u_char * pack_frame( u_char *, char, short, int);
extern u_char * unpack_frame( u_char *, char *, short *, int *);

extern void put_short( u_char *, u_short);
extern short get_short( u_char *);
extern void put_int( u_char *, u_int);
extern int get_int( u_char *);
extern void put_long( u_char *, u_long);
extern long get_long( u_char *);
