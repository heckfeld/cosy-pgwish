#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

#if 0
#include "bfd.h"
#include "dis-asm.h"
#define       BYTES_IN_WORD   4
#include "aout64.h"
#endif

#define CORE_ADDR unsigned int

#ifdef linux
#define SWP4(X)                                 \
    { u_int32_t tmp;                        \
      tmp = (((X) & 0xff ) << 24) |             \
            (((X) & 0xff00 ) << 8) |            \
            (((X) & 0xff0000 ) >> 8) |          \
            (((X) & 0xff000000 ) >> 24) ;       \
      X = tmp;                                  \
    }
#define SWP2(X)                                 \
    { unsigned short tmp;                       \
      tmp = (((X) & 0xff ) << 8) |              \
            (((X) & 0xff00 ) >> 8);             \
      X = tmp;                                  \
    }
#endif

#ifdef hpux
#define SWP4(X) X
#define SWP2(X) X
#endif


#if __STDC__ == 1
#define COERCE_SIGNED_CHAR(ch) ((signed char)(ch))
#else
#define COERCE_SIGNED_CHAR(ch) ((int)(((ch) ^ 0x80) & 0xFF) - 128)
#endif

// #include "m68k.h"

#define MAXLEN	24
#define f_addr		(off_t)(semiadr + sizeof(hfhdr) - semiadr_start)

static char oiopr[1];		/* <-sdis: operand string		*/
static u_int32_t semiadr;		/* ->sdis: addr. of operands	*/
static u_int32_t semiadr_start;
static u_int32_t semiadrn;		/* <-sdis: addr. of foll. operands */
static char seminstr[MAXLEN];		/* ->sdis: instunctions		*/

int print_address();

int	fileptr;
struct exec
{
  u_int32_t a_info;
  u_int32_t a_text; /* length of text section in bytes    */
  u_int32_t a_data; /* length of data section in bytes    */
  u_int32_t a_bss; /* length of bss area in bytes */
  u_int32_t a_syms; /* length of symbol table in bytes    */
  u_int32_t a_entry; /* start address                     */
  u_int32_t a_trsize; /* length of text relocation info   */
  u_int32_t a_drsize; /* length of data relocation info   */
};
struct hp_exec  {
    unsigned short a_machtype;  /* machine type */
    unsigned short a_magic;     /* magic number */
    u_int32_t a_spare1;
    u_int32_t a_spare2;
    u_int32_t a_text;       /* size of text segment */
    u_int32_t a_data;       /* size of data segment */
    u_int32_t a_bss;        /* size of bss segment */
    u_int32_t a_trsize;     /* text relocation size */
    u_int32_t a_drsize;     /* data relocation size */
    u_int32_t a_spare3;     /* HP = pascal interface size */
    u_int32_t a_spare4;     /* HP = symbol table size */
    u_int32_t a_spare5;     /* HP = debug name table size */
    u_int32_t a_entry;      /* entry point */
    u_int32_t a_spare6;     /* HP = source line table size */
    u_int32_t a_spare7;     /* HP = value table size */
    u_int32_t a_syms;       /* symbol table size */
    u_int32_t a_spare8;
  };

#define AOUTHDR struct exec
#define HP_AOUTHDR struct hp_exec

struct exec hfhdr;
struct hp_exec hp_hfhdr;

#ifndef N_MAGIC
#define N_MAGIC(exec) ((exec) . a_info & 0xffff)
#endif
int	machine = 20; 			/* default MC68020 */

main(argc,argv)
int	argc;
char	*argv[];
{	char	*atoper;
	u_int32_t	dum1, *addr = &dum1;
	unsigned short	*atadr;
        unsigned char *catadr;
	register i, lines;
	register xlng;
	char xl[200];
	char out[200];
	int	m, j, k;
	u_int32_t end_ptr = 0;
	int	diff;
	unsigned char buff[1000];
	u_int32_t   a_info;

	int	c;
	extern char *optarg; 
	extern int optind;
	char	*start_symbol = (char*) 0;
	u_int32_t	start_address = 0x0;
	char	*dis_file;
	

	if (argc < 2)
        {
                printf("Usage: %s filename \n", argv[0]);
                exit (1);
        }


	fileptr = open(argv[1],O_RDONLY );
	if (fileptr == -1)
	{
		fprintf(stderr, 
		        "%s: Could not open %s \n", argv[0], argv[optind]);
		exit (1);
	}

        i = read (fileptr, &a_info , sizeof(u_int32_t));
        SWP4(a_info);
        hfhdr.a_info = a_info;
                if (a_info == 0x020c0108)         /* Altes Format ****/
        {
           i = read(fileptr, &(hp_hfhdr.a_spare1),
                    sizeof(HP_AOUTHDR) - sizeof(u_int32_t) );

           SWP4(hp_hfhdr.a_spare1);
           SWP4(hp_hfhdr.a_spare2);
           SWP4(hp_hfhdr.a_text);
           SWP4(hp_hfhdr.a_data);
           SWP4(hp_hfhdr.a_bss);                              
           SWP4(hp_hfhdr.a_trsize);                           
           SWP4(hp_hfhdr.a_drsize);                           
           SWP4(hp_hfhdr.a_spare4);                           
           SWP4(hp_hfhdr.a_spare5);                           
           SWP4(hp_hfhdr.a_entry);                            
           SWP4(hp_hfhdr.a_spare6);                           
           SWP4(hp_hfhdr.a_spare7);                           
           SWP4(hp_hfhdr.a_syms);                             
           SWP4(hp_hfhdr.a_spare8);                           
 
           hfhdr.a_entry = hp_hfhdr.a_entry;
        }
        else if (a_info == 0x0000010b)
        {
           i = read(fileptr, &(hfhdr.a_text) ,
                 sizeof(AOUTHDR) - sizeof(u_int32_t) );
                                                             
           SWP4(hfhdr.a_text);                                
           SWP4(hfhdr.a_data);                                
           SWP4(hfhdr.a_bss);                                 
           SWP4(hfhdr.a_syms);                                
           SWP4(hfhdr.a_entry);                               
           SWP4(hfhdr.a_trsize);                              
           SWP4(hfhdr.a_drsize);                              
        }
        else
        {
           fprintf(stderr, "Bad magic number\n");
           exit(1);
        }

        printf ("%8.8x T start\n",hfhdr.a_entry);
	exit (0);
}

