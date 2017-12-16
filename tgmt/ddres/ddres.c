/*
 *
 * Copyright (c) 1991 Forschungszentrum Juelich
 * Author : Michael Simon (AP)
 * All rights reserved.
 *
 * %Z%%M%	%I% (mike) %H%
 *
 */
/*
 * resolver fœr Device Driver Namen
 * Bedient Anfragen vom Network mit dem Output von $CPC/bin/tgtmgt
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>
#include <netdb.h>

#include <tgtmgt.h>

extern int errno;

char buffer[BUFSIZ];	/* buffer for packets to be read into */
char * cpc;		/* pointer auf CPC im Environment von ddres */
char * adm;		/* default Pfad fuer Datenbanken */
int verbose;
int standalone;

struct servent *sp;     /* pointer to service information */

struct sockaddr_in myaddr_in;   /* for local socket address */
struct sockaddr_in clientaddr_in;   /* for client's socket address */

#define DEFAULT_DDRESPORT	20002
#define FAILURE			-1
#define CHILD			0

main(argc, argv)
int argc;
char *argv[];
{
    char * prog;
    int ls, s;              /* socket descriptors */
    int addrlen;
    short port;
    int val;
    int which;

    standalone = verbose = 0;
    port = DEFAULT_DDRESPORT;
    prog = argv[0];

    while( --argc > 0 && (*++argv)[0] == '-') {
	char * s;

	for( s = argv[0]+1; *s != '\0'; s++)
	    switch(*s) {
	    case 'p':
		port = atoi( argv[1]);
		argv ++; argc --;
		break;
	    case 's':
		standalone ++;
		break;
	    case 'v':
		verbose ++;
		break;
	    default:
		fprintf( stderr, "Unknown option: %c\n", *s);
		fprintf( stderr, "Usage: %s [-vs]\n", prog);
		break;
	    }
    }

        /* clear out address structures */
    memset ((char *)&myaddr_in, 0, sizeof(struct sockaddr_in));
    memset ((char *)&clientaddr_in, 0, sizeof(struct sockaddr_in));

        /* Set up address structure for the socket. */
    myaddr_in.sin_family = AF_INET;
        /* The server should receive on the wildcard address,
         * rather than its own internet address.  This is
         * generally good practice for servers, because on
         * systems which are connected to more than one
         * network at once will be able to have one server
         * listening on all networks at once.  Even when the
         * host is connected to only one network, this is good
         * practice, because it makes the server program more
         * portable.
         */
    myaddr_in.sin_addr.s_addr = INADDR_ANY;
        /* Find the information for the "bpmed" server
         * in order to get the needed port number.
         */

    myaddr_in.sin_port = htons (port);

        /* Create the socket. */
    ls = socket (AF_INET, SOCK_STREAM, 0);
    if (ls == -1) {
        perror(prog);
        fprintf(stderr,"%s: unable to create socket\n", prog);
        exit(1);
    }
	/* allow reuse of PORT # */
    val = 1;
    if(setsockopt(ls, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int)) == -1) {
	perror(prog);
	fprintf(stderr,"%s: unable to set SO_REUASEADDR\n", prog);
	exit(1);
    }
        /* Bind the server's address to the socket. */
    if (bind(ls, &myaddr_in, sizeof(struct sockaddr_in)) == -1) {
        perror(prog);
        fprintf(stderr,"%s: unable to bind address\n", prog);
        exit(1);
    }
    if( listen( ls, SOMAXCONN) == -1) {
        perror(prog);
        fprintf(stderr,"%s: unable to listen socket\n", prog);
        exit( 1);
    }

    cpc = getenv("CPC");	/* might be a NULL pointer */
    if( cpc == NULL)
	cpc = "/mnt/cc-x4/ccp/cpc3p0";
    adm = getenv("TMDB");;
    if( adm == NULL)
	adm = "/mnt/cc-x4/ccp/cpc3p0/tgmt/adm";

    if(!standalone) {

	setpgrp();

	if(( which = fork()) == FAILURE)
	    exit (1);
/* CHILD comes here, do the ddres service here */
	else if( which == CHILD)
            signal(SIGCLD, SIG_IGN);
	else
	    exit(0);
    }

    for( ;; ) {

	addrlen = sizeof( clientaddr_in);
/* warten auf Anfragen
 */
	s = accept( ls, & clientaddr_in, & addrlen);
	if( s == -1) exit( 1);

	if( !standalone) switch( fork()) {
	case -1:
	    fprintf(stderr, "%s: unable to fork daemon\n", prog);
	    exit( 1);
	case 0:     /* CHILD comes here */

	    close(ls);
	    server( s);
	    exit( 0);

	default:    /* PARENT comes here */

	    close( s);

	} else
	    server( s);
	}

/*NOTREACHED*/
}

server( s)	/* service a ddres request, resolve a logical Name */
int s;		/* our stream socket, connection to client */
{
    int    nread;
    char * hostname;
    long   linger = 1;
    FILE * fin = fdopen( s, "r");
    FILE * fout = fdopen( s, "w");

    if( fin == NULL || fout == NULL) {
	perror( "fdopen socket");
	return;
    }

    /* status message */
    if( verbose)
    {
	struct hostent *hp;     /* pointer to host info for requested host */

	hp = gethostbyaddr ((char *) &clientaddr_in.sin_addr.s_addr,
                                sizeof (clientaddr_in.sin_addr.s_addr),
                                clientaddr_in.sin_family);
	if( hp == NULL)
             hostname = inet_ntoa(clientaddr_in.sin_addr);
	else
             hostname = hp->h_name;

	fprintf( stderr, "\n\tConnected to %s[%d]\n",
		hostname, clientaddr_in.sin_port);
    }

    if( fgets( buffer, sizeof(buffer), fin) != NULL) {
	char temp[BUFSIZ];
	char * tmdb;	/* datenbank pfad name */
	FILE * fp;
        char * ergebnis;

	buffer[strlen(buffer)-1] = '\0';	/* remove End-Of-Line */
	if( verbose)
	    fprintf( stderr, "\tRequest: %s\n", buffer);

/*
 * get environment TMDB form request line
 */
#ifdef ALTE_VERSION
	tmdb = strpbrk( buffer, " \t");
	if( tmdb == NULL)
	    tmdb = adm;
	else {
	    char * p;

	    *tmdb++ = '\0';			/* Ende des logischen Namens */
	    while(isspace( *tmdb))
		tmdb ++;
	    p = strpbrk( tmdb, " \t");
	    if( p )				/* Ende von TMDB */
		*p = '\0';
	}
	sprintf( & temp[0], "TMDB=%s", tmdb);
	if( verbose)
	    fprintf( stderr, "\tEnvironmnet: %s\n", temp);
	putenv( strdup(temp));
/*
 * build commando for popen syscall
 */
	sprintf( & temp[0], "%s/bin/tgtmgt %s", cpc, buffer);
	if( verbose)
	    fprintf( stderr, "\tKommando ist: %s\n", temp);
	if(( fp = popen( &temp[0], "r")) == NULL) {
	    perror( "Cannot popen");
 	} else if(fgets( buffer, sizeof( buffer), fp) != NULL) {
	    int ret;

	    switch(ret = pclose( fp)) {
	    case -1:
		if( errno != ECHILD) {		/* siehe signal( SIGCHLD, .. */
		    perror( "pclose");
		    break;
		}
		/* FALL THROUGH */
	    case 0:
		if( verbose)
		    fprintf( stderr, "\tAnswer: %s\n", buffer);
		fputs( buffer, fout);
		break;
	    default:
		fprintf( stderr,"tgtmgt failed with %d\n", ret);
		break;
	    }
	} else {
	    if( ferror( fp)) perror( "Cannot fgets");
	    pclose(fp);
	}
#else
	tmdb = strpbrk( buffer, " \t");
	if( tmdb )
	    *tmdb = '\0';                       /* Ende des logischen Namens */

	if( verbose)
	    fprintf( stderr, "\tKommando ist: tgtmgt (%s)\n", buffer);

        /* buffer enthaelt den logischen Namen */
        if (tgtmgt (buffer, &ergebnis) == 0) {
            strcat (ergebnis, "\n");
	    if( verbose)
	        fprintf( stderr, "\tAnswer: %s\n", ergebnis);
	    fputs( ergebnis, fout);
        } else {
	    if( verbose)
	        fprintf( stderr, "\tError: %s\n", ergebnis);
	    fprintf( stderr,"tgtmgt failed with \"%s\"\n", ergebnis);
        }
        free ((void *) ergebnis);

#endif
    } else {
	perror( "Cannot read from Network");
    }
    fclose( fout);
    fclose( fin);
}
