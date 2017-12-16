/*
 *	upload from target memory, communicate with remote miniloader
 *	write to stdout
 *
 *	Copyright (c) KFA Juelich
 *	Autor: Michael Simon
 */

#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<unistd.h>
#include 	<string.h>
#include 	<fcntl.h>
#include 	<ctype.h>
#include 	<sys/types.h>
#include 	<sys/errno.h>

#ifdef HAVE_CONFIG_H
#include 	"config.h"
#endif
#include 	"pcap-int.h"
#include 	"defs.h"
#include 	"miniload.h"

#define 	MAXSREC	560

/* static char sccs_version[] = "@(#)upload.c 1.13	(mike)	10/14/91"; */

extern int errno;
extern u_char * atohex();
extern char *printVersion();

char        * ether_adr;
char        * lan_dev;
unsigned long adr;
int           bytes_to_read;
u_char        fmt;
int           timeout;
int	      retry;
int verbose;

void give_help( argv)
char * argv;
{
    char * prog = PACKAGE_NAME;

    puts( printVersion());
    printf( "\n%s - upload data from VMEbus-Targets", prog);
    printf( "\n%s [options] ether_adr", prog);
    printf( "\n    options are :");
    printf( "\n       -a   define address in hex");
    printf( "\n       -b   set fromat to BINARY");
    printf( "\n       -h   ...");
    printf( "\n       -l   set length as decimal");
    printf( "\n       -o   output lan device name");
    printf( "\n       -r   retry, default is %d", retry);
    printf( "\n       -s   set format to S_RECORD");
    printf( "\n       -t   timeout, default is %d", timeout);
    printf( "\n       -v   be verbose");
    printf( "\n    ether_adr is the Ethernet Address of the Commmunication\n");
    printf( "\n");
}

BOOLEAN get_options( argc, argv)
int argc;
char *argv[];
{
       extern int optind;
       extern int opterr;
    extern char * optarg;
              int n;
              int fflg = 0;
	      int errflg = 0;

    while(( n = getopt( argc, argv, "a:bhl:o:r:st:v")) != EOF) {
	switch( n) {
	case '?':
	    errflg++;
	    break;
	case 'a':
	    adr = strtoul( optarg, 0, 16);
	    break;
    	case 'b':
	    if( fflg)
		errflg++;
	    else {
		fmt = BINARY_FMT;
		fflg++;
	    }
	    break;
	case 'h':
	    give_help( argv[0]);
	    exit( 0);
	case 'l':
	    bytes_to_read = atoi( optarg);
	    break;
        case 'o':
            lan_dev = optarg;
            break;
	case 's':
	    if( fflg)
		errflg++;
	    else {
		fmt = S_RECORD;
		fflg++;
	    }
	    break;
	case 'r':
	    retry = atoi( optarg);
	    if( retry < 0) retry = 0;
	    break;
	case 't':
	    timeout = atoi( optarg);
	    if( timeout < 0) timeout = 0;
	    break;
	case 'v':
	    verbose=1;
	    break;
	}
    }
    if( errflg || optind >= argc) {
	give_help( argv[0]);
	return( FALSE);
    } else
	ether_adr = argv[optind];

    return( TRUE);
}

int
main( int argc, char *argv[])
{
    int result;
    int max_bytes;

      u_char *rxbuf, *txbuf;
      pcap_t *pd;

/*
 *	Initialize and setup with default values
 */
    fmt = S_RECORD;
    adr = 0x400;
    bytes_to_read = MAXSREC;
    lan_dev = NULL;
    timeout = 1;
    retry = 2;
    verbose = 0;
/*
 *	Configure and init LLA
 */
    if( get_options( argc, argv) == FALSE)
	exit( 1);
    if(( pd = init_lla( lan_dev, ether_adr, REQ_SAP, SER_SAP, UI_CONTROL, timeout)) == NULL)
	exit( 123);

    txbuf = pd->tp;
    rxbuf = pd->rp;

/*
 * calculate max_bytes from fmt
 */
    if( fmt == S_RECORD)
	max_bytes = MAXSREC;
    else if( fmt == BINARY_FMT)
	max_bytes = PACKET_SIZE - OVHEAD_SIZE;
    else
	max_bytes = 128;	/* willkuerlich */

/*
 *	Main Loop, write request to target, read data and output to stdout
 */
    for( ; bytes_to_read > 0; ) {
	int rty;
	u_char * ptr;
	   short len;

	rty = retry;
	len = bytes_to_read>max_bytes ? max_bytes: bytes_to_read;
	ptr = pack_frame( txbuf, UPLOAD|fmt, len, adr);
	bytes_to_read -= len;
	adr += len;

	if(( result = pcap_write( pd, OVHEAD_SIZE)) < 1) {
	    fprintf( stderr, "%s: cannot write, errno = %d\n", argv[0], errno);
	    exit( 2);
	}
        while( rty-- > 0) {
	    if( verbose ) fprintf( stderr, "ask for %lx %d\n", adr-len, len);
	    if((result=pcap_read( pd)) != -2) {
                break;
            } else if(  pcap_write( pd, OVHEAD_SIZE) < 1) {
		fprintf( stderr, "%s: cannot write, errno = %d, retry = %d\n",
				argv[0], errno, rty);
		exit( 3);
	    }
        }
        if( result == -1) {
            fprintf( stderr, "%s: Network read failed, errno = %d\n",
                     argv[0], errno);
            exit( 4);
        } else if (rty <= 0) {
            fprintf( stderr, "%s: Network read timeout\n", argv[0]);
            exit( 5);
	} else {
	    int got;
	    ptr = unpack_frame( rxbuf,  0, 0, & got);
	    if( got < 0) {
		fprintf( stderr, "%s: bad answer[%d]\n", argv[0], -got);
	    } else if( got > 0)
		(void)write( 1, ptr, got);
	}
    }
    exit( 0);
}
