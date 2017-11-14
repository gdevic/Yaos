/******************************************************************************
*                                                                             *
*   Module:     Init.c                                                        *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       8/4/97                                                        *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

       This module contains the kernel init code.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 8/4/97     1.00  Original                                       Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "device.h"                     // Include devices header file

#include "display.h"                    // Debugger display device

#include "dosx.h"                       // Include dos extender header file

#include "file.h"                       // Include file system header

#include "inline.h"                     // Inline functions

#include "int.h"                        // Interrupt functions

#include "kernel.h"                     // Include kernel data structures

#include "keyboard.h"                   // Include keyboard defines

#include "mm.h"                         // Include memory management header

#include "tty.h"                        // Include terminal file header

#include "process.h"                    // Include process header

#include "pc.h"                         // Include hardware specific defines

#include "v86.h"                        // Include virtual 86 machine header

#include "assertk.h"                    // Include kernel assert macro

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

char sCmd[80];

int sys_pid = 0;                        // System VM process ID

// Process execution group ID is only incremented for each new group.  (This
// is different from the file gid).

int next_gid = 10;                      // Next available process group ID

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

extern void Deb_Keyboard_Handler( TIntStack Stack );

extern void Page_Fault_Handler( TIntStack Stack );

extern void DebugOut( char *sFormat, ... );

extern void Scheduler( TIntStack Stack );

extern BOOL VM_RefreshScreen;

KernelDie( char* sFile, int nLine )
{
    dprintf("\nKERNEL PANIC - ABORT AT `%s' line %d\n", sFile, nLine );
    Int3();
}


/******************************************************************************
*                                                                             *
*   void _assertk(char *cond, char *msg, char *file, int line)                *
*                                                                             *
*******************************************************************************
*
*   This is a kernel assert function.  It prints a message and exits into the
#   debugger via INT 3 call.
*
*   Where:
*       cond is the string describing the assert condition that failed
#       msg is the program-supplied message string, may be NULL
#       file is the file name
#       line is the line number
*
*   Returns:
*       Breaks into the debugger
*
******************************************************************************/
void _assertk(char *cond, char *msg, char *file, int line)
{
    dprintf("\nAssertion `%s' failed in %s line %d", cond, file, line );
    if( msg != NULL )
        dprintf(": %s", msg);

    // Exit into the debugger

    Int3();
}


/******************************************************************************
*                                                                             *
*   main()                                                                    *
*                                                                             *
*******************************************************************************
*
*   This is the main C-entry point.
*
*   Where:
*       no args
*
*   Returns:
*       returns upon unload from the debugger
*
******************************************************************************/
void kmain()
{
    char *pArgs[] =
    {
       "Init", NULL
    };

    char *pEnviron[] =
    {
        "HOME=/", "Path=::.:/Yaos::/Tmp:.:/", "TERM=VT100", NULL
    };

    int pid, init_pid;


    // This step is just cosmetics - set consistent screen attribute

    abs_memsetw( LIN_SCREEN + 160 * 12, ATTR_SYSTEM << 8, 80 * (50-12-12) );
    dprintf("%c%c%c%c%c", DP_SETCURSOR, 0, 11, DP_SETWRITEATTR, ATTR_SYSTEM );

    // Before all, init memory manager.  This call will initialize kernel heap

    Init_MM();

    // Initialize debugger data structures

    Init_Debug();

    // Initialize file system

    Init_FS();

    // Initialize Task State Segment

    Init_TSS();

    // Initialize interrupts subsystem

    Init_Interrupts();

    // Register interrupt handler function for keyboard handler

    Register_Interrupt_Handler( 0x1, Int_Keyboard_Handler );

    // Initialize the virtual mode subsystem

    Init_V86();

    // Set the display mode to use an 8x8 character set and 50 lines

    dprintf("\nUsing a V86 call to set the video mode");

    Reg.eax = 0x1112;
    Reg.ebx = 0x0000;
    V86_Int( 0x10, &Seg, &Reg, NULL );

    dprintf(".");


    Init_Process();

    // Register process module and page fault handler

    Register_Exception_Handler( 14, Page_Fault_Handler );

    // Register the process scheduler

    Register_Interrupt_Handler( 0, Scheduler );

    EnableInterrupts();

//-----------------------------------------------------------------------------
//  Create and initialize system virtual machine
//-----------------------------------------------------------------------------

    dprintf("\nCreating a system virtual machine");

    sys_pid = CreateVM();
    pProc[ sys_pid ]->Flags |= PROCESS_SYSTEM_VM;
    ProcessHotKey[ SF1 ] = sys_pid;

    dprintf(" and setting it up");

    // Init DOS extender using the system VM

    SetCurrentVM( sys_pid, NULL );

    dprintf(".");


    Init_DosX();


    // Initialize devices
    // ------------------
    // This call must come after Init_V86() and Init_DosX() since it calls
    // individual devices' init routines that may use DOS extender and/or
    // V86 memory

    Init_Dev();

    // Load INIT process

    dprintf("\nLoad `init' process");

    init_pid = do_execve("Test.exp", pArgs, pEnviron );

    dprintf(".");

    if( init_pid < 0 )
    {
        dprintf("%c%c", DP_SETWRITEATTR, ATTR_SYSTEMHI );
        dprintf("\nCannot load init process.  Error %d", init_pid );
        dprintf("\nPress F5 to run VM...");
        dprintf("%c%c", DP_SETWRITEATTR, ATTR_SYSTEM );

        Int3();
    }
    else
    {
        // Make that process current

        pCurProcPM = pProc[init_pid];

        // Set the system VM process registers to start executing init process
        // when activated

        memcpy( &pProc[sys_pid]->Reg, &pProc[init_pid]->Reg, sizeof(TIntStack) );
    }

//-----------------------------------------------------------------------------
//  Create additional DOS virtual machines
//-----------------------------------------------------------------------------
//    dprintf("\nAllocating VMs");

    // Allocate 2 VM processes and assign hotkeys

    pid = CreateVM();
    ProcessHotKey[ SF2 ] = pid;

    pid = CreateVM();
    ProcessHotKey[ SF3 ] = pid;

//-----------------------------------------------------------------------------
//  Set up the stack for the jump into System VM to start executing init proc
//-----------------------------------------------------------------------------

    SubESP( sizeof(TIntStack) );

    pCurProc = NULL;
    SetCurrentVM( sys_pid, (TIntStack *) GetESP() );
    VM_RefreshScreen = TRUE;

    LiftOff();

    // Catch the execution if something went wrong

    dprintf("\nCould not execute init process!");
    while( 1 ) KERNEL_DIE;
}

