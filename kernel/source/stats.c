/******************************************************************************
*                                                                             *
*   Module:     Stats.c                                                       *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       11/7/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This module contains the code for the stat() and fstat() system calls.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 11/7/97    Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include <errno.h>                      // Include error numbers header

#include <sys\syscall.h>                // Include system call header file

#include <sys\ioctl.h>                  // Include io control header file

#include <sys\stat.h>                   // Include file information header

#include "inline.h"

#include "device.h"                     // Include device drivers header

#include "kernel.h"                     // Include kernel header file

#include "ksyscall.h"                   // Include kernel system call header

#include "tty.h"                        // Include terminal file header

#include "process.h"                    // Include process header file

#include "file.h"                       // Include kernel file header

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
*   int Sys_Stat()                                                            *
*                                                                             *
*******************************************************************************
*
*   This is a stat() system call.
*
*   Where:
#       SYS_ARG1 is the address of the file name in the user space
#       SYS_ARG2 is the address of the stat structure in the user space
*
*   Returns:
*       0 on success
#       error code on failure
*
******************************************************************************/
int Sys_Stat()
{
    char *pFile;
    struct stat *pStat;


    // Check and get the file name

    if( (pFile = CheckProcessString( (char *) SYS_ARG1, WRITE_BUF_LEN ))==NULL )
        sys_end( ENAMETOOLONG );

    // Check and get the stat buffer address

    if( (pStat = (struct stat *) CheckProcessBuf( SYS_ARG2, sizeof(struct stat), sizeof(struct stat) ) )==NULL )
        sys_end( EACCESS );

    // Resolve the file name into a stat structure

//    return( PathResolve( pFile, &pStat ) );

    return( 0 );
}


/******************************************************************************
*                                                                             *
*   int Sys_Fstat()                                                           *
*                                                                             *
*******************************************************************************
*
*   This is a fstat() system call.
*
*   Where:
#       SYS_ARG1 is the process file descriptor
#       SYS_ARG2 is the address of the stat structure in the user space
*
*   Returns:
*       0 on success
#       error code on failure
*
******************************************************************************/
int Sys_Fstat()
{
    int i, f;
    DWORD *pStat;


    // Look for the entry in the process file table

    i = SYS_ARG1;
    if( (i<0) || (i>=FOPEN_MAX) || ((f = pCurProcPM->iFile[i]) == 0) )
        return( EBADF );

    // Check and get the stat buffer address

    if( (pStat = (struct stat *) CheckProcessBuf(
                SYS_ARG2,
                sizeof(struct stat),
                sizeof(struct stat) ) )==NULL )

        sys_end( EACCESS );

    // Copy the stat buffer from the system file slot into the user buffer

    memcpy( pStat, &Files[f].Stat, sizeof(struct stat) );

    sys_end( 0 );
}



