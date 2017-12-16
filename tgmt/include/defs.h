typedef int BOOLEAN;

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif /* TRUE */

#define	repeat	do
#define until( expr)	while( !(expr))

#define lowbyte(w) ((w) & 0377)
#define highbyte(w) lowbyte((w)>>8)

#define GET_SHORT( s, p)	\
		s = (short)(( (p)[0]<<8) + (p)[1])
#define PUT_SHORT( s, p)	\
		p[0] = ((s)>>8)&0xff; p[1] = (s)
#define GET_INT( i, p)		\
		i = (int)(( (p)[0]<<24) + ((p)[1]<<16) + ((p)[2]<<8) + (p)[3])
#define PUT_INT( i, p)		\
		p[0] = ((i)>>24);p[1] = ((i)>>16);p[2] = ((i)>>8);p[3] = (i)
