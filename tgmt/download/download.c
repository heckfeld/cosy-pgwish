/*
 *  download a file to target, communicate with remote miniloader
 */

static char sccs_version[] = "%Z%%M% %I%    (mike)  %H%";

#include    <stdio.h>
#include    <stdlib.h>
#include    <unistd.h>
#include    <string.h>
#include    <fcntl.h>
#include    <ctype.h>
#include    <errno.h>
#include    <sys/types.h>
#include    <sys/time.h>
#include <signal.h>

#ifdef HAVE_CONFIG_H
#include    "config.h"
#endif
#include    "pcap-int.h"
#include    "defs.h"
#include    "miniload.h"

extern int errno;
extern char *printVersion();

char * ether_adr;
char * lan_device;
int verbose;
int timeout;
int retry;
int packetnum;
int reset;
int debug;

void give_help( argv)
char * argv;
{
    char * prog = PACKAGE_NAME;

    puts( printVersion());
    printf( "\n%s - load programs on VMEbus\n", prog);
    printf( "\nUsage: %s [ -Shortv] ether_adr\n", prog);
    printf( "\noptions are :\n");
    printf( "        -h ...\n");
    printf( "        -o output lan device name\n");
    printf( "        -t timeout, default is %d\n", timeout);
    printf( "        -r retry, default is %d\n", retry);
    printf( "        -v verbose\n");
    printf( "        -S reset\n");
}

void get_options( argc, argv)
int argc;
char *argv[];
{
    extern int optind, opterr;
    extern char * optarg;
    int c, errflg;

    retry = 1;
    timeout = 2;
    packetnum = 0;
    verbose = 0;
    errflg = 0;
    reset = 0;
    debug = 0;
    while(( c = getopt( argc, argv, "Sdho:r:t:v")) != EOF) {
        switch( c) {
        case 'o':
	    lan_device = optarg;
            break;
        case 'r':
	    retry = atoi(optarg);
            break;
        case 't':
	    timeout = atoi(optarg);
            break;
        case 'v':
	    verbose++;
            break;
        case 'S':
	    reset++;
            break;
        case 'd':
	    debug++;
            break;
        case 'h':
	    errflg++;
            break;
        case '?':
	    errflg++;
            break;
        }
    }
    if( errflg) {
	give_help( argv[0]);
	exit( 99);
    }
    if( optind < argc)
        ether_adr = argv[optind];
    else {
        give_help( argv[0]);
        exit( 99);
    }
    if( retry < 0 ) retry = 0;
    if( retry > 10 ) retry = 10;
    if( timeout < 1 ) timeout = 1;
    if( timeout > 20 ) timeout = 20;
}

static char *numtostr[] = {
	"first", "second", "third", "fourth", "fifth", "sixth", "seventh",
	"eight", "nineth", "tenth" };

char * progname;

int
download( pcap_t *pd, int len, int nr)
{
    int cc, result, xi;
    extern int packetnum;

    xi = nr;

    if (verbose>2) fprintf( stderr, "%s[%d]: %s timeout %d retry %d\n",
				__FILE__, __LINE__, __func__,
				timeout, nr);

    result = -2;	/* timeout */
    while( nr >= 0) {
	if (verbose>3) {
		fprintf( stderr, "download frame #%d the %s time\n",
			packetnum, numtostr[xi-nr]);
	}
	if (verbose) {
	    if(nr==xi) {
		fputc( '.', stderr);
	    } else {
		fputc( 'x', stderr);
	    }
	}
	nr--;
	if(( cc = pcap_write( pd, len)) != len) {
	    return -1;
	}

	if ((cc = pcap_read( pd)) == -1) {	/* error */
	    if (verbose) fprintf( stderr, "%s[%d]: %s pcap_read failed with %d\n"
			__FILE__, __LINE__, __func__, cc);
	    return -1;
	} else if (cc >= 0) {		/* ok */
	    packetnum++;
	    result = cc;
	    break;
	} else {
	    int x = nr;			/* timeout */
	}
    }
    return( result);			/* timeout or cc */
}

extern pcap_t * init_lla();

int
main( argc, argv)
int argc;
char *argv[];
{
    char line[ 256];
    int bytes_to_write = 0,
        result,
        len;
    BOOLEAN full;
    char * out;
    u_char *rxbuf,
           *txbuf;
    pcap_t *pd;

    full = FALSE;
    lan_device = NULL;

    get_options( argc, argv);

    progname = argv[0];

    if(( pd = init_lla( lan_device, ether_adr, REQ_SAP, SER_SAP, UI_CONTROL, timeout)) == NULL)
        exit( 1);

    txbuf = pd->tp;
    rxbuf = pd->rp;

    out = pack_frame( txbuf, 0, 0, 0);

    while( fgets( line, sizeof(line), stdin) != NULL) {
        if( line[0] != 'S') 
            continue;
        if( line[1] == '0')
            continue;
	if( line[strlen(line)-1] == '\n')
	    line[strlen(line)-1] = '\0';

        if((PACKET_SIZE-OVHEAD_SIZE) - bytes_to_write < (len = strlen( line)))
            full = TRUE;
        else {
            strncpy( out+bytes_to_write, line, len);
            bytes_to_write += len;
        }

        /*
         *  Buffer is full, write Contents to NETWORK
         */
        if( full == TRUE) {
            (void) pack_frame( txbuf, DOWNLOAD|S_RECORD, bytes_to_write, 0);
            bytes_to_write += OVHEAD_SIZE;
            result = download( pd, bytes_to_write, retry);
            if( result == -1) {
                fprintf( stderr, "I/O Fehler auf Ethernet\n");
                exit( 2);
            } else if( result == -2) {
                fprintf(stderr, "Timeout beim Lesen, Target antwortet nicht\n");
                exit( 3);
            } else {
                int l;

                (void) unpack_frame( rxbuf, 0, 0, & l);
                if( l < 0) {
                    fprintf( stderr, "%s: Fehler[%d] auf dem Target\n",
			progname, l);
                } else if( verbose)
		    /*printf("Record %d\n", l)*/;
            }

            full = FALSE;
            bytes_to_write = 0;

            /*  copy bytes left in line-Buffer to output    */
            strncpy( out, line, len);
            bytes_to_write += len;
        } /* END full */
    } /* END while */

    if( bytes_to_write > 0) {
        (void) pack_frame( txbuf, DOWNLOAD | S_RECORD, bytes_to_write, 0);
        result = download( pd, bytes_to_write+OVHEAD_SIZE, retry);
        if( result == -1) {
            fprintf( stderr, "I/O Fehler auf Ethernet\n");
            exit( 2);
        } else if( result == -2) {
            fprintf(stderr, "Timeout beim Lesen, Target antwortet nicht\n");
            exit( 3);
        } else {
            int l;

            (void) unpack_frame( rxbuf, 0, 0, & l);
            if( l == 0)
                fprintf( stderr, "%s: Fehler[%d] auf dem Target\n",
			progname, l);
            else if( verbose)
		 /*printf( "Record %d\n", l)*/;
        } 
    } /* END bytes_to_write */

    if( reset) {
        (void) pack_frame( txbuf, debug == TRUE ? DEBUG : START, 0, 0);
        if( verbose) printf( debug == TRUE ? "Debug remote process\n" : "RESET VMEbus\n");
        result = download( pd, OVHEAD_SIZE, 0);
    } /* END start */


    if( verbose)
	 printf( "\n");
    exit( 0);
}
