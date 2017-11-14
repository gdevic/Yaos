/******************************************************************************
*                                                                             *
*   Module:     dpmi.c                                                        *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       10/4/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This module contains the code for the DPMI 0.9 support for virtual
        DOS machine under YAOS.

        DPMI ...
        This is something to be done.....
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 10/4/97    Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "dpmi.h"                       // Include its own header file

#include "inline.h"                     // Include inline macros

#include "intel.h"                      // Include Intel specific defines

#include "kernel.h"                     // Include kernel header file

#include "mm.h"                         // Include memory management header

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/


DWORD dwDPMISegOffs;                    // Address of the DPMI mode-switch fn


/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

#define DPMI_MAX_LDT        30          // Maximum number of LDT entries

#define DPMI_LIMIT          65535       // Limit for a PM client

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

extern void SetupSelector( TDesc *pDesc, DWORD dwBase, DWORD dwLimit,
                           BYTE Type, BOOL fSystem, BYTE DPL );


/******************************************************************************
*                                                                             *
*   BOOL Int2F_Handler( TIntStack *pStack )                                   *
*                                                                             *
*******************************************************************************
*
*   This is the multiplex interrupt handler in support to DPMI host.
*
*   Where:
*       pStack pointer to a current stack frame
*
*   Returns:
*       True if the interrupt is to be reflected in VM
#       False if the interrupt should be ignored
*
******************************************************************************/
BOOL Int2F_Handler( TIntStack *pStack )
{
    WORD ax = pStack->eax & 0xFFFF;


    if( ax == 0x1686 )
    {
        // Detect mode function returns 0 in eax to signal the DPMI is already
        // operating in protected mode

        pStack->eax = 0;

        return( FALSE );
    }

    if( ax == 0x1687 )
    {
        // DMPI Installation check

dprintf("\nInt 2F Called");

        pStack->eax = 0;                // DPMI server installed
        pStack->ebx = 1;                // 32-bit mode support
        pStack->ecx = 0x0004;           // Assume Intel 80486
        pStack->edx = 0x005A;           // DPMI 0.90
        pStack->esi = 0;                // Private data

        pStack->v86_es  = dwDPMISegOffs >> 16;
        pStack->edi = dwDPMISegOffs & 0xFFFF;

        return( FALSE );

    }

    // By the way, use AX=163F to detect YAOS.  Returns the version
    // in the DX register (DH=major/DL=minor)

    if( ax == 0x163F )
    {
        pStack->eax = 0;
        pStack->edx = 0x0200;           // YAOS 2.0

        return( FALSE );
    }

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL Int21_Handler( TIntStack *pStack )                                   *
*                                                                             *
*******************************************************************************
*
*   This is the interrupt 21 handler in support to DPMI.
*
*   Where:
*       pStack pointer to a current stack frame
*
*   Returns:
*       True if the interrupt is to be reflected in VM
#       False if the interrupt should be ignored
*
******************************************************************************/
BOOL Int21_Handler( TIntStack *pStack )
{
    if( (pStack->eax & 0xFF00) == 0x4C )
    {
        // If the process running is a protected mode DPMI, reset its state

        if( (pStack->eflags & VM_MASK) != 0 )
        {
            DPMI_Exit( pStack );
        }
    }

    // Reflect all other services

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   void DPMI_Switch( TIntStack *pStack )                                     *
*                                                                             *
*******************************************************************************
*
*   This function switches a DPMI client into protected mode.  It is called
#   after the client called our DMPI VM hook.
*
*   Where:
*       Stack is the stack frame of the current interrupted process
*
*   Returns:
*       void
*
******************************************************************************/
void DPMI_Switch( TIntStack *pStack )
{
    TDesc *pLDT;
    DWORD RetSegOffs;
    int sel;

    dprintf("\nTrying to switch");


    // Allocate memory for the LDT table

    pLDT = (TDesc *) MALLOC( sizeof(TDesc) * DPMI_MAX_LDT );

    if( pLDT != NULL )
    {
        sel = 0;

        // Set up the Local descriptors:

        memset( pLDT, 0, sizeof(TDesc) * DPMI_MAX_LDT );

        // Set up code selector

        SetupSelector( &pLDT[sel++],
                        pStack->cs << 4,
                        DPMI_LIMIT,
                        DESC_TYPE_EXEC, FALSE, 3 );

        // Set up data selector

        SetupSelector( &pLDT[sel++],
                        pStack->v86_ds << 4,
                        DPMI_LIMIT,
                        DESC_TYPE_DATA, FALSE, 3 );

        // Set up stack selector only if it differs from data segment

        if( pStack->ss != pStack->v86_ds )
        {
            SetupSelector( &pLDT[sel++],
                            pStack->v86_ds << 4,
                            DPMI_LIMIT,
                            DESC_TYPE_DATA, FALSE, 3 );
        }

        // Insert the ldt pointer into the gdt at the pid position

//        SetupSelector( &GDT[pCurProc->pid],
//                        (DWORD)(LIN_KERNEL + (int)pLDT),
//                        DPMI_MAX_LDT * 8 - 1,
//                        DESC_TYPE_LDT, TRUE, 3 );

        // The returning LDT is in fact the pid, that is the index into GDT.

    }
    else
    {
        // Could not allocate memory


    }

    // Set up the returning address from the VM stack

    RetSegOffs = abs_peekdw( (pStack->ss << 4) + pStack->esp);

    pStack->esp = pStack->esp - 4;
    pStack->eip = RetSegOffs & 0xFFFF;

    // Use segment to set up selector

    pStack->cs = 0;///

    // Set up selectors

//    pStack->cs = Sel_Code;
//    pStack->v86_ds = Sel_Data;
//    pStack->ss = Sel_Stack;
//    pStack->v86_es = Sel_Psp;
//    pStack->v86_fs = pStack->gs = 0;

    // Turn off the VM bit.  That will make a returning program PM

    pStack->eflags &= ~VM_MASK;

}


/******************************************************************************
*                                                                             *
*   void DPMI_Exit( TIntStack *pStack )                                       *
*                                                                             *
*******************************************************************************
*
*   This function terminates protected mode excution.
*
*   Where:
*       Stack is the stack frame of the current interrupted process
*
*   Returns:
*
*
******************************************************************************/
void DPMI_Exit( TIntStack *pStack )
{
    dprintf("\nExit");
}


/******************************************************************************
*                                                                             *
*   BOOL Int31_Handler( TIntStack *pStack )                                   *
*                                                                             *
*******************************************************************************
*
*   This function is the hook for Int 31h, DPMI server.
*
*   Where:
*       pStack pointer to a current stack frame
*
*   Returns:
*       True if the interrupt is to be reflected in VM
#       False if the interrupt should be ignored
*
******************************************************************************/
BOOL Int31_Handler( TIntStack *pStack )
{
    return( FALSE );
}


