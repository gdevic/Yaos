/******************************************************************************
*                                                                             *
*   Module:     Int.c                                                         *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       9/7/97                                                        *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

    This module contains the C portion of the code for the interrupt handling.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 9/7/97     1.00  Original                                       Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/
#include <stdio.h>                      // Include standard I/O header file

#include "inline.h"                     // Inline functions

#include "pc.h"                         // Hardware specific defines


/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

BYTE bSaved8259;                        // State of the master interrupt mask


/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

extern void (*Exception_Table[])();
extern void (*IRQ_Table[16])();

extern DWORD IDT_Descriptor;           // Address of the IDT Descriptor

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   void Init_Interrupts()                                                    *
*                                                                             *
*******************************************************************************
*
*   This function initializes interrupts
*
*   Returns:
*       global variable `bSaved8259' the state of the master interrupt mask
*
******************************************************************************/
void Init_Interrupts()
{
    dprintf("\nInit interrupts");

    // Save the state of the master PIC mask

    bSaved8259 = inp(PIC1_MASK);

    // Load IDT structure

    LoadIDT( &IDT_Descriptor );

    // Enable keyboard interrupt (bit of 0 - enable)

//    outp(PIC1_MASK, ~0x2 );
//    outp(PIC1_MASK, ~(2 | 1) );
//    outp(PIC1_MASK, ~((1<<0) | (1<<1) | (1<<6)) );

    outp(PIC1_MASK, ~0xFF );
    outp(PIC2_MASK, ~0xFF );

    dprintf(".");
}



/******************************************************************************
*                                                                             *
*   Register_Exception_Handler( int IntNum, void (* handler)() )              *
*                                                                             *
*******************************************************************************
*
*
*
*   Where:
*
*
*   Returns:
*
*
******************************************************************************/
void Register_Exception_Handler( int IntNum, void (* handler)() )
{
    Exception_Table[ IntNum ] = handler;
}


/******************************************************************************
*                                                                             *
*   Register_Interrupt_Handler( int IRQNum, void (* handler)() )              *
*                                                                             *
*******************************************************************************
*
*
*
*   Where:
*
*
*   Returns:
*
*
******************************************************************************/
void Register_Interrupt_Handler( int IRQNum, void (* handler)() )
{
    IRQ_Table[ IRQNum ] = handler;
}


