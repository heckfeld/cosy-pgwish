/*
 * Include-File for those, using low-level I/O-Routines
 *
 * Description:	call trap intstead of I/O-Routine.
 *		change exit -> return.
 *
 */
#include 	"lantrap.h"

#define	_IO_CALL	lantrap

#define	close(f)	_IO_CALL( LAN_CLOSE,(f))
#define	exit(r)		return( r)
#define	open(p,opt)	_IO_CALL( LAN_OPEN,(p),(opt))
#define	read(f,b,l)	_IO_CALL( LAN_READ,(f),(b),(l))
#define	write(f,b,l)	_IO_CALL( LAN_WRITE,(f),(b),(l))
#define	ioctl(f,r,a)	_IO_CALL( LAN_IOCTL,(f),(r),(a))
