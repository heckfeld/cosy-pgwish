/******************************************************************************/
/*                                                                            */
/* $Header$
/*                                                                            */
/* udpteln:                                                                   */
/* ---------                                                                  */
/* Dieser Prozess soll nach aussen so aussehen, als ob 'telnet <host> <port>' */
/* benutzt wuerde.                                                            */
/* Statt Stream Sockets (TCP) werden allerdings Datagram Sockets (UDP) ver-   */
/* wendet.                                                                    */
/*                                                                            */
/******************************************************************************/

# include <stdio.h>
# include <string.h>
# include <stdlib.h>            /* fuer 'exit()' */
# include <errno.h>             /* fuer 'errno' */

# include <unistd.h>            /* fuer 'gethostname()' */

# include <sys/socket.h>
# include <netinet/in.h>
# include <netdb.h>

# define HOSTLEN        40
# define BUFFER_SIZE  1024

main (argc, argv)
int argc;
char *argv[];

{
    extern int errno;           /* for error in system calls */

    char host[HOSTLEN];         /* fuer 'gethostname()' */
    char *buffer;               /* zum einlesen und verschicken */
    int cnt;                    /* zaehlt die zeichen */

    int telnet_getline();
    void telnet_putline();

    int s,                      /* socket descriptor */
        rlen;                   /* size of 'remote' */
    struct sockaddr_in sin,
                       remote;
    struct hostent *hp;         /* fuer 'gethostbyname()' */

    if (argc != 3)
    {
        (void) fprintf (stdout, "usage: %s <host> <port>\n", argv[0]);
        (void) fflush (stdout);
        exit (1);
    }

/*
 * Ermoeglichen der Kommunikation ...
 */
    if ((s = socket (AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        (void) fprintf (stdout, "%s: failure of 'socket()' - errno %d\n",
          argv[0], errno);
        (void) fflush (stdout);
        exit (1);
    }
/*
 * Damit dem 'Client' eine eindeutige Adresse/Port zugeordnet werden.
 * (Absender auf dem verschickten Paket.)
 */
    (void) memset ((void *) &sin, 0, sizeof(sin));
    sin.sin_port = 0;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    if (bind (s, (struct sockaddr *)&sin, sizeof(sin)) == -1)
    {
        (void) fprintf (stdout, "%s: failure of 'bind()' - errno %d\n",
          argv[0], errno);
        (void) fflush (stdout);
        exit (1);
    }
/*
 * Fuer die Adresse/Port des 'Server' ("UDP_Tgtd_s" auf dem Target).
 */
    if ((hp = gethostbyname (argv[1])) == (struct hostent *) 0)
    {
        (void) fprintf (stdout, "%s: failure of 'gethostbyname()' - errno %d\n",
          argv[0], errno);
        (void) fflush (stdout);
        exit (1);
    }
    (void) memset ((void *) &remote, 0, sizeof(remote));
    remote.sin_port = htons (atoi (argv[2]));
    remote.sin_family = AF_INET;
    remote.sin_addr.s_addr = ((struct in_addr *) (hp->h_addr))->s_addr;

    buffer = (char *) malloc (BUFFER_SIZE);
    cnt = sprintf (&buffer[0], "%s: communication established\n", argv[0]);
    telnet_putline (stdout, &buffer[0], cnt);
    cnt = sprintf (&buffer[0], "%s: Escape character is '^D'\n", argv[0]);
    telnet_putline (stdout, &buffer[0], cnt);

    while ((cnt = telnet_getline (stdin, &buffer[0])) != EOF)
    {
/*
 * Klappt das Verschicken nicht, macht das auch nicht viel ...
 */
        rlen = sizeof(remote);
        if (sendto (s, &buffer[0], cnt, 0, (struct sockaddr *)&remote, rlen) != -1)
        {
            cnt = recvfrom (s, &buffer[0], BUFFER_SIZE, 0, (struct sockaddr *)&remote, (socklen_t *)&rlen);
        }
        else
        {
            cnt += sprintf (&buffer[cnt-1], "UDP-ERROR (%d)\n", errno) - 1;
        }
        telnet_putline (stdout, &buffer[0], cnt);
    }

    (void) fprintf (stdout, "%s: close connection because of EOF\n", argv[0]);
    (void) fflush (stdout);

    close (s);
    exit (0);
}


int telnet_getline (fp, buf)
FILE *fp;
char *buf;

{
    int i,
        c;

    i = 0;
    while ((c = getc (fp)) != '\n' && c != EOF)
    {
        buf[i++] = c;
    }
    if (i > 0 && buf[i-1] == '\r')
        i--;

    buf[i++] = '\n';

    return ((c == EOF) ? c : i);
}


void telnet_putline (fp, buf, len)
FILE *fp;
char *buf;
int len;

{
    int i;

    buf[len-1] = '\n';
    for (i=0; i<len; ++i)
    {
        putc (buf[i], fp);
    }
}
