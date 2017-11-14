;/*****************************************************************************
comment ~                                                                     #
*******************************************************************************
*                                                                             *
*   Module:     Intel.h                                                       *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       9/4/97                                                        *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This is a header file containing Intel-processor specific stuff.

        This file may be included in the "C" code and in the assembly code
        exactly as it is.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 9/4/97     1.00  Original                                       Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _INTEL_H_
#define _INTEL_H_

/******************************************************************************
*                                                                             *
*   Include Files                                                             *
*                                                                             *
******************************************************************************/

#include "types.h"

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Defines of the Intel processor
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// CR0 Bits
//-----------------------------------------------------------------------------

#define PE_BIT              0
#define PG_BIT              31

//-----------------------------------------------------------------------------
// EFLAGS Bits
//-----------------------------------------------------------------------------

#define CF_BIT              0
#define PF_BIT              2
#define AF_BIT              4
#define ZF_BIT              6
#define SF_BIT              7
#define TF_BIT              8
#define IF_BIT              9
#define DF_BIT              10
#define OF_BIT              11
#define IOPL_BIT0           12
#define IOPL_BIT1           13
#define NT_BIT              14

#define RF_BIT              16
#define VM_BIT              17
#define AC_BIT              18
#define VIF_BIT             19
#define VIP_BIT             20
#define ID_BIT              21


#define CF_MASK             (1 << CF_BIT)
#define RES1_MASK           (1 << 1)
#define PF_MASK             (1 << PF_BIT)
#define RES3_MASK           (1 << 3)
#define AF_MASK             (1 << AF_BIT)
#define RES5_MASK           (1 << 5)
#define ZF_MASK             (1 << ZF_BIT)
#define SF_MASK             (1 << SF_BIT)
#define TF_MASK             (1 << TF_BIT)
#define IF_MASK             (1 << IF_BIT)
#define DF_MASK             (1 << DF_BIT)
#define OF_MASK             (1 << OF_BIT)
#define IOPL_MASK           (3 << IOPL_BIT0)
#define NT_MASK             (1 << NT_BIT)
#define RES15_MASK          (1 << 15)

#define RF_MASK             (1 << RF_BIT)
#define VM_MASK             (1 << VM_BIT)
#define AC_MASK             (1 << AC_BIT)
#define VIF_MASK            (1 << VIF_BIT)
#define VIP_MASK            (1 << VIP_BIT)
#define ID_MASK             (1 << ID_BIT)
#define RES22_MASK          (0x3FF << 22)


//-----------------------------------------------------------------------------
// Page directory and page table bits
//-----------------------------------------------------------------------------

typedef struct
{
    DWORD fPresent    : 1;              // Page is present
    DWORD fWrite      : 1;              // Page can be written as well as read
    DWORD fUser       : 1;              // Page can be accessed by the user
    DWORD res1        : 2;              // Reserved field
    DWORD fAccessed   : 1;              // Page has been accessed
    DWORD fDirty      : 1;              // Page has been modified
    DWORD res2        : 2;              // Reserved field
    DWORD Flags       : 3;              // Page's kernel flags
    DWORD Index       : 20;             // Physical page index

} TPage;

// Process flags

#define PROCESS_FLAG_USED   0x0         // 000 - Slot is used
#define PROCESS_FLAG_AVAIL  0x5         // 101 - Slot available

//-----------------------------------------------------------------------------
// Selector Descriptor Entry
//-----------------------------------------------------------------------------

typedef struct
{
    DWORD Limit0:16;
    DWORD Base0:16;
    DWORD Base1:8;
    DWORD Type:4;
    DWORD CodeData:1;
    DWORD DPL:2;
    DWORD Present:1;
    DWORD Limit1:4;
    DWORD Avail:1;
    DWORD Res:1;
    DWORD Size:1;
    DWORD Granularity:1;
    DWORD Base2:8;

} TDesc;

#define GET_DESCBASE(Sel)   ((Sel)->Base0 + ((Sel)->Base1 << 16) + ((Sel)->Base2 << 24))
#define GET_DESCLIMIT(Sel)  ((Sel)->Limit0 + ((Sel)->Limit1 << 16))

extern TDesc GDT[];

#define DESC_TYPE_EXEC      0xA         // Execute/read
#define DESC_TYPE_DATA      0x2         // Read/Write
#define DESC_TYPE_LDT       0x2         // LDT (CodeData must be 0)


typedef struct
{
    DWORD Offset0:16;
    DWORD Selector:16;
    DWORD Res0:8;
    DWORD Type:3;
    DWORD Size:1;
    DWORD Res1:1;
    DWORD DPL:2;
    DWORD Present:1;
    DWORD Offset1:16;

} TIntDesc;

#define GET_INTOFFSET(Sel)   ((Sel)->Offset0 + ((Sel)->Offset1 << 16))

extern TIntDesc IDT[];

#define INT_TYPE_TASK       0x5         // Task gate
#define INT_TYPE_INT        0x6         // Interrupt gate
#define INT_TYPE_TRAP       0x7         // Trap gate


//-----------------------------------------------------------------------------
// Debug Registers Defines
//-----------------------------------------------------------------------------

#define DR7_L0_BIT          0
#define DR7_G0_BIT          1
#define DR7_L1_BIT          2
#define DR7_G1_BIT          3
#define DR7_L2_BIT          4
#define DR7_G2_BIT          5
#define DR7_L3_BIT          6
#define DR7_G3_BIT          7

#define DR7_RW0_BIT         16
#define DR7_LEN0_BIT        18
#define DR7_RW1_BIT         20
#define DR7_LEN1_BIT        22
#define DR7_RW2_BIT         24
#define DR7_LEN2_BIT        26
#define DR7_RW3_BIT         28
#define DR7_LEN3_BIT        30

#define DR7_L0_MASK         (1 << DR7_L0_BIT)
#define DR7_G0_MASK         (1 << DR7_G0_BIT)
#define DR7_L1_MASK         (1 << DR7_L1_BIT)
#define DR7_G1_MASK         (1 << DR7_G1_BIT)
#define DR7_L2_MASK         (1 << DR7_L2_BIT)
#define DR7_G2_MASK         (1 << DR7_G2_BIT)
#define DR7_L3_MASK         (1 << DR7_L3_BIT)
#define DR7_G3_MASK         (1 << DR7_G3_BIT)

#define DR7_RW0_MASK        (1 << DR7_RW0_BIT)
#define DR7_LEN0_MASK       (1 << DR7_LEN0_BIT)
#define DR7_RW1_MASK        (1 << DR7_RW1_BIT)
#define DR7_LEN1_MASK       (1 << DR7_LEN1_BIT)
#define DR7_RW2_MASK        (1 << DR7_RW2_BIT)
#define DR7_LEN2_MASK       (1 << DR7_LEN2_BIT)
#define DR7_RW3_MASK        (1 << DR7_RW3_BIT)
#define DR7_LEN3_MASK       (1 << DR7_LEN3_BIT)


#define DR6_B0_BIT          0
#define DR6_B1_BIT          1
#define DR6_B2_BIT          2
#define DR6_B3_BIT          3

#define DR6_BD_BIT          13
#define DR6_BS_BIT          14
#define DR6_BT_BIT          15

#define DR6_B0_MASK         (1 << DR6_B0_BIT)
#define DR6_B1_MASK         (1 << DR6_B1_BIT)
#define DR6_B2_MASK         (1 << DR6_B2_BIT)
#define DR6_B3_MASK         (1 << DR6_B3_BIT)

#define DR6_BD_MASK         (1 << DR6_BD_BIT)
#define DR6_BS_MASK         (1 << DR6_BS_BIT)
#define DR6_BT_MASK         (1 << DR6_BT_BIT)


/******************************************************************************
*
* Assembler defines
*
*******************************************************************************
end comment ~

;==============================================================================
; PROCESSOR CONSTANTS
;==============================================================================

;------------------------------------------------------------------------------
; Offsets into the Task State Segment structure
;------------------------------------------------------------------------------

TSS_ESP0       Equ   4

;------------------------------------------------------------------------------
; CR0 Bits
;------------------------------------------------------------------------------
PE_BIT         Equ   0
PG_BIT         Equ   31

;------------------------------------------------------------------------------
; EFLAGS Bits
;------------------------------------------------------------------------------
CF_BIT         Equ   0
PF_BIT         Equ   2
AF_BIT         Equ   4
ZF_BIT         Equ   6
SF_BIT         Equ   7
TF_BIT         Equ   8
IF_BIT         Equ   9
DF_BIT         Equ   10
OF_BIT         Equ   11
IOPL_BIT0      Equ   12
IOPL_BIT1      Equ   13
NT_BIT         Equ   14
RF_BIT         Equ   16
VM_BIT         Equ   17
AC_BIT         Equ   18
VIF_BIT        Equ   19
VIP_BIT        Equ   20
ID_BIT         Equ   21


CF_MASK        Equ   1
RES1_MASK      Equ   (1 shl 1)
PF_MASK        Equ   (1 shl 2)
RES3_MASK      Equ   (1 shl 3)
AF_MASK        Equ   (1 shl 4)
RES5_MASK      Equ   (1 shl 5)
ZF_MASK        Equ   (1 shl 6)
SF_MASK        Equ   (1 shl 7)
TF_MASK        Equ   (1 shl 8)
IF_MASK        Equ   (1 shl 9)
DF_MASK        Equ   (1 shl 10)
OF_MASK        Equ   (1 shl 11)
IOPL_MASK      Equ   3000h
NT_MASK        Equ   (1 shl 14)
RES15_MASK     Equ   (1 shl 15)
RF_MASK        Equ   (1 shl 16)
VM_MASK        Equ   (1 shl 17)
AC_MASK        Equ   (1 shl 18)
VIF_MASK       Equ   (1 shl 19)
VIP_MASK       Equ   (1 shl 20)
ID_MASK        Equ   (1 shl 21)
RES22_MASK     Equ   (3FFh shl 22)


; Define operand size override byte and the jump far instruction opcode

OPERAND_SIZE   Equ   066h
JMP_FAR        Equ   0EAh

;==============================================================================
; DESCRIPTORS
;==============================================================================

TDescriptorHi  Record  DS_Base2:8, DS_Granularity:1, DS_Size:1, DS_Res:1, DS_Avail:1, DS_Limit1:4, DS_Present:1, DS_Dpl:2, DS_CodeData:1, DS_Type:4, DS_Base1:8
TDescriptorLo  Record  DS_Base0:16, DS_Limit0:16

DESC_4K_GRAN   Equ   1
DESC_1B_GRAN   Equ   0

DESC_32        Equ   1
DESC_16        Equ   0

DESC_PRESENT   Equ   1
DESC_NOT_PRESENT   Equ   0

DESC_DPL0      Equ   0
DESC_DPL3      Equ   3

DESC_APP       Equ   1
DESC_SYS       Equ   0

DESC_TYPE_EXEC Equ   1010b             ; Execute/Read
DESC_TYPE_DATA Equ   0010b             ; Read/Write
DESC_TYPE_LDT  Equ   0010b             ; LDT system
DESC_TYPE_TSS  Equ   1001b             ; TSS Descriptor



comment ~ */
#endif //  _INTEL_H_
/*
end comment ~
;*/
