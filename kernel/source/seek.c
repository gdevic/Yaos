/******************************************************************************
*                                                                             *
*   Module:     Seek.c                                                        *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       11/5/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This module contains the code for the seek system call.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 11/5/97    Original                                             Goran Devic *
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
*   int Sys_LSeek()                                                           *
*                                                                             *
*******************************************************************************
*
*   This is a seek system call function.
*
*   Where:
#       SYS_ARG1 is the process file descriptor
#       SYS_ARG2 is the new offset
#       SYS_ARG3 is the position code
*
*   Returns:
*       SYS_BLOCKED if seek cannot be accomplished at this time
#       SYS_END if seek is finished
*
#       Client process is returned with an error code or
#           the new position within a file after seek
*
******************************************************************************/
int Sys_LSeek()
{
    int f, seek;

dprintf("\nSeek %X %X %X", SYS_ARG1, SYS_ARG2, SYS_ARG3 );

    if( (SYS_ARG3 != SEEK_SET)
     && (SYS_ARG3 != SEEK_CUR)
     && (SYS_ARG3 != SEEK_END) )
        sys_end( EINVAL );

    if( SYS_ARG1 >= FOPEN_MAX ) sys_end( EBADF );
    if( (f = pCurProcPM->iFile[SYS_ARG1]) == 0 ) sys_end( EBADF );
    if( Dev[Files[f].bMajor].dev_lseek == NULL ) sys_end( EIO );

    // Now we have an index to the kernel file table.
    // Call a device to perform the seek operation

    pCurProcPM->Rq.s = (char *) &Files[f].Stat;
    pCurProcPM->Rq.p = (char *) SYS_ARG2;   // Offset value
    pCurProcPM->Rq.n = SYS_ARG3;            // Position code

    seek = (Dev[Files[f].bMajor].dev_lseek)(Files[f].bMinor, &pCurProcPM->Rq);

    sys_end( seek );
}


