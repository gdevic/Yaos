/* LOAD.H - Definitions for LOAD.C */
/*********************************************************************/
/*	Copyright (C) 1986-1989 Phar Lap Software, Inc.                  */
/*	Unpublished - rights reserved under the Copyright Laws of the    */
/*	United States.  Use, duplication, or disclosure by the           */
/*	Government is subject to restrictions as set forth in            */
/*	subparagraph (c)(1)(ii) of the Rights in Technical Data and      */
/*	Computer Software clause at 252.227-7013.                        */
/*	Phar Lap Software, Inc., 60 Aberdeen Ave., Cambridge, MA 02138   */
/*********************************************************************/
/*		REVISION LOG

Rev. 1.00, ALS, 31-Mar-89	- File creation date.
*/

#define	BLK_SIZE	512	/* size of disk block */
#define	ABS_SEEK	0  /* seek from beginning of file command */
#define  REL32	0x80000000		/* if bit set: relocate 32 bit value
					   if reset  : relocate 16 bit value */

/* Compute number of pages needed to fit "val" bytes of memory */

#define	MIN_PAGES(val) (val ? (( (val-1) >> PAGE_SHIFT) +1):0)

/*
Error Types
*/

#define NO_ERR		0		/* no errors detected */

#define OPN_ERR	0x10000		/* FILE OPEN ERRORS */
#define IFSC_ERR	1		/* invalid file sharing code */
#define FNF_ERR	2 		/* file not found */
#define PNF_ERR	3 		/* path not found */
#define TMOF_ERR	4 		/* too many open files */
#define ACCD_ERR	5 		/* access denied */
#define IFAC_ERR	12		/* invalid file access code */

#define RD_ERR		0x20000		/* READ ERRORS */
#define BAD_LEN	1		/* didn't read number of bytes requested */
#define INVH_ERR	6		/* invalid file handler */

#define HDR_ERR	0x30000		/* FILE HEADER ERRORS */
#define BAD_HDR	1 		/* bad/missing file header */
#define BAD_RTP	2 		/* bad/missing run time parameter block */
#define NZ_RBR		3 		/* non-zero realbreak parameter */
#define BAD_OFF	4 		/* offset parameter not multiple of 4K */

#define SK_ERR		0x40000		/* SEEK ERRORS */
#define INVO_ERR	1 		/* invalid origin */

#define CLS_ERR	0x50000		/* CLOSE FILE ERROR */

#define MEM_ERR	0x60000		/* MEMORY-RELATED ERRORS */
#define AS_MEM		1     		/* insuf mem to allocate segments */
#define MP_MEM		2		/* insuf mem to map physical memory */
#define LP_MEM		3		/* insuf mem to load prog and/or mindata */
#define LDT_ERR	8		/* couldn't create LDT entry */
#define BAD_SEL	9		/* invalid segment selector */
#define LD_PROG	10		/* error when reading load image */
#define FRE_SEG	11 		/* couldn't free segments */
						    
/*
	Register Structure
*/

typedef struct
{
	ULONG eax;
	ULONG ebx;
	ULONG ecx;
	ULONG edx;
	ULONG esi;
	ULONG edi;
	ULONG ebp;
	ULONG esp;
	USHORT cs;
	USHORT ds;
	USHORT es;
	USHORT fs;
	USHORT gs;
	USHORT ss;
	ULONG eip;
	ULONG eflags;
} REGS;

/******* EXTERNAL SUBROUTINES *******/

extern
int	intr(ULONG num,REGS *regstp);

extern
int	far_call(REGS *regst);

/******** TYPEDEFS *******/

typedef int (*FUNCP)();			/* FUNCP is a pointer to a function */


