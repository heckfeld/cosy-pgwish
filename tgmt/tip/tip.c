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
#include <errno.h>

#include <netdb.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pcap/pcap.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


/*
 * tip - pruefe TARGET Erreichbarkeit fuer eine gegebene WORKCELL
 *
 *       Verwendung von Namen:
 *		target - IP Adresse des Targets
 *		workcell - DOMAIN Name der Workcell
 *		host - name/ip/interface des lokalen rechners auf dem tip ausgefuehrt wird
 */

int
main(argc, argv)
int argc;
char *argv[];
{
#ifdef HAVE_GETADDRINFO
	struct addrinfo *workcellHostEntry = NULL;
	struct addrinfo workcellHints;
#else
	struct hostent *workcellHostEntry = NULL;
#endif
	char *prog = NULL;
	char *target = NULL;
	char *workcell = NULL;
	struct in_addr targetAddr, workcellAddr;
	char ip[BUFSIZ];
	char netmask[BUFSIZ];

	char errbuf[PCAP_ERRBUF_SIZE];

	int result;
	int found;

	/* libpcap nur fuer USER root erreichbar */
	pcap_if_t *pif;
	pcap_addr_t *pad;

	prog = strrchr(argv[0], '/') ? strrchr(argv[0], '/') + 1 : argv[0];

#ifdef HAVE_GETADDRINFO
	memset( &workcellHints, 0, sizeof(workcellHints));
	workcellHints.ai_family = AF_INET;
	workcellHints.ai_protocol = AF_INET;
#endif

	switch (argc) {
	case 3:
		target = argv[2];
		if (!isdigit(*target) || inet_aton(target, &targetAddr) == 0) {
			fprintf(stderr, "target bad format : %s\n", target);
			exit(1);
		}
		/*FALLTHROUGH*/
	case 2:
		workcell = argv[1];
		if (isdigit(*workcell)) {
			if (inet_aton(workcell, &workcellAddr) == 0) {
				fprintf(stderr, "workcell bad format : %s\n", workcell);
				exit(2);
			}
#ifdef HAVE_GETADDRINFO
		} else if (getaddrinfo(workcell, NULL, &workcellHints, &workcellHostEntry) != 0) {
#else
		} else if ((workcellHostEntry = gethostbyname(workcell)) == NULL) {
#endif
			fprintf(stderr, "%s: workcell %s not found\n", prog, workcell);
			exit(4);
		}
		break;
	default:
		fprintf(stderr,"compiled on host %s by user %s %s %s\n",
			HOST, USER, __DATE__, __TIME__);
		exit(5);
	}

	memset(ip, 0, BUFSIZ);
	memset(netmask, 0, BUFSIZ);

	memset(errbuf, 0, sizeof(errbuf));
	result = pcap_findalldevs(&pif, errbuf);
	if (result < 0) exit(6);

	/*
	 * lookup interfaces for target network,
	 * IS TARGET REACHABLE from this HOST
	 */
	found = 0;
	for (; pif; pif = pif->next) {

		if (pif->flags & PCAP_IF_LOOPBACK)
			continue;

		for (pad = pif->addresses; pad; pad = pad->next) {
			struct sockaddr_in *ifAddr, *ifNetmask;
			in_addr_t ifNetmaskAddr, loopback;

			if (pad->addr->sa_family != AF_INET)
				continue;

			/* Found a valid IPV4 NETWORK */

			ifAddr = (struct sockaddr_in *)(pad->addr);
			ifNetmask = (struct sockaddr_in *)(pad->netmask);
			ifNetmaskAddr = ifNetmask->sin_addr.s_addr;
#define hequals(a,b,mask) (((a).s_addr & (mask)) == ((b).s_addr & (mask)))

			sprintf(ip, "%s", inet_ntoa(ifAddr->sin_addr));
			sprintf(netmask, "%s", inet_ntoa(ifNetmask->sin_addr));

			loopback = inet_netof( ifAddr->sin_addr);
			if ( loopback == IN_LOOPBACKNET)
				continue;
// fprintf( stderr, "Found a valid IPV4 NETWORK: %s addr %s netmask %s\n", pif->name, ip, netmask);
			/*
			 * Is WORKCELL in this NETWORK
			 */
			if (workcellHostEntry) {
				struct in_addr hip;
				/*
				 * Die Liste der IP-Adresse fuer diesen Namen
				 * mit der IP des IF vergleichen
				 */
				/* process workcell name */
#ifdef HAVE_GETADDRINFO
			{
				struct addrinfo *ap;

				for (ap = workcellHostEntry; ap; ap = ap->ai_next) {
					hip = ((struct sockaddr_in*)ap->ai_addr)->sin_addr;
					if (hequals(hip, ifAddr->sin_addr, ifNetmaskAddr))
						break;
				}
			}
#else
			{
				in_addr_t **hpp;

				for (hpp = (in_addr_t **)workcellHostEntry->h_addr_list; *hpp; hpp++) {
					hip.s_addr = **hpp;
					if (hequals(hip, ifAddr->sin_addr, ifNetmaskAddr))
						break;
				}
				if (*hpp == NULL)
					continue;
			}
#endif
				sprintf(ip, "%s", inet_ntoa(hip));
				/* IP-Adresse passt zu IP des IF */
			} else {
				/* process workcell ip */
				if (!hequals(workcellAddr, ifAddr->sin_addr, ifNetmaskAddr))
					continue;

				sprintf(ip, "%s", inet_ntoa(workcellAddr));
			}
			/*
			 * If target then compare with I/F else found
			 */
			if (argc == 3) {
				if (hequals(targetAddr, ifAddr->sin_addr, ifNetmaskAddr)) {
					printf("%s %s\n", ip, pif->name);
					found++;
				} else {
					// fprintf( stderr, "ip passt nicht zum target %s\n", target);
				}
			} else {
				printf("%s\n", ip);
				found++;
			}
			if (found) {
				break;
			}
		}
		if (found) {
			break;
		}
	}
#ifdef HAVE_GETADDRINFO
	if (workcellHostEntry)
		freeaddrinfo( workcellHostEntry);
#endif
	pcap_freealldevs(pif);
	exit(found?0:7);
}
