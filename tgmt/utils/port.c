/*
 *
 * Copyright (c) 1991 Forschungszentrum Juelich
 * Author : Michael Simon (AP)
 * All rights reserved.
 *
 * %Z%%M%	%I% (mike) %H%
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <termio.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <getopt.h>


#define USE_SELECT
#define VERSION "0.98"

int             output;

int             port;
char          * hostname;

int		lflag;
char          * lname;

char            buf[BUFSIZ];

int get_sock_desc(char *ip_addr, int remote_port, int timeout);

int my_errno = 0;

void
error(s)
    char           *s;
{
    char * e = strerror( my_errno);

    if( s)
	fprintf( stderr, "Fehler: %s\n", e);
    exit(1);
}

void terminate()
{
    fprintf( stderr, "\n\tBye\n\n");
    exit( 1);
}

int
main(argc, argv)
    int         argc;
    char      * argv[];
{
    char      * prog;

    int         errflg;
    int         n;
    struct timeval tv;
    extern char * optarg;
    extern int  optind;

                                                             /* preset values */
    hostname  = NULL;
    port      = 20000;                                                /* port */

    prog = strrchr( argv[0], '/') ? strrchr( argv[0], '/') + 1: argv[0];
                                                               /* get options */

    errflg = 0;
    tv.tv_usec = 0;
    tv.tv_sec = -1;

    while(( n = getopt( argc, argv, "ht:v")) != EOF) {
        switch( n) {
        case '?':
            errflg++;
            break;
        case 't':
            tv.tv_sec = atoi(optarg);
            break;
        case 'v':
	    fprintf( stdout, "%s version %s\n", prog, VERSION);
            exit( 0);
	    break;
        case 'h':
	    fprintf( stderr, "USAGE: %s [-vh] [-t timeout] host [port]\n", prog);
            exit( 0);
	    break;
        }
    }

    switch( argc - optind) {
    case 2:
        port = atoi( argv[--argc]);
    case 1:
        hostname = argv[ --argc];
	break;
    default:
	fprintf( stderr, "USAGE: %s [-t timeout] remote_host [remote_port]\n", prog);
	exit( 1);
    }
    if(( signal( SIGALRM, terminate)) == SIG_ERR) {
	error("signal");
    }
                              /* File-Descriptor fuer Berkeley Socket oeffnen */
    if ((output = get_sock_desc(hostname, port, tv.tv_sec)) == EOF) {
	error("cannot open socket");
    }

    if(( signal( SIGTERM, terminate)) == SIG_ERR) {
	error("signal");
    }

#ifdef USE_SELECT
    {
	struct timeval *tvp;
	fd_set readfds, mask;
	char buffer[BUFSIZ];
	int n, nfound;

#define MASK(f)	(1<<(f))

	FD_ZERO( &mask);
	FD_SET( output, &mask);
	FD_SET( fileno(stdin), &mask);

	if( tv.tv_sec < 0)
	    tvp = NULL;
	else
	    tvp = &tv;

	for(readfds = mask;
		(nfound = select( output+1, &readfds, 0, 0, tvp)) > 0;
			readfds = mask)
	{
	    if( FD_ISSET( fileno(stdin), &readfds))
	    {
		while((n = read(fileno(stdin), buffer,BUFSIZ)) > 0)
		{
		    write(output, buffer, n);
		    if( n < BUFSIZ)
			break;
		}
		if( n == 0)
		{
		    shutdown( output, 1);	/* tell remote that the */
						/* will be closed */
		    while((n = read(output, buffer,BUFSIZ)) > 0)
		    {
			write(1, buffer, n);
		    }
		    break;
		}
	    }
	    if( FD_ISSET( output, &readfds))
	    {
		while((n = read(output, buffer,BUFSIZ)) > 0)
		{
		    write(1, buffer, n);
		    if( n < BUFSIZ)
			break;
		}
		if( n == 0)
		{
		    shutdown( output, 0);	/* we will not accept any */
						/* more receives */
		    while((n = read(0, buffer,BUFSIZ)) > 0)
		    {
			write(output, buffer, n);
		    }
		    break;
		}
	    }
	}
    }
#else

    parent = getpid();
    switch( fork()) {
    case -1:
	perror( "fork");
	exit( 1);

    case 0:
        while(( ret = read( 0, buf, BUFSIZ)) > 0)
	    write( output, buf, ret);

/*      (void) kill (parent, SIGTERM);		*/
	exit( 0);

    default:
        while(( ret = read( output, buf, BUFSIZ)) > 0)
	    write( 1, buf, ret);
	break;
    }
#endif
    exit(0);
}

struct hostent *hp;
struct sockaddr_in peer_addr;

int 
get_sock_desc(char *ip_addr, int remote_port, int timeout)
{
    int             s;
    int             addrlen;
    u_long          inet_addr();
    int retry = 3;
    timeout = 0;

    addrlen = sizeof(struct sockaddr_in);
    memset((char *) &peer_addr, 0, addrlen);

    peer_addr.sin_family = AF_INET;

    if (!isdigit(*ip_addr)) {
	hp = gethostbyname(ip_addr);
	if (hp == NULL) {
	    fprintf(stderr, "get_sock_desc: %s not found\n", ip_addr);
	    exit(1);
	}
	peer_addr.sin_addr.s_addr = ((struct in_addr *) (hp->h_addr))->s_addr;
    } else
	peer_addr.sin_addr.s_addr = inet_addr(ip_addr);

    peer_addr.sin_port = htons(remote_port);

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1)
	error("get_sock_desc: socket");

    while( retry -- > 0) {
	if( timeout > 0) alarm( timeout);
	if (connect(s, (struct sockaddr *)&peer_addr, addrlen) < 0) {
	    /* FEHLER !!! */
	    if ( timeout > 0) alarm( 0);

	    my_errno = errno;
	    if ( errno == EHOSTUNREACH) {
		sleep(1);
	    } else if ( errno == ECONNREFUSED) {
		sleep(1);
	    } else {
		close(s);
		error("get_sock_desc: connect");
	    }
	} else {
	    /* CONNECTED !!!!! */
	    if ( timeout > 0) alarm( 0);
	    break;
	}
	if ( timeout > 0) alarm( 0);
    }
    if ( retry < 0) {
	close( s);
	s = -1;
    }
    return (s);
}
