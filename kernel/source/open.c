/******************************************************************************
*                                                                             *
*   Module:     Open.c                                                        *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       10/17/97                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This module contains the code for the `creat', `open' and `close'
        system calls.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 10/17/97   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include <stdio.h>                      // Include standard io header file

#include <string.h>                     // Include strings file header

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


int Sys_Ioctl()
{



    sys_end( 0 );
}



/******************************************************************************
*                                                                             *
*   int Sys_Creat()                                                           *
*                                                                             *
*******************************************************************************
*
*   This is a `creat' system call function.
*
*   Where:
#       SYS_ARG1 is the address of the file name in the user space
#       SYS_ARG2 is the creation mode
*
*   Returns:
*       SYS_BLOCKED if create cannot be accomplished at this time
#       SYS_END if create is finished
*
#       Client process is returned with an error code or
#           the handle of an opened file
*
******************************************************************************/
int Sys_Creat()
{
    TDevRq Rq;
    char *pBuf;
    int i, f;
    int err;
    BYTE bMajor, bMinor;
    char *path;


    // Look for the entry in the process file table

    for( i=0; i<FOPEN_MAX; i++ )
        if( pCurProcPM->iFile[i] == 0 )
            break;

    // Return if we could not find an empty file slot

    if( i==FOPEN_MAX ) sys_end( EMFILE );

    // Look in the system file table for an empty slot

    for( f=1; f<MAX_FILES; f++ )
        if( Files[f].Flags == 0 )
            break;

    // Return if we could not find an empty file slot

    if( f==MAX_FILES ) sys_end( EMFILE );

    if( (pBuf = CheckProcessString( (char *)SYS_ARG1, WRITE_BUF_LEN ))==NULL )
        return( ENOENT );

    // Get the absolute path and the device handling it

    if( (path = PathResolve( pBuf, &bMajor, &bMinor ))==NULL )
        sys_end( EMFILE );

//dprintf("\nPath: %s (%d.%d)", path, bMajor, bMinor );

    // Set the major and minor number

    Files[f].Stat.st_dev = (bMajor << 8) | bMinor;
    Files[f].bMajor = bMajor;
    Files[f].bMinor = bMinor;

    // Create a file

    Rq.p = path;
    Rq.n = SYS_ARG2;
    Rq.s = (char *) &Files[f].Stat;
    if( (err = (Dev[Files[f].bMajor].dev_creat)(Files[f].bMinor, &Rq)) < 0)
        sys_end( err );

    // Set up the process file index and make a descriptor busy

    pCurProcPM->iFile[i] = f;
    Files[f].Flags |= 1;

    // Return the process file index (fd)

    sys_end( i );
}

/******************************************************************************
*                                                                             *
*   int Sys_Open()                                                            *
*                                                                             *
*******************************************************************************
*
*   This is a `open' system call function.
*
*   Where:
#       SYS_ARG1 is the address of the file name in the user space
#       SYS_ARG2 is the access mode
*
*   Returns:
*       SYS_BLOCKED if open cannot be accomplished at this time
#       SYS_END if open is finished
#
#       Client process is returned with an error code or
#           the handle of an opened file
*
******************************************************************************/
int Sys_Open()
{
    TDevRq Rq;
    char *pBuf;
    int i, f;
    int err;
    BYTE bMajor, bMinor;
    char *path;


//dprintf("\nSys_Open %X %X", SYS_ARG1, SYS_ARG2 );

    // Look for the entry in the process file table

    for( i=0; i<FOPEN_MAX; i++ )
        if( pCurProcPM->iFile[i] == 0 )
            break;

    // Return if we could not find an empty file slot

    if( i==FOPEN_MAX ) sys_end( EMFILE );

    // Look in the system file table for an empty slot

    for( f=1; f<MAX_FILES; f++ )
        if( Files[f].Flags == 0 )
            break;

    // Return if we could not find an empty file slot

    if( f==MAX_FILES ) sys_end( EMFILE );

    if( (pBuf = CheckProcessString( (char *)SYS_ARG1, WRITE_BUF_LEN ))==NULL )
        return( ENOENT );

    // Get the absolute path and the device handling it

    if( (path = PathResolve( pBuf, &bMajor, &bMinor ))==NULL )
        sys_end( EMFILE );

//dprintf("\nPath: %s (%d.%d)", path, bMajor, bMinor );

    // Get the file's stats into the kernel file table

    Rq.p = path;
    Rq.n = (int) &Files[f].Stat;
    if( (err = (Dev[bMajor].dev_stat)(bMinor, &Rq )) != 0 )
        sys_end( err );

    // Duplicate major and minor number

    Files[f].bMajor = Files[f].Stat.st_dev >> 8;
    Files[f].bMinor = Files[f].Stat.st_dev & 0xFF;

    // Open a device that we just have `stat'

    Rq.n = SYS_ARG2;
    Rq.s = (char *) &Files[f].Stat;
    if( (err = (Dev[Files[f].bMajor].dev_open)(Files[f].bMinor, &Rq)) < 0)
        sys_end( err );

    // Set up the process file index and make a descriptor busy

    pCurProcPM->iFile[i] = f;
    Files[f].Flags |= 1;

    // Return the process file index (fd)

    sys_end( i );
}


/******************************************************************************
*                                                                             *
*   int Sys_Close()                                                           *
*                                                                             *
*******************************************************************************
*
*   This is a `close' system call function.
*
*   Where:
#       SYS_ARG1 is the file handle of the file to close
*
*   Returns:
#       0 if successful
#       EBADF
*
******************************************************************************/
int Sys_Close()
{
    TDevRq Rq;
    int fd, f, err;


//dprintf("\nSys_Close %X", SYS_ARG1 );

    // Check that the closing descriptor is valid in the user file array

    fd = SYS_ARG1;

    if( (fd < 0) || (fd >= FOPEN_MAX) )  sys_end( EBADF );
    if( (f = pCurProcPM->iFile[fd]) == 0 )  sys_end( EBADF );

    // Check that the file is valid in the kernel file list

    if( Files[f].Flags == 0 )  sys_end( EBADF );

    // Close a file using a file's device stat

    Rq.s = (char *) &Files[f].Stat;
    if( (err = (Dev[Files[f].bMajor].dev_close)(Files[f].bMinor, &Rq)) < 0)
        sys_end( err );

    // Free the data structures in the kernel and in the process

    Files[f].Flags = 0;
    pCurProcPM->iFile[fd] = 0;

    sys_end( 0 );
}

