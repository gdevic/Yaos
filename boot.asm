;
; BOOT.ASM
;
;
; This program resides on the first sector of a floppy disk
; and boots the system up.
;

SYSTEM_SEG   equ     3000h   ; starting segment to load into
SYSTEM_POS   equ     2       ; starting sector on the floppy
SYSTEM_LEN   equ     1       ; in sectors

.386

STARTUP_CODE    segment use16
        assume ds:7c0h

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

Table1: db   "Yet Another OS", 0

        ;
        ; First, print the boot message and then load the system
        ;
bootup:
        cld        

        mov  ax, 7c0h
        mov  ds, ax
        assume ds:7c0h
        mov  ss, ax
        mov  sp, 4096

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

        ; Load the system
        ;
        ; The system resides on a floppy at sector 2
        ;
load:        
        mov  ah, 0
        mov  dl, ah    ; reset the floppy drive
        int  13h
     
        mov  ah, 2
        mov  al, SYSTEM_LEN
        mov  ch, 0     ; cyl 0
        mov  cl, SYSTEM_POS
        mov  dh, ch    ; head 0
        mov  dl, ch    ; drive 0
        mov  bx, SYSTEM_SEG
        mov  es, bx
        mov  bx, 0     ; es:bx
        int  13h
        jnc  success

        jc   load      ; try again and again if not successful

success:
        ; Signal successful load by a "."
        mov  al, "."
        mov  ah, 0Eh
        mov  bh, 0
        int  10h

        mov  ax, SYSTEM_SEG
        mov  ds, ax

        ; Jump to the system code
        jmp  far SYSTEM_SEG, 0

boot    endp

STARTUP_CODE      ends
    
        end  boot
