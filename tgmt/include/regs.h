typedef volatile unsigned char mreg;

typedef struct pit_reg_mod {
   mreg	pgcr,	/* port general control register */
		psrr,	/* port service request register */
		paddr,	/* port A data direction register */
		pbddr,	/* port B data direction register */
		pcddr,	/* port C data direction register */
		pivr,	/* port interrupt vector register, READ/WRITE */
		pacr,	/* port A control register */
		pbcr,	/* port B control register */
		padr,	/* port A data register, READ/WRITE */
		pbdr,	/* port B data register, READ/WRITE */
		paadr,	/* port A alternate register */
		pbadr,	/* port B alternate register */
		pcdr,	/* port C control register */
		psr,	/* port status register */
		res0,	/*  */
		res1,	/*  */
		tcr,	/* timer control register */
		tivr,	/* timer interrupt vector register, READ/WRITE, VALID: Bit 0-7 */
		res2,	/*  */
		cprh,	/* counter preload register High, WRITE, VALID: Bit 0-7 */
		cprm,	/* counter preload register Mid, WRITE, VALID: Bit 0-7 */
		cprl,	/* counter preload register Low, WRITE, VALID: Bit 0-7 */
		res3,	/*  */
		crh,	/* count register High, READ, VALID: Bit 0-7 */
		crm,	/* count register Mid, READ, VALID: Bit 0-7 */
		crl,	/* count register Low, READ, VALID: Bit 0-7 */
		tsr;	/* Timer status register, READ, VALID: Bit 0 */
} * PITPTR;

typedef struct ihan_reg_mod {
	mreg reg0, ihpr,	/* pointer register */
		 reg1, ihcr,	/* control register */
		 reg2, ihlv,	/* local interrupt request vector register */
		 reg3, ihlm,	/* local interrupt mask register */
		 reg4, ihls,	/* local interrupt status register */
		 reg5, ihbm,	/* VMEbus interrupt mask register */
		 reg6, ihbs,	/* VMEbus interrupt status register */
		 reg7, ihia;	/* last interrupt acknowledge register */
} * IHANPTR;

typedef union duart_reg_mod {
	struct read {
		mreg mra;		/* mode register A */
		mreg sra;		/* status register A */
		mreg reg2;
		mreg rhra;		/* receiver holding register A */
		mreg ipcr;
		mreg isr;
		mreg ctu;
		mreg ctl;
		mreg mrb;		/* mode register B */
		mreg srb;
		mreg reg10;
		mreg rhrb;
		mreg reg12;
		mreg ip;
		mreg start;
		mreg stop;
	} read;
	struct write {
		mreg mra;
		mreg csra;		/* clock select */
		mreg cra;		/* command register A */
		mreg thra;		/* transmit holding register A */
		mreg acr;
		mreg imr;
		mreg ctur;
		mreg ctlr;
		mreg mrb;
		mreg csrb;
		mreg crb;			/* commend register B */
		mreg thrb;
		mreg reg12;
		mreg opcr;
		mreg sopbc;
		mreg ropbc;
	} write;
} * SCNPTR;
