
       Page    58, 132
       Title   Int.asm                                       Copyright (c) 1997

;******************************************************************************
;                                                                             *
;   Module:     Int.asm                                                       *
;                                                                             *
;   Revision:   1.00                                                          *
;                                                                             *
;   Date:       8/12/97                                                       *
;                                                                             *
;   Author:     Goran Devic                                                   *
;                                                                             *
;******************************************************************************
;.-
;
;   Module Description:
;
;        This module contains low-level functions and data structures
;        for interrupt handling.  All first-level handlers are here.
;
;-.
;*****************************************************************************#
;                                                                             *
;   Changes:                                                                  *
;                                                                             *
;   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
; --------   ----  ---------------------------------------------  ----------- *
; 8/12/97    1.00  Original                                       Goran Devic *
; --------   ----  ---------------------------------------------  ----------- *
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
       Include Include\PC.h            ; Include PC hardware constants

;******************************************************************************
;                                                                             *
;      Public Declarations                                                    *
;                                                                             *
;******************************************************************************

Public Exception_Table
Public IRQ_Table

Public IDT_Descriptor
Public IDT_Address                     ; These are exported so that the
Public IDT                             ; debugger can access them


;******************************************************************************
;                                                                             *
;      External Declarations                                                  *
;                                                                             *
;******************************************************************************

;------------------------------------------------------------------------------
; Code
;------------------------------------------------------------------------------

Extern Reflect_V86_Int:near

Extern SysCallx:near                   ; System call handler

;------------------------------------------------------------------------------
; Data
;------------------------------------------------------------------------------


;******************************************************************************
;                                                                             *
;      Local Constants and Macros                                             *
;                                                                             *
;******************************************************************************

NUM_IDT        Equ   079h


;******************************************************************************
;                                                                             *
;      Data Segment                                                           *
;                                                                             *
;******************************************************************************
_DATA          segment use32 para public 'DATA'    ; 32 bit data segment


;==============================================================================
; INTERRUPT GATES
;==============================================================================

GATE_PRESENT   Equ   1
GATE_DPL0      Equ   0
GATE_DPL3      Equ   3
GATE_IDT       Equ   0Eh

GATE0          Equ   (GATE_PRESENT shl 15) or (GATE_DPL0 shl 13) or (GATE_IDT shl 8)
GATE3          Equ   (GATE_PRESENT shl 15) or (GATE_DPL3 shl 13) or (GATE_IDT shl 8)


;------------------------------------------------------------------------------
; Interrupt Descriptor Record
;------------------------------------------------------------------------------

IDT_Descriptor dw    (NUM_IDT * 8) - 1
IDT_Address    dd    dword ptr IDT + LIN_KERNEL


;------------------------------------------------------------------------------
; Interrupt Descriptor Table
;------------------------------------------------------------------------------
align 8

IDT            Equ   $

IDT0   dw      offset Int_0_Handler , SEL_CODE, GATE0, 0
IDT1   dw      offset Int_1_Handler , SEL_CODE, GATE0, 0
IDT2   dw      offset Int_2_Handler , SEL_CODE, GATE0, 0
IDT3   dw      offset Int_3_Handler , SEL_CODE, GATE3, 0
IDT4   dw      offset Int_4_Handler , SEL_CODE, GATE0, 0
IDT5   dw      offset Int_5_Handler , SEL_CODE, GATE0, 0
IDT6   dw      offset Int_6_Handler , SEL_CODE, GATE0, 0
IDT7   dw      offset Int_7_Handler , SEL_CODE, GATE0, 0

IDT8   dw      offset Int_8_Handler , SEL_CODE, GATE0, 0
IDT9   dw      offset Int_9_Handler , SEL_CODE, GATE0, 0
IDT10  dw      offset Int_A_Handler , SEL_CODE, GATE0, 0
IDT11  dw      offset Int_B_Handler , SEL_CODE, GATE0, 0
IDT12  dw      offset Int_C_Handler , SEL_CODE, GATE0, 0
IDT13  dw      offset Int_D_Handler , SEL_CODE, GATE0, 0
IDT14  dw      offset Int_E_Handler , SEL_CODE, GATE0, 0
IDT15  dw      offset Int_F_Handler , SEL_CODE, GATE0, 0

IDT16  dw      offset Int_10_Handler, SEL_CODE, GATE0, 0
IDT17  dw      offset Int_11_Handler, SEL_CODE, GATE0, 0
IDT18  dw      offset Int_12_Handler, SEL_CODE, GATE0, 0
IDT19  dw      offset Int_13_Handler, SEL_CODE, GATE0, 0
IDT20  dw      offset Int_14_Handler, SEL_CODE, GATE0, 0
IDT21  dw      offset Int_15_Handler, SEL_CODE, GATE0, 0
IDT22  dw      offset Int_16_Handler, SEL_CODE, GATE0, 0
IDT23  dw      offset Int_17_Handler, SEL_CODE, GATE0, 0

IDT24  dw      offset Int_18_Handler, SEL_CODE, GATE0, 0
IDT25  dw      offset Int_19_Handler, SEL_CODE, GATE0, 0
IDT26  dw      offset Int_1A_Handler, SEL_CODE, GATE0, 0
IDT27  dw      offset Int_1B_Handler, SEL_CODE, GATE0, 0
IDT28  dw      offset Int_1C_Handler, SEL_CODE, GATE0, 0
IDT29  dw      offset Int_1D_Handler, SEL_CODE, GATE0, 0
IDT30  dw      offset Int_1E_Handler, SEL_CODE, GATE0, 0
IDT31  dw      offset Int_1F_Handler, SEL_CODE, GATE0, 0

       rept (70h - 20h)
       dw      offset Int_0_Handler , SEL_CODE, GATE0, 0
       endm

IDT112 dw      offset Int_70_Handler, SEL_CODE, GATE0, 0
IDT113 dw      offset Int_71_Handler, SEL_CODE, GATE0, 0
IDT114 dw      offset Int_72_Handler, SEL_CODE, GATE0, 0
IDT115 dw      offset Int_73_Handler, SEL_CODE, GATE0, 0
IDT116 dw      offset Int_74_Handler, SEL_CODE, GATE0, 0
IDT117 dw      offset Int_75_Handler, SEL_CODE, GATE0, 0
IDT118 dw      offset Int_76_Handler, SEL_CODE, GATE0, 0
IDT119 dw      offset Int_77_Handler, SEL_CODE, GATE0, 0

; Interrupt 0x78 is the system call interrupt gate

IDT78  dw      offset Int_SysCall_Handler, SEL_CODE, GATE3, 0


;------------------------------------------------------------------------------
;
; Processor exceptions: interrupts 0 - 31
;
;------------------------------------------------------------------------------
Exception_Table:

       Rept 32
         dd    Default_Exception_Handler
       Endm


;------------------------------------------------------------------------------
;
; Hardware generated interrupts: IRQ0-IRQ7 and IRQ8-IRQF
;
;------------------------------------------------------------------------------
IRQ_Table:

       Rept 16
         dd    Default_IRQ_Handler
       Endm


_DATA          ends


;******************************************************************************
;******************************************************************************
;                                                                             *
;      Code Segment                                                           *
;                                                                             *
;******************************************************************************
_TEXT          segment use32 para public 'CODE'    ; 32 bit code segment


       Subttl  Int_SysCall_Handler
       Page    +
;******************************************************************************
;
;      Int_SysCall_Handler
;
;******************************************************************************
;
;      This is a handler for a PM system call.  It builds the standard stack
;      frame and calls SysCall "C" handler.
;
;******************************************************************************
Int_SysCall_Handler proc

       push    eax                     ; Fake the error code
       pushad                          ; Store all registers on the handler stk

       xor     eax, eax                ; Push all segment/selector registers
       mov     ax, gs
       push    eax
       mov     ax, fs
       push    eax
       mov     ax, ds
       push    eax
       mov     ax, es
       push    eax

       mov     ax, SEL_DATA            ; Set the data selector
       mov     ds, ax                  ;   ds
       mov     es, ax                  ;   es
       mov     ax, SEL_GLOBAL          ; Set the global selector
       mov     gs, ax                  ;   gs
       xor     ax, ax                  ; null selector
       mov     fs, ax                  ;   fs

       cld                             ; Set the direction bit
       call    SysCallx                ; Call the system call function
       jmp     Int_Handler_Ret         ; Standard int return function

Int_SysCall_Handler endp


       Subttl  Default_Exception_Handler
       Page    +
;******************************************************************************
;
;      Default_Exception_Handler
;
;******************************************************************************
;
;      This is the default exception handler for exceptions that are not
;      handled by the kernel.
;
;      Registers on entry:
;              esp+4 is the start of the stack structure
;
;      Registers on exit:
;              unchanged
;
;******************************************************************************
Default_Exception_Handler proc

       ret                             ; Return control to the caller
Default_Exception_Handler endp


       Subttl  Default_IRQ_Handler
       Page    +
;******************************************************************************
;
;      Default_IRQ_Handler
;
;******************************************************************************
;
;      This is the default interrupt handler for the interrupts that are not
;      registered by the kernel.
;
;      Registers on entry:
;              esp+4 is the start of the stack structure
;
;      Registers on exit:
;              The interrupt is being reflected to the current VM
;
;******************************************************************************
Default_IRQ_Handler proc

       lea     eax, ss:[esp+4]         ; Stack structure
       push    eax
       push    ecx                     ; Push the interrupt number
       call    Reflect_V86_Int         ; Reflect the interrupt into VM
       add     esp, 4 * 2              ; Pop the arguments from the stack

       mov     al, PIC_ACK             ; Acknowledge the interrupt controller
       out     PIC1, al

; How about PIC2 ??

       ret                             ; Return control to the caller
Default_IRQ_Handler endp


       Subttl  Int_Handler_Ret
       Page    +
;******************************************************************************
;
;      Int_Handler_Ret
;
;******************************************************************************
;
;      This the the default returning point for the interrupt handlers.
;
;      Registers on entry:
;              esp is on the stack structure
;
;      Registers on exit:
;              the control is returned to the interrupted process
;
;******************************************************************************
Int_Handler_Ret proc

       pop     eax                     ; Get the selector value
       mov     es, ax                  ;  es
       pop     eax                     ;
       mov     ds, ax                  ;  ds
       pop     eax                     ;
       mov     fs, ax                  ;  fs
       pop     eax                     ;
       mov     gs, ax                  ;  gs
       popad                           ; Restore all general registers

       add     esp, 4                  ; Skip the error code
       iretd                           ; Return control to the caller

Int_Handler_Ret endp


;******************************************************************************
;
; Define macro for dual interrupt handler (8-0Fh)
;
;******************************************************************************
Int_Dual_Handler  macro Function, Index, IRQ_Index, CPUErrorCode
Local  IRQ_Intr
Function       proc Public

       push    eax                     ; Store eax register on the stack

       mov     al, PIC_READ_IRQ        ; PIC OCW3, read servicing IRQ
       out     PIC1, al                ; Read servicing IRQ
       in      al, PIC_ACK             ; Read the IRQ in service
       or      al, al                  ; Is this a hardware interrupt?
       jnz     IRQ_Intr                ; Jump forward if it is

Ifb <CPUErrorCode>
       xchg    eax, ss:[esp]           ; CPU did not generate an error code,
                                       ; so leave a dummy value
Else
       pop     eax                     ; CPU generated an error code,
                                       ; so restore eax from the stack
Endif
       pushad                          ; Store all registers on the handler stk

       xor     eax, eax                ; Push all segment/selector registers
       mov     ax, gs
       push    eax
       mov     ax, fs
       push    eax
       mov     ax, ds
       push    eax
       mov     ax, es
       push    eax

       mov     ax, SEL_DATA            ; Set the data selector
       mov     ds, ax                  ;   ds
       mov     es, ax                  ;   es
       mov     ax, SEL_GLOBAL          ; Set the global selector
       mov     gs, ax                  ;   gs
       xor     ax, ax                  ; null selector
       mov     fs, ax                  ;   fs

       cld                             ; Set the direction bit and push the
       push    dword ptr offset cs:Int_Handler_Ret ; returning address
       mov     ecx, Index              ; Store the interrupt index into ecx

       lea     ebx, ds:[Exception_Table + Index*4]
       jmp     [ebx]                   ; Jump to the handler function

; -----------------------------------------------------------------------------
IRQ_Intr:

       xchg    eax, ss:[esp]           ; CPU did not generate an error code,
                                       ; so leave a dummy value
       pushad                          ; Store all registers on the handler stk

       xor     eax, eax                ; Push all segment/selector registers
       mov     ax, gs
       push    eax
       mov     ax, fs
       push    eax
       mov     ax, ds
       push    eax
       mov     ax, es
       push    eax

       mov     ax, SEL_DATA            ; Set the data selector
       mov     ds, ax                  ;   ds
       mov     es, ax                  ;   es
       mov     ax, SEL_GLOBAL          ; Set the global selector
       mov     gs, ax                  ;   gs
       xor     ax, ax                  ; null selector
       mov     fs, ax                  ;   fs

       cld                             ; Set the direction bit and push the
       push    dword ptr offset cs:Int_Handler_Ret ; returning address
       mov     ecx, Index              ; Store the interrupt index into ecx

       ; Here we are dealing with the hardware interrupts generated by the
       ; PIC.  We leave it programmed as is, with low 8 interrupts at INT 8
       ; - INT 15 and high 8 interrupts at INT 70h - INT 77h.

       lea     ebx, ds:[IRQ_Table + IRQ_Index*4]
       jmp     [ebx]                   ; Jump to the handler function

Function       endp
       endm

;******************************************************************************
;
;  Define macro primitive for CPU exception - only handler
;  This is a default handler for exceptions 0 - 7 and 16 - 31.
;
;******************************************************************************
Int_Exception_Handler  macro Function, Index
Function       proc Public

       push    eax                     ; Fake the error code
       pushad                          ; Store all registers on the handler stk

       xor     eax, eax                ; Push all segment/selector registers
       mov     ax, gs
       push    eax
       mov     ax, fs
       push    eax
       mov     ax, ds
       push    eax
       mov     ax, es
       push    eax

       mov     ax, SEL_DATA            ; Set the data selector
       mov     ds, ax                  ;   ds
       mov     es, ax                  ;   es
       mov     ax, SEL_GLOBAL          ; Set the global selector
       mov     gs, ax                  ;   gs
       xor     ax, ax                  ; null selector
       mov     fs, ax                  ;   fs

       cld                             ; Set the direction bit
       push    dword ptr offset cs:Int_Handler_Ret
       mov     ecx, Index              ; Store the interrupt index into ecx

       lea     ebx, ds:[Exception_Table + Index*4]
       jmp     [ebx]                   ; Jump to the handler function

Function       endp
       endm

;******************************************************************************
;
;  Define macro primitive for IRQ - only handler
;  This is a default handler for interrupts 70h - 77h.
;
;******************************************************************************
Int_IRQ_Handler  macro Function, Index, IRQ_Index
Function       proc Public

       push    eax                     ; Fake the error code
       pushad                          ; Store all registers on the handler stk

       xor     eax, eax                ; Push all segment/selector registers
       mov     ax, gs
       push    eax
       mov     ax, fs
       push    eax
       mov     ax, ds
       push    eax
       mov     ax, es
       push    eax

       mov     ax, SEL_DATA            ; Set the data selector
       mov     ds, ax                  ;   ds
       mov     es, ax                  ;   es
       mov     ax, SEL_GLOBAL          ; Set the global selector
       mov     gs, ax                  ;   gs
       xor     ax, ax                  ; null selector
       mov     fs, ax                  ;   fs

       cld                             ; Set the direction bit
       push    dword ptr offset cs:Int_Handler_Ret
       mov     ecx, Index              ; Store the interrupt index into ecx

       lea     ebx, ds:[IRQ_Table + IRQ_Index*4]
       jmp     [ebx]                   ; Jump to the handler function

Function       endp
       endm

;******************************************************************************
;
;  Using the macros above, define interrupt handlers 0 through 7
;
;******************************************************************************

Int_Exception_Handler Int_0_Handler, 0
Int_Exception_Handler Int_1_Handler, 1
Int_Exception_Handler Int_2_Handler, 2
Int_Exception_Handler Int_3_Handler, 3
Int_Exception_Handler Int_4_Handler, 4
Int_Exception_Handler Int_5_Handler, 5
Int_Exception_Handler Int_6_Handler, 6
Int_Exception_Handler Int_7_Handler, 7


;******************************************************************************
;
;  Using the macros above, define interrupt handlers 8  through 15
;
;******************************************************************************

Int_Dual_Handler  Int_8_Handler,  8, 0, CPU_Gerenates_Error_Code
Int_Dual_Handler  Int_9_Handler,  9, 1
Int_Dual_Handler  Int_A_Handler, 10, 2, CPU_Gerenates_Error_Code
Int_Dual_Handler  Int_B_Handler, 11, 3, CPU_Gerenates_Error_Code
Int_Dual_Handler  Int_C_Handler, 12, 4, CPU_Gerenates_Error_Code
Int_Dual_Handler  Int_D_Handler, 13, 5, CPU_Gerenates_Error_Code
Int_Dual_Handler  Int_E_Handler, 14, 6, CPU_Gerenates_Error_Code
Int_Dual_Handler  Int_F_Handler, 15, 7


;******************************************************************************
;
;  Using the macros above, define interrupt handlers 16 through 31
;
;******************************************************************************

Int_Exception_Handler Int_10_Handler, 16
Int_Exception_Handler Int_11_Handler, 17
Int_Exception_Handler Int_12_Handler, 18
Int_Exception_Handler Int_13_Handler, 19
Int_Exception_Handler Int_14_Handler, 20
Int_Exception_Handler Int_15_Handler, 21
Int_Exception_Handler Int_16_Handler, 22
Int_Exception_Handler Int_17_Handler, 23
Int_Exception_Handler Int_18_Handler, 24
Int_Exception_Handler Int_19_Handler, 25
Int_Exception_Handler Int_1A_Handler, 26
Int_Exception_Handler Int_1B_Handler, 27
Int_Exception_Handler Int_1C_Handler, 28
Int_Exception_Handler Int_1D_Handler, 29
Int_Exception_Handler Int_1E_Handler, 30
Int_Exception_Handler Int_1F_Handler, 31


;******************************************************************************
;
;  Using the macros above, define interrupt handlers 70h through 77h
;
;******************************************************************************

Int_IRQ_Handler Int_70_Handler, 70h, 8
Int_IRQ_Handler Int_71_Handler, 71h, 9
Int_IRQ_Handler Int_72_Handler, 72h, 10
Int_IRQ_Handler Int_73_Handler, 73h, 11
Int_IRQ_Handler Int_74_Handler, 74h, 12
Int_IRQ_Handler Int_75_Handler, 75h, 13
Int_IRQ_Handler Int_76_Handler, 76h, 14
Int_IRQ_Handler Int_77_Handler, 77h, 15



_TEXT          ends

       End                             ; End of the module

