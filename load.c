/* LOAD.C - Sample Flat-Model .EXP and .REX Loader */
/*********************************************************************/
/*	Copyright (C) 1986-1989 Phar Lap Software, Inc.               */
/*	Unpublished - rights reserved under the Copyright Laws of the */
/*	United States.  Use, duplication, or disclosure by the        */
/*	Government is subject to restrictions as set forth in         */
/*	subparagraph (c)(1)(ii) of the Rights in Technical Data and   */
/*	Computer Software clause at 252.227-7013.                     */
/*	Phar Lap Software, Inc., 60 Aberdeen Ave., Cambridge, MA 02138*/
/*********************************************************************/
/*
		DESCRIPTION

		LOAD.C contains a simple flat-model .EXP and .REX file 
	loader. It can be linked into another program to enable that
	program to load another, execute it and cleanup after it.
		The loader has three entry points and below is a brief 
	description of each of them (more detailed descriptions can be 
	found further down in this file below the function declarations 
	of each entry point) :

	load :	This entry point is used to load a file into memory and
		to set things up to prepare for execution.

	execute: This entry point is used to transfer control to the loaded
		routine. Note: The loaded routine must not terminate by
		issuing a INT 21H function 4ch call for this will terminate
		the parent program also. For control to be returned to the
		loader correctly, the loaded program must terminate with
		a "far return" if it is a .EXP file and a "near return" if
		it is a .REX file. For more information, please see the
		file HELLO.ASM which is provided in this diskette.

	unload: This entry point is used to free the memory that was 
		allocated to the program after it has finished executing.
		Calling 'unload' will also close the file containing the
		loaded program.
*/
	

/*		REVISION LOG

Rev. 1.00, ALS, 31-Mar-89	- File creation date.
*/

/******* INCLUDE FILES *******/

#include <stdio.h>
//#include "pltypes.h"

typedef unsigned short int USHORT;
typedef unsigned long int ULONG;
typedef unsigned char UCHAR;
typedef char BOOL;
typedef long int LONG;

#define TRUE 1
#define FALSE 0

#include "hw386.h"
#include "plexp.h"
#include "load.h"

/******* ERROR MESSAGE CONTROL *******/

#define DEBUG 0			/* if 1 then DEBUG mode on
				   		if 0 then DEBUG mode off */
BOOL	ld_err_msg;	/* Load Error Msg Flag: If TRUE, print error msgs */

/******* START OF CODE *******/

/* ULONG	ERROR (ERRNUM):

		This is a general purpose error routine that informs the user
	that an error occurred. The type of error is passed to this routine
	in ERRNUM.  The high 16 bits are the general error category and the
	low 16 bits give more specific information about the error.

*/

static ULONG	error(errnum)

ULONG	errnum;		/* Error number */

{
ULONG	gen_err;
ULONG	spec_err;

      gen_err=(errnum & 0xffff0000);
      spec_err=(errnum & 0xffff);

      switch(gen_err)
      {
      case OPN_ERR:
	if (ld_err_msg)
	{		
 		printf("OPEN ERROR: ");
 		switch (spec_err)
 		{
 			case IFSC_ERR: 
				printf("Invalid file sharing code.\n"); break;
 			case FNF_ERR: 
				printf("File not found.\n"); break;
 			case PNF_ERR: 
				printf("Path not found.\n"); break;
 			case TMOF_ERR: 
				printf("Too many open files.\n"); break;
 			case ACCD_ERR: 
				printf("Access denied.\n"); break;
 			case IFAC_ERR: 
				printf("Invalid file access code.\n"); break;
 			default: printf("Unknown cause.\n");break;
 		}
	}
	break;
      case RD_ERR:
	if (ld_err_msg)
	{		
 		printf("READ ERROR: ");
 		switch (spec_err)
 		{
			case BAD_LEN:
				printf("Could not read requested number of bytes.\n");
				break;
 			case ACCD_ERR: 
				printf("Access denied.\n");
				break;
 			case INVH_ERR: 
				printf("Invalid file handle.\n");
				break;
 			default: printf("Unknown cause.\n");break;
 		}
	}
	break;
      case HDR_ERR:
	if (ld_err_msg)
	{		
 		printf("HEADER ERROR: ");
 		switch (spec_err)
 		{
			case BAD_HDR:
				printf("Header of .EXP file is of incorrect length.\n");
				break;
			case BAD_RTP:
				printf("Could not read run-time parameter block.\n");
				break;
			case	NZ_RBR:
				printf("Realbreak switch not supported.\n");
				break;
			case	BAD_OFF:
				printf("Offset parameter must be a multiple of 4K.\n");
				break;
  			default: printf("Unknown cause.\n");break;
 		}
	}
	break;
      case SK_ERR:
	if (ld_err_msg)
	{		
 		printf("SEEK ERROR: ");
 		switch (spec_err)
 		{
			case INVO_ERR:printf("Invalid Origin.\n");break;
			case INVH_ERR:printf("Invalid Handle.\n");break;
 			default: printf("Unknown cause.\n");break;
 		}
	}
	break;
      case CLS_ERR:
	if (ld_err_msg)
	{		
 		printf("CLOSE FILE ERROR: ");
 		switch (spec_err)
 		{
			case INVH_ERR:printf("Invalid Handle.\n");break;
 			default: printf("Unknown cause.\n");break;
 		}
	}
	break;
      case MEM_ERR:
	if (ld_err_msg)
	{		
 		printf("MEM ERROR: ");
 		switch (spec_err)
 		{
			case AS_MEM:
				printf("Insufficient memory to allocate segments.\n");
				break;
			case MP_MEM:
				printf("Insufficient memory to map physical memory.\n");
				break;
			case LP_MEM:
				printf("Insufficient memory to load program and/or extra data.\n");
				break;
			case LDT_ERR:
				printf("Couldn't create LDT entry.\n");
				break;
			case BAD_SEL:
				printf("Invalid segment selector.\n");
				break;
			case LD_PROG:
				printf("Error when reading load image.\n");
				break;
			case FRE_SEG:
				printf("Couldn't free segments.\n");
				break;
 			default: printf("Unknown cause.\n");break;
 		}
	}
	break;
      default: printf("ERROR: Unknown Cause.\n");
      } /* switch */

      return errnum;
}


/* INT	LOAD (FNAMP,REGSTP,MSG,REXPP,HANDP):

		This routine loads a file into memory and sets things up
	to prepare for execution. It's inputs: FNAMP, a pointer
	to a zero-terminated string containing the name of the
	file to be loaded (path accepted) and REGSTP, a pointer to a 
	register structure which will contain the initial register values
	required for the execution of the loaded program. HANDP is a pointer
	to a ULONG which will contain the file handle of the loaded program
	if LOAD terminates successfully. Its initial value is not important.
	MSG is a flag that controls the display of error messages. If it is 
	TRUE, error messages will be displayed; if FALSE, only the return code
	of this	function will indicate that an error occurred (error messages 
	will not be displayed). REXPP is a pointer to a pointer that will be 
	used to "remember" what type of file we loaded - it's initial value is 
	irrelevant, but it ,*REGSTP and *HANDP must be kept intact to enable 
	files to be closed properly and allocated memory to be freed by 
	"unload". In the case of a .REX file, *REXPP will be a near pointer 
	to where the file got loaded. For a .EXP file, it will be NULL.
	
*/

int 	load(fnamp,regstp,msg,rexpp,handp)

char	*fnamp;
REGS	*regstp;
BOOL	msg;
char	**rexpp;
ULONG	*handp;

{
int		retcode;
OEXP_HDR	ohdrs;			/* Old .EXP header */
EXP_HDR	hdrs;			/* New .EXP header */

	ld_err_msg = msg;	/* set error message control flag */

	*rexpp=NULL; 		/* assume we won't be loading a .REX file */

	#if DEBUG
		printf("Opening file ...\n");
	#endif
     	if ((retcode=open(fnamp,0,handp)))
			return retcode;
	#if DEBUG
		printf("Reading header ...\n");
	#endif
     	if ((retcode=read(*handp,&ohdrs,sizeof(OEXP_HDR),0)))
		return retcode;
	#if DEBUG
		printf("File Signature: %c%c\n",
				(ohdrs.exe_sign & 0xff),
				(ohdrs.exe_sign & 0xff00) >> 8);
	#endif

	memset(&hdrs,0,sizeof(EXP_HDR));  /* fill new header with zeros */

	if ((ohdrs.exe_sign == EXP_OLD)||(ohdrs.exe_sign == EXP_386))
		retcode=load_flat(*handp,&hdrs,&ohdrs,regstp);
     	else	if (ohdrs.exe_sign == REX_OLD)
		retcode=load_rex(*handp,&hdrs,&ohdrs,regstp,rexpp);
	else
	{
		printf("ERROR: Cannot load this type of file.\n");
		return TRUE;
	}

	return retcode;
}

/* EXECUTE (REGSTP,REXP):

		This routine is used to transfer control to the loaded 
	program. It takes as input REGSTP, a pointer to a register 
	structure containing the desired initial register values and
	REXP, a pointer that was updated by "load" indicating how to 
	transfer control to this file. 

	Note: In the case of .EXP files, it is extremely important that
		this register structure be not the same as the one returned
		by "load" for it will be updated by the external "far_call"
		used to call the loaded program. This can cause the file to
		be impossible to "unload", for "unload" depends on the contents
		of the register structure to know how to free the allocated
		memory. The reccommended procedure is to use a pointer to a 
		copy of the structure instead of to the structure itself. In
		this way the original structure can be used later to free
		the allocated memory.
*/

int	execute(regstp,rexp)

REGS	*regstp;
char	*rexp;
{
ULONG	temp;	       	/* temp var to appease compiler */

	#if DEBUG
		fflush(stdout);
		printf("* TRANSFERRING CONTROL TO LOADED PROGRAM *\n\n");
	#endif

	if (rexp == NULL)		/* .EXP file */
		far_call(regstp);
	else
	{
		temp = (ULONG)rexp;
		(*((FUNCP) temp))();
	}
		
	#if DEBUG
		printf("* CONTROL RETURNED FROM PROGRAM *\n");
	#endif

}		

/* INT	UNLOAD (REGSTP,REXP,HANDLE) :

		This routine "unloads" a program that was previously
	loaded with the LOAD function. For it to work, its parameters, 
	REGSTP and REXP, must be exactly as returned by the LOAD function.
	The parameter HANDLE must be the handle to the file that was 
	loader so that the file can be closed.

*/

int	unload (regstp,rexp,handle)

REGS	*regstp;
char	*rexp;
ULONG	handle;

{
int	retcode;
/*
	Before we can start freeing the memory that was allocated
	to load the program, we must find out how we allocated that
	memory. For a .REX file we used "malloc" to allocate from the heap
	and for .EXP files we created new LDT entries. To find out which 
	type of allocation was used, we will use REXP. If it is NULL then 
	this is a .EXP file, otherwise it is a .REX. In the case of a .REX 
	file, we know that the memory was allocated using "malloc" and can
	be freed using "free". In the case of a .EXP file, we will use
	int 21h AH=49h (free segment) to free the code and data segments.
*/

	if (retcode=close(handle))  
		return retcode;		 /* Report error closing file */

	if (rexp != NULL)
		free(rexp);
	else
	{	/* Free code and data segments */
		retcode=	free_segment(regstp->cs) | 
				free_segment(regstp->ds);
		if (retcode)
			return error(MEM_ERR+FRE_SEG);
	}
}

/* INT	LOAD_FLAT (HANDLE,HDRSP,OHDRSP,REGSTP):

		This routine finishes loading files that are flat model
	.EXP type.  It takes as input HANDLE, the handle to the .EXP
	file and HDRSP and OHDRSP, pointers to structures of the new and
	old format .EXP headers,respectively, and REGSTP, a pointer to a 
	structure containing fields for each of the registers.
*/

static int	load_flat(handle,hdrsp,ohdrsp,regstp)

ULONG	handle;
EXP_HDR	*hdrsp;
OEXP_HDR	*ohdrsp;
REGS		*regstp;

{
int		retcode;
DOSX_RTP	rtpsp;		/* run-time parameter block */
ULONG		avail;		/* available (free) pages */
ULONG		min_mem;	/* minimum free pages needed to load program */
ULONG		max_mem;	/* maximum free pages to load program */ 
ULONG		alloc_mem;	/* amount of memory that will be allocated */
USHORT		cs_sel;		/* code segment selector for loaded program */
USHORT		ds_sel;		/* data segment selector for loaded program */
BOOL		err_state;	/* temporary variable to save ld_err_msg */
ULONG		temp;		/* temp var used for checking overflow */

/*
	Find out what kind of header we just read in.  Take the following
	action depending on the type:

	- If old .EXP header, convert it to the new one.
	- If new .EXP header, finish reading it.
*/

	if (ohdrsp->exe_sign == EXP_OLD)
		conv_old(hdrsp,ohdrsp);		/* convert old header to new */
     	else if (ohdrsp->exe_sign == EXP_386)
	{
		memcpy(hdrsp,ohdrsp,sizeof(OEXP_HDR));/* copy into new header */
     		if ((retcode=read(handle, (UCHAR *) (hdrsp) + sizeof(OEXP_HDR)
			,sizeof(EXP_HDR)-sizeof(OEXP_HDR),0)))
				return error(HDR_ERR+BAD_HDR);
		if ((hdrsp->exp_boff) & LA_POFFS)
			/* Offset not Multiple of 4K */
			return error(HDR_ERR+BAD_OFF);
     	}
	else
	{
		printf("\nError: Bug in loader.\n");
		return TRUE; 		/* say there was an error */
	}

	if (hdrsp->exp_rtp !=0)
	{
     		retcode=seek(handle,hdrsp->exp_rtp,ABS_SEEK);
    		retcode=retcode | read(handle,&rtpsp,sizeof(DOSX_RTP),0);
		if (retcode != NO_ERR)
			return error(HDR_ERR+BAD_RTP); /* failure reading RTP */
		if (rtpsp.rtp_rbr != 0) 
			return error(HDR_ERR+NZ_RBR);	/* non-zero Realbreak parm */
	}

	#if DEBUG
		show_stats(hdrsp);
	#endif

/*
	At this point, we have already loaded the file header and
	done all the necessary conversions and preliminary checks.
	Now we must decide how much memory we would like to allocate 
	for the program and whether there is enough or not. 
*/

	min_mem= hdrsp->exp_minext + hdrsp->exp_ldisize;

	if (min_mem >= hdrsp->exp_minext) /*check if there was an overflow*/
	{
		temp=min_mem;
		min_mem=min_mem+hdrsp->exp_boff;
		if (min_mem < temp)
			min_mem=0xffffffff; /* overflow occurred assign max */
	}
	else
		min_mem=0xffffffff;	/* overflow occurred assign max */

	min_mem=  MIN_PAGES(min_mem);		/* convert to pages */


	max_mem= hdrsp->exp_maxext + hdrsp->exp_ldisize;

	if (max_mem >= hdrsp->exp_maxext) /*check if there was an overflow*/
	{
		temp=max_mem;
		max_mem=max_mem+hdrsp->exp_boff;
		if (max_mem < temp)
			max_mem=0xffffffff; /* overflow occurred assign max */
	}
	else
		max_mem=0xffffffff;	/* overflow occurred assign max */
	
	max_mem=  MIN_PAGES(max_mem); 

	#if DEBUG
		printf("Min Pages: %08xh    Max Pages: %08xh\n",
				min_mem,max_mem);
	#endif


/*
	Now, we'll find out if Phar Lap's Virtual Memory Manager is
	installed, to see if we have to worry about how much physical
	memory is available for loading the program.	If it is, we'll
	allocate the minimal amount of memory required to load the prog.
	If VMM is not present, we have to make sure we have memory to do it.
	We do this by trying to allocate so much memory that there 
	will be an error. When this happens, "allocate_segment" returns 
	the available memory in "avail". 
*/

	if (vmm_present())
		alloc_mem=min_mem;  /* we will allocate minimum memory */
	else
	{
		err_state=ld_err_msg;
		ld_err_msg=FALSE; 	/* Disable error messages , we
				  	are causing an error on purpose
				  	in order to find out how much 
				  	memory is available */

		allocate_segment(0xfffff,&avail);  
		ld_err_msg=err_state;

		#if DEBUG
			printf("Available Memory Pages: %08xh\n",avail);
		#endif

		if ( avail < min_mem)
			return error(MEM_ERR+LP_MEM);/* We have insufficient 
				  		memory to load program and 
				  		meet MINDATA requirements. */

		if (avail < max_mem)	  /* Allocate up to MAX_MEM pages */	
			alloc_mem=avail;
		else
			alloc_mem=max_mem;
	}

	#if DEBUG
		printf("Will allocate %08xh pages for program.\n",
				alloc_mem);
	#endif

	if ((ds_sel=allocate_segment(alloc_mem,&avail)) == 0)
		return error(MEM_ERR+AS_MEM); /* Something went wrong when 
					      	allocating memory for the
					      		program */

	#if DEBUG
		printf("Data segment created. DS = %08xh\n",ds_sel);
		printf("Alias for CS: ");
	#endif

/*
	We have successfully created and allocated memory to the
	DS segment. Now we will create an alias of the DS descriptor
	to use as the CS descriptor.
*/

	if ( (cs_sel=make_code_seg(ds_sel)) == 0 )
		return MEM_ERR+LDT_ERR;

	#if DEBUG
		printf("%08xh\n", cs_sel);
	#endif

/*	
	All is well so far. Load the program at the necessary 
	offset within the data segment we just created. 
*/

	#if DEBUG
		printf("Loading program at offset %08xh ... ",
				hdrsp->exp_boff);
	#endif

	retcode=seek(handle,hdrsp->exp_ldimg,ABS_SEEK);
	retcode=retcode | 
	   read(handle,(UCHAR *) hdrsp->exp_boff,hdrsp->exp_ldisize,ds_sel);

	if (retcode)
		return(MEM_ERR+LD_PROG); /* Say something happened while 
					 	reading program image */

	#if DEBUG
		printf("OK\n");
	#endif

/*
	Well... Looks like nothing went wrong. Load up initial values
	into register structure and return to Papa.
*/

	regstp->cs=cs_sel;
	regstp->eip=hdrsp->exp_ieip;
	regstp->ds=regstp->es=regstp->fs=regstp->gs=regstp->ss=ds_sel;
	regstp->eax=regstp->ebx=regstp->ecx=regstp->edx=
	regstp->esi=regstp->edi=regstp->ebp=0;
	regstp->esp=hdrsp->exp_iesp;

	return NO_ERR;
}


/* INT	LOAD_REX (HANDLE,HDRSP,OHDRSP,REGSTP,REXPP):

		This routine finishes loading files that are .REX type.  
	It takes as input HANDLE, the handle to the .REX file and HDRSP 
	and OHDRSP, pointers to structures of the new .EXP header and old
	.REX header,respectively, and REGSTP, a pointer to a structure
	containing fields for each of the registers. REXPP is a pointer to
	pointer that will serve to indicate that a .REX file was loaded by 
	containing the address of the beginning of memory allocated for the 
	program. REXPP must be kept intact for "unload" to work.
*/

static int	load_rex(handle,hdrsp,ohdrsp,regstp,rexpp)

ULONG	handle;
EXP_HDR	*hdrsp;
OEXP_HDR	*ohdrsp;
REGS	*regstp;
char	**rexpp;

{
ULONG	mem_alloc;		/* Memory to allocate to program */
ULONG	retcode;		/* Function return code */
ULONG	*load_addr;		/* Address in which to load program */
ULONG	rel_addr;		/* Addr of value to be relocated */

/*  Convert .REX to new .EXP to make things easier to deal with. */

	conv_old(hdrsp,ohdrsp);

	#if DEBUG
		show_stats(hdrsp);
	#endif
/*
	Figure out how much memory we will need to load this program
	(we will always be loading MINDATA extra bytes, we will ignore
	the MAXDATA parameter) and call the C routine "malloc" to try 
	to allocate it. If there is an error, give up. If all works out, 
	read load image into memory.
*/
        
	mem_alloc=(hdrsp->exp_minext) + (hdrsp->exp_ldisize);

	#if DEBUG
		printf("Will allocate %08xh bytes.\n",mem_alloc);
	#endif

	load_addr=(ULONG *) malloc (mem_alloc); /* allocate the memory */

	*rexpp = (char *)load_addr; /* save pointer so we can free the 
					memory later (with "unload"). */						

	if (load_addr == NULL)
		return error(MEM_ERR+LP_MEM);  /* error, not enough memory */

	#if DEBUG
		printf("Reading Load Image into memory.\n");
	#endif

	retcode=seek(handle,hdrsp->exp_ldimg,ABS_SEEK);
    	retcode=retcode | read(handle,(UCHAR *) load_addr,
						hdrsp->exp_ldisize,0);

	if (retcode != NO_ERR)
		return error(MEM_ERR+LD_PROG); /* error when reading image */

	#if DEBUG
		printf("Will relocate %08xh symbols.\n",
			(hdrsp->exp_relsize)>>2);
	#endif

	if (retcode=seek(handle,hdrsp->exp_rel,ABS_SEEK))
		return retcode;

/*
	Now that the program is loaded in memory, we must relocate
	all address that need relocation. The following loop does
	this.
*/

	for (;(hdrsp->exp_relsize != 0);(hdrsp->exp_relsize -= 4))
	{
    	retcode=read(handle,&rel_addr,4,0); /* read a relocation entry */

	if (rel_addr & REL32)
		*((ULONG *)(((ULONG)load_addr)+(rel_addr&(~REL32)))) += 
						(ULONG) load_addr;
	else
		*((USHORT *)(((ULONG)load_addr)+(rel_addr))) += 
						(ULONG) load_addr;
	} /* for loop */

/*
	We're done. Go back to caller.
*/

	return NO_ERR;
}




/* INT	VMM_PRESENT():

		This routine is used to find out if Phar Lap's Virtual
	Memory Manager is installed. If it is, then we don't have to
	worry about not having enough available physical memory to
	load the program. The way we find out if VMM is installed is
	by issuing a Get Memory Statistics system call (function 2520h).
	If we get an error, VMM is not present. If we don't get an error,
	then we have to check the buffer we got back from the call to
	see if it indicates that VMM is present. The function result
	is TRUE if VMM is present and FALSE if it is not.
*/

static int	vmm_present()

{
REGS	regst;
ULONG	buffer[100];

	regst.eax=0x2520;		/* Get Memory Statistics Function */
	regst.ebx=0;	 		/* Don't reset Statistics */
	regst.edx=(ULONG)buffer;	/* Address of buffer */
	regst.ds=_mwgetds();		/* Get value for DS */
	regst.es=			/* Init other Seg Regs*/
		regst.fs=regst.gs=regst.ds;

	if (intr(0x21,&regst))		/* Mem Stats */
	{
		#if DEBUG
			printf("Get Mem Stats does not exist.\n");
		#endif
		return FALSE;   	/* say VMM not present */
	}

	if (buffer[0])
	{
		#if DEBUG
			printf("VMM present.\n");
		#endif
		return TRUE;		/* VMM present */
	}

	#if DEBUG
		printf("VMM not present.\n");
	#endif

	return FALSE;	 
}

#if DEBUG


/* INT	SHOW_STATS (HDRSP):

		This routine is used to print some relevant file header
	values on the screen for debug purposes. The pointer to the
	header structure is passed in HDRSP.
*/

static int	show_stats (hdrsp)

EXP_HDR	*hdrsp;

{
	printf("\nFile Parameters:\n");
	printf("Load image      File Offset: %08xh     Size: %08xh\n",
			hdrsp->exp_ldimg,hdrsp->exp_ldisize);
	printf("Run-Time Parms  File Offset: %08xh     Size: %08xh\n",
			hdrsp->exp_rtp,hdrsp->exp_rtpsize);
	printf("Relocation Table Offset: %08xh     Size: %08xh\n",
			hdrsp->exp_rel,hdrsp->exp_relsize);
	printf("Base Offset parameter: %08xh\n",
		    	hdrsp->exp_boff);
	printf("Min Data: %08xh    Max Data: %08xh\n",
			hdrsp->exp_minext,hdrsp->exp_maxext);
	printf("Initial EIP : %08xh    ESP : %08xh\n\n",
			hdrsp->exp_ieip,hdrsp->exp_iesp);
}

#endif

/* INT	ALLOCATE_SEGMENT (SIZE,AVAIL):

		This routine allocates a new segment, that is, it creates
	a new entry in the LDT for this segment and allocates the number
	of 4 KByte pages contained in SIZE to the segment. If successful,
	the function result will be the new selector. If it fails, the 
	function result will be 0 and AVAIL will contain the number of
	pages currently available.
*/

static int	allocate_segment(size,avail)

ULONG	size;
ULONG	*avail;

{

REGS	regst;

	regst.eax=0x4800; 		/* Allocate Segment Function */
	regst.ebx=size;			/* Number of pages to allocate */
	regst.ds=_mwgetds();		/* Get value for DS */
	regst.es=			/* Init other Seg Regs*/
		regst.fs=regst.gs=regst.ds;

	if (intr(0x21,&regst))		/* Allocate Segment */
	{
		*avail=regst.ebx;	/* Put mem available in AVAIL */
		return 0;   	 	/* Return Error */
	}

	return (LONG) regst.eax; 	/* Return Segment Selector */
}

/* INT	FREE_SEGMENT (SELECTOR):

		This routine frees the segment given in SELECTOR. The LDT
	entry is marked as not used and all the associated memory is
	returned to the operating system.
*/

static int	free_segment(selector)

USHORT	selector;

{

REGS	regst;

	regst.eax=0x4900; 	 	/* Free Segment Function */
	regst.es=selector; 	 	/* Selector of seg to free */
	regst.fs=regst.gs=regst.ds=_mwgetds(); /* Set seg regs */

	if (intr(0x21,&regst))	 	/* Allocate Segment */
		return error(MEM_ERR+BAD_SEL); /* Return Error */

	return NO_ERR;		 	/* Return Segment Selector */
}
	
/* INT	MAKE_CODE_SEG (SELECTOR):

		This routine creates an alias in the LDT for the segment
	that is specified by SELECTOR. This new segment will be given
	the access type of a code segment and a USE32 type. The function
	result will be the new selector if all goes well and 0 if some-
	thing goes wrong.
*/

static int	make_code_seg(selector)

ULONG	selector;

{

REGS	regst;

	regst.eax=0x2513; 	 	/* Alias Segment Descriptor Function */
	regst.ebx=selector;	     	/* Selector of descriptor to alias */
	regst.ecx=AR_CODE+(DOS_32<<8); /* Specify USE32 code segment */
	regst.ds=_mwgetds();		/* Get value for DS */
	regst.es=			/* Init other Seg Regs*/
		regst.fs=regst.gs=regst.ds;

	if (intr(0x21,&regst))		/* Alias Segment */
	{
		error(MEM_ERR+regst.eax);	/* Return Error */
		return 0;
	}

	return (LONG) regst.eax;	 /* Return Segment Selector */
}
	
/* INT	CONV_OLD (HDRS,OHDRS):
		
		This routine converts a header in the old .EXP format to
	the new format.  HDRS and OHDRS are pointers to the new and old
	format .EXP headers, respectively. The conversion is not complete,
	only things that will be needed get converted.
*/

static int	conv_old(hdrsp,ohdrsp)

OEXP_HDR	*ohdrsp;
EXP_HDR	*hdrsp;

{
	/* file offset to load image */
	hdrsp->exp_ldimg=(ohdrsp->exe_hsize) << 4;

	/* size of load image in bytes */
	hdrsp->exp_ldisize= ((ohdrsp->exe_size)-1) * BLK_SIZE +
		    			(ohdrsp->exe_szrem) - (hdrsp->exp_ldimg);

	/* File offset of relocation table */
	hdrsp->exp_rel = ohdrsp->exe_reloff;

	/* Number of bytes in relocation table */
	hdrsp->exp_relsize = (ohdrsp->exe_relcnt)*4;

	/* Minimum Data to allocate after load image */
	hdrsp->exp_minext=ohdrsp->exe_minpg<<PAGE_SHIFT;

	/* Maximum Data to allocate after load image */
	hdrsp->exp_maxext=ohdrsp->exe_maxpg<<PAGE_SHIFT;
		
	/* Initial EIP */
	hdrsp->exp_ieip= (ohdrsp->exe_eip);

	/* Initial ESP */
	hdrsp->exp_iesp= (ohdrsp->exe_esp);

	#if DEBUG
		printf("** Header converted to new .EXP format **\n");
	#endif

}

/* INT	OPEN (FNAMP,MODE,HANDLE) :

		This routine is used for opening a file. A pointer to a zero
	terminated string containing the file to be opened (path accepted),
	FNAMP, and the access and file-sharing mode, MODE, are passed
	into it. If opening was successful, HANDLE will point to the
	file handle. The function result will contain OPN_ERR if an error
	occurs, or NO_ERR if it succeeds.

*/


static int	open(fnamp,mode,handle)

UCHAR	*fnamp;
USHORT	mode;
USHORT	*handle;

{

REGS	regst;


	regst.eax=0x3d00 | mode; /* Open File Function - AL contains
		    		 	Access and file sharing mode */
	regst.ds=_mwgetds();	 /* Get value for DS */
	regst.es=		 /*Init other Seg Regs*/
		    regst.fs=regst.gs=regst.ds;
	regst.edx=(ULONG) fnamp; /* Point DX at name of file */

	if (intr(0x21,&regst))	  /* Open file */
		return error(OPN_ERR+regst.eax);   /* Report Error */

	*handle=(USHORT) (regst.eax) & 0xffff;
	return NO_ERR;
}

/* INT	READ (HANDLE,BUFFER,LENGTH,SELECTOR):

		This routine reads a previously opened file into a given buffer.
	As input, this routine takes HANDLE, which is the handle of the
	file to be read from, BUFFER, which is a near pointer to the
	buffer to read data into and LENGTH, which is the number of bytes
	to read. If SELECTOR is non-zero, then the value within it will be
	used instead of the current DS to access BUFFER.
	If successful, the function result is NO_ERR, otherwise,it is RD_ERR.
*/

static int	read(handle,buffer,length,selector)

ULONG	handle;
UCHAR	*buffer;
ULONG	length;
ULONG	selector;

{

REGS	regst;

	regst.eax=0x3f00;			/* Read File Function */
	regst.ebx=handle;			/* File handle */
	regst.ecx=length;			/* Byte count */
	regst.es=		  		/*Init Seg Regs*/
		regst.fs=regst.gs=_mwgetds();

	if (selector)		  	/* If selector is specified, set DS */
		regst.ds=selector;	/*   equal to it, otherwise set DS */
	else			  		/*   equal to the current DS */
		regst.ds=regst.es;
		regst.edx=(ULONG) buffer; 	/* Point DX at buffer */

	if (intr(0x21,&regst))			/* Read file */
		return error(RD_ERR+regst.eax);   /* Report Error */

	if (regst.eax != length)
		return error(RD_ERR+BAD_LEN);	/* Didn't read enough bytes */

	return NO_ERR;
}

/* INT	SEEK (HANDLE,OFFSET,ORIGIN):

		This routine moves the file pointer of the file in HANDLE 
	to the OFFSET specified. ORIGIN specifies the origin from which
	the offset should be computed:

		ORIGIN = 0: Offset from beginning of file
		ORIGIN = 1: Offset from current file pointer
		ORIGIN = 2: Offset from end of file
	
*/

static int	seek(handle,offset,origin)

ULONG	handle;
LONG	offset;
USHORT	origin;

{

REGS	regst;

	regst.eax=0x4200 | (origin & 0xff); /* Seek Function */
	regst.ebx=handle;			/* File handle */
	/* High part of offset */
	regst.ecx=(ULONG) (offset & 0xffff0000) >> 16;
	/* Low part of offset */
	regst.edx=(ULONG) offset & 0xffff;
	regst.ds=_mwgetds();		/* Get value for DS */
	regst.es=  			/*Init other Seg Regs*/
		regst.fs=regst.gs=regst.ds;

	if (intr(0x21,&regst))			/* Seek file */
		return error(SK_ERR+regst.eax);   /* Report Error */

	return NO_ERR;
}


/* INT	CLOSE (HANDLE):

		This routine closes the file specified by HANDLE.
	
*/

static int close(handle)

ULONG	handle;

{

REGS	regst;

	regst.eax=0x3e00; 			/* Close File Function */
	regst.ebx=handle;			/* File handle */
	regst.ds=_mwgetds();			/* Get value for DS */
	regst.es=				/* Init other Seg Regs */
		regst.fs=regst.gs=regst.ds;

	if (intr(0x21,&regst))			/*  Close file */
		return error(CLS_ERR+regst.eax);   /* Report Error */

	return NO_ERR;
}
