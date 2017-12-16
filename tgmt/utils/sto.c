#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <ctype.h>
#include <sys/types.h>

#define		MAXS2ADDR	0xffffff
#define 	MAXS3ADDR	0xffffffff
#define 	MAXBYTE		38

static char mikeid[] = "@(#)sto.c 1.5 09/19/90";

int to = 2;	/* default convert to S2 */
FILE *out_fp;
u_char *atohex();

main( argc, argv)
int argc;
char * argv[];
{
	char line[81];
	char lino[81];
	u_int addr;
	int temp;

	get_options( argc, argv);

	lino[0] ='\0';
	while( fgets( line, sizeof(line), stdin) != NULL) {
		if( line[0] != 'S')
		{
#ifdef DEBUG
			fprintf( stderr, "%s: Not a S-Record >%s<\n", argv[0], line);
#endif
			;
		}
		else if(( line[1] == '2' || line[1] == '8') && to == 3)
		{
			temp = line[1]-'0';
			temp = (temp==2)? 3 : 7;
			convert( lino, line, temp);
		}
		else if(( line[1] == '3' || line[1] == '7') && to == 2)
		{
			if( sscanf( &line[4], "%8x", &addr) == 1)
			{
				if( addr > MAXS2ADDR)
				{
					fprintf( stderr, "%s: address %x to big\n", argv[0], addr);
					exit( 1);
				}
			}
			temp = line[1] - '0';
			temp = (temp == 3) ? 2: 8;
			convert( lino, line, temp);
		}

		fprintf( out_fp, "%s\n", (lino[0])?lino:line);
		lino[0] ='\0';
	}
}
convert( out, line, to)
char *out;
char *line;
int to;
{
	static char hex[] = "0123456789ABCDEF";
	u_char stream[40];
	int i;
	u_int bytecnt;
	u_char checksum;

	out[0] ='S';
	out[1] = to + '0';
	if( sscanf( &line[2], "%2x", &bytecnt) == 1)
	{
		if( bytecnt <= MAXBYTE) {
			if( to == 2 || to == 8) {
				bytecnt -= 1;
				copy( &out[4], &line[6], bytecnt*2-2);
			}
			else if( to == 3 || to == 7) {
				out[4] = '0';
				out[5] = '0';
				copy( &out[6], &line[4], bytecnt*2-2);
				bytecnt += 1;
			}
			else
				return( -1);
			
			out[2] = hex[ (bytecnt>>4) &0x0f];
			out[3] = hex[ bytecnt & 0x0f];
			atohex( stream, &out[2], bytecnt);
			checksum = 0;
			for( i=0; i< bytecnt; i++)
				checksum += stream[i];
			checksum = ~checksum;
			bytecnt = (bytecnt << 1) + 2;
			out[bytecnt] = hex[ (checksum>>4) & 0x0f];
			out[bytecnt+1] = hex[ checksum & 0x0f];
			out[bytecnt+2] = '\0';
		}
	}
}

copy( s, t, l)
register char *s, *t;
register u_int l;
{
	while( l--)
		*s++ = *t++;
}

get_options(argc,argv)
int argc;
char *argv[];
{
    int	c;
    extern char	*optarg;
    extern int optind;
	int temp;
	char *basename, *string;
	char out_file[ 256];

    while ((c =	getopt(argc, argv, "?hs:")) != EOF)
	{
		basename = (string=strrchr( argv[0], '/'))? string+1: argv[0];
		switch	(c) 
		{
		case '?':
	 	case 'h':
			give_help( basename);
			exit( 0);
			break;
	 	case 's':
	      	sscanf (optarg, "%d", &temp);
			if( temp != 2 && temp != 3)
				fprintf( stderr, "%s: option ignored ( use -s[23])\n", basename);
			else
				to = temp;
#ifdef DEBUG
			fprintf( stderr, "get_options: convert from S%d to S%d\n",
				(to==2)?3:2, to);
#endif
	      	break;
		default:
			give_help( basename);
			exit( 1);
			break;
		}
	}

	if( optind < argc)
	{
		strcpy( out_file, argv[optind++]);
		if(( out_fp = fopen( out_file, "w")) == NULL)
		{
			fprintf( stderr, "%s: cannot open file %s\n", basename, out_file);
			exit( 1);
		}
	}
	else
		out_fp = stdout;
#ifdef DEBUG
	fprintf( stderr, "get_options: Output to %s\n", out_file);
#endif
}


give_help( progname)
char *progname;
{
	fprintf( stderr, "Das Programm &dC%s&d@ convertiert MOTOROLA S-Record", progname);
	fprintf( stderr, " Files. Dabei besteht die\nMoeglichkeit, vom S2-Format");
	fprintf( stderr, " zum S3-Format( default) und umgekehrt zu wandeln.\n");
	fprintf( stderr, "Eingabe:  ist der Standard Input\n");
	fprintf( stderr, "Ausgabe:  ist der Standard Output, oder der letzte\n");
	fprintf( stderr, "          Parameter aus der Kommandozeile\n");
	fprintf( stderr, "Aufruf:   &dC%s [options] [output-file]&d@\n", progname);
	fprintf( stderr, "Optionen:\n");
	fprintf( stderr, "          -?\n");
	fprintf( stderr, "          -h      dieser Ausdruck\n");
	fprintf( stderr, "          -s {23} Angabe des Zielformates\n");
	fprintf( stderr, "\n");
}

static u_char _hexshift[] = {
000,001,002,003,004,005,006,007,010,011,012,013,014,015,016,017,
020,021,022,023,024,025,026,027,030,031,032,033,034,035,036,037,
040,041,042,043,044,045,046,047,050,051,052,053,054,055,056,057,
000,001,002,003,004,005,006,007,010,011,072,073,074,075,076,077,
0100,012,013,014,015,016,017,0107,0110,0111,0112,0113,0114,0115,0116,0117,
0120,0121,0122,0123,0124,0125,0126,0127,0130,0131,0132,0133,0134,0135,0136,0137,
0140,012,013,014,015,016,017,0147,0150,0151,0152,0153,0154,0155,0156,0157,
0160,0161,0162,0163,0164,0165,0166,0167,0170,0171,0172,0173,0174,0175,0176,0177,
0200,0201,0202,0203,0204,0205,0206,0207,0210,0211,0212,0213,0214,0215,0216,0217,
0220,0221,0222,0223,0224,0225,0226,0227,0230,0231,0232,0233,0234,0235,0236,0237,
0240,0241,0242,0243,0244,0245,0246,0247,0250,0251,0252,0253,0254,0255,0256,0257,
0260,0261,0262,0263,0264,0265,0266,0267,0270,0271,0272,0273,0274,0275,0276,0277,
0300,0301,0302,0303,0304,0305,0306,0307,0310,0311,0312,0313,0314,0315,0316,0317,
0320,0321,0322,0323,0324,0325,0326,0327,0330,0331,0332,0333,0334,0335,0336,0337,
0340,0341,0342,0343,0344,0345,0346,0347,0350,0351,0352,0353,0354,0355,0356,0357,
0360,0361,0362,0363,0364,0365,0366,0367,0370,0371,0372,0373,0374,0375,0376,0377
};

u_char *atohex( s, b, len)
register u_char *s;		/* Byte-String zurÏeck */
register char *b;		/* ASCII-String als Eingabe */
u_int len;				/* LÌnge vom ASCII-String */
{   
    u_int i, t, u;

    for( i=0; i<len; i++) {
        t = (u_char)b[ i<<1];  u = (u_char)b[ (i<<1)+1] ;
        if( isxdigit( t) && isxdigit( u))
            s[ i] = _hexshift[ u] + (_hexshift[ t]<<4);
        else
            return( NULL);
    }
    return( s);

}       /* ENDE atohex */
