/*********************************************************************
*                                                                    *
*   Module:     interrpt.h
*                                                                    *
*   Revision:   1.00                                                 *
*                                                                    *
*   Date:       06/24/95                                             *
*                                                                    *
*   Author:     Goran Devic                                          *
*                                                                    *
**********************************************************************
*                                                                    *
*   Module Description:                                              *

          Header file for the interrupt subsystem (interrpt.c)
          
**********************************************************************
*                                                                    *
*   Changes:                                                         *
*                                                                    *
*   DATE     REV   DESCRIPTION OF CHANGES                 AUTHOR     *
* --------   ----  -----------------------------------   -----------
  06/24/95   1.00  Original                              Goran Devic

* --------   ----  -----------------------------------   ----------- *
**********************************************************************
*   Important defines/undefines
**********************************************************************/
#ifndef _INTERRPT_H_
#define _INTERRPT_H_

/*********************************************************************
*   Include Files
**********************************************************************/
#include "types.h"              // Include basic data types

/*********************************************************************
*   Header Defines
**********************************************************************/

/*********************************************************************
*   Used with SetIDTEntry( BYTE bNum, *, * )
**********************************************************************/
// CPU-generated interrupts
#define INT_DIVIDE             0x00        // Divide error, divide with zero
#define INT_DEBUG              0x01        // Single step, debug exception
#define INT_NMI                0x02        // Non-maskable interrupt
#define INT_BREAKPOINT         0x03        // Breakpoint by CCh instruction
#define INT_INTO               0x04        // INTO and Overflow flag set
#define INT_BOUND              0x05        // Bound range exceeded
#define INT_INVALID_OPCODE     0x06        // Invalid opcode interrupt
#define INT_COPROCESSOR        0x07        // No coprocessor present
#define INT_DOUBLE_FAULT       0x08        // Multiple exception
#define INT_SEG_OVERRUN        0x09        // Coprocessor faults
#define INT_TSS                0x0A        // Invalid task state segment
#define INT_SEG_NOT_PRESENT    0x0B        // Segment is not present
#define INT_STACK              0x0C        // Stack exception
#define INT_GPF                0x0D        // General protection violation
#define INT_PAGE_FAULT         0x0E        // Page not present

// External interrupts
#define IRQ_TIMER              0
#define IRQ_KEYBOARD           1
#define IRQ_IRQ2               2
#define IRQ_COM2               3
#define IRQ_COM1               4
#define IRQ_HDD                5
#define IRQ_FLOPPY             6
#define IRQ_PRINTER            7

#define IRQ_0                  0
#define IRQ_1                  1
#define IRQ_2                  2
#define IRQ_3                  3
#define IRQ_4                  4
#define IRQ_5                  5
#define IRQ_6                  6
#define IRQ_7                  7
#define IRQ_8                  8
#define IRQ_9                  9
#define IRQ_A                  10
#define IRQ_B                  11
#define IRQ_C                  12
#define IRQ_D                  13
#define IRQ_E                  14
#define IRQ_F                  15


/*********************************************************************
*   Used with SetIDTEntry( *, *, WORD wType )
**********************************************************************/
#define IDT_TRAP       0x8E00       // Software INT X instruction
#define IDT_INTERRUPT  0x8F00       // Hardware interrupts (0-14)

/*********************************************************************
*   Used with IRQControl( BYTE bEnable, * )
**********************************************************************/
#define IRQ_ENABLE     1
#define IRQ_DISABLE    0


/*********************************************************************
*   Local Macros
**********************************************************************/
#pragma aux EnableInterrupts = "sti" parm;
#pragma aux DisableInterrupts = "cli" parm;


/*********************************************************************
*   Structures
**********************************************************************/
typedef struct                 // Interrupt gate structure
{                              // ========================
   WORD wOffsetLow;            // Address offset [15:0]
   WORD wSelector;             // Segment code selector
   union                       //
   {                           //
       WORD wType;             // Type of the interrupt gate
                               //
       struct                  //         or
       {                       //
       WORD res : 13;          // Reserved bits: must be 0xf0
       WORD dpl : 2;           // Descriptor privilege level
       WORD present : 1;       // Present bit
       };                      //
   };                          //
   WORD wOffsetHigh;           // Address offset [31:16]

} IntGateStruct;


/*********************************************************************
*   Function Prototypes
**********************************************************************/
extern void EnableInterrupts();
extern void DisableInterrupts();

extern BOOL RegisterInterruptHandler( BYTE bIntNum, void (*fn)(void), WORD wType );
extern BOOL UnregisterInterruptHandler( BYTE bIntNum );
extern BOOL RegisterIRQHandler( BYTE bIRQNum, void (*fn)(void) );
extern BOOL UnregisterIRQHandler( BYTE bIRQNum );
extern void IRQControl( BYTE bEnable, BYTE bIRQNum );
extern interrupt DummyInt();
extern void V86Interrupt( BYTE bIntNum );


#endif // _INTERRPT_H_
