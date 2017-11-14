/******************************************************************************
*                                                                             *
*   Module:     _Wait.c                                                       *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       10/23/97                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This module contains the code for the standard unix C services

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 10/23/97   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include <sys/syscall.h>                // Include system call header

#include <sys/wait.h>                   // Include its own header

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
*   pid_t waitpid( pid_t pid, int *status, int options )                      *
*                                                                             *
*******************************************************************************
*
*   This is a specific pid wait function.
*
*   Where:
#       pid is one of the following:
#           -1, to wait for any child process (same as wait() )
#           positive, to wait for the child with that specific pid
#           zero, to wait for any child with the same process group id
#           negative, to wait for any child with the specific -process gid
#
*       status is the address to store the resulting status, NULL optionally
#
#       options can be WNOHANG not to suspend execution if the status is not
#           immediately available
#           WUNTRACED, report the status of a stopped child process
*
*   Returns:
*       process child's pid
*
#   System call returns child process id in the upper 16 bits of the result,
#   the lower 16 bits are the status information as follows:
#
#   HIGH 8       LOW 8 bits
#   exit status  00            for a normal termination
#   00           signal number for a termination due to a signal
#   signal       7F            for a stopped process due to a signal
#
******************************************************************************/
pid_t waitpid( pid_t pid, int *status, int options )
{
    int stat;

    stat = SYS_CALL2( SYS_WAITPID, pid, options );

    if( status != NULL )
    {
        *status = stat & 0xFFFF;
    }

    return( stat >> 16 );
}


/******************************************************************************
*                                                                             *
*   pid_t wait( int *status )                                                 *
*                                                                             *
*******************************************************************************
*
*   This is a wait function.
*
*   Where:
*       status is the address to store the resulting status
#       NULL optionally
*
*   Returns:
*       process child's pid
#
******************************************************************************/
pid_t wait( int *status )
{
    // This call is essentially waiting for any child process

    return( waitpid( -1, status, 0) );
}

