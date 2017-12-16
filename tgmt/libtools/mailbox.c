/*
 *  copied from Sto//mann, Hamburg
 *  mailbox driver
 */

#define     ExpressMail
#include    "mailbox.h"

getcharmail(mb)
    struct mb_box *mb;
{   unsigned char c;

    if (mb_isdata(&mb->hostbuf)) {
        mb_getchar(&mb->hostbuf, c)
        return c;
    }
    return 0;
}

putcharmail(mb, c)
    struct mb_box *mb;
    unsigned char c;
{
    while (!(mb_isfree(&mb->serverbuf))) ;  /* just wait    */
    mb_putchar(&mb->serverbuf, c);
}

int receiveints ;

mailserve(mb)               /* called by clock tick or whatever */
    struct mb_box *mb;
{
    if (mb->hostinfo.mb_cmd){       /* receive command  */
        mb->hostinfo.mb_cmd = 0;
        receiveints++;
    }
    return (mb_datacnt(&mb->hostbuf));
}

mailinit(mb, hostbuf, hostbufsize, servbuf, servbufsize)
    struct mb_box *mb;
    char * hostbuf, * servbuf;
    int hostbufsize, servbufsize;
{   ushort *a;

    receiveints = 0;
    for (a = (ushort *) mb; a < (ushort *) &mb[1]; a++)
        *a = 0;
    mb->serverinfo.mb_intby = none;

    mb_init(&mb->serverbuf,servbuf,servbufsize);

    mb_init(&mb->hostbuf,hostbuf,hostbufsize);

    mb->serverinfo.mb_private = (long) mailserve; /* to be called by clock*/
    mb->mb_magic = MBMAGIC;
}
