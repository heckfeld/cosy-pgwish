 /*-------------------------------------------------------------------------
 *
 * pqcomm.c--
 *	  Communication functions between the Frontend and the Backend
 *
 * Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  $Header: /usr/local/cvsroot/pgsql/src/backend/libpq/pqcomm.c,v 1.40 1998/03/02 05:41:53 scrappy Exp $
 *
 *-------------------------------------------------------------------------
 */
/*
 * INTERFACE ROUTINES
 *		pq_gettty		- return the name of the tty in the given buffer
 *		pq_getport		- return the PGPORT setting
 *		pq_close		- close input / output connections
 *		pq_flush		- flush pending output
 *		pq_getstr		- get a null terminated string from connection
 *		pq_getnchar		- get n characters from connection
 *		pq_getint		- get an integer from connection
 *		pq_putstr		- send a null terminated string to connection
 *		pq_putnchar		- send n characters to connection
 *		pq_putint		- send an integer to connection
 *		pq_getinaddr	- initialize address from host and port number
 *		pq_getinserv	- initialize address from host and service name
 *		pq_connect		- create remote input / output connection
 *		pq_accept		- accept remote input / output connection
 *		pq_async_notify - receive notification from backend.
 *
 * NOTES
 *		These functions are used by both frontend applications and
 *		the postgres backend.
 *
 */
#include "postgres.h"

#include <stdio.h>
#if defined(HAVE_STRING_H)
#include <string.h>
#else
#include <strings.h>
#endif
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>				/* for ttyname() */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#if defined(linux)
#ifndef SOMAXCONN
#define SOMAXCONN 5				/* from Linux listen(2) man page */
#endif							/* SOMAXCONN */
#endif							/* linux */

#include "miscadmin.h"
#include "libpq/pqsignal.h"
#include "libpq/auth.h"
#include "libpq/libpq.h"		/* where the declarations go */
#include "storage/ipc.h"

/* ----------------
 *		declarations
 * ----------------
 */
FILE	   *Pfout,
		   *Pfin;
FILE	   *Pfdebug;			/* debugging libpq */
int			PQAsyncNotifyWaiting;		/* for async. notification */

/* --------------------------------
 *		pq_init - open portal file descriptors
 * --------------------------------
 */
void
pq_init(int fd)
{
	Pfin = fdopen(fd, "r");
	Pfout = fdopen(dup(fd), "w");
	if (!Pfin || !Pfout)
		elog(FATAL, "pq_init: Couldn't initialize socket connection");
	PQnotifies_init();
	if (getenv("LIBPQ_DEBUG"))
	{
		Pfdebug = stderr;
	}
	else
	{
		Pfdebug = NULL;
	}
}

/* -------------------------
 *	 pq_getc(File* fin)
 *
 *	 get a character from the input file,
 *
 *	 if Pfdebug is set, also echo the character fetched into Pfdebug
 *
 *	 used for debugging libpq
 */
static int
pq_getc(FILE *fin)
{
	int			c;

	c = getc(fin);
	if (Pfdebug && c != EOF)
		putc(c, Pfdebug);
	return c;
}

/* --------------------------------
 *		pq_gettty - return the name of the tty in the given buffer
 * --------------------------------
 */
void
pq_gettty(char *tp)
{
	strncpy(tp, ttyname(0), 19);
}

/* --------------------------------
 *		pq_getport - return the PGPORT setting
 * --------------------------------
 */
int
pq_getport()
{
	char	   *envport = getenv("PGPORT");

	if (envport)
		return (atoi(envport));
	return (atoi(DEF_PGPORT));
}

/* --------------------------------
 *		pq_close - close input / output connections
 * --------------------------------
 */
void
pq_close()
{
	if (Pfin)
	{
		fclose(Pfin);
		Pfin = NULL;
	}
	if (Pfout)
	{
		fclose(Pfout);
		Pfout = NULL;
	}
	PQAsyncNotifyWaiting = 0;
	PQnotifies_init();
	pq_unregoob();
}

/* --------------------------------
 *		pq_flush - flush pending output
 * --------------------------------
 */
void
pq_flush()
{
	if (Pfout)
		fflush(Pfout);
}

/* --------------------------------
 *		pq_getstr - get a null terminated string from connection
 * --------------------------------
 */
int
pq_getstr(char *s, int maxlen)
{
	int			c = '\0';

	if (Pfin == (FILE *) NULL)
	{
/*		elog(DEBUG, "Input descriptor is null"); */
		return (EOF);
	}

	while (maxlen-- && (c = pq_getc(Pfin)) != EOF && c)
		*s++ = c;
	*s = '\0';

	/* -----------------
	 *	   If EOF reached let caller know.
	 *	   (This will only happen if we hit EOF before the string
	 *	   delimiter is reached.)
	 * -----------------
	 */
	if (c == EOF)
		return (EOF);
	return (!EOF);
}

/*
 * USER FUNCTION - gets a newline-terminated string from the backend.
 *
 * Chiefly here so that applications can use "COPY <rel> to stdout"
 * and read the output string.	Returns a null-terminated string in s.
 *
 * PQgetline reads up to maxlen-1 characters (like fgets(3)) but strips
 * the terminating \n (like gets(3)).
 *
 * RETURNS:
 *		EOF if it is detected or invalid arguments are given
 *		0 if EOL is reached (i.e., \n has been read)
 *				(this is required for backward-compatibility -- this
 *				 routine used to always return EOF or 0, assuming that
 *				 the line ended within maxlen bytes.)
 *		1 in other cases
 */
int
PQgetline(char *s, int maxlen)
{
	if (!Pfin || !s || maxlen <= 1)
		return (EOF);

	if (fgets(s, maxlen - 1, Pfin) == NULL)
	{
		return feof(Pfin) ? EOF : 1;
	}
	else
	{
		for (; *s; s++)
		{
			if (*s == '\n')
			{
				*s = '\0';
				break;
			}
		}
	}

	return 0;
}

/*
 * USER FUNCTION - sends a string to the backend.
 *
 * Chiefly here so that applications can use "COPY <rel> from stdin".
 *
 * RETURNS:
 *		0 in all cases.
 */
int
PQputline(char *s)
{
	if (Pfout)
	{
		fputs(s, Pfout);
		fflush(Pfout);
	}
	return (0);
}

/* --------------------------------
 *		pq_getnchar - get n characters from connection
 * --------------------------------
 */
int
pq_getnchar(char *s, int off, int maxlen)
{
	return pqGetNBytes(s + off, maxlen, Pfin);
}

/* --------------------------------
 *		pq_getint - get an integer from connection
 *	 we receive an integer a byte at a type and reconstruct it so that
 *	 machines with different ENDIAN representations can talk to each
 *	 other
 * --------------------------------
 */
int
pq_getint(int b)
{
	int			n,
				status = 1;

	if (!Pfin)
		return EOF;

	/*
	 * mjl: Seems inconsisten w/ return value of pq_putint (void). Also,
	 * EOF is a valid return value for an int! XXX
	 */

	switch (b)
	{
		case 1:
			status = ((n = fgetc(Pfin)) == EOF);
			break;
		case 2:
			status = pqGetShort(&n, Pfin);
			break;
		case 4:
			status = pqGetLong(&n, Pfin);
			break;
		default:
			fprintf(stderr, "** Unsupported size %d\n", b);
	}

	if (status)
	{
		sprintf(PQerrormsg,
				"FATAL: pq_getint failed: errno=%d\n", errno);
		fputs(PQerrormsg, stderr);
		pqdebug("%s", PQerrormsg);
		n = 0;
	}

	return n;
}

/* --------------------------------
 *		pq_putstr - send a null terminated string to connection
 * --------------------------------
 */
void
pq_putstr(char *s)
{
	if (pqPutString(s, Pfout))
	{
		sprintf(PQerrormsg,
				"FATAL: pq_putstr: fputs() failed: errno=%d\n", errno);
		fputs(PQerrormsg, stderr);
		pqdebug("%s", PQerrormsg);
	}
}

/* --------------------------------
 *		pq_putnchar - send n characters to connection
 * --------------------------------
 */
void
pq_putnchar(char *s, int n)
{
	if (pqPutNBytes(s, n, Pfout))
	{
		sprintf(PQerrormsg,
				"FATAL: pq_putnchar: fputc() failed: errno=%d\n",
				errno);
		fputs(PQerrormsg, stderr);
		pqdebug("%s", PQerrormsg);
	}
}

/* --------------------------------
 *		pq_putint - send an integer to connection
 *	 we chop an integer into bytes and send individual bytes
 *	 machines with different ENDIAN representations can still talk to each
 *	 other
 * --------------------------------
 */
void
pq_putint(int i, int b)
{
	int			status;

	if (!Pfout)
		return;

	status = 1;
	switch (b)
	{
		case 1:
			status = (fputc(i, Pfout) == EOF);
			break;
		case 2:
			status = pqPutShort(i, Pfout);
			break;
		case 4:
			status = pqPutLong(i, Pfout);
			break;
		default:
			fprintf(stderr, "** Unsupported size %d\n", b);
	}

	if (status)
	{
		sprintf(PQerrormsg,
				"FATAL: pq_putint failed: errno=%d\n", errno);
		fputs(PQerrormsg, stderr);
		pqdebug("%s", PQerrormsg);
	}
}

/* ---
 *	   pq_sendoob - send a string over the out-of-band channel
 *	   pq_recvoob - receive a string over the oob channel
 *	NB: Fortunately, the out-of-band channel doesn't conflict with
 *		buffered I/O because it is separate from regular com. channel.
 * ---
 */
int
pq_sendoob(char *msg, int len)
{
	int			fd = fileno(Pfout);

	return (send(fd, msg, len, MSG_OOB));
}

int
pq_recvoob(char *msgPtr, int *lenPtr)
{
	int			fd = fileno(Pfout);
	int			len = 0;

	len = recv(fd, msgPtr + len, *lenPtr, MSG_OOB);
	*lenPtr = len;
	return (len);
}

/* --------------------------------
 *		pq_getinaddr - initialize address from host and port number
 * --------------------------------
 */
int
pq_getinaddr(struct sockaddr_in * sin,
			 char *host,
			 int port)
{
	struct hostent *hs;

	MemSet((char *) sin, 0, sizeof(*sin));

	if (host)
	{
		if (*host >= '0' && *host <= '9')
			sin->sin_addr.s_addr = inet_addr(host);
		else
		{
			if (!(hs = gethostbyname(host)))
			{
				perror(host);
				return (1);
			}
			if (hs->h_addrtype != AF_INET)
			{
				sprintf(PQerrormsg,
						"FATAL: pq_getinaddr: %s not on Internet\n",
						host);
				fputs(PQerrormsg, stderr);
				pqdebug("%s", PQerrormsg);
				return (1);
			}
			memmove((char *) &sin->sin_addr,
					hs->h_addr,
					hs->h_length);
		}
	}
	sin->sin_family = AF_INET;
	sin->sin_port = htons(port);
	return (0);
}

/* --------------------------------
 *		pq_getinserv - initialize address from host and servive name
 * --------------------------------
 */
int
pq_getinserv(struct sockaddr_in * sin, char *host, char *serv)
{
	struct servent *ss;

	if (*serv >= '0' && *serv <= '9')
		return (pq_getinaddr(sin, host, atoi(serv)));
	if (!(ss = getservbyname(serv, NULL)))
	{
		sprintf(PQerrormsg,
				"FATAL: pq_getinserv: unknown service: %s\n",
				serv);
		fputs(PQerrormsg, stderr);
		pqdebug("%s", PQerrormsg);
		return (1);
	}
	return (pq_getinaddr(sin, host, ntohs(ss->s_port)));
}

/*
 * register an out-of-band listener proc--at most one allowed.
 * This is used for receiving async. notification from the backend.
 */
void
pq_regoob(void (*fptr) ())
{
	int			fd = fileno(Pfout);

#if defined(hpux)
	ioctl(fd, FIOSSAIOOWN, MyProcPid);
#elif defined(sco)
	ioctl(fd, SIOCSPGRP, MyProcPid);
#else
	fcntl(fd, F_SETOWN, MyProcPid);
#endif							/* hpux */
	pqsignal(SIGURG, fptr);
}

void
pq_unregoob()
{
	pqsignal(SIGURG, SIG_DFL);
}


void
pq_async_notify()
{
	char		msg[20];

	/* int len = sizeof(msg); */
	int			len = 20;

	if (pq_recvoob(msg, &len) >= 0)
	{
		/* debugging */
		printf("received notification: %s\n", msg);
		PQAsyncNotifyWaiting = 1;
		/* PQappendNotify(msg+1); */
	}
	else
	{
		extern int	errno;

		printf("SIGURG but no data: len = %d, err=%d\n", len, errno);
	}
}

/*
 * Streams -- wrapper around Unix socket system calls
 *
 *
 *		Stream functions are used for vanilla TCP connection protocol.
 */

/*
 * StreamServerPort -- open a sock stream "listening" port.
 *
 * This initializes the Postmaster's connection
 *		accepting port.
 *
 * ASSUME: that this doesn't need to be non-blocking because
 *		the Postmaster uses select() to tell when the socket
 *		is ready.
 *
 * RETURNS: STATUS_OK or STATUS_ERROR
 */

static char sock_path[MAXPGPATH + 1] = "";

/* do_unlink()
 * Shutdown routine for backend connection
 * If a Unix socket is used for communication, explicitly close it.
 */
void
StreamDoUnlink()
{
	if (sock_path[0])
		unlink(sock_path);
}

int
StreamServerPort(char *hostName, short portName, int *fdP)
{
	SockAddr	saddr;
	int			fd,
				err,
				family;
	size_t		len;
	int			one = 1;

	family = ((hostName != NULL) ? AF_INET : AF_UNIX);

	if ((fd = socket(family, SOCK_STREAM, 0)) < 0)
	{
		sprintf(PQerrormsg,
				"FATAL: StreamServerPort: socket() failed: errno=%d\n",
				errno);
		fputs(PQerrormsg, stderr);
		pqdebug("%s", PQerrormsg);
		return (STATUS_ERROR);
	}
	if ((setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &one,
					sizeof(one))) == -1)
	{
		sprintf(PQerrormsg,
				"FATAL: StreamServerPort: setsockopt (SO_REUSEADDR) failed: errno=%d\n",
				errno);
		fputs(PQerrormsg, stderr);
		pqdebug("%s", PQerrormsg);
		return (STATUS_ERROR);
	}
	bzero(&saddr, sizeof(saddr));
	saddr.sa.sa_family = family;
	if (family == AF_UNIX)
	{
		len = UNIXSOCK_PATH(saddr.un, portName);
		strcpy(sock_path, saddr.un.sun_path);
	}
	else
	{
		saddr.in.sin_addr.s_addr = htonl(INADDR_ANY);
		saddr.in.sin_port = htons(portName);
		len = sizeof(struct sockaddr_in);
	}
	err = bind(fd, &saddr.sa, len);
	if (err < 0)
	{
		sprintf(PQerrormsg,
				"FATAL: StreamServerPort: bind() failed: errno=%d\n",
				errno);
		pqdebug("%s", PQerrormsg);
		strcat(PQerrormsg, "\tIs another postmaster already running on that port?\n");
		if (family == AF_UNIX)
			strcat(PQerrormsg, "\tIf not, remove socket node (/tmp/.s.PGSQL.<portnr>)and retry.\n");
		else
			strcat(PQerrormsg, "\tIf not, wait a few seconds and retry.\n");
		fputs(PQerrormsg, stderr);
		return (STATUS_ERROR);
	}

	listen(fd, SOMAXCONN);

	/*
	 * MS: I took this code from Dillon's version.  It makes the listening
	 * port non-blocking.  That is not necessary (and may tickle kernel
	 * bugs).
	 *
	 * fcntl(fd, F_SETFD, 1); fcntl(fd, F_SETFL, FNDELAY);
	 */

	*fdP = fd;
	if (family == AF_UNIX)
	{
		chmod(sock_path, 0777);
	}
	return (STATUS_OK);
}

/*
 * StreamConnection -- create a new connection with client using
 *		server port.
 *
 * This one should be non-blocking.
 *
 * RETURNS: STATUS_OK or STATUS_ERROR
 */
int
StreamConnection(int server_fd, Port *port)
{
	int			len,
				addrlen;
	int			family = port->raddr.sa.sa_family;

	/* accept connection (and fill in the client (remote) address) */
	len = family == AF_INET ?
		sizeof(struct sockaddr_in) : sizeof(struct sockaddr_un);
	addrlen = len;
	if ((port->sock = accept(server_fd,
							 (struct sockaddr *) & port->raddr,
							 &addrlen)) < 0)
	{
		elog(ERROR, "postmaster: StreamConnection: accept: %m");
		return (STATUS_ERROR);
	}

	/* fill in the server (local) address */
	addrlen = len;
	if (getsockname(port->sock, (struct sockaddr *) & port->laddr,
					&addrlen) < 0)
	{
		elog(ERROR, "postmaster: StreamConnection: getsockname: %m");
		return (STATUS_ERROR);
	}
	if (family == AF_INET)
	{
		struct protoent *pe;
		int			on = 1;

		pe = getprotobyname("TCP");
		if (pe == NULL)
		{
			elog(ERROR, "postmaster: getprotobyname failed");
			return (STATUS_ERROR);
		}
		if (setsockopt(port->sock, pe->p_proto, TCP_NODELAY,
					   &on, sizeof(on)) < 0)
		{
			elog(ERROR, "postmaster: setsockopt failed");
			return (STATUS_ERROR);
		}
	}

	/* reset to non-blocking */
	fcntl(port->sock, F_SETFL, 1);

	return (STATUS_OK);
}

/*
 * StreamClose -- close a client/backend connection
 */
void
StreamClose(int sock)
{
	close(sock);
}

/* ---------------------------
 * StreamOpen -- From client, initiate a connection with the
 *		server (Postmaster).
 *
 * RETURNS: STATUS_OK or STATUS_ERROR
 *
 * NOTE: connection is NOT established just because this
 *		routine exits.	Local state is ok, but we haven't
 *		spoken to the postmaster yet.
 * ---------------------------
 */
int
StreamOpen(char *hostName, short portName, Port *port)
{
	int			len,
				err;
	struct hostent *hp;
	extern int	errno;

	/* set up the server (remote) address */
	MemSet((char *) &port->raddr, 0, sizeof(port->raddr));
	if (hostName)
	{
		if (!(hp = gethostbyname(hostName)) || hp->h_addrtype != AF_INET)
		{
			sprintf(PQerrormsg,
					"FATAL: StreamOpen: unknown hostname: %s\n",
					hostName);
			fputs(PQerrormsg, stderr);
			pqdebug("%s", PQerrormsg);
			return (STATUS_ERROR);
		}
		memmove((char *) &(port->raddr.in.sin_addr),
				(char *) hp->h_addr,
				hp->h_length);
		port->raddr.in.sin_family = AF_INET;
		port->raddr.in.sin_port = htons(portName);
		len = sizeof(struct sockaddr_in);
	}
	else
	{
		port->raddr.un.sun_family = AF_UNIX;
		len = UNIXSOCK_PATH(port->raddr.un, portName);
	}
	/* connect to the server */
	if ((port->sock = socket(port->raddr.sa.sa_family, SOCK_STREAM, 0)) < 0)
	{
		sprintf(PQerrormsg,
				"FATAL: StreamOpen: socket() failed: errno=%d\n",
				errno);
		fputs(PQerrormsg, stderr);
		pqdebug("%s", PQerrormsg);
		return (STATUS_ERROR);
	}
	err = connect(port->sock, &port->raddr.sa, len);
	if (err < 0)
	{
		sprintf(PQerrormsg,
				"FATAL: StreamOpen: connect() failed: errno=%d\n",
				errno);
		fputs(PQerrormsg, stderr);
		pqdebug("%s", PQerrormsg);
		return (STATUS_ERROR);
	}

	/* fill in the client address */
	if (getsockname(port->sock, &port->laddr.sa, &len) < 0)
	{
		sprintf(PQerrormsg,
				"FATAL: StreamOpen: getsockname() failed: errno=%d\n",
				errno);
		fputs(PQerrormsg, stderr);
		pqdebug("%s", PQerrormsg);
		return (STATUS_ERROR);
	}

	return (STATUS_OK);
}
