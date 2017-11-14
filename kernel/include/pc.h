;/*****************************************************************************
comment ~                                                                     #
*******************************************************************************
*                                                                             *
*   Module:     PC.h                                                          *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       9/19/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This is a header file containing defines specific to the IBM PC
        and 100% compatibles architecture.

        This file may be included in the "C" code and in the assembly code
        exactly as it is.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 9/19/97    Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _PC_H_
#define _PC_H_

/******************************************************************************
*                                                                             *
*   Include Files                                                             *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

//-----------------------------------------------------------------------------
// PIC1 - Programmable Interrupt Controller defines (8259)
//-----------------------------------------------------------------------------

#define PIC1                0x20
#define PIC1_MASK           0x21
#define PIC_ACK             0x20
#define PIC_READ_IRQ        0x0B

//-----------------------------------------------------------------------------
// PIC2 - Programmable Interrupt Controller defines (8259)
//-----------------------------------------------------------------------------

#define PIC2                0xA0
#define PIC2_MASK           0xA1

//-----------------------------------------------------------------------------
// Keyboard Controller (8041)
//-----------------------------------------------------------------------------

#define KBD_DATA            0x60
#define KBD_CONTROL         0x61

//-----------------------------------------------------------------------------
// Real Time Clock command port
//-----------------------------------------------------------------------------

#define RTC_CMD             0x70
#define DISABLE_NMI         0x80
#define ENABLE_NMI          0x00

//-----------------------------------------------------------------------------
// Port that causes slight delay without side effects
//-----------------------------------------------------------------------------

#define PORT_DUMMY          0x80

/******************************************************************************
*
* Assembler defines
*
*******************************************************************************
end comment ~

;------------------------------------------------------------------------------
; PIC - Programmable Interrupt Controller defines (8259)
;------------------------------------------------------------------------------

PIC1            Equ         020h
PIC1_MASK       Equ         021h
PIC_ACK         Equ         020h
PIC_READ_IRQ    Equ         00Bh

;------------------------------------------------------------------------------
; Keyboard Controller (8041)
;------------------------------------------------------------------------------

KBD_DATA        Equ         060h
KBD_CONTROL     Equ         061h

;------------------------------------------------------------------------------
; Real Time Clock command port
;------------------------------------------------------------------------------

RTC_CMD         Equ         70h
DISABLE_NMI     Equ         80h
ENABLE_NMI      Equ         00h

;------------------------------------------------------------------------------
; Port that causes slight delay without side effects
;------------------------------------------------------------------------------

PORT_DUMMY      Equ         080h


comment ~ */
#endif //  _PC_H_
/*
end comment ~
;*/
