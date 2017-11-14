;/*****************************************************************************
comment ~                                                                     #
*******************************************************************************
*                                                                             *
*   Module:     Kernel.h                                                      *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       8/5/97                                                        *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This is a header file for the main kernel definitions.

        This file may be included in the "C" code and in the assembly code
        exactly as it is.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 8/5/97     1.00  Original                                       Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _KERNEL_H_
#define _KERNEL_H_

/******************************************************************************
*                                                                             *
*   Include Files                                                             *
*                                                                             *
******************************************************************************/

#include "types.h"                      // Include basic data types

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

/******************************************************************************
end comment ~
;------------------------------------------------------------------------------
;.-
;     P H Y S I C A L   M E M O R Y
;

PHY_VM               Equ   00000000h   ; Phys System Virtual Machine starting address
PHY_PD               Equ   00110000h   ; Phys page directory page
PHY_PT               Equ   00111000h   ; Phys page table arena
PHY_PT_VM            Equ   00111000h   ; Phys page of VM page table
PHY_PT_PD            Equ   00112000h   ; Phys page of PD page table
PHY_PT_PT            Equ   00113000h   ; Phys page of page tables
PHY_PT_KERNEL        Equ   00114000h   ; Phys page of kernel page table
PHY_PT_HEAP          Equ   00115000h   ; Phys page of kernel heap page table
PHY_KERNEL           Equ   00116000h   ; Phys start of the kernel image

;------------------------------------------------------------------------------
;
;     L I N E A R   M E M O R Y
;
; Name            Base addr  Mapped
; --------------- ---------  --------------
; LIN_VM          00000000   VM_LEN        Virtual Machine arena
;
; LIN_KERNEL      00400000   KERNEL_LEN    Kernel code/data
;
; LIN_PD          00800000   4K            Page directory page
;
; LIN_PT          00C00000                 Page tables
;   LIN_PT_VM     00C00000   4K            VM Page (V86) entry
;   LIN_PT_KERNEL 00C01000   4K            Kernel page entry
;   LIN_PT_PD     00C02000   4K            Page Directory page entry
;   LIN_PT_PT     00C03000   4K            Page Table page entry
;   LIN_PT_HEAP   00C04000   4K            Kernel heap page entry
;
; LIN_HEAP        01000000                 Kernel heap address
;-.

LIN_VM               Equ   00000000h   ; Linear address of the System Virtual Machine arena
LIN_KERNEL           Equ   00400000h   ; Linear address of the kernel
LIN_PD               Equ   00800000h   ; Linear address of the page directory page
LIN_PT               Equ   00C00000h   ; Linear address of the page table arena
LIN_PT_VM            Equ   00C00000h   ; Linear address of the VM page table
LIN_PT_KERNEL        Equ   00C01000h   ; Linear address of the kernel page table
LIN_PT_PD            Equ   00C02000h   ; Linear address of the PD page table
LIN_PT_PT            Equ   00C03000h   ; Linear address of the PT page table
LIN_PT_HEAP          Equ   00C04000h   ; Linear address of the heap page table
LIN_HEAP             Equ   01000000h   ; Linear address of the kernel heap


VM_LEN               Equ   (1024*1024 + 64*1024)    ; Init size of the VM committed memory
;KERNEL_LEN           Equ   (256 * 1024)             ; Size of the kernel committed memory
KERNEL_LEN           Equ   (512 * 1024)             ; Size of the kernel committed memory

VM_ENTRIES           Equ   VM_LEN / 4096
KERNEL_ENTRIES       Equ   KERNEL_LEN / 4096


; First free physical page after initial constant allocation

PHY_FREE_PAGE        Equ   (PHY_KERNEL / 4096) + KERNEL_ENTRIES

;==============================================================================
; DESCRIPTORS
;==============================================================================

MAX_LIMIT0     Equ   0FFFFh                        ; Maximum limit
MAX_LIMIT1     Equ   0Fh
KERNEL_BASE2   Equ   LIN_KERNEL shr 24             ; High base address of a kernel
KERNEL_BASE1   Equ   (LIN_KERNEL shr 16) and 0FFh  ;  -
KERNEL_BASE0   Equ   LIN_KERNEL and 0FFFFh         ; Low base address of a kernel
ZERO_BASE      Equ   0                             ; Zero base

;------------------------------------------------------------------------------
comment ~ */
//-----------------------------------------------------------------------------
// Kernel memory arenas
//-----------------------------------------------------------------------------

#define VM_LEN          ((1024 + 64) *1024)
#define VM_ENTRIES      (VM_LEN / 4096)

#define PHY_PD          0x00110000      // Physical address of the page dir

#define LIN_KERNEL      0x00400000      // Linear address of the kernel
#define LIN_PD          0x00800000      // Linear address of the page dir
#define LIN_PT          0x00C00000      // Linear address of page tables arry
#define LIN_PT_PT       0x00C03000      // Page table page tables arry
#define LIN_PT_HEAP     0x00C04000      // Linear address of the fifth page table (Heap)
#define LIN_HEAP        0x01000000      // Linear address of a kernel heap


/******************************************************************************
end comment ~
;.-
;==============================================================================
; SELECTORS
;==============================================================================
; Define 6 selector descriptors:
;
; GDT0 0000 is a dummy descriptor, it will never be used (indexed by SEL_NULL)
; GDT1 0008 is a kernel code descriptor (indexed by SEL_CODE)
; GDT2 0010 is a kernel data descriptor (indexed by SEL_DATA)
; GDT3 0018 is a global data descriptor (indexed by SEL_GLOBAL)
; GDT4 0020 is a global 16 bit code descriptor (indexed by SEL_CODE16)
; GDT5 0028 is a global 16 bit data descriptor (indexed by SEL_DATA16)
; GDT5 0030 is a TSS selector (indexed by SEL_TSS)
;-.

TSelector      Record  SEL_Desc:13, SEL_Ldt:1, SEL_Rpl:2

SEL_GDT_0      Equ   0
SEL_RPL_0      Equ   0

;------------------------------------------------------------------------------

SEL_NULL       Equ   TSelector <0, 0, 0>
SEL_CODE       Equ   TSelector <1, SEL_GDT_0, SEL_RPL_0>
SEL_DATA       Equ   TSelector <2, SEL_GDT_0, SEL_RPL_0>
SEL_GLOBAL     Equ   TSelector <3, SEL_GDT_0, SEL_RPL_0>
SEL_CODE16     Equ   TSelector <4, SEL_GDT_0, SEL_RPL_0>
SEL_DATA16     Equ   TSelector <5, SEL_GDT_0, SEL_RPL_0>
SEL_TSS        Equ   TSelector <6, SEL_GDT_0, SEL_RPL_0>

;------------------------------------------------------------------------------
comment ~ */
//-----------------------------------------------------------------------------
// Constant predefined selectors
//-----------------------------------------------------------------------------

#define SEL_NULL        0x0000          // Null-selector
#define SEL_CODE        0x0008          // Kernel code selector
#define SEL_DATA        0x0010          // Kernel data selector
#define SEL_GLOBAL      0x0018          // Kernel global data selector
#define SEL_CODE16      0x0020          // 16 bit startup/exit code selector
#define SEL_DATA16      0x0028          // 16 bit startup/exit data selector
#define SEL_TSS         0x0030          // Task segment selector

#define SEL_APP_NULL    0x0000          // Application null selector
#define SEL_APP_CODE    0x000F          // Application code selector (LDT)
#define SEL_APP_DATA    0x0017          // Application data selector (LDT)

/******************************************************************************
end comment ~
;==============================================================================
; KERNEL CONSTANTS
;==============================================================================

StackSize      Equ   1024 * 16

TSS_IOPL_LEN   Equ   (0FFFFh / 8)

TSS_LIMIT      Equ   (104 + TSS_IOPL_LEN + 1)
TSS_LIMIT0     Equ   TSS_LIMIT and 0FFFFh
TSS_LIMIT1     Equ   TSS_LIMIT shr 16

;------------------------------------------------------------------------------
comment ~ */

//-----------------------------------------------------------------------------
// Memory allocations inside the kernel heap 4Mb arena
//-----------------------------------------------------------------------------

#define HEAP_LEN        (1024 * 512)    // Length of the kernel heap
#define HEAP_DEBUG_LEN  (1024 * 32)     // Length of the kernel debugger heap

#define TSS_IOPL_LEN    (0xFFFF / 8)    // Length of the IOPL field in the TSS

/******************************************************************************
end comment ~
;==============================================================================
; MEMORY STRUCTURES
;==============================================================================

;------------------------------------------------------------------------------
; Standard "C" stack frame
;------------------------------------------------------------------------------

ArgStruct      Struc
Arg_ebp        Dd    ?
Arg_ret        Dd    ?
Arg1           Dd    ?
Arg2           Dd    ?
Arg3           Dd    ?
Arg4           Dd    ?
ArgStruct      Ends


;------------------------------------------------------------------------------
; Segment structure
;------------------------------------------------------------------------------

Seg_Struct     Struc                ;*/ typedef struct {      /*
Seg_ES         dw    ?              ;*/      WORD    es;      /*
Seg_res1       dw    ?              ;*/      WORD    esh;     /*
Seg_DS         dw    ?              ;*/      WORD    ds;      /*
Seg_res2       dw    ?              ;*/      WORD    dsh;     /*
Seg_FS         dw    ?              ;*/      WORD    fs;      /*
Seg_res3       dw    ?              ;*/      WORD    fsh;     /*
Seg_GS         dw    ?              ;*/      WORD    gs;      /*
Seg_res4       dw    ?              ;*/      WORD    gsh;     /*
Seg_Struct     Ends                 ;*/ } TSeg;               /*

;------------------------------------------------------------------------------
; Register structure
;------------------------------------------------------------------------------

Regs_Struct    Struc                ;*/ typedef struct {      /*
Regs_EDI       dd    ?              ;*/     DWORD   edi;      /*
Regs_ESI       dd    ?              ;*/     DWORD   esi;      /*
Regs_EBP       dd    ?              ;*/     DWORD   ebp;      /*
Regs_ESP       dd    ?              ;*/     DWORD   esp;      /*
Regs_EBX       dd    ?              ;*/     DWORD   ebx;      /*
Regs_EDX       dd    ?              ;*/     DWORD   edx;      /*
Regs_ECX       dd    ?              ;*/     DWORD   ecx;      /*
Regs_EAX       dd    ?              ;*/     DWORD   eax;      /*
Regs_Struct    Ends                 ;*/ } TReg;               /*

;------------------------------------------------------------------------------
; Stack structure
;------------------------------------------------------------------------------

Stack_Struct   Struc                ;*/ typedef struct {      /*
Stack_ESP      dd    ?              ;*/     DWORD   esp;      /*
Stack_SS       dw    ?              ;*/     WORD    ss;       /*
Stack_res      dw    ?              ;*/     WORD    ssh;      /*
Stack_Struct   Ends                 ;*/ } TStack;             /*

;------------------------------------------------------------------------------
; Code structure
;------------------------------------------------------------------------------

Code_Struct    Struc                ;*/ typedef struct {      /*
Code_EIP       dd    ?              ;*/     DWORD   eip;      /*
Code_CS        dw    ?              ;*/     WORD    cs;       /*
Code_res       dw    ?              ;*/     WORD    csh;      /*
Code_EFLAGS    dd    ?              ;*/     DWORD   eflags;   /*
Code_Struct    Ends                 ;*/ } TCode;              /*


;------------------------------------------------------------------------------
; Stack after an V86 mode program has been interrupted.
;
; End of the structure contains entry for V86-mode case where we have
; segment registers pushed onto the stack
;------------------------------------------------------------------------------

Int_Stack      Struc                ;*/ typedef struct {      /*
Int_PM_ES      dw    ?              ;*/     WORD  pm_es;      /*
Int_res0       dw    ?              ;*/     WORD  r0;         /*
Int_PM_DS      dw    ?              ;*/     WORD  pm_ds;      /*
Int_res1       dw    ?              ;*/     WORD  r1;         /*
Int_PM_FS      dw    ?              ;*/     WORD  pm_fs;      /*
Int_res2       dw    ?              ;*/     WORD  r2;         /*
Int_PM_GS      dw    ?              ;*/     WORD  pm_gs;      /*
Int_res3       dw    ?              ;*/     WORD  r3;         /*
Int_EDI        dd    ?              ;*/     DWORD edi;        /*
Int_ESI        dd    ?              ;*/     DWORD esi;        /*
Int_EBP        dd    ?              ;*/     DWORD ebp;        /*
Int_Tmp        dd    ?              ;*/     DWORD tmp;        /*
Int_EBX        dd    ?              ;*/     DWORD ebx;        /*
Int_EDX        dd    ?              ;*/     DWORD edx;        /*
Int_ECX        dd    ?              ;*/     DWORD ecx;        /*
Int_EAX        dd    ?              ;*/     DWORD eax;        /*
Int_ErrorCode  dd    ?              ;*/     DWORD ErrorCode;  /*
Int_EIP        dd    ?              ;*/     DWORD eip;        /*
Int_CS         dw    ?              ;*/     WORD  cs;         /*
Int_res5       dw    ?              ;*/     WORD  r5;         /*
Int_EFLAGS     dd    ?              ;*/     DWORD eflags;     /*
Int_ESP        dd    ?              ;*/     DWORD esp;        /*
Int_SS         dw    ?              ;*/     WORD  ss;         /*
Int_res6       dw    ?              ;*/     WORD  r6;         /*
Int_V86_ES     dw    ?              ;*/     WORD  v86_es;     /*
Int_res7       dw    ?              ;*/     WORD  r7;         /*
Int_V86_DS     dw    ?              ;*/     WORD  v86_ds;     /*
Int_res8       dw    ?              ;*/     WORD  r8;         /*
Int_V86_FS     dw    ?              ;*/     WORD  v86_fs;     /*
Int_res9       dw    ?              ;*/     WORD  r9;         /*
Int_V86_GS     dw    ?              ;*/     WORD  v86_gs;     /*
Int_res10      dw    ?              ;*/     WORD  r10;        /*
Int_Stack      Ends                 ;*/ } TIntStack;          /*

;------------------------------------------------------------------------------
; Stack after a protected mode program has been interrupted.
;------------------------------------------------------------------------------

PM_Stack       Struc                ;*/ typedef struct {      /*
PM_PM_ES       dw    ?              ;*/     WORD  pm_es;      /*
PM_res0        dw    ?              ;*/     WORD  r0;         /*
PM_PM_DS       dw    ?              ;*/     WORD  pm_ds;      /*
PM_res1        dw    ?              ;*/     WORD  r1;         /*
PM_PM_FS       dw    ?              ;*/     WORD  pm_fs;      /*
PM_res2        dw    ?              ;*/     WORD  r2;         /*
PM_PM_GS       dw    ?              ;*/     WORD  pm_gs;      /*
PM_res3        dw    ?              ;*/     WORD  r3;         /*
PM_EDI         dd    ?              ;*/     DWORD edi;        /*
PM_ESI         dd    ?              ;*/     DWORD esi;        /*
PM_EBP         dd    ?              ;*/     DWORD ebp;        /*
PM_Tmp         dd    ?              ;*/     DWORD tmp;        /*
PM_EBX         dd    ?              ;*/     DWORD ebx;        /*
PM_EDX         dd    ?              ;*/     DWORD edx;        /*
PM_ECX         dd    ?              ;*/     DWORD ecx;        /*
PM_EAX         dd    ?              ;*/     DWORD eax;        /*
PM_ErrorCode   dd    ?              ;*/     DWORD ErrorCode;  /*
PM_EIP         dd    ?              ;*/     DWORD eip;        /*
PM_CS          dw    ?              ;*/     WORD  cs;         /*
PM_res5        dw    ?              ;*/     WORD  r5;         /*
PM_EFLAGS      dd    ?              ;*/     DWORD eflags;     /*
PM_ESP         dd    ?              ;*/     DWORD esp;        /*
PM_SS          dw    ?              ;*/     WORD  ss;         /*
PM_res6        dw    ?              ;*/     WORD  r6;         /*
PM_Stack       Ends                 ;*/ } TPMStack;           /*

;------------------------------------------------------------------------------
comment ~ */

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

#define KERNEL_DIE     KernelDie(__FILE__,__LINE__)

extern void Unload();


/******************************************************************************
*
* Assembler defines
*
*******************************************************************************
end comment ~


;******************************************************************************
;                                                                             *
;      External Declarations                                                  *
;                                                                             *
;******************************************************************************


comment ~ */
#endif // _KERNEL_H_
/*
end comment ~
;*/
