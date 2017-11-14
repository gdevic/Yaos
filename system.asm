;
; SYSTEM.ASM
;
; This program sets up the environment for the
; YAOS kernel to be loaded and executed
;

SYSTEM_SEG   equ     3000h   ; starting segment of the system

.386p

CODE16       segment use16 'CODE'
             ASSUME cs:SYSTEM_SEG, ds:SYSTEM_SEG

start:
             ; Signal successful load by (another) "."
             mov     al, "."
             mov     ah, 0Eh
             mov     bh, 0
             int     10h

             mov     ax, 0
             int     16h

             ;
             ; Disable interrupts and NMI
             ;
             cli
        
             mov     al, 80h
             out     (70h), al ; Disable NMI

             ;
             ; Now we have to enable A20
             ;
             call    empty_8042
             mov     al, 0D1h
             out     (64h),al
             call    empty_8042
             mov     al, 0DFh
             out     (60h),al
             call    empty_8042

             ;
             ; Load gdt and idt
             ;

             push    cs
             pop     ds

             ; Form the base address
             mov     ax, offset GDT
             mov     [gdt_fix], ax
        
             lgdt    fword [gdt_ptr]

             mov     ax, offset IDT
             mov     [idt_fix], ax
        
             lidt    fword [idt_ptr]

             ; Calculate jump offset
             mov     ax, offset protected
             mov     [jmpoffs], ax

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
             mov     es, ax
             mov     fs, ax
             mov     gs, ax
             mov     ss, ax ; sp?!

             ;
             ; Jump into the 32 bit code
             ;
             ;jmp     far protected
             db      066h, 0EAh
jmpoffs:     dw      0, 3
             dw      CODESEL

protected:
             
             ;
             ; So some small, infinite, visible task
             ;                                    
db 0b9h, 021h, 000h, 000h, 000h    ;    mov ecx,00000021H
db 0b8h, 04fh, 000h, 000h, 000h    ;L10 mov eax,00000050H
db 0bbh, 000h, 080h, 00bh, 000h    ;    mov ebx,000b8000H
db 090h, 090h, 090h, 090h, 090h    ;     fix the bug
db 0c6h, 003h, 087h                ;L11 mov byte ptr [ebx],87H
db 088h, 04bh, 001h                ;    mov +1H[ebx],cl
db 083h, 0c3h, 002h                ;    add ebx,00000002H
db 048h                            ;    dec eax
db 075h, 0f4h                      ;    jne L11
db 041h                            ;    inc ecx
db 0ebh, 0e2h                      ;    jmp L10

;
; empty_8042
;
; Empties 8042

empty_8042:
             call    delay
             in      al,(64h)        ; 8042 status port
             test    al, 1           ; output buffer?
             jz      no_output
             call    delay
             in      al,(60h)        ; read it
             jmp     empty_8042
no_output:
             test    al, 2           ; is input buffer full?
             jnz     empty_8042      ; yes - loop
             ret

delay:
             jmp     short delay2
delay2:      ret    


             ;
             ; Keep these data here for easy references
             ;
             align   2

gdt_ptr:     dw      (3 * 8) - 1     ; 3 descriptors
gdt_fix:     dw      ?
             dw      SYSTEM_SEG / 4096

idt_ptr:     dw      0               ; no interrupts for now
idt_fix:     dw      ?
             dw      SYSTEM_SEG / 4096

             ;
             ; Global descriptor table
             ;
CODESEL      equ     0008h
DATASEL      equ     0010h

             align   16
GDT:         

GDT0:        ; Dummy descriptor
             dw      0, 0, 0, 0

GDT1:        ; 32 bit kernel code
             dw      007ffh          ; limit 15..0
             dw      00000h          ; base 15..0
             db      00              ; base 23..16
             db      10011010b       ; 32 bit code, present, dpl 0,
                                     ; readable, nonconf, accessed
             db      11000000b       ; page granular, 32 bit, limit 31..28
             db      00              ; base 31..24

GDT2:        ; 32 bit kernel data
             dw      0ffffh          ; limit 15..0
             dw      00000h          ; base 15..0
             db      00              ; base 23..16
             db      10010011b       ; 32 bit data, present, dpl 0,
                                     ; exp up, writable, accessed
             db      11001111b       ; page granular, 32 bit, limit 31..28
             db      00              ; base 31..24

             ;
             ; Interrupt descriptor table
             ;
             align   16
IDT:         
IDT0:        dd      0


CODE16       ends


CODE32       segment use32 'CODE'

start32:     mov     eax, data32
             ret
             
data32:      dd      ?

CODE32       ends

             end     start
                
