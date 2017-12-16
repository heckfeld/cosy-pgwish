/*
 *
 * Copyright (c) 1991 Forschungszentrum Juelich
 * Author : Michael Simon (AP)
 * All rights reserved.
 *
 * %Z%%M%	%I% (mike) %H%
 *
 */
#ifndef PAD_INCLUDED
#define PAD_INCLUDED

#ifndef EOF
#define 	EOF		(-1)
#endif
#ifndef NULL
#define 	NULL		0
#endif

#ifndef MIN
#define MIN(a,b)	((a)<(b) ? (a) : (b))
#endif

#define 	SER_PAD		0x20
#define 	REQ_PAD		0x22

#define 	LAN_TIMEOUT	1000

/*
 *      -----------------
 *      | | | |P| | | |T|      Flag Field of pad_pdu
 *      -----------------
 *       ^ ^ ^ ^ ^ ^ ^ ^
 *       | | | | | | | -- Frame-Type
 *       | | | | -------- Function in SUPER / Send Sequence Number in INFO
 *       | | | ---------- Poll/Final Bit
 *       ---------------- Receive Sequence Number
 */

#define 	FRAME_TYPE	0x01	/* Bit Mask */
#define 	SUPERVISOR	1
#define 	INFORMATION	0
#define 	FUNCTION	0x0C	/* Bit Mask */
#define 	ACK		0x0C
#define 	NACK		0x04
#define 	POLL_FINAL	0x10	/* Bit Mask */
#define 	SEND_SEQ_NR	0x0E	/* Bit Mask */
#define 	REC_SEQ_NR	0xE0	/* Bit Mask */

/*	Values for state */
#define 	IDLE		001
#define 	WAIT_ACK	(~(IDLE))
#define 	CONNECTED	002

#define 	PADDATA		1497

struct pad_handle {
    int      fildes;

    short     state;
    u_char   station_adr[6];

    char   * inptr;
    int      incnt;

    char   * outptr;
    int      outcnt;

    char     outbase[PADDATA];
    char     inbase[PADDATA];
};

#define IDLE_STATE(hp)     ((hp)->state & IDLE)

extern void padres();
extern struct pad_handle * initPAD();

#define PUT_INT(a,b,c)  {   \
        a.reqtype=b;    \
        a.vtype=INTEGERTYPE;    \
        a.value.i=c; \
}

#define PUT_STR( a, b, c, d) {  \
        a.reqtype = b;  \
        a.vtype = d;    \
        memcpy( a.value.s, c, d); \
}

#define 	MAX_PAD_LCI		8

struct pad_config_entry {
	u_char lci;
	u_short type;
	char dest[12];
};
typedef struct pad_config_entry PAD_CONF;

#define 	DEST_SET	1
#define 	NORMAL		2

#define 	TRUE		1
#define 	FALSE		0

#endif
