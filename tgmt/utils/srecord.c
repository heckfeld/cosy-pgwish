/* Conversion of GNU-File-Format into MOTOROLA-S-RECORDS */

#include <stdio.h>
#include <sys/types.h>


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

#include <sys/fcntl.h>
#include <unistd.h>
#include <stdio.h>
#define AOUTHDR struct exec
#define HP_AOUTHDR struct hp_exec
#define MAXPAGES 12
#define MAXSEGMENTS	8	/* maximum number of segments */
#define TEXT_SEGMENT	1
#define DATA_SEGMENT	2
#define BSS_SEGMENT	3

char tohex[]="0123456789ABCDEF";
int text_offset, data_offset, bss_offset;
short reccount;
char pgname[MAXPAGES];


main(argc,argv)
int argc; char *argv[]; 
{
	int	fp;
	int i; char c;

	data_offset = bss_offset = 0;
	reccount = 0; 

	if (argc == 1) 
		convert_to_srec(stdin);

	while (--argc > 0) 
	{
           argv++;
	   if ((*argv[0]) == '-' ) 
	   {
		if (--argc <= 0 ) 
			erropt(argv[0]);
		switch (*(argv[0]+1))
		{
		   case 'b' :	
			sscanf(*++argv,"%x",&bss_offset); break;
		   case 't' :	
			sscanf(*++argv,"%x",&text_offset); break;
		   case 'd' :	
			sscanf(*++argv,"%x",&data_offset); break;
		   default :	
			erropt(argv[0]); break;
		}
	   }
	   else
	   { 
		if((fp=open(*argv,O_RDONLY, 0)) < 0 ) 
		{
			fprintf(stderr,"%s: can`t open %s\n",argv[0], *argv);
			exit(1);
		}
		else 				/* save name */
		{
			for (i=0;i < MAXPAGES;i += 1) pgname[i] = '\0';
			i = 0;
			while ((c=(*(argv[0] + i++))) != 0)
				if (i < MAXPAGES) pgname[i - 1]=c;
			i = convert_to_srec(fp);
		}
		close(fp);
	   }
	}
	exit(0);
}

/*	error option exit  */

erropt(str)
	char *str;
	{
	fprintf(stderr,
		"usage: %s [-t offs] [-d offs] [-b offs] [files]\n", str);
	exit(1);
	}


/*  convert_to_srec: output a segment in Motorola S Format */


convert_to_srec(fp)
int	fp;
{
	int i;
	AOUTHDR	        hfhdr;
	HP_AOUTHDR	hp_hfhdr;
        u_int32_t   a_info;
        u_int32_t   offset;

				/* get header to see number of segments */
        
	i = read(fp, &a_info , sizeof(u_int32_t));
        SWP4(a_info);
        hfhdr.a_info = a_info;

        if (a_info == 0x020c0108)         /* Altes Format ****/
        {
	   i = read(fp, &(hp_hfhdr.a_spare1), 
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

           hfhdr.a_text = hp_hfhdr.a_text;
           hfhdr.a_data = hp_hfhdr.a_data;
           hfhdr.a_entry = hp_hfhdr.a_entry;
           hfhdr.a_trsize = hp_hfhdr.a_entry;
           hfhdr.a_drsize = hp_hfhdr.a_spare6;
           offset = sizeof(HP_AOUTHDR);
        }

        else if (a_info == 0x0000010b)
        {
	   i = read(fp, &(hfhdr.a_text) , 
                 sizeof(AOUTHDR) - sizeof(u_int32_t) );

           SWP4(hfhdr.a_text);
           SWP4(hfhdr.a_data);
           SWP4(hfhdr.a_bss);
           SWP4(hfhdr.a_syms);
           SWP4(hfhdr.a_entry);
           SWP4(hfhdr.a_trsize);
           SWP4(hfhdr.a_drsize);

           offset = sizeof(AOUTHDR);
        }
        else
        {
           fprintf(stderr, "Bad magic number\n");
           exit(1);
        } 

	header_record();
	
	if (convert_segment(fp,&hfhdr,TEXT_SEGMENT,offset))
		reccheck();
	if (convert_segment(fp,&hfhdr,DATA_SEGMENT,offset))
		reccheck();

	end_record(hfhdr.a_entry);
	return(0);
}

/* convert_segment : convert a segment into Motorola S-Format */

#define MAX_REC_BYTES  32	/* Anzahl der Datenbytes pro Zeile */

convert_segment(fp,scnptr,s_name, header_offset)
int 	fp;
AOUTHDR *scnptr;
int	s_name;
u_int32_t header_offset;
{
	unsigned char data_got[MAX_REC_BYTES] ,*data_got_ptr , checksum ;
	int get, section_end_fptr, current_fptr; 
	u_int32_t section_offset, section_size, section_beg_fptr;
	u_int32_t section_end;
	int got, i;

	switch (s_name) 
	{
	   case TEXT_SEGMENT:
		section_offset = scnptr->a_trsize + text_offset;
		section_size = scnptr->a_text;
		section_beg_fptr = header_offset;
		break;

	   case DATA_SEGMENT:
		section_offset = scnptr->a_drsize + data_offset;
		section_size = scnptr->a_data;
		section_beg_fptr = header_offset + scnptr->a_text;
		break;

	   default:
		break;
	}

	section_end = section_offset + section_size;

	if ( (section_beg_fptr == NULL) || (section_size == 0) )
		return(0);

	current_fptr = section_beg_fptr;	
       	lseek(fp, current_fptr, SEEK_SET);
	section_end_fptr = section_beg_fptr + section_size;

	do		{
		get = ((section_end_fptr - current_fptr) < MAX_REC_BYTES) ? 
		    section_end_fptr - current_fptr : 
		    MAX_REC_BYTES;

		got = 0;
                while (got < get)
		{
			i = read(fp, data_got, get - got);
			got += i;
		}

		checksum = got + 4;
		if (section_end > 0x00ffffff)
		{
		    printf("S3%2.2X%8.8X",checksum+1,section_offset);/* S3 rec */
		}
		else
		{
		    printf("S2%2.2X%6.6lX",checksum,section_offset); /* S2 rec */
		}
		data_got_ptr = data_got;
		checksum += ((section_offset & 0xff) + 
		        (section_offset >> 8 & 0xff) + 
			(section_offset >> 16 & 0xff));
		if (section_end > 0x00ffffff)
			checksum += ((section_offset >> 24) & 0xff) +1;
		while (got--) 
		{ 
			putchar (tohex[(*data_got_ptr) >> 4]); 
			putchar (tohex[(*data_got_ptr) & 0xf]);
			checksum += *data_got_ptr++; 
		}
		checksum = ~checksum;

		putchar(tohex[checksum >> 4]); 
		putchar(tohex[checksum & 0xf]);

		printf("\n");
		reccount += 1;
		section_offset = section_offset + get;
		current_fptr = current_fptr + get;
      }	while (current_fptr < section_end_fptr);


	return 1;
}

/* Output endrecord  with startaddress  */

end_record(start)
register int start;	
{
	register unsigned char chck;

	chck = (start + (start>>8&0377) + (start>>16&0377) + 4);
	if (start & 0xff000000)
		chck += ((start>>24) &0xff) +1;
	chck = ~chck;
	if (start & 0xff000000)
		printf("S705%8.8X%2.2X\n",start,chck);
	else
		printf("S804%6.6X%2.2X\n",start,chck);
}

/* output of a header record */

header_record()
{
	unsigned char chck; register int i;

	chck = 0;
/************* Was soll das ????? ************
	for (i = 0; i < 10; i++) putchar('\0');
*********************************************/

	printf("S00F0000");

	for (i=0; i < MAXPAGES; i += 1) {
		printf("%2.2X",pgname[i]);
		chck += pgname[i];
		}
	chck = ~( chck + 15 );
	printf("%2.2X",chck);
	printf("\n");
}

/* output of a check record */

reccheck()
	{
	unsigned char chck;

	chck = ~(reccount + (reccount >> 8 & 0xff) + 3);
	printf("S503%4.4X%2.2X\n",reccount,chck);
	}

