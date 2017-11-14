/******************************************************************************
*                                                                             *
*   Module:     kernel.c                                                      *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       08/26/96                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This module contains the basic kerner initialization code.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 08/26/96   1.00  Original                                       Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/
#include "debugger.h"                   // Include debugger types
#include "interrpt.h"                   // Include interrupt header
#include "kernel.h"                     // Include kernel types
#include "mm.h"                         // Include memory management header
#include "sched.h"                      // Include scheduler header
#include "timers.h"                     // Include timers header


#include "stdlib.h"
#include "dis386.h"
#include "types.h"
#include "ports.h"                      // Include basic ports I/O



/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/
REGS TmpRegs;


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

/*********************************************************************
*
*   void KernelPanic( char *pszMessage, ... )
*   
*   Whenever there is an error in the kernel code, this function is
*   called.  The arguments is a normal, printf-like argument list
*   that will be printed on the kernel console for debugging
*   purposes.
*
*   Where:
*       pszMessage, ...  debug message describing the function where
*           error has occurred and the additional parameters such as
*           invalid argument or permission.
*
*   Returns:
*       This function returns to internal debugger.
*
**********************************************************************/
void KernelPanic( char *pszMessage, ... )
{
    // More code here...

    printk("KERNEL PANIC %s\n", pszMessage);
    
    INT1();
}


void Halt( char *pszMessage, ... )
{
    printk("SYSTEM HALTED: %s\n", pszMessage);

    DisableInterrupts();
    while( 1 ){}
}


interrupt FloppyIntHandler()
{
    static char letter = 'F';

    AbsPokew( 0xb8000+78*2, 0x700 + letter );
    if( letter=='F' )
        letter=' ';
    else
        letter='F';

#if 0
    // Just call back IRQ 6 BIOS handler
    //
    V86Interrupt( 8 + 0x6 );
    PassV86Interrupt( 8 );
#else
    // Ack the interrupt controller
    //
    Outpb( 0x20, 0x20 );
#endif
}

interrupt TimerIntHandler()
{
    static char letter = 'T';

    AbsPokew( 0xb8000 + 79*2, 0x700 + letter );
    if( letter=='T' )
        letter=' ';
    else
        letter='T';



#if 0
    // Just call back IRQ 0 BIOS handler
    //
    PassV86Interrupt( 8 );
//    Outpb( 0x20, 0x20 );
#else
    // Ack the interrupt controller
    //
    Outpb( 0x20, 0x20 );
#endif
    PassV86Interrupt( 8 );
}

/*********************************************************************
*
*   Yaos
*
*   The main kernel "C" entry point
*
**********************************************************************/
void Yaos()
{
    REGS regs;


    InitTTYs();

    InitInterrupts();

    InitTimers();

    // Init the real timer handler
    if( !RegisterIRQHandler( IRQ_TIMER, (void(*)()) TimerIntHandler ) )
       Halt("Unable to register timer handler");
    IRQControl( IRQ_ENABLE, IRQ_TIMER );
    printk("Timer\n");

    // Init interrupt 13 handler
    if( !RegisterInterruptHandler( INT_GPF, (void(*)()) Int13_Handler, IDT_INTERRUPT ) )
       Halt("Unable to register GPF handler");
    printk("GPF Handler\n");

    InitDebugger();

    EnableInterrupts();
    printk("Interrupts Enabled\n");

    // Set ROM 8x8 Character Set
    //
    regs.w.ax = 0x1112;
    regs.b.bl = 0;
    Intv86( 0x10, &regs );


    InitMemoryManager( &HeapStart );

    InitSched();
    printk("Process scheduler\n");


    // Cursor shape
    //
    regs.w.ax = 0x0100;
    regs.w.cx = 0x0108;
    Intv86( 0x10, &regs );


    printk("\nReady\n");


    // Finish up by trapping in the debugger
    //
    INT1();
}



