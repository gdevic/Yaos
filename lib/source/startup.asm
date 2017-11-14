;******************************************************************************
;
; Module:       Startup.asm
;
; Revision:     1.00
;
; Date:         02/21/96
;
; Author:       Goran Devic
;
;******************************************************************************
;
; Module Description:
;
;  This module contains the run-time startup code for the YAOS c-library
;
;******************************************************************************
;
; Major Changes:
;
;   DATE     REV   DESCRIPTION OF CHANGES                 AUTHOR
; --------   ----  -----------------------------------   -----------
; 09/02/96   1.00  Original                              Goran Devic
; --------   ----  -----------------------------------   -----------
;******************************************************************************
.386p


;******************************************************************************
;  Imported
;******************************************************************************

EXTRN          main            :near   ; The "C" entry function


;******************************************************************************
;  Exported
;******************************************************************************

GLOBAL         _cstart_        :near   ; The entry point
GLOBAL         sys_call        :near   ; Process a system call

;******************************************************************************
;                  Start code segment
;
_TEXT          segment use32 para public 'CODE'
;******************************************************************************


;******************************************************************************
;
;  Entry address
;
;******************************************************************************
_cstart_:

       push    __exit
       jmp     main                    ; Jump to the user 'main()' function

__exit:
       mov     eax, 0
       int     80h

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
       mov     ebx, ss:[esp+14h]
       int     80h

       ret

;==============================================================================

;******************************************************************************
_TEXT          ends                    ; Text segment ends
;******************************************************************************

;******************************************************************************
;                  Start data segment
;
_DATA          segment use32 para public 'DATA'
;******************************************************************************


;******************************************************************************
_DATA          ends                    ; Data segment ends
;******************************************************************************


;******************************************************************************
;                  Start stack segment
;
_STACK         segment use32 para public 'STACK'
;******************************************************************************

;******************************************************************************
_STACK         ends                    ; Stack segment ends
;******************************************************************************

; Set the ordering of the segment groups
;
DGROUP         group   _DATA, _STACK

               end     _cstart_        ; cstart is the entry address