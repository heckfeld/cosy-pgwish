#include    <stdio.h>
#include    <sys/types.h>

extern char *hextoa( char *, char *, unsigned);

int
get_srec( char *to, char *from, unsigned adr, int len, int *Srp)
{
    int bytes_read, in, out;
    unsigned cks;
    char buf[80];
    char fmt[32];
    unsigned checksum();

    bytes_read = 0;
    for( ; len>0; len-=in, bytes_read+=out) {
        int adj, s;

        if( adr + len < 0x10000) {
            in = (len > 34)? 34: len;
            adj = 4; s=1;
        }
        else if( adr + len < 0x1000000) {
            in = (len > 32)? 32: len;
            adj = 6; s=2;
        }
        else {
            in = (len > 32)? 32: len;
            adj = 8; s=3;
        }
        hextoa( buf, from, in);
        out = in + adj/2 + 1;
        cks = checksum( from, in);
        cks += (unsigned) out;
        cks +=  adr + (adr>>8) + ( adr>>16) + ( adr>>24);
        cks = ~cks;
        sprintf( fmt, "S%d%%2.2X%%%X.%XX%%s%%2.2X\n", s, adj, adj);
        sprintf( to, fmt, out, adr, buf, (u_char)cks);
        out = 2*out + 5;
        to += out;
        from += in;
        adr += in;
        (* Srp)++;
    }
    return( bytes_read);
}

unsigned
checksum( p, i)
u_char * p;
int i;
{
    int n;

    for( n=0;i>0; i--)
        n+=*p++;
    return( (unsigned)n);
}
