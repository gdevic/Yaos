/* HW386.H - Hardware defintions for the 80386 CPU */
/* $Id: hw386.h 1.3 89/10/06 18:38:23 rwm Exp $ */

/************************************************************************/
/*	Copyright (C) 1986-1988 Phar Lap Software, Inc.			*/
/*	Unpublished - rights reserved under the Copyright Laws of the	*/
/*	United States.  Use, duplication, or disclosure by the 		*/
/*	Government is subject to restrictions as set forth in 		*/
/*	subparagraph (c)(1)(ii) of the Rights in Technical Data and 	*/
/*	Computer Software clause at 252.227-7013.			*/
/*	Phar Lap Software, Inc., 60 Aberdeen Ave., Cambridge, MA 02138	*/
/************************************************************************/

/*

This file contains C language definitions for the Intel 80386 CPU.
The following definitions are present in the file:

	* Format of selector numbers
	* Format of segment descriptors in the LDT, GDT, and IDT
	* Format of the special 386 registers including the FLAGS,	
	  CR0, and DR7
	* Format of page table entries
	* Reserved interrupt numbers for exceptions
	* 286 and 386 TSS formats

*/


/*

Masks for various fields in segment selector values

*/

#define SEL_IDX 0xFFF8		/* index into GDT or LDT */
#define SEL_GDT	0x0000		/* GDT selector */
#define SEL_LDT	0x0004		/* LDT selector */
#define SEL_RPL	0x0003		/* requested privilege level */


/*

80386 segment descriptors

*/

typedef struct
{
	USHORT limit0_15;		/* Bits 0-15 of the segment limit */
	USHORT base0_15;		/* Bits 0-15 of the segment base */
	UCHAR base16_23;		/* Bits 16-23 of the segment base */
	UCHAR arights;			/* Access rights */
	UCHAR limit16_19;		/* Bits 16-19 of the segment limit
					   plus some flags */
	UCHAR base24_31;		/* Bits 24-31 of the segment base */
} CD_DES;

typedef struct
{
	USHORT	loffs;		/* bits 0-15 of handler offset */
	USHORT	select;		/* segment selector handler is in */
	UCHAR	wcount;		/* word count field;  unused */
	UCHAR	arights;	/* access rights byte */
	USHORT	hoffs;		/* bits 16-31 of handler offset */
} IT_DES;


/* Access rights */

#define AR_ACCESSED	0x01		/* Segment was accessed */

#define AR_CREAD	0x02		/* Code segment is readable flag */
#define AR_CCONF	0x04		/* Code segment is conforming */
#define AR_CSEG		0x08		/* Is a code segment */

#define AR_DREADO	0x00		/* Read only data segment */
#define AR_DWRITE	0x02		/* Read/write data segment */
#define AR_EUP		0x00		/* Expand up */
#define AR_EDOWN	0x04		/* Expand down */

#define AR_SYS		0x00		/* System descriptor */
#define AR_USER		0x10		/* User descriptor */

#define AR_DPLM		0x60		/* DPL mask */
#define AR_DPLSC	5		/* DPL shift count */
#define AR_DPL0		0x00		/* DPL 0 */
#define AR_DPL1		0x20		/* DPL 1 */
#define AR_DPL2		0x40		/* DPL 2 */
#define AR_DPL3		0x60		/* DPL 3 */

#define AR_PRESENT	0x80		/* Segment is present */

#define AR_CODE	(AR_PRESENT | AR_USER | AR_CSEG | AR_CREAD)
#define AR_DATA	(AR_PRESENT | AR_USER | AR_DWRITE)

/* Flags (limit16_19) */

#define SG_BYTE	 0x00			/* Byte segment granularity */
#define SG_PAGE	 0x80			/* Page segment granularity */

#define DOS_16   0x00			/* Default operand size is 16 */
#define DOS_32   0x40			/* Default operand size is 32 */

#define D6_HLIM	 0x0F			/* bits 16-19 of limit */


/*

System segment descriptor types

*/

#define SSDT_A286TSS	1		/* Available 286 TSS */
#define	SSDT_LDT	2		/* LDT */
#define SSDT_B286TSS	3		/* Busy 286 TSS */
#define	SSDT_CG286	4		/* 286 call gate */
#define SSDT_TG		5		/* Task gate */
#define SSDT_IG286	6		/* 286 interrupt gate */
#define SSDT_XG286	7		/* 286 trap gate */

#define SSDT_A386TSS	9		/* Available 386 TSS */
#define SSDT_B386TSS	11		/* Busy 386 TSS */
#define	SSDT_CG386	12		/* 386 call gate */
#define SSDT_IG386	14		/* 386 interrupt gate */
#define SSDT_XG386	15		/* 386 trap gate */

#define SSDT_MASK	0x1F		/* Mask for segment/gate type */
#define SSDT_TSSBSY	0x02		/* TSS is busy */

#define SSDP_MASK	0x60		/* Mask for DPL */
#define SSDP_SHFT	0x05		/* Shift left 5 bits */

/*

EFLAGS register definitions

*/

#define EF_AC	0x00040000L		/* alignment check flag, 486 only */
#define EF_VM	0x00020000L		/* Virtual 8086 mode */
#define EF_RF	0x00010000L		/* Resume flag */
#define EF_NT 	0x00004000L		/* Nested task */
#define EF_IOPL	0x00003000L		/* I/O privilege mask */
#define EF_SC_IOPL 12			/* # bits to shift IOPL */
#define EF_OF	0x00000800L		/* Overflow flag */
#define EF_DF	0x00000400L		/* Direction flag */
#define EF_IF	0x00000200L		/* Interrupt enable flag */
#define EF_TF	0x00000100L		/* Trace flag */
#define EF_SF	0x00000080L		/* Sign flag */
#define EF_ZF	0x00000040L		/* Zero flag */
#define EF_AF	0x00000010L		/* Auxilary carry flag */
#define EF_PF	0x00000004L		/* Parity flag */
#define EF_CF	0x00000001L		/* Carry flag */
#define EF_1BITS 0x00000002L		/* Bits always set to 1 in EFLAGS */
#define EF_DEFINED	0x00037FD7L	/* mask for all defined bits on 386 */
#define EF_486DEFINED	0x00077FD7L	/* mask for all defined bits on 486 */

/*

Control register 0 (CR0) definitions

*/

#define CR0_PG	0x80000000L		/* Paging enabled */
#define CR0_CD	0x40000000L		/* cache disable, 486 only */
#define CR0_NW	0x20000000L		/* not write-through, 486 only */
#define CR0_AM	0x00040000L		/* alignment check enable, 486 only */
#define CR0_WP	0x00010000L	/* write protect at priv lev 0-2, 486 only */
#define CR0_NE	0x00000020L	/* numerics exception enable, 486 only */
#define CR0_ET  0x00000010L		/* 287 vs 387 flag */
#define CR0_TS  0x00000008L		/* Task switched */
#define CR0_EM  0x00000004L		/* Emulate coprocessor */
#define CR0_MP	0x00000002L		/* Monitor coprocessor */
#define CR0_PE	0x00000001L		/* Protected mode */
#define CR0_DEFINED 0x8000001FL		/* mask for all defined bits on 386 */
#define CR0_486DEFINED 0xE005003FL	/* mask for all defined bits on 486 */


/*
Control register 3 (CR3) definitions
*/
#define CR3_PCD		0x00000010L	/* page cache disable, 486 only */
#define CR3_PWT		0x00000008L	/* page write-thru, 486 only */
#define CR3_DEFINED	0xFFFFF000L	/* mask for all defined bits on 386 */
#define CR3_486DEFINED	0xFFFFF018L	/* mask for all defined bits on 486 */


/*

Debug register 6 definitions

*/

#define DR6_B0 0x00000001L		/* Breakpoint 0 */
#define DR6_B1 0x00000002L		/* Breakpoint 1 */
#define DR6_B2 0x00000004L		/* Breakpoint 2 */
#define DR6_B3 0x00000008L		/* Breakpoint 3 */
#define DR6_BD 0x00002000L		/* Debug register accessed */
#define DR6_BS 0x00004000L		/* Single step */
#define DR6_BT 0x00008000L		/* Task switch */
#define DR6_DEFINED 0x0000E00FL		/* mask for all defined bits */


#define DR6_BSHIFT(n)	(n)		/* Number of bits to shift to get Bn */


/*

Debug register 7 definitions

*/

#define DR7_LE 0x00000100L		/* Enable all local breakpoints */
#define DR7_GE 0x00000200L		/* Enable all global breakpoints */

#define DR7_GLMASK 0x00000003L		/* Mask for G and L bits for a bkpt */
#define DR7_GLSHIFT(n) ((n) * 2)	/* Number of bits to shift to get */
					/* G and L bits for bkpt Bn */
#define DR7_GEMASK 0x00000002L		/* global enable (G bit) mask */
#define DR7_LEMASK 0x00000001L		/* local enable (L bit) mask */

#define DR7_RWLMASK 0x0000000FL		/*Mask for R/W and LEN bits for a bkpt*/
#define DR7_RWLSHIFT(n) (16 + (n) * 4)	/* Number of bits to shift to get */
					/* R/W/LEN bits for bkpt Bn */
#define DR7_RWMASK 0x00000003L		/* R/W bits mask */
#define DR7_BINST 0x00000000L		/* R/W specifies break on instr exec */
#define DR7_BWR	0x00000001L		/* R/W specifies break on data write */
#define DR7_BRW	0x00000003L		/* R/W specifies break on data read */
					/* or write */
#define DR7_LMASK 0x0000000CL		/* LEN bits mask */
#define DR7_L1	0x00000000L		/* LEN specifies 1-byte length */
#define DR7_L2	0x00000004L		/* LEN specifies 2-byte length */
#define DR7_L4	0x0000000CL		/* LEN specifies 4-byte length */

#define DR7_GD	0x00002000L	/* global debug register access detect */

#define DR7_DEFINED 0xFFFF23FFL		/* mask for all defined bits  */

/*
Test registers (TR3, TR4, TR5 are 486 only.  TR6 and TR7 are 386 and 486)
*/
#define TR4_DEFINED	0xFFFFFFF8L	/* mask for all defined bits in TR4 */

#define TR5_DEFINED	0x000007FFL	/* mask for all defined bits in TR5 */

#define TR6_DEFINED	0xFFFFFFE3L	/* mask for all defined bits in TR6 */

#define TR7_DEFINED	0xFFFFF01CL	/* mask for all defined bits on 386 */
#define TR7_486DEFINED	0xFFFFFF9CL	/* mask for all defined bits on 486 */


/*

Bit definitions for a page table entry

*/

#define PE_PFA		0xFFFFF000L	/* page frame address */
#define PE_DIRTY	0x00000040L	/* page dirty */
#define PE_ACCESSED	0x00000020L	/* page accessed */
#define PE_PCD		0x00000010L	/* page cache disable -- 486 only */
#define PE_PWT		0x00000008L	/* page write through -- 486 only */
#define PE_USER		0x00000004L	/* page can be accessed by user */
					/* (privilege level 3) */
#define PE_WRITE	0x00000002L	/* page can be written */
#define PE_PRESENT	0x00000001L	/* page is present in memory */
#define PE_STDPROT	(PE_PRESENT | PE_WRITE | PE_USER) /* std prot bits */
#define PE_DEFINED 0xFFFFFE67L	/* defined bits in a page table entry on 386 */
#define PE_486DEFINED 0xFFFFFE7FL /* defined bits in a pg tbl entry on 486 */


/*

Masks for fields in a linear address 
 
*/

#define LA_PDE		0xFFC00000L	/* page directory entry index */
#define LA_PTE		0x003FF000L	/* page table entry index */
#define LA_POFFS	0x00000FFFL	/* byte offset in page */
#define PDE_SHIFT	22	/* # bits to shift a page directory index */
#define PTE_SHIFT	12	/* # bits to shift a page table index */


/*

Page table constants

*/

#define PAGE_SIZE	0x1000		/* page size, in bytes */
#define PAGE_SHIFT	12		/* # bits to shift a page number */
#define NPARA_PAGE	0x100		/* number of paragraphs in a page */


/*

Field definitions for error code returned by some interrupts.

*/

#define IE_SELX		0x0000FFF8L	/* segment selector index */
#define IE_TI		0x00000004L	/* table indicator: 0 => GDT, 1 => LDT*/
#define IE_IDT		0x00000002L	/* selector is in IDT (TI bit ignored */
#define IE_EXT		0x00000001L	/* an external event caused interrupt */


/*

Reserved interrupt numbers

*/

#define INT_DIV		0		/* Divide by zero */
#define INT_DBG		1		/* Debug */
#define	INT_NMI		2		/* Non-maskable interrupt */
#define INT_INT3	3		/* One-byte interrupt (INT3) */
#define	INT_INTO	4		/* Interrupt on overflow */
#define	INT_BND		5		/* BOUND interrupt */
#define INT_ILL		6		/* Illegal instruction */
#define INT_DNA		7		/* Device not available */
#define INT_DBLF	8		/* Double fault */
#define INT_CPSEG	9		/* Coprocessor segment overrun */
#define	INT_ITSS	10		/* Invalid TSS */
#define INT_SNP		11		/* Segment not present */
#define INT_STKF	12		/* Stack fault */
#define INT_PROTF	13		/* Protection fault */
#define INT_PAGEF	14		/* Page fault */
#define INT_CPE		16		/* Coprocessor error */


/*

TSS for the 286

*/

typedef struct
{
	USHORT tss_backl;	/* Back link selector to TSS */
	USHORT tss_sp0;		/* SP for CPL 0 */
	USHORT tss_ss0;		/* SS for CPL 0 */
	USHORT tss_sp1;		/* SP for CPL 1 */
	USHORT tss_ss1;		/* SS for CPL 1 */
	USHORT tss_sp2;		/* SP for CPL 2 */
	USHORT tss_ss2;		/* SS for CPL 2 */
	USHORT tss_ip;		/* IP */
	USHORT tss_flags;	/* Flags */
	USHORT tss_ax;		/* AX */
	USHORT tss_cx;		/* CX */
	USHORT tss_dx;		/* DX */
	USHORT tss_bx;		/* BX */
	USHORT tss_sp;		/* SP */
	USHORT tss_bp;		/* BP */
	USHORT tss_si;		/* SI */
	USHORT tss_di;		/* DI */
	USHORT tss_es;		/* ES */
	USHORT tss_cs;		/* CS */
	USHORT tss_ss;		/* SS */
	USHORT tss_ds;		/* DS */
	USHORT tss_ldtr;	/* LDTR */
} TSS286;


/*

TSS for the 386

*/

typedef struct
{
	USHORT tss_backl;	/* Back link selector to TSS */
	USHORT tss_fl1;		/* Filler */
	ULONG tss_esp0;		/* SP for CPL 0 */
	USHORT tss_ss0;		/* SS for CPL 0 */
	USHORT tss_fl2;		/* Filler */
	ULONG tss_esp1;		/* SP for CPL 1 */
	USHORT tss_ss1;		/* SS for CPL 1 */
	USHORT tss_fl3;		/* Filler */
	ULONG tss_esp2;		/* SP for CPL 2 */
	USHORT tss_ss2;		/* SS for CPL 2 */
	USHORT tss_fl4;		/* Filler */
	ULONG tss_cr3;		/* CR3 */
	ULONG tss_eip;		/* EIP */
	ULONG tss_eflags;	/* Extended flags */
	ULONG tss_eax;		/* EAX */
	ULONG tss_ecx;		/* ECX */
	ULONG tss_edx;		/* EDX */
	ULONG tss_ebx;		/* EBX */
	ULONG tss_esp;		/* ESP */
	ULONG tss_ebp;		/* EBP */
	ULONG tss_esi;		/* ESI */
	ULONG tss_edi;		/* EDI */
	USHORT tss_es;		/* ES */
	USHORT tss_fl5;		/* Filler */
	USHORT tss_cs;		/* CS */
	USHORT tss_fl6;		/* Filler */
	USHORT tss_ss;		/* SS */
	USHORT tss_fl7;		/* Filler */
	USHORT tss_ds;		/* DS */
	USHORT tss_fl8;		/* Filler */
	USHORT tss_fs;		/* FS */
	USHORT tss_fl9;		/* Filler */
	USHORT tss_gs;		/* GS */
	USHORT tss_fl10;	/* Filler */
	USHORT tss_ldtr;	/* LDTR */
	USHORT tss_fl11;	/* Filler */
	USHORT tss_dflags;	/* Debugger flags */
	USHORT tss_iomapb;	/* I/O map base */
} TSS386;

#define TSS_TRAPF 0x0001	/* Debug trap flag in TSS */

#define TSS286_SIZE 0x2C	/* Size of a 286 TSS */
#define TSS386_SIZE 0x68	/* Size of a 386 TSS */
