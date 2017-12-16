#ifndef lint
static char sccsid[] = "@(#)srec.c	1.7	(MIKE)	01/25/91";
#endif

#include <sys/types.h>
#include <sys/fcntl.h>
#include <stdio.h>
#include <memory.h>
#include <ctype.h>

static char Version[] = "@(#)srec.c      1.7     (MIKE)  01/25/91";

#define	TEXTLEN		128
#ifndef NULL
#define NULL		0
#endif

#define MAX_BYTE_ADDR	0x20000		/* bezogen auf EPROM 27010 */
#define PRESET_VALUE	0x00

char *help_str[] = {
	"Das Programm srec.c liest einen Motorola S-Record File vom",
	"Standard-Input ein und erzeugt einen convertierten S-Record",
	"Files oder ein binÌres File.",
	"Eingabe :   standard input ",
	"Ausgabe :   standard output oder die Files B_FILE [-b] oder S_FILE",
	"Aufruf  :  srec [ options] [ out_file]",
	"Options :",
	"     -b                             create binary data file",
	"     -c                             cut print addr max. 6 hex-digits",
	"     -d low_addr-high_addr          delete memory area",
	"     -m low_addr-high_addr:to_addr  map memory from low_addr to",
	"                                    high_addr to to_addr",
	"                                    Notice that base_addr will be",
	"                                    subtracted",
	"     -o hex_addr                    set offset_address (default 0x0)",
	"                                    all addresses will be addr - hex_addr",
	"     -s size                        size of EPROM (default64k)",
	"     -v                             Version of ..",
	NULL };

/*
 MODE:
        |  user     |  group    |  other    |
		| 1 | 1 | 0 | 1 | 0 | 0 | 1 | 0 | 0 |  = 1A4 hex
          r   w   x   r   w   x   r   w   x
*/
int	MODE = 0x1A4;      	
char in_text[TEXTLEN];
char out_text[TEXTLEN];

int	binary = 0;
int eprom_maxsize;
int eprom_out;
u_char	byte[MAX_BYTE_ADDR];
u_int	base_addr = 0x0;
int	cut = 0;
u_int	l_map = 0x0;
u_int	u_map = 0x0;
u_int 	t_map = 0x0;

int		delete = 0;
u_int	l_del = 0x0;
u_int	u_del = 0x0;

int		LINE_NUM = 0;
int		LINE_S = 0;

unsigned char 	cir;
unsigned char 	cic;
unsigned char 	coc;
char in_rec_info[80];
int	irt;	/* input record type */
int	ort;	/* output record type */
int	irc;
int	orc;
unsigned int 	ira;
unsigned int 	ora;
char in_rec_dat[TEXTLEN];
char out_rec_dat[TEXTLEN];
int 	ind;
int 	ond;
int 	in_nrecord;
int	itcp;
int	otcp;
int	itp;
int	otp;

char s_file[256];	/* help filename of output file */

FILE	*fopen(), *fd_even, *fd_odd, *fd_s;
int	fd_bin;

unsigned char *atohex();
char *hextoa();
void ocs();
unsigned read_in_check_sum(),
		 in_check_sum();

extern void get_options( int , char **);

int
main(int argc, char *argv[])
{
	int	i,j,k;

	get_options(argc,argv);

    eprom_maxsize = MAX_BYTE_ADDR;
    eprom_out = 0;

	if (binary)
		memset (&byte[0], PRESET_VALUE, eprom_maxsize);

	while ( fgets (in_text, TEXTLEN, stdin) != NULL) {
	   LINE_NUM++;
	   analyse_type();
	   switch (irt) {
	   case   0:
			get_info();

/* ignore this record Fri Jan 25 09:31:13 MEZ 1991 mike
			if( read_in_check_sum() != in_check_sum())
				fprintf( stderr, "error on input crc\n");	*/

			if( ! binary) {
				fprintf (fd_s, "%s", in_text);
			}
			break;

	   case   1:
	   case   2:
	   case   3:
			get_data();
			if( read_in_check_sum() != in_check_sum())
				fprintf( stderr, "error on input crc\n");;

			if (put_data()) {
				otcp = otp;
				ocs();
				LINE_S++;

				if (binary) {
					if ( (ora + ond -1) >  eprom_maxsize  ) {
						binary = 0;
						fprintf(stderr,"%s: binary limit reached: %x (%x)\n",
							argv[0], ira, ora);
					} else {
						eprom_out += ond;
						memcpy(&byte[ora],&out_rec_dat[0],ond);
					}
				} else
					fprintf (fd_s, "%s", out_text);
			}
			break;

		case   5:
			if( read_in_check_sum() != in_check_sum())
				fprintf( stderr, "error on input crc\n");;
			if( ! binary) {
				sprintf (&out_text[0], "%c", in_text[0]);
				sprintf (&out_text[1], "%c", in_text[1]);
				sprintf (&out_text[2], "03");
				sprintf (&out_text[4], "%4.4X", LINE_S);
				otcp = 8;
				ocs();
				fprintf (fd_s, "%s", out_text);
			}
			break;

		case   7:
		case   8:
		case   9:
			if( read_in_check_sum() != in_check_sum())
				fprintf( stderr, "error on input crc\n");;

			if( ! binary) {
				if (put_data()) {
					otcp = otp;
					ocs();
					fprintf (fd_s, "%s", out_text);
				}
			}
			break;

		default:
			printerr();
			break;
	   }
	}
	if (binary)
	{
		if((fd_bin=open("B_FILE", O_RDWR | O_CREAT, MODE )) == -1) 
		{
			perror("B_FILE ");
			exit(1);
		}
		write(fd_bin, byte, eprom_out);
	}
	exit (0);
}

/************************************************************************
   analyse_type liest Record-Typ (z.b. S3) und Anzahl der Bytes-2 (rec_cnt).
   Ein Record Byte ist im eingelesenen Text-String durch zwei hexdezimale
   Ziffern dargestellt. Abhaengig vom Typ wird die Adresse eingelesen
   (ira) und die Startposition fuer die Daten (itp) im String
   bestimmt.
************************************************************************/
int	analyse_type()
{
	int	i,j,k;


	if (in_text[0] == 'S')
	{
	   irt = isdigit( in_text[1])? in_text[1]-'0': -1;
	   ort = irt;
	   sscanf (&in_text[2], "%2x", &irc);
	   itcp = irc * 2;
	   orc = irc;
	   
	   switch (irt)
	   {
		case 1:
		case 9:
	      	sscanf(&in_text[4], "%4x", &ira);
 	      	itp = 8;
			ind = irc - 1 - (itp -4)/2;
			break;

		case 2:
		case 8:
	      	sscanf (&in_text[4], "%6x", &ira);
 	      	itp = 10;
			ind = irc - 1 - (itp -4)/2;
			break;

		case 3:
	      	sscanf (&in_text[4], "%8x", &ira);
 	      	itp = 12;
			ind = irc - 1 - (itp -4)/2;
	      	if (cut)
	      	{
	       		ort = 2;
		 		orc = irc - 1;
           	}
			break;
	   
		case 7:
	      	sscanf (&in_text[4], "%8x", &ira);
 	      	itp = 12;
			ind = irc - 1 - (itp -4)/2;
			if (cut)
	      	{
		 		orc = irc - 1;
	         	ort = 8;
            }
			break;

	   
		case 0:
 	      	itp = 4;
			ind = irc - 1;
			break;
	   
		case 5:
            sscanf (&in_text[4], "%4x", &in_nrecord);
			ind = 0;
			break;
	   
		default:
	      		irt = -2;
			break;
	   }

	}

}


get_data()
{
	atohex( &in_rec_dat[0], &in_text[itp], ind);
}

/***********************************************************************
   get_info liest die File-Information in einem S0-Record und
   speichert sie im Feld in_rec_info ab.
***********************************************************************/
get_info()
{
	   atohex( &in_rec_info[0], &in_text[4], ind);
}

/***********************************************************************
   read_in_check_sum liest die Check-Summe eines Eingabe-Records und
   speichert sie in cir. (Muss mit cic uebereinstimmen)
***********************************************************************/
unsigned read_in_check_sum()
{
	int	position;
	int	tmp_check;

	position = (irc + 1) * 2 ;
        sscanf (&in_text[position], "%2x", &tmp_check);
	return( cir = (char) tmp_check);
}


/***********************************************************************
   in_check_sum berechnet die Check-Summe eines Eingabe-Records und
   speichert sie in check_in_cal. (Muss mit cir uebereinstimmen)
***********************************************************************/

static char tbuf[TEXTLEN];

unsigned in_check_sum()
{
	int 	j;
	int 	sum;
	int		end;

	end = itcp/2;
	atohex( tbuf, &in_text[2], end);
	for (sum = 0, j = 0; j < end; j ++) {
	   sum += tbuf[j];
	}
	return( cic =  (char) ~sum);
}

static char hex[] = "0123456789ABCDEF";

void ocs()
{
	int 	j;
	int 	sum;
	int		end;

	end = otcp/2 - 1;
	atohex( tbuf, &out_text[2], end);
	for (sum = 0, j = 0; j < end; j ++) {
	   sum += tbuf[j];
	}
	coc =  (char) ~sum;
	out_text[otcp] = hex[( coc>>4)&0x0f];
	out_text[otcp+1] = hex[ coc&0x0f];
	out_text[otcp+2] = '\n';
	out_text[otcp+3] = '\0';
}


printerr()
{
	fprintf (stderr, "Unknown record format at line %d : \n", LINE_NUM);
	fprintf (stderr, "%s \n", in_text);
}

put_data()
{
	int	j,k;
	unsigned int i;
	int	cnt;

	if ( (irt < 4) && (delete) )
		if ( (ira >= l_del) && (ira <= u_del) )
			return (0);

	out_text[0] = 'S';

	i = ira;
	if ((ira >= l_map) && (ira <= u_map))
	{
           i = ira - l_map + t_map;
	}
	ora = ira;

	if ( (ort <= 3) && (ort >= 1) )
	   ora = (i - base_addr);

	if ( ora > 0xffffff && (ort < 3 && ort > 0)) {
	    ort = 3;
	    orc = irc + 1;
	}

	out_text[1] = ort+'0';

	memcpy( &out_rec_dat[0], &in_rec_dat[ 0], ind);

	ond = ind;
	cnt = orc - ind + ond ;
	out_text[2] = hex[(cnt>>4)&0x0f];
	out_text[3] = hex[cnt&0x0f];
	otp = 4;

	switch (ort)
	{
	   case 1:
	   case 9:
	   	sprintf (&out_text[4], "%4.4X", ora);
	   	otp += 4;
		break;

	   case 2:
	   case 8:
	   	sprintf (&out_text[4], "%6.6X", ora);
	   	otp += 6;
		break;

	   case 3:
	   case 7:
	   	sprintf (&out_text[4], "%8.8X", ora);
	   	otp += 8;
		break;

	   default:
		break;
	}

	hextoa( &out_text[otp], &out_rec_dat[0], ond);
	otp += 2*ond;
	return (1);

}

void
get_options(int argc, char *argv[])
{
    int	c;
    extern char	*optarg;
    extern int optind, opterr;
    char	*string;
    int 	bflg, errflg, aflg;

    errflg = aflg = bflg = 0;
    opterr = 0;
    while ((c =	getopt(argc, argv, "bcd:hm:o:s:v")) != EOF)
	switch	(c) {
	 	case 'b':
              	binary = 1;
	      		break;
	 	case 'c':
			cut = 1;
	      		break;
	 	case 'd':
	      		string = optarg;
				delete = 1;
	      		get_del_args(string);
	      		break;
	 	case 'h':
			give_help();
			exit( 0);
	 	case 'm':
	      		string = optarg;
	      		get_map_args(string);
	      		break;
	 	case 'o':
	      		string = optarg;
	      		sscanf (string, "%x", &base_addr);
	      		break;
	 	case 's':
	      		string = optarg;
	      		sscanf (string, "%x", &eprom_maxsize);
				if( eprom_maxsize > MAX_BYTE_ADDR)
					errflg = 1;
	      		break;
	 	case 'v':
			printf( "%s", Version);
			exit(0);
		case '?':
			errflg++;
			break;
	}
    if (errflg)	{
	fprintf( stderr, "Usage:%s [-bchv] [-d adr] [-o adr] [-m adr] [-s size] [output_filename]\n", argv[0]);
	exit (2);
    }
    if( optind < argc) {
	strcpy( s_file, argv[optind++]);
	if ((fd_s=fopen (s_file,"w")) == 0) {
	    perror(s_file);
	    exit(1);
    	}
    } else
	fd_s = stdout;
}

get_map_args(string)
char	string[];
{
	char	c;
	char	ctmp[80];
	int	i,j;

	i = 0;
	j = 0;
	while ( (c = string[i]) != '-')
	{
		ctmp[j] = c;
		i++;
		j++;
	}
	ctmp[j] = '\0';
	sscanf (ctmp, "%x", &l_map);

	j = 0;
	i++;
	while ( (c = string[i]) != ':')
	{
		ctmp[j] = c;
		i++;
		j++;
	}
	ctmp[j] = '\0';
	sscanf (ctmp, "%x", &u_map);
	u_map--;

	j = 0;
	i++;
	while ( (c = string[i]) != '\0' )
	{
		ctmp[j] = c;
		i++;
		j++;
	}
	ctmp[j] = '\0';
	sscanf (ctmp, "%x", &t_map);
}


get_del_args(string)
char	string[];
{
	char	c;
	char	ctmp[80];
	int	i,j;

	i = 0;
	j = 0;
	while ( (c = string[i]) != '-')
	{
		ctmp[j] = c;
		i++;
		j++;
	}
	ctmp[j] = '\0';
	sscanf (ctmp, "%x", &l_del);

	j = 0;
	i++;
	while ( (c = string[i]) != '\0')
	{
		ctmp[j] = c;
		i++;
		j++;
	}
	ctmp[j] = '\0';
	sscanf (ctmp, "%x", &u_del);
	u_del--;
}

give_help()
{
	char **p;

	for( p = &help_str[0]; *p; p++)
		printf( "%s\n", *p);
}

static unsigned _hexshift[] = {
000,001,002,003,004,005,006,007,010,011,012,013,014,015,016,017,
020,021,022,023,024,025,026,027,030,031,032,033,034,035,036,037,
040,041,042,043,044,045,046,047,050,051,052,053,054,055,056,057,
000,001,002,003,004,005,006,007,010,011,072,073,074,075,076,077,
100,012,013,014,015,016,017,107,110,111,112,113,114,115,116,117,
120,121,122,123,124,125,126,127,130,131,132,133,134,135,136,137,
140,012,013,014,015,016,017,147,150,151,152,153,154,155,156,157,
160,161,162,163,164,165,166,167,170,171,172,173,174,175,176,177,
200,201,202,203,204,205,206,207,210,211,212,213,214,215,216,217,
220,221,222,223,224,225,226,227,230,231,232,233,234,235,236,237,
240,241,242,243,244,245,246,247,250,251,252,253,254,255,256,257,
260,261,262,263,264,265,266,267,270,271,272,273,274,275,276,277,
300,301,302,303,304,305,306,307,310,311,312,313,314,315,316,317,
320,321,322,323,324,325,326,327,330,331,332,333,334,335,336,337,
340,341,342,343,344,345,346,347,350,351,352,353,354,355,356,357,
360,361,362,363,364,365,366,367,370,371,372,373,374,375,376,377
};

u_char *atohex( s, b, len)
register u_char *s;		/* Byte-String zurÏeck */
register char *b;				/* ASCII-String als Eingabe */
unsigned int len;			/* LÌnge vom ASCII-String */
{   
    unsigned int i, t, u;

    for( i=0; i<len; i++) {
        t = (u_char)b[ i<<1];  u = (u_char)b[ (i<<1)+1] ;
        if( isxdigit( t) && isxdigit( u))
            s[ i] = _hexshift[ u] + (_hexshift[ t]<<4);
        else
            return( NULL);
    }
    return( s);

}       /* ENDE atohex */

char *hextoa( s1,s2,len)
register char *s1;				/* ASCII-String zurÏck */
register u_char *s2;			/* Byte-String als Eingabe */
register unsigned len;				/* LÌnge vom Byte_String */
{
    unsigned int  i;

    s1[ len<<1] = '\0';         /*  Endemarke des String s1 setzen */
    
    for( i=0; i<len; i++) {
        s1[ i<<1]    = hex[ 0x0F & (s2[ i]>>4)];
        s1[ (i<<1)+1] = hex[ 0x0F & s2[ i]];
    }
    
    return( s1);
    
}       /* ENDE hextoa  */
