
       Page    58, 132
       Title   _Startup.asm                                  Copyright (c) 1997

;******************************************************************************
;
; Module:       _Startup.asm
;
; Revision:     1.00
;
; Date:         02/21/96
;
; Author:       Goran Devic
;
;******************************************************************************
Comment ^

       Module Description:

       This module contains the run-time startup code for the YAOS C library
       binding.

       (The following is also presented in Exec.c)

       The loader sets up the stack as follows:

       ========================== 4Mb <- top of the process memory
       .
       .  This block is reserved for argument and environment strings
       .  and is defined as ARG_MAX.
       .
       .                                     (open top end)
       (environment variable n)`0     <- ASCIIZ strings (env. variables)
       (environment variable 2)`0
       (environment variable 1)`0
       (argument string 0)`0          <- ASCIIZ strings (arguments)
       (argument string 1)`0
       (argument string 2)`0
       (argument string m)`0
       -------------------------- 4Mb - ARG_MAX virtual limit
       NULL
       p->(argument string m)         <- Array of pointers to arguments
       p->(argument string 2)
       p->(argument string 1)
   +-> p->(argument string 0)
   |   envc                           <- Number of environment strings
   |   p->(environment variable 1)    <- Pointer to a first env. string
   +-- argp    <-- ESP                <- Pointer to the arguments array
                  - | -
                  - V -  Stack grows down


   Function _Init_Environ sets up the array of pointers to environment
   strings in dynamic memory, so it can be resized eventually.

       NULL
       p->(environment variable 1)
       p->(environment variable 2)
       p->(environment variable n) <-- `environ' (and `envp', 3rd argument)



End Comment ^
;******************************************************************************
;
; Major Changes:
;
;   DATE     REV   DESCRIPTION OF CHANGES                 AUTHOR
; --------   ----  -----------------------------------   -----------
; 09/02/96   1.00  Original                              Goran Devic
; --------   ----  -----------------------------------   -----------
;******************************************************************************
       Page
       .486p                           ; 486 protected mode

;******************************************************************************
;                                                                             *
;      Public Declarations                                                    *
;                                                                             *
;******************************************************************************

       Public _cstart_

       Public _argc

;******************************************************************************
;                                                                             *
;      External Declarations                                                  *
;                                                                             *
;******************************************************************************

       Extern main:near

       Extern exit:near

       Extern _Init_Alloc:near

       Extern _Init_Environ:near

;******************************************************************************
;                                                                             *
;      Local Constants and Macros                                             *
;                                                                             *
;******************************************************************************

STACK_SIZE     Equ   1024 * 256        ; 256K for the stack

;******************************************************************************
;                                                                             *
;      Data Segment                                                           *
;                                                                             *
;******************************************************************************
_DATA          segment use32 para public 'DATA'    ; 32 bit data segment

       Extern _dwDynamic:dword

_argc          dd    -1

_DATA          ends


;******************************************************************************
;                                                                             *
;      Uninitialized Data Segment                                             *
;                                                                             *
;******************************************************************************
_BSS           segment use32 para public 'BSS'    ; 32 bit data segment

_BSS           ends


;******************************************************************************
;                                                                             *
;      Last segment in a segment list                                         *
;                                                                             *
;******************************************************************************
_LAST          segment use32 para public 'LAST'    ; 32 bit data segment

FreeArea       Equ   $

_LAST          ends


;******************************************************************************
;                                                                             *
;      Code Segment                                                           *
;                                                                             *
;******************************************************************************
_TEXT          segment use32 para public 'CODE'    ; 32 bit code segment


;******************************************************************************
;
;  Entry address
;
;******************************************************************************
_cstart_:
;int 3

       ; Initialize dynamic memory allocator starting at the first free
       ; address after the BSS segment and running up to the stack minus
       ; the maximum allowed stack space

       mov     eax, esp                ; Get the stack address
       sub     eax, STACK_SIZE         ; Give it some space
       mov     ebx, offset ds:FreeArea ; Get the first free address
       mov     ds:[_dwDynamic], ebx    ; Store it to the dynamic mem start addr
       sub     eax, ebx                ; Calculate the size
       push    eax                     ; Size paramater
       push    ebx                     ; Start address parameter
       call    _Init_Alloc             ; Initialize dynamic allocator
       add     esp, 4 * 2              ; pop the arguments

       call    _Init_Environ           ; Initialize environment variables

       call    main                    ; Call the user 'main()' function

       push    eax
       jmp     exit

;==============================================================================

_TEXT          ends

; Set the ordering of the segment groups to make _LAST the last group

DGROUP         group   _DATA, _BSS, _LAST


       End     _cstart_                ; cstart is the entry address
