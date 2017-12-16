/*
 *	Copyright (c) 1989 KFA Juelich GmbH
 *	Author Michael Simon
 *
 *	Definitions for calling the lanio-Functions via TRAP #15
 */

#define 	LAN_CLOSE	0
/* exit -> return   1 */
#define 	LAN_OPEN	2
#define 	LAN_READ	3
#define 	LAN_WRITE	4
#define 	LAN_IOCTL	5
