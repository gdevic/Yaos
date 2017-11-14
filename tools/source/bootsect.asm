;******************************************************************************
;                                                                             *
;   Module:     BootSect.Asm                                                  *
;                                                                             *
;   Revision:   1.00                                                          *
;                                                                             *
;   Date:       3/12/96                                                       *
;                                                                             *
;   Author:     Goran Devic                                                   *
;                                                                             *
;******************************************************************************
;
;   Module Description:
;
;      This module contains the code for the boot sector of a floppy disk
;      of the Yaos kernel.
;
;******************************************************************************
;                                                                             *
;   Changes:                                                                  *
;                                                                             *
;   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
; --------   ----  ---------------------------------------------  ----------- *
; 3/12/96    1.00  Original                                       Goran Devic *
; 3/14/96    1.10  Added Checksum                                 Goran Devic *
; 3/30/97    1.11                                                 Goran Devic *
; --------   ----  ---------------------------------------------  ----------- *
;******************************************************************************
COMMENT *

       This program resides on the first sector of a floppy disk and boots the
       kernel.

       On entry: CS = 0000h
                 DS = 07C0h

*******************************************************************************

.386p                                  ; Use 386 processor in protected mode


; Defines for data located on a floppy disk:
DIR_START_SEC  equ   2                 ; Directory information sector
DIR_LENGTH     equ   14                ; Directory number of sectors
DATA_SECTOR    equ   31                ; First data sector on a floppy
DIR_SEG        equ   1000h             ; Sector to load directory in memory

; Initial segments and selectors
BOOT_SEG       equ   07C0h             ; Initail segment of this code
KERNEL_SEG     equ   1000h             ; Kernel load segment
CODESEL        equ   0008h             ; Initial code selector value
DATASEL        equ   0010h             ; Initial data selector value

; The linear kernel address is 32 bytes more due to the .EXP header of a
; kernel exectable
KERNEL_LIN     equ   ((KERNEL_SEG * 16) + 32)
BOOT_LIN       equ   (BOOT_SEG * 16)


;------------------------------------------------------------------------------
STARTUP_CODE   segment use16

Boot           proc

       jmp     short Bootup            ; Jump forward to skip the boot record
       nop

;------------------------------------------------------------------------------
; Boot record
;
; To save some space, variable FileSize is stored in this structure since it is
; not used after the boot sector had already been loaded loaded.
;
              db     "YAOS 96 "
              dw     512
              db     1
              dw     1
              db     2
              dw     224
              dw     2880
              db     0f0h
              dw     9
              dw     18
              dw     2
              dw     0
              dw     0
              dd     0
              dw     0
              db     029h
              dd     -1
              db     "YAOS 96 :-)"
              db     "FAT12   "

;------------------------------------------------------------------------------
; Data section - initial global descriptor table,
; use gdt descriptor entry 0 as the pointer to itself
;
GDT:
GDT0:        ; Dummy descriptor

gdt_ptr:     dw      (3 * 8) - 1       ; 3 descriptors
gdt_fix:     dw      ?                 ; fixup place
             dw      0                 ; keep it 4 words
             dw      0

GDT1:        ; 32 bit kernel code
             dw      0ffffh               ; limit 15..0
             dw      KERNEL_LIN mod 65536 ; base 15..0
             db      KERNEL_LIN shr 16    ; base 23..16
             db      10011010b            ; 32 bit code, present, dpl 0,
                                          ; readable, nonconf, accessed
             db      11001111b            ; page granular, 32 bit, limit 31..28
             db      00                   ; base 31..24

GDT2:        ; 32 bit kernel data
             dw      0ffffh               ; limit 15..0
             dw      KERNEL_LIN mod 65536 ; base 15..0
             db      KERNEL_LIN shr 16    ; base 23..16
             db      10010011b            ; 32 bit data, present, dpl 0,
                                          ; exp up, writable, accessed
             db      11001111b            ; page granular, 32 bit, limit 31..28
             db      00                   ; base 31..24

FileSize:    dd      0                 ; Kernel size in bytes


;------------------------------------------------------------------------------
; Boot code starts here
;------------------------------------------------------------------------------

Bootup:
       cld                             ; Disable interrupts

       mov     ax, BOOT_SEG            ; Just to be on the safe side :-)
       mov     ds, ax                  ; Load DS with the expected value
       assume  ds:BOOT_SEG             ; Assume it now

       mov     si, offset Copyright    ; Print the copyright message
       call    printstr


; Load the kernel
;
; We have to load the directory from the floppy and search for the kernel file.
; We will load side 1 sectors 2 trough 15 and scan for the known kernel name.
;
load_dir:
       mov     ax, 200h + DIR_LENGTH   ; BIOS Disk service to read 2 sector(s)
       mov     bx, DIR_SEG             ; Into the segment DIR_SEG
       mov     es, bx                  ; set the segment register
       xor     bx, bx                  ; Offset zero
       mov     cx, DIR_START_SEC       ; First sector to load (cl), cyl (ch)=0
       mov     dx, 0100h               ; head (dh)=1 track (dl)=0
       int     13h                     ; Call the service
       jnc     success_dir             ; Proceed if successful load

       xor     ah, ah                  ; Reset the floppy drive
       mov     dl, ah                  ; since the load was unsuccessful
       int     13h                     ; BIOS Disk service to reset

       jmp     short load_dir          ; try again if not successful

success_dir:
       ; We search for the kernel name within the directory table
       ; es still points to DIR_SEG
       ;
       xor     di, di                  ; es:di points to the directory table
       mov     cx, DIR_LENGTH*16       ; so many entries (fdd has 224)

search_dir:
       cmp     dword ptr[ es:di+0 ], "REK_" ;==> "_KERnel_"
       jnz     skip1                   ; Compare first 4 bytes and jump if not equal

       cmp     dword ptr[ es:di+4 ], "_LEN"  ; "_kernEL_"
       jz      kernel_found            ; Compare second 4 bytes and jump if equal

skip1:
       add     di, 32                  ; Next name within the directory
       loop    search_dir              ; Loop back for each directory entry

; No kernel was found.  Can't do anything, but die...
;
       mov     si, offset Missing      ; Message about the missing kernel
       call    printstr                ; Print the message
die:
       jmp     die                     ; Infinite loop until user reboots


; Kernel was found, and es:di points to its directory entry.
; Get the kernel size and load it.
;
kernel_found:
       mov     ecx,[es:di+01Ch]        ; Get the file size 4 bytes
       mov     [FileSize], ecx         ; Store it for later
       shr     ecx, 9                  ; div 512 to
       inc     cx                      ; get the number of sectors to load

       mov     ax, [ es:di+01Ah ]      ; Get the starting cluster (sector)
       add     ax, DATA_SECTOR         ; starting data sector
       mov     bx, KERNEL_SEG          ; loading segment

load_sector:
       push    ax                      ; Store ax
       push    bx                      ; Store bx
       push    cx                      ; Store cx

       mov     ch, 36                  ; Prepare for division by 36
       div     ch                      ; Get the cylinder number
       mov     ch, al
       xor     dx, dx                  ; head 0, drive 0
       cmp     ah, 18                  ; Head 1?
       jl      head0
       sub     ah, 18
       inc     dh                      ; head 1
head0:
       mov     cl, ah                  ; sector number + 1
       inc     cl

       mov     es, bx                  ; segment to load
       xor     bx, bx                  ; es:0

       mov     ax, 0201h               ; read 1 sector, functon 2
       int     13h
       jnc     success_sector

       xor     ah, ah                  ; Unsuccessful attempt
       mov     dl, ah                  ; reset the floppy drive
       int     13h                     ; BIOS disk service
       jc      load_sector             ; Try again

success_sector:

       pop     cx                      ; Prepare to load next sector
       pop     bx                      ; Restore cx, bx and ax
       pop     ax

       add     bx, 512/16              ; next 512 block
       inc     ax                      ; next sector on the disk
       loop    load_sector             ; load next sector

; Signal successful load by a message "Kernel "
;
       mov     si, offset Kernel
       call    printstr                ; Print it

; Reset the floppy disk motor
       mov     dx, 03F2h
       xor     al, al
       out     dx, al

; Loop through the loaded kernel and make a checksum;
; In fact, we will add only first dword in every 16 byte chunk
;
       mov     bx,KERNEL_SEG+2         ; Skip 32 byte header from checking
       xor     eax, eax                ; accumulator = 0
       mov     ecx, [FileSize]         ; Get the kernel size in bytes
       sub     ecx, 32+4               ; Length -= 32

checksum:
       mov     es, bx                  ; reach the segment
       add     eax, [es:0]             ; Add the dword to the checksum
       inc     bx                      ; next segment
       sub     ecx, 16                 ; Subtract 16 from the length
       jnc     checksum                ; loop back

       mov     bx, KERNEL_SEG          ; Get the starting segment of the kernel
       mov     es, bx                  ; Set it
       cmp     eax, [es:2]             ; Compare it to the checksum dword

       jz      chksumOK                ; Jump forth if passed

; Report fragmented kernel
;
       mov     si, offset Fragmented   ; Message about the fragmented kernel
       call    printstr                ; Print the message
die2:
       jmp     die2                    ; Infinite loop until user reboots

chksumOK:
       mov     si, offset Loaded
       call    printstr

; Load the kernel size into ecx register, so that will be
; the argument to the kernel main() function
       mov     ecx, [FileSize]

; Set protected mode with 32 bit code and data selectors, so
; that we can jump into the kernel
;
       cli                             ; Disable interrupts
       mov     al, 80h
       out     (70h), al               ; Disable NMI

; Now we have to enable A20
;
       call    empty_8042
       mov     al, 0D1h
       out     (64h),al
       call    empty_8042
       mov     al, 0DFh
       out     (60h),al
       call    empty_8042

; Get the starting address from the .EXP header at offset 14h
;
       mov     ax, KERNEL_SEG          ; Get the kernel .EXP header
       mov     es, ax                  ; Set data segment to the kernel segment
       mov     ebx, dword ptr es:[ 014h ]

       mov     [cstart], ebx           ; Set the kernel starting address

; Load temporary gdt
;
       mov     ax, (offset GDT) + BOOT_LIN
       mov     [gdt_fix], ax

       lgdt    fword [gdt_ptr]

; Enter protected mode
;
       mov     eax, cr0
       or      eax, 1
       mov     cr0, eax

       jmp     flush
flush:
       mov     ax, DATASEL
       mov     ds, ax

; Jump into the protected 32 bit kernel code and set the code
; selector on a way.
;
               db    066h, 0EAh
cstart:        dd    0
               dw    CODESEL


;
; empty_8042
;
empty_8042:
       call    delay                   ; delay
       in      al,(64h)                ; 8042 status port
       test    al, 1                   ; output buffer?
       jz      no_output               ; output empty, go and test input
       call    delay                   ; output full, delay
       in      al,(60h)                ; and read data
       jmp     empty_8042              ; repeat wait for output empty
no_output:
       test    al, 2                   ; is input buffer full?
       jnz     empty_8042              ; yes - loop back and poll
;      ...
;
; Delays a little bit by jumping ahead to return
;
delay:
       jmp     return                  ; Jump ahead to return
return:
       ret                             ; Return

;
; Prints a zero-terminated string from the location ds:si
;
printstr:
       mov     ah, 0Eh                 ; BIOS teletype echo character function
       mov     al, byte ptr [si]       ; fetch the character
       or      al, al                  ; is it the end of string?
       je      return                  ; return if it is
       xor     bh, bh                  ; page 0
       push    si                      ; store pointer to string
       int     10h                     ; request BIOS Video service
       pop     si                      ; retrieve pointer to string
       inc     si                      ; next character
       jmp     printstr                ; loop back


;------------------------------------------------------------------------------
; Boot data starts here
;------------------------------------------------------------------------------

Copyright:     db    "(c) Yet Another OS", 0Dh, 0Ah, 0
Kernel:        db    "Kernel ", 0
Loaded:        db    "loaded", 0
Missing:       db    "missing", 0
Fragmented:    db    "fragmented", 0


;------------------------------------------------------------------------------
; End of the boot sector
;------------------------------------------------------------------------------

               org   510

               db    055h, 0AAh

;------------------------------------------------------------------------------
Boot           endp

STARTUP_CODE   ends

               end    Boot

