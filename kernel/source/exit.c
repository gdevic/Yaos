/******************************************************************************
*                                                                             *
*   Module:     Exit.c                                                        *
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

        This module contains the code for the exit system call.
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

#include <sys\wait.h>                   // Include wait header

#include <errno.h>                      // Include error codes header

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
*   int Sys_Exit()                                                            *
*                                                                             *
*******************************************************************************
*
*   This is a exit system call.  It stores the exit status in the process
#   structure and looks if any process is waiting for it to revive it.
*
*   Where:
*       SYS_ARG1 is the exit status
*
*   Returns:
*       SYS_END
*
******************************************************************************/
int Sys_Exit()
{
    int i, status, pid;


    pid = pCurProcPM->pid;
    status = (pid << 16) | (SYS_ARG1 << 8);

    pCurProcPM->status = status;
    pCurProcPM->Flags |= PROCESS_ZOMBIE;

    // Look if any process is waiting for its termination.  It has to be the
    // parent of the current terminating process

    for( i=PROCESS_ID_MIN; i<PROCESS_ID_MAX; i++ )
        if( pProc[i] != NULL )
        if( (pProc[i]->Flags & PROCESS_WAITING_PID)
          &&(pProc[i]->pid == pCurProcPM->ppid) )
        {
            // Depending upon the waiting type, see if this process qualifies
            // nWait is one of the following:
            //     -1, to wait for any child process
            //     positive, to wait for the child with that specific pid
            //     zero, to wait for any child with the same process group id
            //     negative, to wait for any child with the specific -process gid

            if( pProc[i]->nWait == -1 )
                break;

            if( pProc[i]->nWait == pid )
                break;

            if( (pProc[i]->nWait == 0) && (pProc[i]->gid == pCurProcPM->gid) )
                break;

            if( -pProc[i]->nWait == pCurProcPM->gid )
                break;
        }

    // Look if we have found a waiting parent process

    if( i < PROCESS_ID_MAX )
    {
        // Unblock the parent and return the status

        pProc[i]->Reg.eax = status;
        pProc[i]->Flags &= ~PROCESS_WAITING_PID;

        // Clean up this child process

        DestroyProcess( pid );
    }

    // Did not found any waiting parent process - stay zombie

    // Reschedule the new process

    pCurProcPM = NULL;

    return( SYS_BLOCKED );
}


/******************************************************************************
*                                                                             *
*   int Sys_Wait()                                                            *
*                                                                             *
*******************************************************************************
*
*   This function performs the wait() system call.
#
#   It may block the process until the desired status can be obtained.
*
*   Where:
#       SYS_ARG1 is one of the following:
#           -1, to wait for any child process
#           positive, to wait for the child with that specific pid
#           zero, to wait for any child with the same process group id
#           negative, to wait for any child with the specific -process gid
*
#       SYS_ARG2 can be WNOHANG, not to suspend execution if the status is not
#           immediately available
#           WUNTRACED, report the status of a stopped child process (unused)
#
*   Returns:
#       SYS_END when call is finished
#       SYS_RET is the status of a child process
*
******************************************************************************/
int Sys_Wait()
{
    int i, child = 0, zombie = 0;
    int sys = (signed) SYS_ARG1;

    // Switch on a waiting request

    if( sys == -1 )
    {
        // Search for any child process

        for( i=PROCESS_ID_MIN; i<PROCESS_ID_MAX; i++ )
            if( pProc[i] != NULL )
            if( (pProc[i]->ppid == pCurProcPM->pid) )
            {
                child = i;
                if( (pProc[i]->Flags & PROCESS_ZOMBIE) ) zombie = i;
            }
    }
    else
    if( sys > 0 )
    {
        // Search for a child with the specific pid

        for( i=PROCESS_ID_MIN; i<PROCESS_ID_MAX; i++ )
            if( pProc[i] != NULL )
            if( (pProc[i]->pid == SYS_ARG1 )
              &&(pProc[i]->ppid == pCurProcPM->pid) )
            {
                child = i;
                if( (pProc[i]->Flags & PROCESS_ZOMBIE) ) zombie = i;
            }
    }
    else
    if( sys == 0 )
    {
        // Search for any child with the same process group id

        for( i=PROCESS_ID_MIN; i<PROCESS_ID_MAX; i++ )
            if( pProc[i] != NULL )
            if( (pProc[i]->gid == pCurProcPM->gid )
              &&(pProc[i]->ppid == pCurProcPM->pid) )
            {
                child = i;
                if( (pProc[i]->Flags & PROCESS_ZOMBIE) ) zombie = i;
            }
    }
    else
    if( sys < 0 )
    {
        // Search for any child with the specific group id

        for( i=PROCESS_ID_MIN; i<PROCESS_ID_MAX; i++ )
            if( pProc[i] != NULL )
            if( (pProc[i]->gid == -SYS_ARG1 )
              &&(pProc[i]->ppid == pCurProcPM->pid) )
            {
                child = i;
                if( (pProc[i]->Flags & PROCESS_ZOMBIE) ) zombie = i;
            }
    }
    else
        sys_end( ENOSYS );

    // If there were no children found, return with error

    if( child == 0 )  sys_end( ECHILD );

    // If there were some children running, but no info available, either
    // return or suspend and wait

    if( zombie == 0 )
    {
        // If the option WNOHANG is specified and there were no info available
        // return

        if( SYS_ARG2 & WNOHANG )  sys_end( ECHILD );

        // Set up the waiting state and the group id to wait for

        pCurProcPM->Flags |= PROCESS_WAITING_PID;
        pCurProcPM->nWait = SYS_ARG1;

        // Return with blocking

        return( SYS_BLOCKED );
    }

    // Found a zombie child, return its status and clean it up

    SYS_RET = pProc[zombie]->status;

    // Clean up the process `zombie'

    DestroyProcess( zombie );

    // Return without blocking

    return( SYS_END );
}
