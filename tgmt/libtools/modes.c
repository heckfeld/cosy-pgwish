#include    <stdio.h>
#include    <termio.h>

static struct termio tbufforg;

setraw( fildes)
int fildes;
{
    struct termio tbuff;

    if( ioctl(  fildes, TCGETA, &tbuff) == -1)
        return( -1);
    tbufforg = tbuff;
    tbuff.c_iflag &= ~(INLCR | ICRNL | IUCLC | ISTRIP | IXON | BRKINT);
    tbuff.c_lflag &= ~(ICANON | ISIG | ECHO);
    tbuff.c_cc[ VEOF] = 5;
    tbuff.c_cc[ VEOL] = 2;

    if( ioctl( fildes, TCSETAF, &tbuff) == -1)
        return( -1);
    return( 0);
}

void restore_mode( fd)
int fd;
{
    if( ioctl( fd, TCSETAF, &tbufforg) == -1)
        fprintf( stderr, "ioctl-restore");
}
