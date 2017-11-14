
       Page    58, 132
       Title   Startup.asm                                   Copyright (c) 1997

;******************************************************************************
;
; Module:       _SysCall.asm
;
; Revision:     1.00
;
; Date:         02/21/96
;
; Author:       Goran Devic
;
;
;      This file contains a system call function.
;
;      Conviniently, setjump function is here.
;
;******************************************************************************
;
; Major Changes:
;
;   DATE     DESCRIPTION OF CHANGES                                      AUTHOR
; --------   ----  ----------------------------------------------   -----------
; 11/04/97   Original                                               Goran Devic
; --------   ----  ----------------------------------------------   -----------
;******************************************************************************
       Page
       .486p                           ; 486 protected mode

;******************************************************************************
;                                                                             *
;      Public Declarations                                                    *
;                                                                             *
;******************************************************************************

       Public sys_call
       Public _setjmp

;******************************************************************************
;                                                                             *
;      External Declarations                                                  *
;                                                                             *
;******************************************************************************

;******************************************************************************
;                                                                             *
;      Local Constants and Macros                                             *
;                                                                             *
;******************************************************************************

SYSINTR        Equ   078h              ; System call interrupt gate number

;******************************************************************************
;                                                                             *
;      Code Segment                                                           *
;                                                                             *
;******************************************************************************
_TEXT          segment use32 para public 'CODE'    ; 32 bit code segment


;==============================================================================
;
;  syscall - process a system call
;
;==============================================================================
sys_call:

       mov     eax, ss:[esp+04h]
       mov     ebx, ss:[esp+08h]
       mov     ecx, ss:[esp+0Ch]
       mov     edx, ss:[esp+10h]

       int     SYSINTR

       ret

;==============================================================================


       Subttl  setjmp
       Page    +
;******************************************************************************
;
;      This function is a back-end to
;
;      int setjmp( jmp_buf env )
;
;      that is defined as macro in `setjmp.h'
;
;******************************************************************************
;
;      Registers:  ebx - pointer to jmp_buf structure
;
;      Out: eax - 0
;
;******************************************************************************
_setjmp        proc

       mov     ds:[ebx+00], ecx        ;  Store a register
       mov     ds:[ebx+04], edx        ;  Store a register
       mov     ds:[ebx+08], esi        ;  Store a register
       mov     ds:[ebx+12], edi        ;  Store a register

       lea     eax, ss:[esp + 4]       ; Old esp value
       mov     ds:[ebx+16], eax
       mov     ds:[ebx+20], ebp

       pushfd                          ; Flags
       pop     eax
       mov     ds:[ebx+24], eax

       mov     eax, ss:[esp]           ; Return address is the jump point
       mov     ds:[ebx+28], eax

       xor     eax, eax                ; Return 0

       ret                             ; Return control to the caller
_setjmp        endp


_TEXT          ends

       End
