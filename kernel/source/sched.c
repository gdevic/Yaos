/******************************************************************************
*                                                                             *
*   Module:     Sched.c                                                       *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       10/22/97                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This module contains the code for the process scheduler.  Only PM
        processes are scheduled using the preemptive, round-robin scheme.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 10/22/97   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "inline.h"                     // Port operations, interrupts

#include "intel.h"                      // Include processor defines

#include "kernel.h"                     // Include kernel data structures

#include "pc.h"                         // Include hardware specific defines

#include "tty.h"                        // Include terminal file header

#include "device.h"                     // Include device header file

#include "process.h"                    // Include its own header

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
*   void Scheduler( TIntStack Stack )                                         *
*                                                                             *
*******************************************************************************
*
*   This function is hooked to the timer interrupt and it performs the
#   scheduling of PM processes.
*
*   Where:
*       Stack frame of currently running process
*
*   Returns:
*       void
*
******************************************************************************/
void Scheduler( TIntStack Stack )
{
    static x = 'O';

    // If we have interrupted V86 mode execution, we need to reflect this
    // interrupt `down' so that the current VM would get it

    if( Reflect_V86_Int( 8, &Stack )==FALSE )
    {
        // Keyboard interrupts has not been sent to a VM - that means we
        // have interrupted either kernel or a PM process.

        // We will schedule only if a PM process is running

        if( (Stack.cs & 3) == 3 )
        {
abs_pokew( 0xB8000 + 79*2, x + 0x700 );
x = x=='O'?'X':'O';

            // First, ack the interrupt controller

            outp( PIC1, PIC_ACK );

            // PM process is interrupted, reschedule it

            Reschedule( (TPMStack *) &Stack );

            return;
        }
    }

    // Ack the interrupt controller

    outp( PIC1, PIC_ACK );
}



/******************************************************************************
*                                                                             *
*   int Reschedule( TPMStack *pStack )                                        *
*                                                                             *
*******************************************************************************
*
*   This function reschedules a current PM process by the virtue of pCurProcPM
#   variable.
*
*   Where:
*       pStack is the pointer to a current interrupted process stack frame.
#       pCurProcPM must point to a process control block of that process.
*
*   Returns:
#       New process ID number
#
*       Values on the stack are swapped with a new running process
#       pCurProcPM points to a new running process.
*
******************************************************************************/
int Reschedule( TPMStack *pStack )
{
    int i;

    // Copy the stack frame to the process control block

    if( pCurProcPM != NULL )
    {
        memcpy( &pCurProcPM->Reg, pStack, sizeof(TPMStack) );

        i = pCurProcPM->pid + 1;
    }
    else
        i = PROCESS_ID_MIN;

    // Find another PM process that is not waiting - round robin

    while( 1 )
    {
        if( i==PROCESS_ID_MAX )
            i = PROCESS_ID_MIN;

        // Find a PM process that is ready to run...

        if( pProc[i]->Flags & PROCESS_PM )
        {
            // Process must not be blocked in any way

            if( (pProc[i]->Flags & PROCESS_BLOCKED) == 0 )
            {
                // Found it!  Set the new process pointer

                pCurProcPM = pProc[i];

                // Increase the time slice count

                pCurProcPM->slices++;

                // Copy the stack structure so we return to it

                memcpy( pStack, &pCurProcPM->Reg, sizeof(TPMStack) );

                // Set the process LDT

                SetLDT( pCurProcPM->LDT );

                // Return with the process ID

                return( pCurProcPM->pid );
            }
        }

        i++;
    }
}

