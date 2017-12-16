/* 
 *  Copyright (c) 1989 KFA Juelich GmbH
 *  Author Michael Simon
 *
 *  Toolbox mit Funktionen zum Schreiben und Lesen von
 *  int, long, short, .. in/aus Felder vom Typ 'unsigned char'.
 */

#include    "miniload.h"

#define UC(b,s) ((b)<<(s))

short get_short( u_char *p)
{
    return((short)( UC( p[0],8) | p[1]));
}

void put_short(u_char *p, u_short s)
{
    p[0] = (s>>8) & 0xff; p[1] = s;
}

int get_int( u_char *p)
{
    return((int) ( UC(p[0],24) | UC( p[1],16) | UC( p[2],8) | p[3]));
}

void put_int( u_char *p, u_int i)
{
    p[0] = (i>>24) & 0xff;p[1] = (i>>16) & 0xff;p[2] = (i>>8) & 0xff;p[3] = (i & 0xff);
}

long get_long( u_char *p)
{
    return((long) (UC(p[0],24) | UC(p[1],16) | UC(p[2],8) | p[3]));
}

void put_long( u_char *p, u_long l)
{
    p[0] = (l>>24) & 0xff;p[1] = (l>>16) & 0xff;p[2] = (l>>8) & 0xff;p[3] = l;
}
