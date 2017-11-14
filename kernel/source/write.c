/******************************************************************************
*                                                                             *
*   Module:     Write.c                                                       *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       10/12/97                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This module contains the code for the write system call.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 10/12/97   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include <sys\syscall.h>                // Include system call header file

#include <errno.h>                      // Include error defines

#include <stdio.h>                      // Number of open files

#include "device.h"                     // Include device driver header

#include "file.h"                       // Include file management hader

#include "kernel.h"                     // Include kernel header file

#include "ksyscall.h"                   // Include kernel system call header

#include "tty.h"                        // Include terminal file header

#include "process.h"                    // Include process header file

#include "qqueue.h"                     // Include quick queue header file

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   int Sys_Write()                                                           *
*                                                                             *
*******************************************************************************
*
*   This is a write system call function.
*
*   Where:
#       SYS_ARG1 is the process file descriptor
#       SYS_ARG2 is the address of memeory block to write
#       SYS_ARG3 is the number of bytes to write
*
*   Returns:
*       SYS_BLOCKED if write cannot be accomplished at this time
#       SYS_END if write is finished
*
******************************************************************************/
int Sys_Write()
{
    char *pBuf;
    int i;

//dprintf("\nWrite %X %X %X", SYS_ARG1, SYS_ARG2, SYS_ARG3 );

    pBuf = CheckProcessBuf( SYS_ARG2, SYS_ARG3, WRITE_BUF_LEN );

    if( pBuf == NULL ) sys_end( EFAULT );
    if( SYS_ARG1 >= FOPEN_MAX ) sys_end( EBADF );
    if( (i = pCurProcPM->iFile[SYS_ARG1]) == 0 ) sys_end( EBADF );
    if( Dev[Files[i].bMajor].dev_write == NULL ) sys_end( EIO );

    // Now we have an index to the kernel file table.
    // Call a device to perform the write operation

//dprintf("\nWDev %08X", Dev[Files[i].bMajor].dev_write );

    pCurProcPM->Rq.s = (char *) &Files[i].Stat;
    pCurProcPM->Rq.p = pBuf;            // Write this buffer
    pCurProcPM->Rq.n = SYS_ARG3;        // Length of a buffer

    SYS_RET = (Dev[Files[i].bMajor].dev_write)(Files[i].bMinor, &pCurProcPM->Rq);

    return( SYS_END );
}


