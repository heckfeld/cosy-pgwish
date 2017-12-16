/* %Z%%M%	%I% (mike) %H% */

#ifndef HARDWARE_INCLUDED
#    define HARDWARE_INCLUDED

/*
 * Macro Definitionen fuer Hardwareabhaengige Adressen auf den Targets.
 * Diese sind :
 *
 * 	IBAM, E5, CPU30, CPU37
 *
 * Entsprechend ist beim Aufruf des Compilers mit der -D Option das
 * Target auszuwaehlen.
 */

#define OSTG_IBAM		1
#define OSTG_E5			2
#define OSTG_CPU30		3
#define OSTG_CPU37		4
#define OSTG_CPU33		5

#define MFPBASE   0x380000
#define LANCEIBAM 0x510000

#if defined(IBAM)		/* ELTEC IBAM */

#   define BOARDTYPE OSTG_IBAM
#   define KERNELBASE 0x30000
#   define EPROMBASE 0x200000
#   define DUARTBASE 0x380000	/* MFP 68901 */
#   define DISPLAY(disp) (*(char *)0x300001 = (disp))
#   define IHANBASE  "ERROR: no IHAN on IBAM"
#   define IGENBASE  0x340001	/* IGEN 68154 */
#   define PITBASE   MFPBASE

#endif

#define SCNBASE 0xFEC00000	/* SCN 2681 Dual Port */
#define LANCEE5 0xFE401300

#if defined(E5)			/* ELTEC EUROCOM 5 */

#   define BOARDTYPE OSTG_E5
#   define KERNELBASE 0xA0000
#   define EPROMBASE 0xFE800000
#   define DUARTBASE 0xFEC00000	/* SCN 2681 Dual Port */
#   define DISPLAY(disp) (*(char *)0xFEC04000 = (disp) << 4)
#   define IHANBASE  0xFEC01000
#   define PITBASE   0xFEC02000

#endif

#define DUSCCBASE 0xFF802000
#define PIT1BASE   0xFF800C00	/* PI/T 1 */
#define PIT2BASE   0xFF800E00	/* PI/T 2 */
#define LANCECPU30 0xFEF80000

#if defined(CPU30)		/* FORCE CPU 30 */

#   define BOARDTYPE OSTG_CPU30
#   define KERNELBASE 0xA0000
#   define EPROMBASE 0xFF000000
#   define DUARTBASE 0xFF802000
#   define DISPLAY(N)
#   define FGABASE   0xFFD00000

#endif

#define MFP37BASE 0xFF805000
#define LANCECPU37 0xFF804000

#if defined(CPU37)		/* FORCE CPU 37 */

#   define BOARDTYPE OSTG_CPU37
#   define EPROMBASE 0xFF000000
#   define DISPLAY(N)
#   define IHANBASE  0xFF805800

#endif


#define	DATA_PORT_OFFSET	0x0
#define VECTOR_ADR_OFFSET	0x101
#define ETHER_ADR_OFFSET	0x1D81

#if defined( IBAM)
#   define TIMER_INTR_PRIORITY	5
#endif
#if defined(E5)
#   define TIMER_INTR_PRIORITY	6
#endif
#if defined( CPU30)
#   define TIMER_INTR_PRIORITY	2		/* LOCAL02 on FGA-002 */
#endif
#if defined( CPU37)
#   define TIMER_INTR_PRIORITY	5
#endif

#if defined( IBAM) || defined(E5)
#   define LANCE_INTR_PRIORITY	5
#endif
#if defined( CPU30)
#   define LANCE_INTR_PRIORITY	6		/* LOCAL06 on FGA-002 */
#endif
#if defined( CPU37)
#   define LANCE_INTR_PRIORITY	2
#endif

#if defined( IBAM)
#   define DUART_INTR_PRIORITY	4
#endif
#if defined( E5)
#   define DUART_INTR_PRIORITY	6
#endif
#if defined( CPU30)
#   define DUART_INTR_PRIORITY	4		/* LOCAL04 on FGA-002 */
#endif
#if defined( CPU37)
#   define DUART_INTR_PRIORITY	3
#endif

#endif /* HARDWARE_INCLUDED */
