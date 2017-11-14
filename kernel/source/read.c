/******************************************************************************
*                                                                             *
*   Module:     Read.c                                                        *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       11/4/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This module contains the code for the read system call.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 11/4/97    Original                                             Goran Devic *
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

// Queue of pid's that are blocked on read

QQ(q_read,32);


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   int Sys_Read()                                                            *
*                                                                             *
*******************************************************************************
*
*   This is a read system call function.
*
*   Where:
#       SYS_ARG1 is the process file descriptor
#       SYS_ARG2 is the address to put the string that should be read
#       SYS_ARG3 is the number of bytes to read
*
*   Returns:
*       SYS_BLOCKED if read cannot be accomplished at this time
#       SYS_END if read is finished
*
******************************************************************************/
int Sys_Read()
{
    char *pBuf;
    int i, read;

//dprintf("\nRead %X %X %X", SYS_ARG1, SYS_ARG2, SYS_ARG3 );

    pBuf = CheckProcessString( (char *)SYS_ARG2, WRITE_BUF_LEN );

    if( pBuf == NULL ) sys_end( EFAULT );
    if( SYS_ARG1 >= FOPEN_MAX ) sys_end( EBADF );
    if( (i = pCurProcPM->iFile[SYS_ARG1]) == 0 ) sys_end( EBADF );
    if( Dev[Files[i].bMajor].dev_read == NULL ) sys_end( EIO );

    // Now we have an index to the kernel file table.
    // Call a device to perform the read operation

//dprintf("\nDev %08X", Dev[Files[i].bMajor].dev_read );

    pCurProcPM->Rq.s = (char *) &Files[i].Stat;
    pCurProcPM->Rq.p = pBuf;            // Read into this buffer
    pCurProcPM->Rq.n = SYS_ARG3;        // Length of a buffer

    read = (Dev[Files[i].bMajor].dev_read)(Files[i].bMinor, &pCurProcPM->Rq);

    // See if the process needs to be blocked for this device

    if( read==EAGAIN )
    {
        pCurProcPM->Flags |= PROCESS_WAITING_DEV;
        pCurProcPM->nWait = Files[i].bMajor * 256 + Files[i].bMinor;

        QQEnqueue(q_read, pCurProcPM->pid );

        return( SYS_BLOCKED );
    }

    sys_end( read );
}


/******************************************************************************
*                                                                             *
*   int IsBlocked_Read( TDevRq **pRq )                                        *
*                                                                             *
*******************************************************************************
*
*   Checks if a process is blocked on read.  If pRq is not NULL and a process
#   is waiting for read, pRq will be set to the address of the process device
#   request buffer containing the request address and length.
*
*   Where:
*       pRq is the address of a pointer to the device request buffer
*
*   Returns:
*       0 if no process is in queue waiting on read
#       pid of a process in wait read queue
*
******************************************************************************/
int IsBlocked_Read( TDevRq **pRq )
{
    int pid;

    // If no process is queued for read, return zero

    if( QQIsEmpty(q_read) )
        return( 0 );

    // Just peek a process number, do not dequeue it (use Unblock_Read() to
    // dequeue this process).

    pid = QQPeek(q_read);

    if( pRq != NULL )
    {
        *pRq = &pProc[pid]->Rq;
    }

    return( pid );
}


/******************************************************************************
*                                                                             *
*   void Unblock_Read()                                                       *
*                                                                             *
*******************************************************************************
*
*   This function dequeues a process from the wait reading queue.  Use
#   IsBlocked_Read() to find out if, and which, process is queued.
*
*   Where:
*       no args
*
*   Returns:
*       void
*
******************************************************************************/
void Unblock_Read()
{
    QQDequeue(q_read);
}

