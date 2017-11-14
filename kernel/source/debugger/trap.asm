
       Page    58, 132
       Title   Trap.asm                                      Copyright (c) 1997

;******************************************************************************
;                                                                             *
;   Module:     Trap.asm                                                      *
;                                                                             *
;   Revision:   1.00                                                          *
;                                                                             *
;   Date:       9/19/97                                                       *
;                                                                             *
;   Author:     Goran Devic                                                   *
;                                                                             *
;******************************************************************************
;.-
;
;   Module Description:
;
;        This module contains the routines to implement debugger traps.
;
;-.
;*****************************************************************************#
;                                                                             *
;   Changes:                                                                  *
;                                                                             *
;   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
; --------   ---------------------------------------------------  ----------- *
; 9/19/97    Original                                             Goran Devic *
; --------   ---------------------------------------------------  ----------- *
;******************************************************************************
       Page
       .486p                           ; 486 protected mode

;******************************************************************************
;                                                                             *
;      Include Files                                                          *
;                                                                             *
;******************************************************************************

       Include Include\Intel.h         ; Include Intel specific defines
       Include Include\Kernel.h        ; Include global constants and defines

;******************************************************************************
;                                                                             *
;      Public Declarations                                                    *
;                                                                             *
;******************************************************************************

Public Debug_Trace_Handler
Public Debug_Breakpoint_Handler
Public DebugRun

;******************************************************************************
;                                                                             *
;      External Declarations                                                  *
;                                                                             *
;******************************************************************************

Extern Debug_Int1    : near
Extern Debug_Int3    : near

Extern pDebFrame     : dword
Extern pDebStack     : dword
Extern pDebCode      : dword
Extern pDebReg       : dword
Extern pDebSeg       : dword
Extern CPU_ErrorCode : dword

;******************************************************************************
;                                                                             *
;      Local Constants and Macros                                             *
;                                                                             *
;******************************************************************************

;******************************************************************************
;                                                                             *
;      Data Segment                                                           *
;                                                                             *
;******************************************************************************
_DATA          segment use32 para public 'DATA'          ; 32 bit data segment

DebStack       Stack_Struct <>

_DATA          ends


;******************************************************************************
;                                                                             *
;      Code Segment                                                           *
;                                                                             *
;******************************************************************************
_TEXT          segment use32 para public 'CODE'          ; 32 bit code segment


;******************************************************************************
;
;  Define handler for interrupt 1 - debug step
;
;******************************************************************************
Debug_Trace_Handler  proc

       mov     edi, offset cs:Debug_Int1 ; Get the address of the int 1 function
       jmp     Int_1_3_Handler         ; Jump to the common code

Debug_Trace_Handler  endp

;******************************************************************************
;
;  Define handler for interrupt 3 - breakpoint
;
;******************************************************************************

Debug_Breakpoint_Handler  proc

       mov     edi, offset cs:Debug_Int3 ; Get the address of the int 3 function
Int_1_3_Handler:                       ; Entry point for int 1

       add     esp, 4                  ; Skip returning address

; We need to set 4 basic debugger pointers: pointer to a stack record,
; code record, register record and the segment/selector record.

; current esp points to a PM selector record

       mov     eax, esp
       mov     ds:[pDebSeg], eax
       mov     ds:[pDebFrame], eax
       add     eax, 4 * 4

; Immediately following a segment record is a register record

       mov     ds:[pDebReg], eax
       add     eax, 4 * 8

; After the register record there is an error code

       mov     ds:[CPU_ErrorCode], eax
       add     eax, 4 * 1

; After the error code there is a code record

       mov     ds:[pDebCode], eax
       mov     cx, ss:[eax+4]          ; Get the code selector for later
       add     eax, 4 * 3

; After the code record there may be a stack record

       mov     ds:[pDebStack], eax

; Now depending on the mode of the interrupted program, we may have to do some
; exceptions:  If a V86 mode was interrupted, segment record is before the
; stack...

       bt      dword ptr ss:[eax-4], VM_BIT
       jnc     Int3Skip1

       mov     ebx, eax
       add     ebx, 2 * 4
       mov     ds:[pDebSeg], ebx
       jmp     Int3Skip2

Int3Skip1:

; There is also a case if the kernel code is interrupted, there is no
; stack record (only when rings 1-3 are interrupted, there is one).

       cmp     cx, SEL_CODE
       jnz     Int3Skip2

       mov     ds:[DebStack.Stack_esp], eax
       mov     ds:[DebStack.Stack_ss], ss
       mov     ds:[pDebStack], offset ds:DebStack

Int3Skip2:

; Jump to the debugger interrupt handler

       push    edi
       ret

Debug_Breakpoint_Handler  endp


       Subttl  DebugRun
       Page    +
;******************************************************************************
;
;      DebugRun
;
;******************************************************************************
;
;      This function is used from the debugger when the interrupted code
;      is being run.
;
;******************************************************************************
DebugRun       proc

; Get the pDebSeg pointer to set esp since this is the bottom of the register
; stack of the interrupted program in PM.  In V86, do not restore selectors
; since the segment version will be restored via iretd

       mov     eax, ds:[pDebCode]
       bt      dword ptr ss:[eax+8], VM_BIT
       jc      RunV86

;----------- PM MODE DEBUGEE --------------------------------------------------

       mov     esp, ds:[pDebSeg]       ; Bottom of the stack for PM

       pop     eax                     ; Pop and set selectors
       mov     es, ax
       pop     eax
       mov     ds, ax
       pop     eax
       mov     fs, ax
       pop     eax
       mov     gs, ax

       popad                           ; Restore genaral registers
       add     esp, 4                  ; Skip over the error code
       iretd                           ; Return to the interrupted program

;----------- V86 MODE DEBUGEE -------------------------------------------------
RunV86:

       mov     esp, ds:[pDebReg]       ; Get to the registers

       popad                           ; Restore genaral registers
       add     esp, 4                  ; Skip over the error code
       iretd                           ; Return to the interrupted program

DebugRun       endp


_TEXT          ends


       End                             ; End of the module

