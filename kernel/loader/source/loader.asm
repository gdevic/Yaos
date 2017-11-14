
       Page    58, 132
       Title   Loader                                        Copyright (c) 1997

;******************************************************************************
;                                                                             *
;   Module:     Loader.Asm                                                    *
;                                                                             *
;   Revision:   1.00                                                          *
;                                                                             *
;   Date:       7/30/97                                                       *
;                                                                             *
;   Author:     Goran Devic                                                   *
;                                                                             *
;******************************************************************************
;.-
;
;   Module Description:
;
;        This module contains the assembly portion of the loader code.
;
;-.
;*****************************************************************************#
;                                                                             *
;   Changes:                                                                  *
;                                                                             *
;   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
; --------   ----  ---------------------------------------------  ----------- *
; 7/30/97    1.00  Original                                       Goran Devic *
; --------   ----  ---------------------------------------------  ----------- *
;******************************************************************************
       Page
       .386p                           ; 386 protected mode

;******************************************************************************
;                                                                             *
;      Include Files                                                          *
;                                                                             *
;******************************************************************************

       Include ..\Include\Intel.h      ; Include Intel specific defines
       Include ..\Include\Kernel.h     ; Include global constants and defines
       Include ..\Include\PC.h         ; Include PC specific defines

;******************************************************************************
;                                                                             *
;      Public Declarations                                                    *
;                                                                             *
;******************************************************************************

Public _ExecuteOS
Public _GetPageWord


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

BUFFER_SIZE    Equ   16000             ; This is duplicated in LoadOS.c


DEBUG_ATTRIB   Equ   17h               ; Default debug screen attribute
DEBUG_NL       Equ   13h               ; Debug output new line code

INITIAL_FLAGS_MASK  Equ  (Not (IOPL_MASK or NT_MASK or VM_MASK))

;------------------------------------------------------------------------------
; Local Structures
;------------------------------------------------------------------------------

BootStruct     Struc                   ; ARGUMENT STRUCTURE
Boot_bp        Dw    ?                 ;  Saved bp register
Boot_ret       Dw    ?                 ;  Returning offset
Boot_pBootRec  Dw    ?                 ; Pointer to a boot record
BootStruct     Ends

BootRecord     Struc                   ; BOOT RECORD
Boot_pBuf      Dd    ?                 ;  Pointer to kernel array buffer
Boot_EIP       Dd    ?                 ;  Starting EIP
Boot_Flags     Dd    ?                 ;  Flags
BootRecord     Ends

;Flags bitfield

FLAGS          Record  FLAGS_VERBOSE:1

;******************************************************************************
;                                                                             *
;      Data Segment                                                           *
;                                                                             *
;******************************************************************************
LocalData      segment use16                 ; Start of the 16 bit data segment


Code16         dw    0                 ; Current code segment
Code4          dd    0                 ; Current code segment << 4
Data16         dw    0                 ; Current data segment
Data4          dd    0                 ; Current data segment << 4
ThisData4      dd    0                 ; This data segment << 4
StackSegment   dw    0                 ; Current stack segment
StackPointer   dd    0                 ; Current stack pointer

Linear_Ret     dd    0                 ; Linear returning address of a loader

DebugIndex     dw    0                 ; Debug output screen index
DebugNLIndex   dw    160               ; Debug output new line index

pKernel        dd    0                 ; Linear address of a kernel source


;------------------------------------------------------------------------------
; Debug messages
;------------------------------------------------------------------------------

Msg_8042       db    'A20 Enabled ', DEBUG_NL, 0
Msg_gdt        db    'GDT at %% loaded ', DEBUG_NL, 0
Msg_pmode      db    'Protected mode ', DEBUG_NL, 0
Msg_seg        db    'Selectors loaded ', DEBUG_NL, 0
Msg_krel       db    'Kernel relocated ', DEBUG_NL, 0
Msg_kboot      db    'Kernel image at %% para % ', DEBUG_NL, 0
Msg_jmp        db    'Jumping to kernel... ', DEBUG_NL, 0


;------------------------------------------------------------------------------
; IDT descriptor that is used to reset the IDT to the real mode default value
;------------------------------------------------------------------------------

Clean_IDT_Descriptor dw    03FFh
Clean_IDT_Address    dd    0

;------------------------------------------------------------------------------
; GDT descriptor
;------------------------------------------------------------------------------

GDT_Descriptor dw    (GDT_FREE - GDT) - 1
GDT_Address    dd    ?                 ; Linear address of the GDT table

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

GDT_FREE       Equ   $


;------------------------------------------------------------------------------
;
;  P A G E   T A B L E S
;
;  For detailed information on the memory partitioning, see kernel.h file
;
;------------------------------------------------------------------------------

TPage_Dir      Record  PD_Lin_Address:20, PD_Res:9, PD_Type:3
TPage_Table    Record  PT_Lin_Address:20, PT_Res:9, PT_Type:3

; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

; User pages are present, writable and user bit is `1'
; Supervisor pages are present, writable and user bit is `0'

PAGE_USER            Equ   7
PAGE_SUPERVISOR      Equ   3

; Free pages are not present and have a characteristic page signature

PHY_PAGE_FREE        Equ   00000A00h

; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; Build page directory page (4K)
; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

PD_ENTRIES           Equ   5

Page_Directory_Image:

       TPage_Dir   < PHY_PT_VM     shr 12, 0, PAGE_USER       >
       TPage_Dir   < PHY_PT_KERNEL shr 12, 0, PAGE_SUPERVISOR >
       TPage_Dir   < PHY_PT_PD     shr 12, 0, PAGE_USER       >
       TPage_Dir   < PHY_PT_PT     shr 12, 0, PAGE_USER       >
       TPage_Dir   < PHY_PT_HEAP   shr 12, 0, PAGE_SUPERVISOR >

       ; Set the rest of entries of the page directory to not present

       dd      1024 - PD_ENTRIES dup (PHY_PAGE_FREE)


; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; Build page tables page:
;  Page table for VM arena (4K)
;  Page table for page directory arena (4K)
;  Page table for page tables arena (4K)
;  Page table for kernel arena (4K)
;
; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

Page_Table_Image:

; Page table for VM: linear 1-1 mapping

       Page_Number = PHY_VM / 4096

       Rept VM_ENTRIES
         TPage_Table < Page_Number, 0, PAGE_USER >
         Page_Number = Page_Number + 1
       Endm

       ; Set the rest of entries of the first 4 Mb arena to not present

       dd      1024 - VM_ENTRIES dup (PHY_PAGE_FREE)


; Page table for the page directory arena

       TPage_Table   < PHY_PD / 4096, 0, PAGE_USER >

       ; Set the rest of entries of the second 4 Mb arena to not present

       dd      1024 - 1 dup (PHY_PAGE_FREE)


; Page table for the page tables arena

       TPage_Table   < PHY_PT_VM / 4096,      0, PAGE_USER       >
       TPage_Table   < PHY_PT_KERNEL / 4096 , 0, PAGE_SUPERVISOR >
       TPage_Table   < PHY_PT_PD / 4096,      0, PAGE_USER       >
       TPage_Table   < PHY_PT_PT / 4096,      0, PAGE_USER       >
       TPage_Table   < PHY_PT_HEAP / 4096,    0, PAGE_SUPERVISOR >

       ; Set the rest of entries of the third 4 Mb arena to not present

       dd      1024 - 5 dup (PHY_PAGE_FREE)


; Page table for the kernel arena

       Page_Number = PHY_KERNEL / 4096

       Rept KERNEL_ENTRIES
         TPage_Table < Page_Number, 0, PAGE_SUPERVISOR >
         Page_Number = Page_Number + 1
       Endm

       ; Set the rest of entries of the fourth 4 Mb arena to not present

       dd      1024 - KERNEL_ENTRIES  dup (PHY_PAGE_FREE)


Pages_End      Equ   $

; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

nPages         dd    0                 ; Total number of memory pages

; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

wGetPageAddr   dw    Offset Page_Directory_Image


LocalData      ends                    ; End of the 16 bit data segment


;******************************************************************************
;                                                                             *
;      16 bit Code Segment - Compact memory model                             *
;                                                                             *
;******************************************************************************
_TEXT  segment byte use16 public 'CODE'      ; Start of the 16 bit code segment
       assume  cs:_TEXT

       Subttl  _GetPageWord
       Page    +
;******************************************************************************
;
;      _GetPageWord
;
;******************************************************************************
;
;      This informational function is called from the C-loader to get the
;      contents of the page tables upon the command line `-p' option.
;
;      Registers on exit:
;              ax - Successive page table word
;
;******************************************************************************
_GetPageWord   proc

       mov     ax, seg LocalData
       mov     fs, ax
       assume  fs:seg LocalData

       mov     ax, -1
       mov     bx, fs:[wGetPageAddr]
       cmp     bx, offset Pages_End
       jz      _GetEnd
       mov     ax, fs:[bx]
       add     bx, 2
       mov     fs:[wGetPageAddr], bx
_GetEnd:
       ret                             ; Return control to the caller
_GetPageWord   endp


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
;      Registers on entry:
;              bx  - address of a zero-terminated string to print
;              gs  - segment of the screen (B800h)
;              fs  - segment of the local data
;              [DebugIndex] - Index of the next character
;
;      Registers on exit:
;              all preserved!
;
;******************************************************************************
DebugOut       proc
       nop                             ; To patch it with `dont print' RET
       push    bp
       mov     bp, sp
       pushad
       lea     si, ss:[bp.Arg1]        ; Address of the first argumument

       mov     di, fs:[DebugIndex]     ; Get the index to print
Debug_Loop:
       mov     al, fs:[bx]             ; Get the character to print
       or      al, al                  ; Is it zero?
       jz      DebugExit               ; Terminate if it is

       cmp     al, '%'                 ; Is a code numeric prefix?
       jnz     Debug_NewLine           ; Skip if it is not

       inc     bx                      ; Increment the string pointer

       cmp     byte ptr fs:[bx], '%'   ; 32 bit number?
       jnz     Debug_16

       inc     bx                      ; Increment the string pointer
       mov     cx, 8                   ; Nibbnle counter
       mov     edx, ss:[si]            ; Get the word
       add     si, 4                   ; Next argument
       jmp     Debug_Hex1

Debug_16:
       mov     cx, 4                   ; Nibbnle counter
       mov     dx, ss:[si]             ; Get the word
       shl     edx, 16
       add     si, 2                   ; Next argument

Debug_Hex1:
       mov     eax, edx                ; Get the number
       shr     eax, 12 + 16            ; Get the top nibble
       mov     ah, DEBUG_ATTRIB        ; Charater attribute
       add     al, '0'                 ; Add '0' to it
       cmp     al, '9'                 ; Is > '9' ?
       jle     Debug_Hex2
       add     al, 'A' - '9' - 1       ; Add the difference
Debug_Hex2:
       mov     gs:[di], ax             ; Store the character
       add     di, 2                   ; Increment the destination address
       shl     edx, 4                  ; Next nibble
       loop    Debug_Hex1
       jmp     Debug_Loop              ; Loop for the next character

Debug_NewLine:
       cmp     al, DEBUG_NL            ; Is a code a new line?
       jnz     Debug_Print
       mov     di, fs:[DebugNLIndex]   ; Get the index of the next line
       mov     cx, di
       add     cx, 160                 ; Calculate the next new line
       mov     fs:[DebugNLIndex], cx   ; Store the next new line index
       inc     bx                      ; Increment the string pointer
       jmp     Debug_Loop              ; Next character

Debug_Print:
       mov     ah, DEBUG_ATTRIB        ; Charater attribute
       mov     gs:[di], ax             ; Store the character
       inc     bx                      ; Increment the string pointer
       add     di, 2                   ; Increment the destination address
       jmp     Debug_Loop              ; Loop for the next character
DebugExit:
       mov     fs:[DebugIndex], di     ; Store the debug index

       popad
       pop     bp
       ret                             ; Return control to the caller
DebugOut       endp


       Subttl  Empty_8042
       Page    +
;******************************************************************************
;
;      Empty_8042
;
;******************************************************************************
;
;      This routine empties 8042 chip's input and output buffers.
;
;******************************************************************************
Empty_8042     proc

       in      al, (64h)               ; 8042 status port
       test    al, 1                   ; Output buffer clean?
       jz      check_inpbuf            ; yes - go to input buffer check
       in      al, (60h)               ; Get a byte from the output buffer
       jmp     Empty_8042              ; Poll output buffer
check_inpbuf:
       in      al, (64h)               ; 8042 status port
       test    al, 2                   ; Is input buffer clean?
       jnz     Empty_8042              ; Loop back and poll for again
       ret                             ; Return control to the caller

Empty_8042     endp


       Subttl  EnableA20
       Page    +
;******************************************************************************
;
;      EnableA20
;
;******************************************************************************
;
;      This routine enables A20 line
;
;******************************************************************************
EnableA20      proc

       call    Empty_8042              ; Now we have to enable A20 line
       mov     al, 0D0h
       out     (64h), al               ; Read Output Port command
wait_8042:
       in      al, (64h)               ; read the status
       test    al, 1                   ; byte is available?
       jz      wait_8042               ; poll for a byte

       in      al, (60h)               ; read a byte
       or      al, 2                   ; enable A20 line
       mov     ah, al                  ; store a byte in ah
       mov     al, 0D1h                ; Write Output Port command
       out     (64h), al
       mov     al, ah                  ; original byte with A20 enabled
       out     (60h), al               ; new byte with A20 enabled

       call    Empty_8042              ; Perform a delay to settle A20
       mov     al, 0FFh                ;  Pulse output port
       out     (64h), al
       call    Empty_8042              ;  wait for 8042 to accept next command

       lea     bx, fs:[Msg_8042]
       call    DebugOut
       ret                             ; Return control to the caller

EnableA20      endp

       Subttl  _ExecuteOS
       Page    +
;******************************************************************************
;
;      _ExecuteOS
;
;******************************************************************************
;
;      This function boots the kernel.
;
;      Registers on entry:
;
;
;      Registers on exit:
;
;
;******************************************************************************
_ExecuteOS     proc

       push    bp                      ; Build a standard stack frame
       mov     bp, sp
       pushad                          ; Push all registers on the stack
       push    ds
       push    es
       push    fs
       push    gs

;------------------------------------------------------------------------------
; Look for the presence of Yaos and some other DPMI hosts
;------------------------------------------------------------------------------

       mov     ax, 163Fh               ; Yaos installation check
       int     2Fh                     ; Multiplex interrupt
       or      ax, ax                  ; Yaos is present?
       jnz     Check2

       mov     si, ss:[bp.Boot_pBootRec]
       bts     ds:[si.Boot_Flags], 15
       mov     ds:[si.Boot_EIP], edx
       jmp     Loader_V86

Check2:
       mov     ax, 1687h               ; DPMI verify
       int     2Fh                     ; Multiplex interrupt
       or      ax, ax                  ; Other DPMI host is up?
       jnz     Check3

       mov     si, ss:[bp.Boot_pBootRec]
       bts     ds:[si.Boot_Flags], 14
       jmp     Loader_V86

Check3:
;------------------------------------------------------------------------------
; Store the contents of the segment registers and set new segment values
;------------------------------------------------------------------------------
       cli                             ; Disable interrupts during init

       mov     ebx, seg LocalData      ; Set up the local data segment
       mov     fs, bx
       assume  fs:seg LocalData        ; fs is now set up for local data seg.

       shl     ebx, 4                  ; Shift it by 4
       mov     fs:[ThisData4], ebx     ; And store it for later use

       mov     eax, 0B800h             ; Set the debugging display segment
       mov     gs, ax

       mov     ax, ds                  ; Get the current data segment
       mov     fs:[Data16], ax         ; And store it
       shl     eax, 4                  ; Shift it by 4
       mov     fs:[Data4], eax         ; Store linear address

       xor     eax, eax
       mov     ax, cs                  ; Get the current code segment
       mov     fs:[Code16], ax         ; And store it
       shl     eax, 4                  ; Shift it by 4
       mov     fs:[Code4], eax         ; Store linear address

       mov     fs:[StackPointer], esp  ; Store the real-mode stack pointer
       mov     ax, ss
       mov     fs:[StackSegment], ax   ; and segment

; gs     -> global data segment of the text screen at B800h
; fs     -> local data segment

; Get the pointer to a kernel boot record as passed from the "C" portion

       mov     si, ss:[bp.Boot_pBootRec]

; Patch the debug out function with the ret if verbose was on

       bt      ds:[si.Boot_Flags], FLAGS_VERBOSE
       jc      KernelLin

       mov     bx, offset cs:DebugOut
       mov     byte ptr cs:[bx], 0C3h

;------------------------------------------------------------------------------
; Store the linear address of the kernel image and size
;------------------------------------------------------------------------------
KernelLin:
       mov     eax, ds:[si.Boot_pBuf]
       mov     fs:[pKernel], eax

       push    dx
       push    eax
       lea     bx, fs:[Msg_kboot]
       call    DebugOut
       add     sp, 6

;------------------------------------------------------------------------------
; Find the kernel starting offset and embed it in the jump instruction below
;------------------------------------------------------------------------------

       mov     eax, ds:[si.Boot_EIP]
       mov     bx, offset cs:cstart
       mov     cs:[bx], eax

;------------------------------------------------------------------------------
; Reset the floppy disk drive motor in the case we used floppy
;------------------------------------------------------------------------------

       mov     dx, 03F2h
       xor     al, al
;       out     dx, al

;------------------------------------------------------------------------------
; Disable NMI
;------------------------------------------------------------------------------

       mov     al, DISABLE_NMI         ; Disable NMI
       out     (RTC_CMD), al

;------------------------------------------------------------------------------
; Enable the A20 line
;------------------------------------------------------------------------------

       call    EnableA20               ; Enable A20 address line

;------------------------------------------------------------------------------
; Set the base address of the GDT table and load a temporary GDT
;------------------------------------------------------------------------------

       mov     eax, fs:[ThisData4]     ; Get the local data segment
       add     eax, offset GDT         ; Get the linear address of GDT
       mov     fs:[GDT_Address], eax   ; Store the linear addr of the GDT table

       db      OPERAND_SIZE
       lgdt    fword ptr fs:[GDT_Descriptor]

       push    eax
       lea     bx, fs:[Msg_gdt]
       call    DebugOut
       pop     eax

;------------------------------------------------------------------------------
; Enter the protected mode
;------------------------------------------------------------------------------

       mov     ecx, cr0                ; Enter the protected mode
       or      ecx, 1
       mov     cr0, ecx

       jmp     ClearPF1                ; Clear the prefetch queue
ClearPF1:

       pushf                           ; Mask the initial eflags
       and      word ptr ss:[esp], INITIAL_FLAGS_MASK
       popf

       lea     bx, fs:[Msg_pmode]
       call    DebugOut

;------------------------------------------------------------------------------
; Load selectors
;------------------------------------------------------------------------------
; Load data selector with a descriptor that allows to address complete memory
; space with the base address of 0

       mov     ax, SEL_GLOBAL
       mov     ds, ax
       mov     es, ax

       lea     bx, fs:[Msg_seg]
       call    DebugOut

;------------------------------------------------------------------------------
; Find the total amount of physical memory installed
;------------------------------------------------------------------------------

       mov     eax, 1024 * 1024        ; Start with the first megabyte
MemLoop:
       mov     ebx, dword ptr ds:[eax] ; Get the existing dword
       mov     ecx, ebx
       not     ecx                     ; bit-wise complement
       mov     ds:[eax], ecx           ; try to store new value
       mov     ds:[eax + 4], ebx       ; make a bus noise
       cmp     ds:[eax], ecx           ; is the original number still there?
       jnz     MemFailed
       mov     ds:[eax], ebx           ; Return the original value
       add     eax, 4096               ; Next page
       jmp     MemLoop
MemFailed:
       shr     eax, 12                 ; This is the total number of pages
       mov     fs:[nPages], eax

;------------------------------------------------------------------------------
; Copy the page directory to the proper address
;------------------------------------------------------------------------------

       mov     esi, fs:[ThisData4]
       add     esi, offset fs:Page_Directory_Image
       mov     edi, PHY_PD
       mov     ecx, 4096
       rep     movsb byte ptr es:[edi], byte ptr ds:[esi]

;------------------------------------------------------------------------------
; Copy page tables to the proper address
;------------------------------------------------------------------------------

       mov     esi, fs:[ThisData4]
       add     esi, offset fs:Page_Table_Image
       mov     edi, PHY_PT
       mov     ecx, 4096 * 4
       rep     movsb byte ptr es:[edi], byte ptr ds:[esi]

;------------------------------------------------------------------------------
; Load the page directory address and enable paging
;------------------------------------------------------------------------------

       mov     eax, PHY_PD             ; Physical page directory
       mov     cr3, eax

       mov     ecx, cr0                ; Enable paging and set WP bit to enable
       or      ecx, (1 shl 31) or (1 shl 16)
       mov     cr0, ecx                ; Page faults on Supervisor write

;------------------------------------------------------------------------------
; Copy the kernel image high to LIN_KERNEL
;------------------------------------------------------------------------------
cont:
       mov     edi, LIN_KERNEL
       mov     esi, fs:[pKernel]
Copy_Loop:
       push    esi
       mov     esi, ds:[esi]           ; Get the address of a chunk
       or      esi, esi                ; Is that a last chunk (NULL) ?
       jz      Copy_End                ; Jump forward if it is

       mov     ecx, BUFFER_SIZE
       rep     movsb byte ptr es:[edi], byte ptr ds:[esi]

       pop     esi                     ; Restore pointer to chunks
       add     esi, 4                  ; Next address
       jmp     Copy_Loop

Copy_End:
       pop     esi                     ; Restore pointer to chunks
       lea     bx, fs:[Msg_krel]
       call    DebugOut

;------------------------------------------------------------------------------
; Prepare the returning address.  The selector for return is SEL_CODE16, and
; the offset is the linear address of the label `Loader_Ret';
; For the V86 mode return, use the function `Loader_V86'.
;------------------------------------------------------------------------------

       mov     edx, fs:[Code4]
       add     edx, offset cs:Loader_Ret

       mov     cx, fs:[Code16]
       shl     ecx, 16
       mov     cx, offset cs:Loader_V86

;------------------------------------------------------------------------------
; Set the code selector and jump into the protected 32 bit kernel startup code
;------------------------------------------------------------------------------

       lea     bx, fs:[Msg_jmp]
       call    DebugOut

       mov     edi, fs:[nPages]        ; Get the number of memory pages in edi

; Testing: Skip the kernel and return to the loader.
;
;      jmp     Loader_Ret

       db      OPERAND_SIZE
       db      JMP_FAR
cstart dd      0
       dw      SEL_CODE

_ExecuteOS     endp


       Subttl  Loader_Ret
       Page    +
;******************************************************************************
;
;      Loader_Ret
;
;******************************************************************************
;
;      Kernel returns here to unload itself.
;
;******************************************************************************
Loader_Ret     proc

;------------------------------------------------------------------------------
; Reload selectors with the base of 0 and limit FFFF
;------------------------------------------------------------------------------

       mov     ax, SEL_DATA16
       mov     gs, ax
       mov     fs, ax
       mov     ds, ax
       mov     es, ax
       mov     ss, ax

;------------------------------------------------------------------------------
; Back to the Real mode and disable paging
;------------------------------------------------------------------------------

       mov     eax, cr0
       and     eax, 07FFFFFFEh
       mov     cr0, eax                ; Real mode of operation

       xor     eax, eax
       mov     cr3, eax                ; Flush TLB

;------------------------------------------------------------------------------
; Reload all the segment registers
;------------------------------------------------------------------------------

       mov     ax, 0B800h
       mov     gs, ax

       mov     ax, seg LocalData       ; Set up the additional segment
       mov     fs, ax                  ; that is local data segment
       mov     ds, ax

       assume  fs:seg LocalData        ; fs is now set up for local data seg.
       assume  ds:seg LocalData

       mov     ax, ds:[StackSegment]   ; We can use ds now...to get the stack
       mov     ss, ax                  ; segment and..
       mov     esp, ds:[StackPointer]  ; ..pointer

; gs     -> global data segment of the text screen at B800h
; fs     -> local data segment
; ds     -> local data segment
; ss:esp -> old stack segment:pointer

;------------------------------------------------------------------------------
; Clean up IDT to base address of 0 and a limit of 3FFh (256 interrupts)
;------------------------------------------------------------------------------

       db      OPERAND_SIZE
       lidt    fword ptr ds:[Clean_IDT_Descriptor]

;------------------------------------------------------------------------------
; Reload code segment by using a far return to the code below
;------------------------------------------------------------------------------

       push    word ptr ds:[Code16]
       push    word ptr offset cs:Loader_Ret2

       retf                            ; Jump right here...
Loader_Ret2:                           ; bingo!

;------------------------------------------------------------------------------
; Enable NMI
;------------------------------------------------------------------------------

       mov     al, ENABLE_NMI          ; Enable NMI
       out     (RTC_CMD), al

;------------------------------------------------------------------------------
; Pop all the registers and return to the loader
;------------------------------------------------------------------------------

       pop     gs                      ; Pop segment registers
       pop     fs
       pop     es
       pop     ds
       popad                           ; Restore all registers from the stack
       pop     bp
       sti                             ; Enable interrupts
       ret                             ; Return control to the Loader

Loader_Ret     endp


       Subttl  Loader_V86
       Page    +
;******************************************************************************
;
;      Loader_V86
;
;******************************************************************************
;
;      Kernel returns here to run the system VM in the Virtual-86 mode.
;
;      Registers on entry:
;
;
;      Registers on exit:
;
;
;******************************************************************************
Loader_V86     proc

       pop     gs                      ; Pop segment registers
       pop     fs
       pop     es
       pop     ds
       popad                           ; Restore all registers from the stack
       pop     bp
       ret                             ; Return control to the Loader

Loader_V86     endp



_TEXT  ends                            ; End of the 16 bit code segment

       End                             ; End of the module

