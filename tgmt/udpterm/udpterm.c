/******************************************************************************/
/*                                                                            */
/* $Header: /mnt/cc-l4/ccp/cpc3p0/src/lsl/toolsrc/udpterm/rcs/udpterm.c,v 1.4 2000/12/08 13:58:51 tine Exp $
/*                                                                            */
/* udpterm:                                                                   */
/* ---------                                                                  */
/* Dieser Prozess initialisiert den Aufbau einer UDP-Socket-Verbindung als    */
/* Server und schreibt alles, was vom Target (dem "Lsl"-Interpreter) dorthin  */
/* geschrieben wird, auf den Bildschirm (Stdout).                             */
/*                                                                            */
/******************************************************************************/

# include <stdio.h>
# include <string.h>            /* fuer 'exit()' */
# include <stdlib.h>            /* fuer 'exit()' */
# include <errno.h>             /* fuer 'errno' */
# include <signal.h>            /* fuer 'signal()' */
# include <sys/types.h>         /* fuer 'getsockopt()' */
# include <sys/socket.h>        /* fuer 'socket()', 'bind()', 'recvfrom()' */
# include <netinet/in.h>        /* fuer 'bind()', 'struct sockaddr_in' */

# define HOSTLEN        40
# define BUFFER_SIZE  1024

/* Socket-Descriptor (global) */
int s = -1;

/* Verbose-Flag */
int verbose = 0;

main (argc, argv)
int argc;
char *argv[];

{
    extern void sig_handler();
    extern int errno;           /* for error in system calls */

    extern void put_on_term();

    char *buffer;               /* zum einlesen und verschicken */
    int cnt;                    /* zaehlt die zeichen */

    int buffersize;             /* fuer Ergebnis bei getsockopt */
    int optlen;                 /* fuer Laenge des Ergebnisses bei getsockopt */

    int port;                   /* Parameter im Aufruf */
    int rlen;                   /* size of 'remote' */
    struct sockaddr_in sin,
                       remote;

    (void) signal (SIGTERM, sig_handler);
    (void) signal (SIGINT, sig_handler);
    (void) signal (SIGBUS, sig_handler);
    (void) signal (SIGSEGV, sig_handler);

/*
 * Ueberpruefen der Argumente im Aufruf
 */
    if (argc < 2 || argc > 3)
    {
        (void) fprintf (stdout, "usage: %s <port> [-v]\n", argv[0]);
        (void) fflush (stdout);
        exit (1);
    }

    if ((port = atoi (argv[1])) <= 0)
    {
        (void) fprintf (stdout, "%s: unvalid port (%s)\n", argv[0], argv[1]);
        (void) fflush (stdout);
        exit (1);
    }

    if (argc == 3)
    {
        if (strcmp (argv[2], "-v") == 0)
            verbose = 1;
        else
        {
            (void) fprintf (stdout, "%s: unknown option (%s)\n",
              argv[0], argv[2]);
            (void) fflush (stdout);
            exit (1);
        }
    }

/*
 * Ermoeglichen der Kommunikation ...
 */
    if ((s = socket (AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        if (verbose)
        {
            (void) fprintf (stdout, "%s: failure of 'socket()' - errno %d\n",
              argv[0], errno);
            (void) fflush (stdout);
        }
        exit (2);
    }

/*
 * Binden des eigenen Sockets an eine feste Adresse (Port)
 */
    (void) memset ((void *) &sin, 0, sizeof(sin));
    sin.sin_port = htons (atoi (argv[1]));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    if (bind (s, (struct sockaddr *)&sin, sizeof(sin)) == -1)
    {
        close (s);
        if (verbose)
        {
            (void) fprintf (stdout, "%s: failure of 'bind()' - errno %d\n",
              argv[0], errno);
            (void) fflush (stdout);
        }
        exit (3);
    }

/*
 * Initialisierung bzgl. des 'recvfrom()'
 */
    optlen = sizeof(buffersize);
    if (getsockopt (s, SOL_SOCKET, SO_RCVBUF, (void *) &buffersize, &optlen)
      == -1)
    {
        close (s);
        if (verbose)
        {
            (void) fprintf (stdout,
              "%s: failure of 'getsockopt()' - errno %d\n",
              argv[0], errno);
            (void) fflush (stdout);
        }
        exit (4);
    }
    (void) memset ((void *) &remote, 0, sizeof(remote));

    /* hier das Ergebnis von getsockopt allokieren */
    buffer = (char *) malloc (buffersize);    /* wird nicht explizit        */
                                              /* freigegeben, sondern durch */
                                              /* 'exit()' des Programms     */

    if (verbose)
    {
        cnt = sprintf (&buffer[0],
          "%s: UDP-SocketServer (%d) initialized, buffersize is %d\n",
          argv[0], sin.sin_port, buffersize);
        put_on_term (stdout, &buffer[0], cnt);
        (void) fflush (stdout);
    }

    while (1)
    {
        rlen = sizeof(remote);   /* MUSS fuer 'recvfrom()' initialisiert sein */
/*
 * Klappt das Verschicken nicht, macht das auch nicht viel ...
 */
        if ((cnt = recvfrom (s, buffer, buffersize, 0, (struct sockaddr *)&remote, (socklen_t *)&rlen)) != -1)
        {
            put_on_term (stdout, &buffer[0], cnt);
            (void) fflush (stdout);
        }
    }

    close (s);
    exit (0);
}

void put_on_term (fp, buf, len)
FILE *fp;
char *buf;
int len;
{
    int i;

    for (i=0; i<len; ++i)
        putc (buf[i], fp);
}

void sig_handler (num)
int num;
{
/*
 * Socket-Verbindung schliessen
 */
    close (s);

    if (verbose)
    {
        (void) fprintf (stdout, "udpterm: terminated on signal (%d)\n", num);
        (void) fflush (stdout);
    }

    (void) fclose (stdout);

    exit (0);
}
