#include    <sys/types.h>

char *hextoa( s1,s2,len)
char *s1;              /* ASCII-String zurÏck */
u_char *s2;            /* Byte-String als Eingabe */
unsigned len;              /* LÌnge vom Byte_String */
{
    char *p = s1;
    static char hex[] = "0123456789ABCDEF";

    for( ; len-- > 0; s2++) {
        * s1++ = hex[ 0x0F & ( *s2 >> 4)];
        * s1++ = hex[ 0x0F & * s2];
    }
    * s1 = '\0';         /*  Endemarke des String s1 setzen */
    
    return( p);
    
}       /* ENDE hextoa  */
