
       Page    58, 132
       Title   Init.asm                                      Copyright (c) 1997

;******************************************************************************
;                                                                             *
;   Module:     Init.asm                                                      *
;                                                                             *
;   Revision:   1.00                                                          *
;                                                                             *
;   Date:       8/4/97                                                        *
;                                                                             *
;   Author:     Goran Devic                                                   *
;                                                                             *
;******************************************************************************
;.-
;
;   Module Description:
;
;        This module contains the assembly code for initialization.
;        Global Descriptor Table is defined here.
;
;-.
;*****************************************************************************#
;                                                                             *
;   Changes:                                                                  *
;                                                                             *
;   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
; --------   ----  ---------------------------------------------  ----------- *
; 8/4/97     1.00  Original                                       Goran Devic *
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

;******************************************************************************
;                                                                             *
;      Public Declarations                                                    *
;                                                                             *
;******************************************************************************

Public TOS                             ; Top of the stack value
Public GDT                             ; GDT address

Public Loader_Ret
Public Loader_V86
Public Unload

Public GDT_Descriptor                  ; These are exported so that the
Public GDT_Address                     ; debugger can access them
Public GDT

;******************************************************************************
;                                                                             *
;      External Declarations                                                  *
;                                                                             *
;******************************************************************************

Extern         tss : dword

Extern         kmain : near

Extern         nPages : dword       ; Total number of memory pages on a system
Extern         nFreePage : dword    ; Index of the first unused physical page

Extern         VM_SS : dword           ; Register structure for VM call
Extern         VM_SP : dword

Extern         IDT_Descriptor : dword
Extern         IDT_Address : dword
Extern         IDT : dword

;******************************************************************************
;                                                                             *
;      Local Constants and Macros                                             *
;                                                                             *
;******************************************************************************

DEBUG_ATTRIB   Equ   047h
DEBUG_NL       Equ   13
DEBUG_SCREEN   Equ   0B8000h


;******************************************************************************
;                                                                             *
;      Data Segment                                                           *
;                                                                             *
;******************************************************************************
_DATA          segment use32 para public 'DATA'           ; 32 bit data segment

;------------------------------------------------------------------------------

DebugIndex     dd    0                 ; Debug output screen index
DebugNLIndex   dd    160               ; Debug output new line index

Loader_Ret     dd    0                 ; Returning address from the loader
Loader_V86     dd    0                 ; Returning address in V86 mode

;------------------------------------------------------------------------------

GDT_Descriptor dw    (GDT_FREE - GDT) - 1
GDT_Address    dd    dword ptr GDT + LIN_KERNEL

;------------------------------------------------------------------------------
align  8

GDT            Equ   $

GDT0   TDescriptorLo <ZERO_BASE,    MAX_LIMIT0>
       TDescriptorHi <ZERO_BASE,    DESC_4K_GRAN, DESC_32, 0, 0, MAX_LIMIT1, DESC_PRESENT, DESC_DPL0, DESC_APP, DESC_TYPE_EXEC, ZERO_BASE>

GDT1   TDescriptorLo <KERNEL_BASE0, MAX_LIMIT0>
       TDescriptorHi <KERNEL_BASE2, DESC_4K_GRAN, DESC_32, 0, 0, MAX_LIMIT1, DESC_PRESENT, DESC_DPL0, DESC_APP, DESC_TYPE_EXEC, KERNEL_BASE1>

GDT2   TDescriptorLo <KERNEL_BASE0, MAX_LIMIT0>
       TDescriptorHi <KERNEL_BASE2, DESC_4K_GRAN, DESC_32, 0, 0, MAX_LIMIT1, DESC_PRESENT, DESC_DPL0, DESC_APP, DESC_TYPE_DATA, KERNEL_BASE1>

GDT3   TDescriptorLo <ZERO_BASE,    MAX_LIMIT0>
       TDescriptorHi <ZERO_BASE,    DESC_4K_GRAN, DESC_32, 0, 0, MAX_LIMIT1, DESC_PRESENT, DESC_DPL0, DESC_APP, DESC_TYPE_DATA, ZERO_BASE>

GDT4   TDescriptorLo <ZERO_BASE,    MAX_LIMIT0>
       TDescriptorHi <ZERO_BASE,    DESC_4K_GRAN, DESC_16, 0, 0, MAX_LIMIT1, DESC_PRESENT, DESC_DPL0, DESC_APP, DESC_TYPE_EXEC, ZERO_BASE>

GDT5   TDescriptorLo <ZERO_BASE,    MAX_LIMIT0>
       TDescriptorHi <ZERO_BASE,    DESC_4K_GRAN, DESC_16, 0, 0, MAX_LIMIT1, DESC_PRESENT, DESC_DPL0, DESC_APP, DESC_TYPE_DATA, ZERO_BASE>

GDT6   TDescriptorLo <0,            TSS_LIMIT0>
       TDescriptorHi <0,            DESC_1B_GRAN, DESC_32, 0, 0, TSS_LIMIT1, DESC_PRESENT, DESC_DPL0, DESC_SYS, DESC_TYPE_TSS, 0>

GTD_NOT_USED   db    8 * ( 16 - 7 )  dup (0)

; Each process pid directly indexes GDT to get its LDT.  That puts a limit on the
; number of processes to 240.

       Rept    256 - 16

         TDescriptorLo <0, 0 >
         TDescriptorHi <0, 0, 0, 0, 0, 0, DESC_NOT_PRESENT, 0, 0, 0, 0 >

       Endm

GDT_FREE       Equ   $

;------------------------------------------------------------------------------


_DATA          ends


;******************************************************************************
;                                                                             *
;      Stack Segment                                                          *
;                                                                             *
;******************************************************************************
_STACK         segment use32 para public 'STACK'         ; 32 bit stack segment

       dd      StackSize   dup (?)
TOS    Equ     $

_STACK         ends


;******************************************************************************
;                                                                             *
;      Code Segment                                                           *
;                                                                             *
;******************************************************************************
_TEXT          segment use32 para public 'CODE'           ; 32 bit code segment


       Subttl  DebugOut
       Page    +
;******************************************************************************
;
;      DebugOut
;
;******************************************************************************
;
;      This routine prints a string to the screen with optional numeric
;      argument.
;
;      _stdcall DebugOut( char*pStr, ... )
;
;      Registers on entry:
;              ds - Flat kernel data selector
;              gs - Flat linear selector
;              [DebugIndex] - Index of the next character
;
;******************************************************************************
DebugOut       proc Public
       push    ebp                     ; Standard stack frame
       mov     ebp, esp
       pushad

       lea     esi, ss:[ebp.Arg1]      ; Address of the first argumument
       mov     ebx, ss:[esi]           ; Pointer to a debug string message
       add     esi, 4                  ; First argument

       mov     edi, ds:[DebugIndex]    ; Get the index to print
Debug_Loop:
       mov     al, ds:[ebx]            ; Get the character to print
       or      al, al                  ; Is it zero?
       jz      DebugExit               ; Terminate if it is

       cmp     al, '%'                 ; Is a code numeric prefix?
       jnz     Debug_NewLine           ; Skip if it is not

       inc     ebx                     ; Increment the string pointer

       cmp     byte ptr ds:[ebx], '%'  ; 32 bit number?
       jnz     Debug_16

       inc     ebx                     ; Increment the string pointer
       mov     ecx, 8                  ; Nibble counter
       mov     edx, ss:[esi]           ; Get the word
       add     esi, 4                  ; Next argument
       jmp     Debug_Hex1

Debug_16:
       mov     ecx, 4                  ; Nibble counter
       mov     edx, ss:[esi]           ; Get the word
       shl     edx, 16
       add     esi, 2                  ; Next argument

Debug_Hex1:
       mov     eax, edx                ; Get the number
       shr     eax, 12 + 16            ; Get the top nibble
       mov     ah, DEBUG_ATTRIB        ; Charater attribute
       add     al, '0'                 ; Add '0' to it
       cmp     al, '9'                 ; Is > '9' ?
       jle     Debug_Hex2
       add     al, 'A' - '9' - 1       ; Add the difference
Debug_Hex2:
       mov     gs:[DEBUG_SCREEN + edi], ax ; Store the numeric
       add     edi, 2                  ; Increment the destination address
       shl     edx, 4                  ; Next nibble
       loop    Debug_Hex1
       jmp     Debug_Loop              ; Loop for the next character

Debug_NewLine:
       cmp     al, DEBUG_NL            ; Is a code a new line?
       jnz     Debug_Print
       mov     edi, ds:[DebugNLIndex]  ; Get the index of the next line
       mov     ecx, edi
       add     ecx, 160                ; Calculate the next new line
       mov     ds:[DebugNLIndex], ecx  ; Store the next new line index
       inc     ebx                     ; Increment the string pointer
       jmp     Debug_Loop              ; Next character

Debug_Print:
       mov     ah, DEBUG_ATTRIB        ; Store the character with attribute
       mov     gs:[DEBUG_SCREEN + edi], ax
       inc     ebx                     ; Increment the string pointer
       add     edi, 2                  ; Increment the destination address
       jmp     Debug_Loop              ; Loop for the next character
DebugExit:
       mov     ds:[DebugIndex], edi    ; Store the debug index

       popad
       pop     ebp
       ret                             ; Return control to the caller
DebugOut       endp



;******************************************************************************
;******************************************************************************
;#                                                                            #
;# Registers on entry:                                                        #
;#                                                                            #
;#      edx - eip of the returning function in the Loader                     #
;#      ecx - V86 mode returning function                                     #
;#                                                                            #
;#      edi - Total number of memory pages on a system                        #
;#                                                                            #
;******************************************************************************
;******************************************************************************

kstart:

;------------------------------------------------------------------------------
; Set up new global descriptor table that is in the kernel data space using
; a global data selector that is set in the loader code
;------------------------------------------------------------------------------

       lea     ebx, offset GDT_Descriptor + LIN_KERNEL
       lgdt    fword ptr es:[ebx]

;------------------------------------------------------------------------------
; LDT is not used (yet)
;------------------------------------------------------------------------------

       xor     ax, ax
       lldt    ax                      ; LDT is not needed for now

;------------------------------------------------------------------------------
; Set up all the selectors using the descriptor table residing in the kernel
;------------------------------------------------------------------------------

       mov     ax, SEL_GLOBAL
       mov     gs, ax                  ; gs - global memory selector

       mov     ax, SEL_DATA
       mov     ds, ax
       mov     es, ax                  ; ds, es - kernel data

       xor     ebx, ebx                ; Clear upper word
       mov     fs, bx
       mov     bx, ss                  ; Get the old, real mode stack segment
       mov     ds:[VM_SS], ebx         ; store it
       mov     ds:[VM_SP], esp         ; Store real mode stack pointer

       mov     ss, ax                  ; Data selector is for the stack also
       mov     esp, offset ds:TOS      ; Top of the kernel stack

; gs -> global selector: base 0
; ds -> kernel data selector
; es -> kernel data selector
; fs -> null selector
; ss:esp -> kernel data selector aliased with stack data pointer

;------------------------------------------------------------------------------
; Store the returning addresses of the loader code and other parameters
;------------------------------------------------------------------------------

       mov     ds:[Loader_Ret], edx
       mov     ds:[Loader_V86], ecx

       mov     ds:[nPages], edi
       mov     esi, PHY_FREE_PAGE
       mov     ds:[nFreePage], esi

;------------------------------------------------------------------------------
; Fix up the SEL_TSS descriptor address since it is a variable
;------------------------------------------------------------------------------

       lea     eax, offset tss + LIN_KERNEL
       mov     edx, offset GDT6
       mov     ds:[edx + 2], ax
       shr     eax, 16
       mov     ds:[edx + 4], al
       shr     eax, 8
       mov     ds:[edx + 7], al

;------------------------------------------------------------------------------
; Load task state selector
;------------------------------------------------------------------------------

       mov     ax, SEL_TSS
       ltr     ax

;------------------------------------------------------------------------------
; Call main init code
;******************************************************************************
;#                                                                            #
       jmp     kmain
;#                                                                            #
;******************************************************************************


       Subttl  Unload
       Page    +
;******************************************************************************
;
;      Unload
;
;******************************************************************************
;
;      Unloads the OS.
;
;******************************************************************************
Unload         proc

       push    dword ptr SEL_CODE16    ; Push the 16-bit code selector
       push    ds:[Loader_Ret]         ; Push the returning linear address

       retf                            ; Far return with sel:offset
Unload         endp


_TEXT          ends


       End     kstart                  ; End of the module

