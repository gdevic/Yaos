/******************************************************************************
*                                                                             *
*   Module:     SysCall.c                                                     *
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

        This module contains the code for the handling of the system calls
        from the PM clients.
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

#include "inline.h"                     // Include inline functions

#include "kernel.h"                     // Include kernel header file

#include "ksyscall.h"                   // Include kernel system call header

#include "tty.h"                        // Include terminal file header

#include "device.h"                     // Include device header file

#include "process.h"                    // Include process header file

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

// This is a table of pointers to functions that perform system calls

int (*SysCall_Table[ NUM_SYSCALLS ])() =
{
    Sys_Null,               // 0
    Sys_Fork,               // 1
    Sys_Exec,               // 2
    Sys_Wait,               // 3
    Sys_Exit,               // 4
    Sys_Kill,               // 5

    Sys_Creat,              // 6
    Sys_Open,               // 7
    Sys_Close,              // 8
    Sys_Read,               // 9
    Sys_Write,              // 10
    Sys_LSeek,              // 11
    Sys_Ioctl,              // 12

    Sys_Stat,               // 13
    Sys_Fstat,              // 14

//    Sys_Opendir,            // 15
//    Sys_Readdir,            // 16
//    Sys_Closedir            // 17
};


//----------------------------------------------------------------------------
// Global pointer to a current client PM process register structure (stack)
//----------------------------------------------------------------------------

TPMStack *pSysRegs;


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
*   void SysCallx( TPMStack Stack )                                           *
*                                                                             *
*******************************************************************************
*
*   This function handles system calls made from protected mode processes
#   via the syscall interface.
*
*   Where:
*       Stack is the stack frame of the current PM process invoking a syscall
*
*   Returns:
*       void
#
#   Note: the specific system call function returns the statue of the process
#         SYS_BLOCKED if a process is blocked and should be rescheduled,
#         SYS_END if a process system call is accomplished and we can return
#                 to it
*
******************************************************************************/
void SysCallx( TPMStack Stack )
{
    // Set the global variable for use by devices

    pSysRegs = &Stack;

    // We do need to perform a seria of checks on the arguments

//dprintf("\nSyscall %d (0x%X,0x%X,0x%X)", Stack.eax, Stack.ebx, Stack.ecx, Stack.edx );

    if( Stack.eax < NUM_SYSCALLS )
    {
        // Call the system call function

        if( (SysCall_Table[ Stack.eax ])() == SYS_BLOCKED )
        {
            // Process had been blocked on this system call and/or needs to
            // be rescheduled and start running next ready process

            Reschedule( &Stack );
        }
    }
    else
    {
        dprintf("\nInvalid system call %d", Stack.eax );
    }
}


/******************************************************************************
*                                                                             *
*   int Sys_Null()                                                            *
*                                                                             *
*******************************************************************************
*
*   This is a stub for the unused system call #0. It does nothing.
*
******************************************************************************/
int Sys_Null()
{
    sys_end(0);
}


int Sys_Kill()
{
    sys_end(0);
}


/******************************************************************************
*                                                                             *
*   char *CheckProcessBuf( DWORD dwSrc, int nSize, int nMaxSize )             *
*                                                                             *
*******************************************************************************
*
#   This function checks the validity of a process buffer as passed by a
#   system call.  dwSrc is checked to be in the process address space, and
#   the nSize should be less or equal to the limit nMaxSize.  Starting
#   address plus size should not exceed process memory range.
*
*   Where:
#       dwSrc is the source address in the process memory
#       nSize is the size of the buffer
#       nMaxSize is the maximum size of the buffer
*
*   Returns:
#       Buffer address in the kernel address space if buffer looks ok
#       NULL if arguments did not pass all the checks
*
******************************************************************************/
char *CheckProcessBuf( DWORD dwSrc, int nSize, int nMaxSize )
{
    // Check the limit of the source address (process memory size is 4Mb,
    // so it cannot exceed that limit

    if( dwSrc < 4 * 1024 * 1024 )
    {
        // Check that the size does not exceed the maximum allowed size

        if( nSize <= nMaxSize )
        {
            // Check that the starting address plus size does not exceed 4Mb

            if( (dwSrc + nSize) <= 4 * 1024 * 1024 )
                return( (char *)(pCurProcPM->Image + dwSrc) );
        }
    }

    return( NULL );
}


/******************************************************************************
*                                                                             *
*   char *CheckProcessString( char *pStr, int nMaxSize )                      *
*                                                                             *
*******************************************************************************
*
#   This function checks the validity of a process ASCIIZ string buffer.
#   dwStr is checked to be in the process address space, and the terminating
#   '\0' is found within the process address space limits.  Also, the string
#   len should not be larger than nMaxSize value.
*
*   Where:
#       pStr is the string address in the process memory
#       nMaxSize is the maximum size of the string
*
*   Returns:
#       String address in the kernel address space if a string is ok
#       NULL if arguments did not pass all the checks
*
******************************************************************************/
char *CheckProcessString( char *pStr, int nMaxSize )
{
    char *pOrig = pStr;
    int len = 0;

    // Look for the terminating byte of zero and count the length

    while( (pStr < 4 * 1024 * 1024) && *pStr++ ) len++;

    if( (pStr < 4 * 1024 * 1024) && (len<=nMaxSize) )
        return( (char *)(pCurProcPM->Image + pOrig) );

    return( NULL );
}


/******************************************************************************
*                                                                             *
*   int CheckProcessAddress( DWORD dwAddress )                                *
*                                                                             *
*******************************************************************************
*
*   This function checks that the given address is within a current process
#   address space.  It returns that address to be used by the kernel or -1
#   if the address was invalid.
*
*   Where:
*       dwAddress is an address within the current process address space
*
*   Returns:
*       -1 if the address was outside process address space
#       kernel addressable process address
*
#   Note: If the address was 0, it is considered NULL and is returned as such
#
******************************************************************************/
int CheckProcessAddress( DWORD dwAddress )
{
    if( dwAddress == NULL )
        return( NULL );

    // Check the source address (process memory size is 4Mb, so it
    // cannot exceed that limit

    if( dwAddress < 4 * 1024 * 1024 )
    {
        // Return kernel addressable process address

        return( pCurProcPM->Image + dwAddress );
    }

    return( -1 );
}


/******************************************************************************
*                                                                             *
*   int IsBlocked( BYTE bMajor, BYTE bMinor )                                 *
*                                                                             *
*******************************************************************************
*
*   Returns the pid of a process that is blocked on a specific device
*
*   Where:
*       bMajor is the major number of a device
#       bMinor is the minor number of a device
*
*   Returns:
*       pid of a blocked process
#       0 if no process is blocked on this device
*
******************************************************************************/
int IsBlocked( BYTE bMajor, BYTE bMinor )
{
    int i;

    for( i=PROCESS_ID_MIN; i<PROCESS_ID_MAX; i++)
    {
        if( pProc[i]->Flags & PROCESS_WAITING_DEV )
        {
            if( pProc[i]->nWait == (bMajor<<8) + bMinor )
                return( i );
        }
    }

    return( 0 );
}


/******************************************************************************
*                                                                             *
*   void DevUnblock( int pid, int n )                                         *
*                                                                             *
*******************************************************************************
*
*   Unblocks a process pid and finishes system call
*
*   Where:
#       n is a returning parameter or error value
*
*   Returns:
*       void
*
******************************************************************************/
void DevUnblock( int pid, int n )
{
    // Store the return parameter

    pProc[pid]->Reg.eax = n;

    // Unblock the process

    pProc[pid]->Flags &= ~PROCESS_WAITING_DEV;
}

