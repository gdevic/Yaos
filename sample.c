/* SAMPLE.C - Demonstration of Usage of the routines in LOAD.C */
/************************************************************************/
/*	Copyright (C) 1986-1989 Phar Lap Software, Inc.                  */
/*	Unpublished - rights reserved under the Copyright Laws of the    */
/*	United States.  Use, duplication, or disclosure by the           */
/*	Government is subject to restrictions as set forth in            */
/*	subparagraph (c)(1)(ii) of the Rights in Technical Data and      */
/*	Computer Software clause at 252.227-7013.                        */
/*	Phar Lap Software, Inc., 60 Aberdeen Ave., Cambridge, MA 02138   */
/************************************************************************/
/*		REVISION LOG

Rev. 1.00, ALS, 31-Mar-89	- File creation date.
*/

#include <pltypes.h>
#include <load.h>

extern
int	load(char *fnamp,REGS *regstp,BOOL ld_err_msg,char **rexpp,
		ULONG *handp);

extern
int	execute(REGS *regstp,char *rexp);

extern
int	unload(REGS *regstp,char *rexp,ULONG handle);

main (argc,argv)
int	argc;
char	**argv;
{
char	*rexp; 			/* pointer to loaded .REX file */
REGS	regst,regst2;		/* register structure and a copy */
ULONG	handle;			/* This will contain the handle of the file
				loaded after returning from 'load' */

/*	Find out if the file name to load has been specified. */

	if (argc==1)
	{
		printf("\nERROR: Filename not Specified.\n");
		return;
	}
	
	adjust_ctail(); 		/* shift command tail one arg left */

/*
	Call the routine to load the program. If there are no errors,
	execute the program and then unload it.
*/

	if (load(*(argv+1),&regst,TRUE,&rexp,&handle))  
		printf("LOADER: Trouble loading file.\n");
	else 			  
	{
		memcpy(&regst2,&regst,sizeof(regst2)); /* make a copy */
		execute(&regst2,rexp);
		unload(&regst,rexp,handle); 		/* clean up */
	}
}

/* INT	ADJUST_CTAIL();

		This routine shifts the command tail one argument to the
	left so that the loaded program will receive all tokens to the
	right of itself as its command tail.

*/

static int	adjust_ctail()
{
FARPTR	com_tail;		/* pointer to command tail in PSP */
USHORT	comt_len;		/* length of command tail */
USHORT	new_first_arg;		/* offset of new first argument in command tail */
USHORT	new_comt_len;		/* new length of command tail */
UINT	i;			/* loop variable */

	FP_SET(com_tail,0x80,0x24);/* point at command tail in PSP */
	comt_len= (USHORT) *com_tail; /* get command tail length */

	for (i=1; ( *(com_tail+i) != ' ') && (*(com_tail+i) != '\r' )
				;i+=1); /* find first space or carriage return */
	for (;*(com_tail+i) == ' ';i+=1); /* find next non-space */

	new_first_arg=i;		/* save offset to second arg */
	*com_tail=(UCHAR)(new_comt_len=comt_len-new_first_arg+1);
	for (i=0;i <= new_comt_len;i +=1) /* shift command tail left 
						one argument 	*/
		*(com_tail+i+1)=*(com_tail+new_first_arg+i);
}
