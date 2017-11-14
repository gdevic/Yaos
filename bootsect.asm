;
; BOOTSECT.ASM
;
;
; This program resides on the first sector of a floppy disk
; and boots the kernel up.
;

BOOT_SEG          equ  07C0h     ; Segment that this program resides

KERNEL_SEG        equ  3000h     ; starting segment to load kernel

DIR_START_SEC     equ  2         ; starting sector
DIR_LENGTH        equ  14        ; number of sectors for directory
DIR_SEG           equ  4000h     ; Dir table loading segment

DATA_SECTOR       equ  31        ; First data sector

; The linear kernel address is 32 bytes more due to the .EXP header
KERNEL_LIN        equ  ((KERNEL_SEG * 16) + 32)
BOOT_LIN          equ  (BOOT_SEG * 16)


.386p


STARTUP_CODE    segment use16

boot    proc

        jmp  short bootup
        nop

        ;
        ; Boot record
        ;
        db      "YAOS 0.1"
        dw      512
        db      1
        dw      1
        db      1
        dw      224
        dw      2880
        db      0f0h
        dw      9
        dw      18
        dw      2
        dw      0
        dw      0
        dd      0
        dw      0
        db      029h
        dd      -1
        db      "YAOS Ver AA"
        db      "FAT12  "

        ; Data

Table1: db   "(c)Yet Another OS ", 0

        align   2

gdt_ptr:     dw      (3 * 8) - 1     ; 3 descriptors
gdt_fix:     dw      ?
             dw      0

             ;
             ; Global descriptor table
             ;
CODESEL      equ     0008h
DATASEL      equ     0010h

GDT:

GDT0:        ; Dummy descriptor
             dw      0, 0, 0, 0

GDT1:        ; 32 bit kernel code
             dw      007ffh          ; limit 15..0
             dw      KERNEL_LIN mod 65536
                                     ; base 15..0
             db      KERNEL_LIN shr 16
                                     ; base 23..16
             db      10011010b       ; 32 bit code, present, dpl 0,
                                     ; readable, nonconf, accessed
             db      11000000b       ; page granular, 32 bit, limit 31..28
             db      00              ; base 31..24

GDT2:        ; 32 bit kernel data
             dw      0ffffh          ; limit 15..0
             dw      KERNEL_LIN mod 65536
                                     ; base 15..0
             db      KERNEL_LIN shr 16

                                     ; base 23..16
             db      10010011b       ; 32 bit data, present, dpl 0,
                                     ; exp up, writable, accessed
             db      11001111b       ; page granular, 32 bit, limit 31..28
             db      00              ; base 31..24


        ;
        ; First, print the boot message and then load the system
        ;
bootup:
        cld

        mov  ax, BOOT_SEG
        mov  ds, ax
        assume ds:BOOT_SEG

        mov  si, offset Table1
lp:
        mov  ah, 0Eh
        mov  al, byte ptr [si]
        or   al, al
        je   cont
        xor  bh, bh
        push si
        int  10h
        pop  si
        inc  si
        jmp  short lp

cont:
        mov  ax, 0     ; Keypress
        int  16h

        ; Load the kernel
        ;
        ; We have to load the directory and search for the kernel file.  
        ; We will load side 1 sectors 2 trough 15 and scan for the kernel.

load_dir:
        mov  ah, 2
        mov  al, DIR_LENGTH
        mov  bx, DIR_SEG
        mov  es, bx              ; start segment
        xor  bx, bx              ; start offset 0
        mov  ch, bl              ; cyl 0
        mov  cl, DIR_START_SEC
        mov  dx, 0100h           ; head 1 track 0
        int  13h
        jnc  success_dir

        mov  ah, 0
        mov  dl, ah              ; reset the floppy drive
        int  13h
     
        jc   load_dir            ; try again and again if not successful

success_dir:

        ; We search for the kernel name within the directory table
        
        ; es still points to DIR_SEG
        xor  di, di              ; es:di points to the directory table
        mov  cx, DIR_LENGTH*16   ; so many entries (fdd:224)
        
search_dir:        
        cmp  dword ptr[ es:di+0 ], "NREK"  ; "KERNel     "
        jnz  skip1
        
        cmp  dword ptr[ es:di+4 ], "  LE"  ; "kernEL..   "
        jz   kernel_found

skip1:  add  di, 32
        loop search_dir
        
        ;
        ; No kernel was found.  Can't do anything, but die...
        ;
        mov  al, "?"
        mov  ah, 0Eh
        mov  bh, 0
        int  10h
 
die:    jmp  die        

        ;
        ; Kernel was found, and es:di points to its directory entry
        ; Get the kernel size and load it (kernel has to be non-fragmented!).
        ;

kernel_found:

        mov  cx, [ es:di+01Ch ]  ; Get the file size
        shr  cx, 9               ; div 512
        inc  cx                  ; get the number of sectors to load

        mov  ax, [ es:di+01Ah ]  ; Get the starting cluster (=sector on fdd)
        add  ax, DATA_SECTOR     ; starting data sector

        mov  bx, KERNEL_SEG      ; loading segment

load_sector:

        push ax
        push bx
        push cx

        mov  ch, 36
        div  ch                  ; Get the cylinder number
        mov  ch, al
        xor  dx, dx              ; head 0, drive 0
        cmp  ah, 18              ; Head 1?
        jl   head0
        sub  ah, 18
        inc  dh                  ; head 1
head0:
        mov  cl, ah              ; sector number + 1
        inc  cl
        
        mov  es, bx              ; segment to load
        xor  bx, bx              ; es:0
        
        mov  ax, 0201h           ; read 1 sector, functon 2
        int  13h
        jnc  success_sector
        
        mov  ah, 0
        mov  dl, ah              ; reset the floppy drive
        int  13h
     
        jc   load_sector         ; try again and again if not successful

success_sector:
        
        pop  cx
        pop  bx
        pop  ax
        
        add  bx, 512/16          ; next 512 block
        inc  ax                  ; next sector on the disk
        loop load_sector         ; load next sector
        
        ;
        ; Signal successful load by a "."
        ;
        mov  al, 251             ; "û"
        mov  ah, 0Eh
        mov  bh, 0
        int  10h

        ; Reset the floppy disk motor
        mov  dx, 03F2h
        xor  al, al
        out  dx, al

        ;
        ; Set protected mode with 32 bit code and data selectors, so
        ; that we can jump into the kernel
        ;
        cli                      ; Disable interrupts
        mov  al, 80h
        out  (70h), al           ; and NMI

        ;
        ; Now we have to enable A20
        ;
        call empty_8042
        mov  al, 0D1h
        out  (64h),al
        call empty_8042
        mov  al, 0DFh
        out  (60h),al
        call empty_8042

        ;
        ; Load temporary gdt
        ;
        mov  ax, BOOT_SEG
        mov  ds, ax
        assume ds:BOOT_SEG

        ; Form the base address
        mov  ax, offset GDT
        add  ax, BOOT_LIN
        mov  [gdt_fix], ax
        
        lgdt    fword [gdt_ptr]

        ;
        ; Enter protected mode
        ;
        mov     eax, cr0
        or      eax, 1
        mov     cr0, eax

        jmp     flush
flush:

        mov     ax, DATASEL
        mov     ds, ax

        ;
        ; Jump into the protected 32 bit kernel code
        ;
        db   066h, 0EAh
        dw   0
        dw   0
        dw   CODESEL


;
; empty_8042
;
empty_8042:
        call delay
        in   al,(64h)        ; 8042 status port
        test al, 1           ; output buffer?
        jz   no_output
        call delay
        in   al,(60h)        ; read it
        jmp  empty_8042
no_output:
        test al, 2           ; is input buffer full?
        jnz  empty_8042      ; yes - loop
        ret

delay:
        jmp  short delay2
delay2: ret    


        org  510
        
        db   055h, 0AAh

boot    endp

STARTUP_CODE      ends
    
        end  boot
