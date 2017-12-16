/*
 *  Copyright (c) KFA Juelich GmbH
 *  Author Michael Simon
 *
 *  Routinen zum ein- und auspacken von Header- und Datenteil in die
 *  Struktur packet.
 */
#include    "miniload.h"

u_char *pack_frame( u_char *p, char c, short s, int i)
{
    p[ML_FUNC] = c;
    put_short( & p[ML_LENGTH], s);
    put_int( & p[ML_ADDR], i);
    return( & p[ML_DATA]);
}

u_char *unpack_frame( u_char *p, char *c, short *s, int *i)
{
    if( c)
        *c = p[ML_FUNC];
    if( s)
        *s = get_short( & p[ML_LENGTH]);
    if( i)
        *i = get_int( & p[ML_ADDR]);
    return( & p[ML_DATA]);
}
