#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pcap-int.h"

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/utsname.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/if_ether.h>
#include <net/if_arp.h>

#include <signal.h>
#include <setjmp.h>

#ifdef PF_PACKET
# include <linux/if_packet.h>

 /*
  * On at least some Linux distributions (for example, Red Hat 5.2),
  * there's no <netpacket/packet.h> file, but PF_PACKET is defined if
  * you include <sys/socket.h>, but <linux/if_packet.h> doesn't define
  * any of the PF_PACKET stuff such as "struct sockaddr_ll" or any of
  * the PACKET_xxx stuff.
  *
  * So we check whether PACKET_HOST is defined, and assume that we have
  * PF_PACKET sockets only if it is defined.
  */
# ifdef PACKET_HOST
#  define HAVE_PF_PACKET_SOCKETS
# endif /* PACKET_HOST */
#endif /* PF_PACKET */


int
pcap_write( pcap_t * pd, int len)
{
    int cc;
    struct sockaddr_ll to;
    int tolen;
    int bto = len;

    tolen = sizeof( to);
    bzero( &to, tolen);

    to.sll_family = AF_PACKET;
    to.sll_ifindex = pd->ifindex;

    pd->tbuf[14] = pd->dsap;
    pd->tbuf[15] = pd->ssap;
    /*pd->tbuf[16] = 3;				/* Unnumbered Command */
    bto += 3;                               /* LLC Header Length */
    pd->tbuf[12] = (bto >> 8) & 0xff;
    pd->tbuf[13] = bto & 0xff;
    bto += 14;                              /* MAC Header Length */
    if( bto < 60) {			    /* Fill small packets */
	bzero( pd->tbuf + bto, 60-bto);
	bto = 60;
    }
    cc = sendto( pd->fd, pd->tbuf, bto, 0, (struct sockaddr *)&to, tolen);
    if ( cc == -1 ) {
	perror( "pcap_write");
	return -1;
    } else
	return len;
}

static char *
ether_print_addr( unsigned char * up)
{
    static char ebuf[100];

    sprintf( ebuf, "%02x:%02x:%02x:%02x:%02x:%02x",
		up[0], up[1], up[2], up[3], up[4], up[5]);
    return ebuf;
}

static int got_signal;

static void alarm_handler(int signum)
{
    got_signal++;
}

/*
 * pcap_read - read a packet from the net
 *
 *	there are several error reasons:
 *		-2: no answer from remote host
 *		-1: local error
 *	     other: success 
 */

int
pcap_read(pcap_t *pd)
{
    int result;
    int     cc;
    int     bufsize;
    u_char *bp;
    struct sockaddr_ll  from;
    int                 fromlen;
    struct sigaction act, oldact;

    result = -1;

    memset( &act, '\0', sizeof(act));
    act.sa_handler = alarm_handler;
    act.sa_flags = 0;	/* SA_RESTART would block recvfrom, similar to signal() */

    bp = pd->rbuf;
    bufsize = pd->bufsize;

    if( sigaction( SIGALRM, &act, &oldact) == -1) {
	snprintf( pd->errbuf, PCAP_ERRBUF_SIZE , "Failed to install Signal Handler\n");
	return -2;
    }

    if( pd->timeout) alarm( pd->timeout);

again:
    do {
	fromlen = sizeof(from);
	cc = recvfrom(pd->fd, bp, bufsize, 0, (struct sockaddr *)&from, &fromlen);
	if ( got_signal) {
		if( pd->timeout) alarm(0);
		got_signal = 0;
		snprintf( pd->errbuf, PCAP_ERRBUF_SIZE, "%s[%d]: %s signal ALARM\n",
			__FILE__, __LINE__, __FUNCTION__);
		result =  -2;             /* XXX */
		goto cleanup;
	}
	if (cc < 0) {
	    if( pd->timeout) alarm(0);
	    if (errno == EWOULDBLOCK) result = -2;
	    strerror_r( errno, pd->errbuf, PCAP_ERRBUF_SIZE);
	    goto cleanup;
        }
    } while (cc == -1 && errno == EINTR);

/* fprintf( stderr, "Packet from %s ssap %x dsap %x len %d\n",
		ether_print_addr(bp+6), bp[14], bp[15], (bp[12]<<8) + bp[13]); */

    /*
     * compare source of packet with destination of tbuf
     * compare SSAP with DSAP of tbuf
     */
    if ( bcmp( &bp[ETH_ALEN], pd->tbuf, ETH_ALEN) || (bp[15] != pd->tbuf[14]))
	goto again;

/* fprintf( stderr, "%s[%d]: %s Packet match\n", __FILE__, __LINE__, __FUNCTION__); */

    if( pd->timeout) alarm(0);
    result = cc - 17;
    (void)sigaction( SIGALRM, &oldact, NULL);
    return result;

cleanup:
    (void)sigaction( SIGALRM, &oldact, NULL);
    return result;
}

/*
 *  Return the index of the given device name. Fill ebuf and return
 *  -1 on failure.
 */
static int
iface_get_id(int fd, const char *device, char *ebuf)
{
        struct ifreq    ifr;

        memset(&ifr, 0, sizeof(ifr));
        strncpy(ifr.ifr_name, device, sizeof(ifr.ifr_name));

        if (ioctl(fd, SIOCGIFINDEX, &ifr) == -1) {
		sprintf(ebuf, "ioctl: %s: %s", device, strerror(errno));
                return -1;
        }

        return ifr.ifr_ifindex;
}

/*
 *  Bind the socket associated with FD to the given device.
 */
static int
iface_bind(int fd, int ifindex, char *ebuf)
{
        struct sockaddr_ll      sll;
        int                     err;
        socklen_t               errlen = sizeof(err);

        memset(&sll, 0, sizeof(sll));
        sll.sll_family          = AF_PACKET;
        sll.sll_ifindex         = ifindex;
        sll.sll_protocol        = htons(ETH_P_802_2);

        if (bind(fd, (struct sockaddr *) &sll, sizeof(sll)) == -1) {
                sprintf(ebuf, "bind: %s", strerror(errno));
                return -1;
        }

        /* Any pending errors, e.g., network is down? */

        if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &errlen) == -1) {
                sprintf(ebuf, "getsockopt: %s", strerror(errno));
                return -2;
        }

        if (err > 0) {
                sprintf(ebuf, "bind: %s", strerror(err));
                return -2;
        }

        return 0;
}


pcap_t *
pcap_open_live(char *device, int snaplen, int promisc, char *ebuf)
{
	int device_id, err, sock_fd, broadcast;
	register pcap_t *pd;
	struct ifreq ifr;
	char *def_dev = "eth0";
	u_char ethbuf[ETH_ALEN];

	if ( device == NULL) device = def_dev;

	pd = (pcap_t *)malloc(sizeof(*pd));
	if (pd == NULL) {
		sprintf(ebuf, "malloc: %s", strerror(errno));
		return (NULL);
	}
	memset(pd, 0, sizeof(*pd));
	sock_fd = -1;

	sock_fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_802_2));
	if (sock_fd < 0) {
		sprintf(ebuf, "socket: %s", strerror(errno));
		goto bad;
	}
	pd->fd = sock_fd;

	device_id = iface_get_id(sock_fd, device, ebuf);
	if (device_id == -1)
	    goto bad;

	if ((err = iface_bind(sock_fd, device_id, ebuf)) < 0) {
	    goto bad;
	}

	pd->ifindex = device_id;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, device, sizeof(ifr.ifr_name));
	if (ioctl(pd->fd, SIOCGIFHWADDR, &ifr) < 0 ) {
		sprintf(ebuf, "SIOCGIFHWADDR: %s", strerror(errno));
		goto bad;
	}
	broadcast = 0;

	switch (ifr.ifr_hwaddr.sa_family) {

	case ARPHRD_ETHER:
	case ARPHRD_METRICOM:
		pd->linktype = DLT_EN10MB;
		++broadcast;
		break;

	case ARPHRD_EETHER:
		pd->linktype = DLT_EN3MB;
		++broadcast;
		break;

	case ARPHRD_AX25:
		pd->linktype = DLT_AX25;
		++broadcast;
		break;

	case ARPHRD_PRONET:
		pd->linktype = DLT_PRONET;
		break;

	case ARPHRD_CHAOS:
		pd->linktype = DLT_CHAOS;
		break;

	case ARPHRD_IEEE802:
		pd->linktype = DLT_IEEE802;
		++broadcast;
		break;

	case ARPHRD_ARCNET:
		pd->linktype = DLT_ARCNET;
		++broadcast;
		break;

	case ARPHRD_SLIP:
	case ARPHRD_CSLIP:
	case ARPHRD_SLIP6:
	case ARPHRD_CSLIP6:
	case ARPHRD_PPP:
		pd->linktype = DLT_RAW;
		break;

	case ARPHRD_LOOPBACK:
		pd->linktype = DLT_NULL;
		break;


	default:
		sprintf(ebuf, "unknown physical layer type 0x%x",
		    ifr.ifr_hwaddr.sa_family);
		goto bad;
	}

	/* Remember OUR Ethernet Address */
	memcpy( ethbuf, ifr.ifr_hwaddr.sa_data, ETH_ALEN);

	/* Base the buffer size on the interface MTU */
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, device, sizeof(ifr.ifr_name));
	if (ioctl(pd->fd, SIOCGIFMTU, &ifr) < 0 ) {
		sprintf(ebuf, "SIOCGIFMTU: %s", strerror(errno));
		goto bad;
	}

	/* Leave room for link header (which is never large under linux...) */
	pd->bufsize = ifr.ifr_mtu + 256;

	/* Write Buffer */
	pd->tbuf = (u_char *)malloc(pd->bufsize);
	if (pd->tbuf == NULL) {
		sprintf(ebuf, "malloc: %s", strerror(errno));
		goto bad;
	}
	memcpy( &pd->tbuf[ETH_ALEN], ethbuf, ETH_ALEN);

	pd->rbuf = (u_char *)malloc(pd->bufsize);
	if (pd->rbuf == NULL) {
		sprintf(ebuf, "malloc: %s", strerror(errno));
		goto bad;
	}

	/* XXX */
	if (promisc && broadcast) {
		memset(&ifr, 0, sizeof(ifr));
		strcpy(ifr.ifr_name, device);
		if (ioctl(pd->fd, SIOCGIFFLAGS, &ifr) < 0 ) {
			sprintf(ebuf, "SIOCGIFFLAGS: %s", strerror(errno));
			goto bad;
		}
		ifr.ifr_flags |= IFF_BROADCAST;
		if (ioctl(pd->fd, SIOCSIFFLAGS, &ifr) < 0 ) {
			sprintf(ebuf, "SIOCSIFFLAGS: %s", strerror(errno));
			goto bad;
		}
		ifr.ifr_flags &= ~IFF_BROADCAST;
	}

	pd->snapshot = snaplen;

	return (pd);
bad:
	if (sock_fd >= 0)
		(void)close(sock_fd);
	if (pd->rbuf != NULL)
		free(pd->rbuf);
	if (pd->tbuf != NULL)
		free(pd->tbuf);
	free(pd);
	return (NULL);
}

u_char *
net_aton(u_char *dstr, const char *sstr, int size)
{
    extern u_char * atohex(u_char *, const char *, int);

    if ( size < ETH_ALEN) return NULL;

    if ( sstr[0] != '0' || sstr[1] != 'x' || strlen( sstr) < 14 ) return NULL;

    return atohex( dstr, sstr+2, size);
}

pcap_t *
init_lla( char *dev, char *ether_adr, int ssap, int dsap, int control, int timeout)
{
    pcap_t * pd;
    char ebuf[PCAP_ERRBUF_SIZE];

    pd = pcap_open_live( dev, 1514, control != 0x03, ebuf);
    if ( pd == NULL) {
	return NULL;
    }

    /* Einrichten des Write Buffers */
    net_aton( pd->tbuf, ether_adr, ETH_ALEN);

    pd->tbuf[14] = pd->dsap = dsap;
    pd->tbuf[15] = pd->ssap = ssap;
    pd->tbuf[16] = control;				/* Unnumbered Command */

    /* Einrichten des Write Buffers */
    pd->tp = pd->tbuf + 17;			/* LLC Data */

    /* Einrichten des Read Buffers */
    pd->rp = pd->rbuf + 17;

    pd->timeout = timeout;

    got_signal = 0;
    return pd;
}
