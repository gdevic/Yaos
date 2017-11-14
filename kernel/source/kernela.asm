;**********************************************************************
;
; Module:       Kernela.asm
;
; Revision:     1.00
;
; Date:         02/21/96
;
; Author:       Goran Devic
;
;**********************************************************************
;
; Module Description:
;
;   This file contains the basic kernel assembler code.
;   The main starting point is at "kstart", called by the kernel loader.
;   At that point, we are in protected mode, and new selectors have to
;   be set.
;
;**********************************************************************########
;
; Major Changes:
;
;   DATE     REV   DESCRIPTION OF CHANGES                 AUTHOR
; --------   ----  -----------------------------------   -----------
; 02/21/96   1.00  Original                              Goran Devic
; 07/03/96   1.10  Kernel moves itself above 1 Meg       Goran Devic
; --------   ----  -----------------------------------   -----------
;**********************************************************************########
.386p

Include        Kernel.inc              ; Include kernel header

KERNEL_LOW     equ   10020h            ; low kernel loading address
KERNEL_LIN     equ   110000h           ; kernel linear base address
STKSIZE        equ   4096              ; stack size in words


; Selectors
;
GDT_ENTRIES    equ   5                 ; number of GDT entries


;**********************************************************************########
; External data structures
;**********************************************************************########

EXTRN          tss               :byte
EXTRN          IDT               :byte
EXTRN          Ints_CPU          :near
EXTRN          Ints_IRQ          :near

;**********************************************************************
; Internal data used outside of this module
;**********************************************************************

GLOBAL         GDT               :byte
GLOBAL         TopOfStack        :dword
GLOBAL         HeapStart         :byte

;**********************************************************************
; External functions that are being used by this module
;**********************************************************************

EXTRN          init_tss          :near
EXTRN          Yaos              :near
EXTRN          printk            :near

;**********************************************************************
; Public functions provided by this module
;**********************************************************************

GLOBAL         __GETDS           :near
GLOBAL         __8087            :near
GLOBAL         Int14_Handler     :near
GLOBAL         sys_call          :near

PUBLIC         IRQ_0_Startup
PUBLIC         IRQ_1_Startup
PUBLIC         IRQ_2_Startup
PUBLIC         IRQ_3_Startup
PUBLIC         IRQ_4_Startup
PUBLIC         IRQ_5_Startup
PUBLIC         IRQ_6_Startup
PUBLIC         IRQ_7_Startup
PUBLIC         IRQ_8_Startup
PUBLIC         IRQ_9_Startup
PUBLIC         IRQ_A_Startup
PUBLIC         IRQ_B_Startup
PUBLIC         IRQ_C_Startup
PUBLIC         IRQ_D_Startup
PUBLIC         IRQ_E_Startup
PUBLIC         IRQ_F_Startup


;**********************************************************************
; Start code segment
;**********************************************************************

_TEXT          segment   use32 para public 'CODE'

;**********************************************************************
;
; The main kernel starting point
;
; We come here from the boot loader (bootsect.c) that put us in the
; protected mode with the following selectors set:
;      ds: base = 10020h,  limit = ffffff,  page granular
;      cs: base = 10020h,  limit = ffffff,  page granular
;
; The kernel length in bytes is passed in the ecx register.
;
;**********************************************************************
kstart:
               cli                     ; Disable interrupts
               cld                     ; Set the direction bit

; Move the complete kernel above 1 Mb at the address 110000h
; linear.  Now it is at 10020h linear.  Use the kernel size that
; is in the ecx register.  Note that the data selector is set by
; the boot sector loader, so everything is offsetted by 10020h
                                       ; Set the destination address
               mov     edi, KERNEL_LIN - KERNEL_LOW
               mov     esi, 0              ; Set the source address [ds:0]
               shr     ecx, 2              ; Divide the size by 4
               inc     ecx                 ; Add 4 bytes for an odd case
kern_move:     mov     eax, [ds:esi]       ; Get the dword from the source
               mov     [ds:edi], eax       ; Store it above 1 Mb
               add     esi, 4              ; Advance the source address
               add     edi, 4              ; Advance the destination address
               loop    kern_move           ; decrement ecx and loop

; Set up new global descriptor table
;    cs = ds = es = ss = KERNEL_LIN
;    fs = gs = 0 (4G)

           lgdt    fword [gdt_ptr]     ; Set the global descriptor table
                                       ; register
           mov     ax, DATASEL         ; Get the data selector value
           mov     ss, ax              ; Set the stack selector
           mov     esp, offset TopOfStack ; We can address the stack now
                                       ; and call a subroutine,
           call    __GETDS             ; Set all selectors

; It is now safe to jump at its upper copy that resides at 110000h.
; This will effectively reload the cs selector.

           db      0eah                ; jump far
           dd      offset jump_up      ; at the following offset
           dw      CODESEL             ; and set the code selector
jump_up:

;
; Load interrupt descriptor table pointer
;
       lidt    fword [idt_ptr]

;
; Initialize Task State Segment
;
       lea     eax, tss                ; Get the address of the tss structure
       add     eax, KERNEL_LIN         ; Add the absolute kernel offset
       mov     word ptr [ GDT4+2 ], ax ; Load the low 2 bytes of the address
       shr     eax, 16                 ; Get the upper byte
       mov     byte ptr [ GDT4+4 ], al ; Load the upper byte of the address

       call    init_tss                ; Initialize tss (in "tss.c")

       mov     ax, TSS_SEL             ; Load task segment register
       ltr     ax                      ; as the GDT4

       jmp     Yaos                    ; Jump to the main "C" kernel entry point

;**********************************************************************
;
;  Interrupt handlers for the dual interrupts that share external and CPU
;  numbers (IRQ 0-15)
;
;**********************************************************************

IRQ_Setup macro FUNCTION, IRQ_PORT, INDEX
FUNCTION       proc
Local @cpu_intr, @jump_handler

       push    eax                     ; Reserve an entry on the stack
       push    eax                     ; Store eax register
       push    ebx                     ; Store ebx register
       mov     ax, ds                  ; Get the current data selector
       push    eax                     ; Store it on the stack
       mov     ax, DATASEL             ; Get the new data selector value
       mov     ds, ax                  ; Set the new data selector
       lea     ebx, Ints_CPU+INDEX*4   ; Assume CPU/software interrupt
       mov     al, 0Bh                 ; PIC OCW3, read servicing IRQ
       out     IRQ_PORT, al            ; Read servicing IRQ
       in      al, IRQ_PORT            ; Read the IRQ in service
       or      al, al                  ; Any external interrupt in service?
       jz      @cpu_intr               ; No, use CPU interrupt table
       lea     ebx, Ints_IRQ+INDEX*4   ; Load the irq handlers table
@cpu_intr:                             ; Service CPU/software interrupt
       mov     eax, [ebx]              ; Get the address of the service routine
       or      eax, eax                ; Does the handler exists?
       jnz     @jump_handler           ; If yes, jump to it
       pop     eax                     ; Handler is not registered, restore ds
       mov     ds, ax                  ; Set old data selector
       pop     ebx                     ; Restore ebx register
       pop     eax                     ; Restore eax register
       iretd                           ; Return from the interrupt, no handler
@jump_handler:                         ; Execute the interrupt handler
       mov     ss:[esp+12], eax        ; Store it on the stack
       pop     eax                     ; Get old ds
       mov     ds, ax                  ; Restore old data selector
       pop     ebx                     ; Restore ebx register
       pop     eax                     ; Restore eax register
       ret                             ; Simple return to the service routine
FUNCTION       endp
endm

       IRQ_Setup     IRQ_0_Startup, 020h, 0
       IRQ_Setup     IRQ_1_Startup, 020h, 1
       IRQ_Setup     IRQ_2_Startup, 020h, 2
       IRQ_Setup     IRQ_3_Startup, 020h, 3
       IRQ_Setup     IRQ_4_Startup, 020h, 4
       IRQ_Setup     IRQ_5_Startup, 020h, 5
       IRQ_Setup     IRQ_6_Startup, 020h, 6
       IRQ_Setup     IRQ_7_Startup, 020h, 7
       IRQ_Setup     IRQ_8_Startup, 0A0h, 8
       IRQ_Setup     IRQ_9_Startup, 0A0h, 9
       IRQ_Setup     IRQ_A_Startup, 0A0h, 10
       IRQ_Setup     IRQ_B_Startup, 0A0h, 11
       IRQ_Setup     IRQ_C_Startup, 0A0h, 12
       IRQ_Setup     IRQ_D_Startup, 0A0h, 13
       IRQ_Setup     IRQ_E_Startup, 0A0h, 14
       IRQ_Setup     IRQ_F_Startup, 0A0h, 15


;**********************************************************************
;
; Interrupt 14 - Page Fault Handler
;
;**********************************************************************
Int14_Handler:

        push  eax                ; Store some registers
        push  ebx
        push  ecx

; Supervisor stack now contains:
;
VIRT_GS      equ  48     ;  virt GS          (+48) -
VIRT_FS      equ  44     ;  virt FS          (+44) |
VIRT_DS      equ  40     ;  virt DS          (+40) |
VIRT_ES      equ  36     ;  virt ES          (+36) |
VIRT_SS      equ  32     ;  virt SS          (+32) |
VIRT_ESP     equ  28     ;  virt ESP         (+28) - Only from V86 mode
VIRT_EFLAGS  equ  24     ;  virt EFLAGS      (+24)
VIRT_CS      equ  20     ;  virt CS          (+20)
VIRT_EIP     equ  16     ;  virt EIP         (+16)
VIRT_ERROR   equ  12     ;  virt Error Code  (+12)
VIRT_AX      equ  8      ;  eax              (+ 8)
VIRT_BX      equ  4      ;  ebx              (+ 4)
VIRT_CX      equ  0      ;  ecx              (+ 0) <- esp

        call  __GETDS                      ; Set up ds, es, fs and gs selectors

        mov    eax, [esp+VIRT_ERROR]
        mov    ebx, cr2
        push   eax
        push   ebx
        push   offset i14_msg
        call   printk

        int    1

;**********************************************************************
;
; __GETDS is called by the Watcom compiler in an interrupt function
; to set up the selectors.
;
__GETDS:
           push    eax                 ; Store eax register on the stack
           mov     ax, DATASEL         ; Get the data selector value
           mov     ds, ax              ; Set the data selector
           mov     es, ax              ; Set the es
           mov     ax, GLOBALSEL       ; Get the global selector value
           mov     fs, ax              ; Set the fs
           mov     gs, ax              ; Set the gs
           pop     eax                 ; Restore eax register
__8087:    ret                         ; Return to the caller


;**********************************************************************
;
; sys_call is a stub for linking the c-library.  System call can not
; be performed from the kernel itself
;
sys_call:
           int     1                   ; Break into the debugger



;**********************************************************************

_TEXT      ends

;**********************************************************************

_DATA      segment   use32 para public 'DATA'


gdt_ptr:   dw        (GDT_ENTRIES * 8) - 1
           dd        offset GDT + KERNEL_LIN

idt_ptr:   dw        (256 * 8) - 1
           dd        offset IDT + KERNEL_LIN

;**********************************************************************
;
; Global Descriptor Table:
;
;   GDT0 - Null descriptor, not directly used    NULLSEL
;   GDT1 - Kernel code segment                   CODESEL
;   GDT2 - Kernel data segment                   DATASEL
;   GDT3 - Global data segment                   GLOBALSEL
;   GDT4 - Task state segment                    TSS_SEL
;
;**********************************************************************
           align     16
GDT:

GDT0:      ; Dummy descriptor
           dw        0, 0, 0, 0

GDT1:      ; 32 bit kernel code
           dw        0ffffh                   ; limit 15..0 (65536*4K)
           dw        KERNEL_LIN mod 65536     ; base 15..0
           db        KERNEL_LIN shr 16        ; base 23..16
           db        10011010b                ; 32 bit code, present, dpl 0,
                                              ; readable, nonconf, accessed
           db        11000000b                ; page granular, 32 bit, limit 31..28
           db        KERNEL_LIN shr 24        ; base 31..24

GDT2:      ; 32 bit kernel data and stack
           dw        0ffffh                   ; limit 15..0
           dw        KERNEL_LIN mod 65536     ; base 15..0
           db        KERNEL_LIN shr 16        ; base 23..16
           db        10010011b                ; 32 bit data, present, dpl 0,
                                              ; exp up, writable, accessed
           db        11001111b                ; page granular, 32 bit, limit 31..28
           db        KERNEL_LIN shr 24        ; base 31..24

GDT3:      ; 32 bit global memory access
           dw        0ffffh                   ; limit 15..0
           dw        0                        ; base 15..0
           db        0                        ; base 23..16
           db        10010011b                ; 32 bit data, present, dpl 0,
                                              ; exp up, writable, accessed
           db        11001111b                ; page granular, 32 bit, limit 31..28
           db        00                       ; base 31..24

GDT4:      ; Task State Selector
           dw        02068h                   ; limit 15..0 - 104b + 8K ioperm
           dw        ?                        ; base 15..0 - To be filled on run
           db        ?                        ; base 23..16
           db        10001001b                ; present, dpl=0, not busy
           db        00000000b                ; byte granular, limit 31..28
           db        00                       ; base 31..24


;**********************************************************************

i14_msg:   db      13
           db      "Page Fault at %08X, Error code: %08X", 13, 0

msgi: db  13,"Calling @%08X\n", 13, 0

_DATA      ends


;
; Stack segment sets the stack area of the kernel process
;
_STACK     segment   use32 para public 'STACK'

           dd        STKSIZE dup (?)
TopOfStack:dd        ?

HeapStart:  db      ?

_STACK     ends

;
; Set the ordering of the segment groups
;
DGROUP     group _DATA, _STACK

           end       kstart
