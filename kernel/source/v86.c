/******************************************************************************
*                                                                             *
*   Module:     V86.c                                                         *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       9/21/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This module contains the code for V86 mode execution handling.  This
        file adds to the main code in `V86.asm'.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 9/21/97    Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "display.h"                    // Include display defines

#include "dpmi.h"                       // Include dpmi header file

#include "int.h"                        // Include interrupt header

#include "inline.h"                     // Include inline macros

#include "kernel.h"                     // Include kernel header file

#include "info.h"                       // Include information file header

#include "v86.h"                        // Include its own header

#include "tty.h"                        // Include terminal file header

#include "device.h"                     // Include device header file

#include "process.h"                    // Include its own header

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

// Address for V86 mode call termination

DWORD dwBreakSegOffs;

// Global register and segment structures

TReg Reg = { 0, 0, 0, 0, 0, 0, 0, 0 };
TSeg Seg = { 0, 0, 0, 0, 0, 0, 0, 0 };


// Array of pointers to functions to handle VM interrupt calls (INT XX).
// Client can hook these to simulate V86 interrupt handler.

BOOL (*IntXX_Handler[256])(TIntStack *pStack);


/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

#define ARPL        0x63                // Opcode for Arpl instruction

#define ROM         0xC0000             // Address of ROM area to search

#define FAKE_ROM    0x00000             // Fake address for ARPL

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

// The following functions are the interface to the assembly portion

extern void Reflect_Int( int nIntNum, TIntStack *pStack );

extern void V86_Int_Ret( TIntStack * pStack );

extern void Int13_VM_Handler();


// Local function prototypes

void Inv_Opcode_Handler( TIntStack Stack );

BOOL VMint10_Handler( TIntStack *pStack );

/******************************************************************************
*                                                                             *
*   void Init_V86()                                                           *
*                                                                             *
*******************************************************************************
*
*   This function initializes V86 module.  It finds the ARPL instruction
#   within the ROM area to use its address for V86-mode code termination.
*
******************************************************************************/
void Init_V86()
{
    DWORD dwBreakAddress;


    dprintf("\nInit Virtual-8086 mode");

    // Set the break address for VM execution

    dwBreakAddress = GetVMBreakAddress();

    // Store the break address in the selector:offset form

    dwBreakSegOffs = (dwBreakAddress & 0xFFFF)
                   | ((dwBreakAddress & 0xF0000) << 12);

    // Find and set the DPMI mode switch entry point function

    dwBreakAddress = GetVMBreakAddress();

    dwDPMISegOffs = (dwBreakAddress & 0xFFFF)
                  | ((dwBreakAddress & 0xF0000) << 12);

    // Reset the array of pointers to interrupt hooks

    memset( IntXX_Handler, 0, sizeof(IntXX_Handler) );

    // Register interrupt 13 handler to handle V86 mode exceptions that are
    // triggered when an V86 mode program executes a priviledged instruction

    Register_Exception_Handler( 0xD, Int13_VM_Handler );

    // Register interrupt 6 handler to handle invalid opcodes that are
    // used to exit V86 mode program via ARPL instruction

    Register_Exception_Handler( 6, Inv_Opcode_Handler );

    // Set the handler for BIOS int 10h to process some functions

    IntXX_Handler[0x10] = VMint10_Handler;

    // To support DPMI, we need to hook the multiplex interrupt 2F and DPMI
    // interrupt gate 31h.  Also, we need to monitor int 21h for the DPMI
    // exit (function 4Ch)

    IntXX_Handler[0x21] = Int21_Handler;
    IntXX_Handler[0x2F] = Int2F_Handler;
    IntXX_Handler[0x31] = Int31_Handler;

    dprintf(".");
}


/******************************************************************************
*                                                                             *
*   DWORD GetVMBreakAddress()                                                 *
*                                                                             *
*******************************************************************************
*
*   This function searches through the ROM area and looks for the ARPL
#   instruction.  When it finds one, it returns its linear address.
#   If the ARPL can not be found, it sets one in RAM.
#
#   This function may be called multiple times to get many break points.
*
*   Returns:
*       Linear address of an break instruction in VM address space.
*
******************************************************************************/
DWORD GetVMBreakAddress()
{
    static DWORD dwROM = ROM;
    static DWORD dwRAM = 0;

    // Find an ARPL instruction in ROM area

    for( ; dwROM<=ROM + 16384; dwROM += 1)
        if( abs_peekb(dwROM) == ARPL )
            break;

    if( dwROM >= ROM+16384 )
    {
        // We could not find the ARPL instruction in ROM (?!), so we need to
        // insert it manually at some address in RAM.  That is not good, but
        // we need it !

        abs_pokeb( dwRAM, ARPL );

        return( dwRAM++ );
    }

    return( dwROM++ );
}


/******************************************************************************
*                                                                             *
*   BOOL VMint10_Handler( TIntStack *pStack )                                 *
*                                                                             *
*******************************************************************************
*
*   This handler hooks into the INT 10h VM interrupt and controls some of
#   the functions.  E.g. we dont want to allow execution of video mode change.
#
#   Well, here we could virtualize VMs...
*
*   Where:
*       pStack pointer to a current stack frame
*
*   Returns:
*       True if the interrupt is to be reflected in VM
#       False if the interrupt should be ignored
*
******************************************************************************/
BOOL VMint10_Handler( TIntStack *pStack )
{
    BYTE ah;

    // Get the major function code in ah

    ah = (pStack->eax >> 8) & 0xFF;

    switch( ah )
    {
        case 0x00:                      // Can not switch video mode
        case 0x0B:                      // Can not set palette
        case 0x10:                      // Can not mess with the DAC or palette
        case 0x12:                      // Can not use alternate functions
            return( FALSE );
    }

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   void Inv_Opcode_Handler( TIntStack Stack )                                *
*                                                                             *
*******************************************************************************
*
*   This function handles the invalid opcode fault (6).  Since the invalid
#   opcode fault is also used for VM break address (via the arpl instruction)
#   this function will call the code to return to the PM caller if the fault
#   was due to that hook.
*
*   Where:
*       Stack is the stack frame of the current interrupted faulty process
*
*   Returns:
*       int 3
#       return to the PM caller if fault is at the break address
*
******************************************************************************/
void Inv_Opcode_Handler( TIntStack Stack )
{
    DWORD dwFaultAddress;


    // Check if the address of the fault is in fact our V86 break address

    dwFaultAddress = (Stack.cs << 16) | Stack.eip;

    if( dwFaultAddress == dwBreakSegOffs )
    {
        // Call the function to return to the PM caller

        V86_Int_Ret( &Stack );
    }
    else
    if( dwFaultAddress == dwDPMISegOffs )
    {
        // DPMI mode switch entry point

        DPMI_Switch( &Stack );

        Int3();
    }

    dprintf("\n%c%cInvalid opcode at %08X%c%c",
        DP_SETWRITEATTR, ATTR_SCREAM, dwFaultAddress,
        DP_SETWRITEATTR, ATTR_RESPONSE );

    PrintStackFrame( &Stack );

    Int3();
}


/******************************************************************************
*                                                                             *
*   void GPF_Handler( TIntStack Stack )                                       *
*                                                                             *
*******************************************************************************
*
*   This function handles the General Protection Fault
#
#   The Int13_VM_Handler is the first to handle interrupt 13.  It is located
#   in the V86.asm source.  If the cause of the fault was not V86 protected
#   instruction, GPF_Handler is called.
*
*   Where:
*       Stack is the stack frame of the current interrupted faulty process
*
*   Returns:
*       int 3
*
******************************************************************************/
void GPF_Handler( TIntStack Stack )
{
    dprintf("\n%c%cGeneral Protection Violation%c%c",
        DP_SETWRITEATTR, ATTR_SCREAM,
        DP_SETWRITEATTR, ATTR_RESPONSE );

    PrintStackFrame( &Stack );

    // Store the registers from the stack into the faulty process structure

    if( Stack.eflags & VM_MASK )
    {
        // Process was a virtual 86 machine

        memcpy( &pCurProc->Reg, &Stack, sizeof(TStack) );
    }
    else
    if( (Stack.cs & 3) == 3 )
    {
        // Process was a protected mode task

        memcpy( &pCurProcPM->Reg, &Stack, sizeof(TPMStack) );
    }

    Debug_Int3();
}


/******************************************************************************
*                                                                             *
*   BOOL Reflect_V86_Int( int nIntNum, TIntStack *pStack )                    *
*                                                                             *
*******************************************************************************
*
#   This function reflects the interrupt number nIntNum to the current V86
#   VM.  If the VM is not V86 (as read from the Stack->eflags) this function
#   returns FALSE.
#
#   Also, if the interrupts for the current VM are disabled, this function
#   returns FALSE.
#
#   If the interrupt stack can be correctly set to reflect the given
#   interrupt, this function returns TRUE.
*
*   Where:
#       nIntNum - interrupt number to reflect
#       pStack - pointer to a stack of the current interrupted V86 code
*
*   Returns:
#       TRUE if reflection is set up correctly
#       FALSE if the current interrupted code is not in V86 mode or
#           the virtual interrupts are disabled
*
******************************************************************************/
BOOL Reflect_V86_Int( int nIntNum, TIntStack *pStack )
{
    // Check if the current code is V86

    if( pStack->eflags & VM_MASK )
    {
        // Check if the virtual interrupts are enabled

        if( pStack->eflags & IF_MASK )
        {
            Reflect_Int( nIntNum, pStack );

            return( TRUE );
        }
    }

    // We could not reflect interrupt

    return( FALSE );
}

