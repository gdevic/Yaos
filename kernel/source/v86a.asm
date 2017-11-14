;******************************************************************************
;
; Module:       v86a.asm
;
; Revision:     1.00
;
; Date:         07/09/96
;
; Author:       Goran Devic
;
;******************************************************************************
;
; Module Description:
;
;   This module contains the assembly language code for the
;   virtual 86 calls.
;
;******************************************************************************
;
; Major Changes:
;
;   DATE     REV   DESCRIPTION OF CHANGES                 AUTHOR
; --------   ----  -----------------------------------   -----------
; 07/09/96   1.00  Original                              Goran Devic
; --------   ----  -----------------------------------   -----------
;******************************************************************************
       .386p                           ; This is a 386 protected mode module

DATASEL      equ       0010h           ; selector for kernel data/stack

; Offsets in the stack-based parameters
;
PARM1        equ       8               ; offset of the first parameter
PARM2        equ       12              ; offset of the second parameter

V86_SSEG     equ       0f00h           ; stack segment in v86 mode
V86_SPTR     equ       1000h           ; stack pointer in v86 mode


; Offsets in the REGS structure
;
REGS_FLAGS   equ       0
REGS_AX      equ       2
REGS_BX      equ       4
REGS_CX      equ       6
REGS_DX      equ       8
REGS_SI      equ       10
REGS_DI      equ       12
REGS_DS      equ       14
REGS_ES      equ       16



;**********************************************************************
; External data structures
;**********************************************************************

EXTRN        tss                 :byte
EXTRN        Db_int              :dword
EXTRN        Sel                 :byte
EXTRN        TmpRegs             :byte


;**********************************************************************
; Internal data used outside of this module
;**********************************************************************

;**********************************************************************
; External functions that are being used by this module
;**********************************************************************

EXTRN       __GETDS             : near
EXTRN        printk              :near
EXTRN        Debugger            :near

;**********************************************************************
; Public functions provided by this module
;**********************************************************************

GLOBAL       Intv86              :near
GLOBAL       PassV86Interrupt    :near
GLOBAL       Int13_Handler       :near
GLOBAL       DebugINT1_Handler   :near
GLOBAL       DebugINT3_Handler   :near

;**********************************************************************

;**********************************************************************
; Start data segment
;**********************************************************************

_DATA        segment   use32 para public 'DATA'


;**********************************************************************
;
; Messages
;
;**********************************************************************

GP_Msg1:     db        13,"GP Fault GS:%04X FS:%04X DS:%04X ES:%04X SS:%04X ESP:%08X EFLAGS:%08X"
             db        13,"         CS:%04X EIP:%08X Error:%08X"
             db        13,"EAX:%08X EBX:%08X ECX:%08X EDX:%08X ESI:%08X EDI:%08X"
             db        0

msg: db 13, 13, 10, 13, 10
db "%08X  %08X  %08X  %08X  %08X  %08X  %08X  %08X  %08X  %08X %08X  %08X  %08X  "
db "%08X  %08X  %08X  %08X  %08X  %08X  %08X  %08X  %08X  %08X  "
db "%08X  %08X  %08X  %08X  %08X  %08X  %08X  %08X  %08X  %08X  "
db 0


msgi1:     db      13, 13
           db      "%08X %08X %08X %08X %08X %08X %08X %08X %08X", 13, 10
           db      "%08X %08X %08X %08X %08X %08X %08X %08X %08X", 13, 10
           db      "%08X %08X %08X %08X %08X %08X %08X %08X %08X", 13, 10, 0

msgi2:     db      13, 10
           db      "ecx: %08X", 13, 13, 10, 0

;**********************************************************************

;v86nest:     dd        0         ; Interrupt nesting level in v86 mode
v86int:      dw        1         ; Bit 0 enables interrupts in v86 mode
;v86request:  dd        0         ; Requested interrupt to pass on v86 mode


;**********************************************************************
_DATA        ends


;**********************************************************************
; Start code segment
;**********************************************************************

_TEXT        segment   use32 para public 'CODE'


;**********************************************************************
;
;   Intv86( BYTE interrupt_number, REGS * registers )
;
;   This routine will call ROM BIOS in v86 mode.  It can be called only
;   from the ring 0 kernel code.
;
;  Supervisor stack on virtual exit:
;      *REGS  (ebp+12)     (PARM2)
;      intr   (ebp+8 )     (PARM1)
;      caller (ebp+4 )
;      ebp    (ebp+0 )
;       eflags
;       eax     -
;       ecx     |
;       edx     |
;       ebx     |   pushad
;       esp     |
;       ebp     |
;       esi     |
;       edi     -   <- esp at tss
;      <standard v86 stack built here>
;
;**********************************************************************
Intv86_Frame   Struct                  ; Intv86 stack frame structure
Intv86_ebp     dd  ?                   ; Old ebp value
Intv86_Caller  dd  ?                   ; Caller address
Intv86_intr    dd  ?                   ; Interrupt number
Intv86_regs    dd  ?                   ; Address of the register structure
Intv86_Frame   Ends                    ; End of stack frame structure

Intv86  proc

       push    ebp                     ; Standard stack frame
       mov     ebp, esp                ; ebp is the argument reference
       pushfd                          ; Store flags
       pushad                          ; Store all registers on the pmode stack

       lea     eax, tss                ; Get the address of tss
       mov     [eax + 4], esp          ; Store current esp to tss ring 0 esp

; Get the interrupt number and obtain the V86 address of that handler
       movzx   eax, byte ptr [ebp.Intv86_intr]
       mov     ecx, dword ptr fs:[eax*4]

; Start building the v86 type stack so that we can pass control using
; the iretd instruction from the kernel.  Store 0000:0000 on the top of the
; V86 stack so that our monitor knows when to exit V86 program.

       mov     edx, [ebp.Intv86_regs]
       xor     ebx, ebx                ; We will need zero
       push    ebx                     ; 00:gs - set v86 gs to zero
       push    ebx                     ; 00:fs - set v86 fs to zero
       push    bx                      ; 00:
       push    word ptr [edx+REGS_DS]  ;ds
       push    bx                      ; 00:
       push    word ptr [edx+REGS_ES]  ;es

       push    dword V86_SSEG          ; 00ss
       push    dword V86_SPTR          ; 00sp
       mov     fs:[V86_SSEG*16+V86_SPTR], ebx

       ;
       ; Set the flag register:
       ;    V86 mode bit is 1
       ;    IOPL is 0 meaning monitor intercepts sensitive instructions
       ;    IF is 1 to allow interrupts
       ;
       ; 17 16 15 14 13 12 11 10 9  8  7  6  5  4  3  2  1  0   <- Bit
       ; VM RF .. NT IO-PL OF DF IF TF SF ZF .. AF .. PF .. CF  <- Name
       ; 1  0  0  0  0  0  0  0  1  0  0  0  0  0  0  0  1  0   <= Value
       ;      |           |           |           |
       ;  2   |     0     |     2     |     0     |     2       <- Hex
       ;
       push    dword 00020202h         ; Push eflags

       push    bx                      ; 00:
       ror     ecx, 16                 ; Get the cs part of the real vector
       push    cx                      ;    cs

       push    bx                      ; 00:
       ror     ecx, 16                 ; Get the offset part of the real vector
       push    cx                      ;    ip

       mov     ax, [edx+REGS_AX]       ; Set the registers from the REGS struct
       mov     bx, [edx+REGS_BX]
       mov     cx, [edx+REGS_CX]
       mov     dx, [edx+REGS_DX]
       mov     si, [edx+REGS_SI]
       mov     di, [edx+REGS_DI]

       iretd                           ; Jump into a v86 mode interrupt routine

Intv86 endp


;**********************************************************************
;
;   PassV86Interrupt( BYTE interrupt_number )
;
;   This routine is called from the pmode interrupt handler where the
;   interrupt should be passed to the V86 task.  This will be done only
;   if v86 interrupts were enabled.
;
;   At the moment of interruption, processor was executing either
;   the kernel process or a virtual task.  If a kernel process was
;   active, we have to simulate a call to the IntV86() function
;   that will perform the interrupt passing.
;
;   If a virtual process was active, we can simulate the interrupt on
;   its virtual stack.
;
;   Assumptions:
;   *   This function is called from an ring 0 interrupt rutine only.
;   *   The pmode stack was formed by the sequence:
;           pushad
;           push    ds
;           push    es
;           push    fs
;           push    gs
;           mov     ebp, esp
;   *   This call is at the root of the interrupt rutine
;   *   Stack on entry when kernel was interrupted:
;         (caller)
;          old eflags      ebp + 38
;          old cs          ebp + 34
;          old eip         ebp + 30  <-+- stored by the CPU on interrupt
;          eax  (pushad)   ebp + 2c
;          ecx             ebp + 28
;          edx             ebp + 24
;          ebx             ebp + 20
;          esp             ebp + 1c
;          ebp             ebp + 18
;          esi             ebp + 14
;          edi             ebp + 10
;              ds          ebp + 0c
;              es          ebp + 08
;              fs          ebp + 04
;  ebp ->      gs          ebp + 00  <-+- stored by the intr routine (caller)
;              .  .  .  < interrupt handler local variables >
;          int number      ESP + 04
;  esp ->  caller       <- ESP + 00
;
;**********************************************************************
PassV86_Frame  Struct                  ; PassV86Interrupt stack frame structure
PassV86_Caller dd  ?                   ; Caller address
PassV86_intr   dd  ?                   ; Interrupt number
PassV86_Frame  Ends                    ; End of stack structure

PassEbp_Frame  Struct                  ; Caller's stack frame structure
PassEbp_gs     dd  ?                   ; gs
PassEbp_fs     dd  ?                   ; fs
PassEbp_es     dd  ?                   ; es
PassEbp_ds     dd  ?                   ; ds
PassEbp_edi    dd  ?                   ; edi from pusha
PassEbp_esi    dd  ?                   ; esi from pusha
PassEbp_ebp    dd  ?                   ; ebp from pusha
PassEbp_esp    dd  ?                   ; esp from pusha
PassEbp_ebx    dd  ?                   ; ebx from pusha
PassEbp_edx    dd  ?                   ; edx from pusha
PassEbp_ecx    dd  ?                   ; ecx from pusha
PassEbp_eax    dd  ?                   ; eax from pusha
PassEbp_eip    dd  ?                   ; Originally interrupted eip
PassEbp_cs     dd  ?                   ; Originally interrupted code selector
PassEbp_eflags dd  ?                   ; Originally interrupted eflags
NewEbp_Caller  dd  ?                   ; To set original interrupted address
PassEbp_Frame  Ends                    ; End of caller's stack frame structure

PassVirt_Frame Struct                  ; Stack frame of the virtual interceptor
PassVirt_gs    dd  ?                   ; gs
PassVirt_fs    dd  ?                   ; fs
PassVirt_es    dd  ?                   ; es
PassVirt_ds    dd  ?                   ; ds
PassVirt_edi   dd  ?                   ; edi from pusha
PassVirt_esi   dd  ?                   ; esi from pusha
PassVirt_ebp   dd  ?                   ; ebp from pusha
PassVirt_esp   dd  ?                   ; esp from pusha
PassVirt_ebx   dd  ?                   ; ebx from pusha
PassVirt_edx   dd  ?                   ; edx from pusha
PassVirt_ecx   dd  ?                   ; ecx from pusha
PassVirt_eax   dd  ?                   ; eax from pusha
PassVirt_eip   dd  ?                   ; Originally interrupted V86 eip
PassVirt_cs    dd  ?                   ; V86 code segment
PassVirt_flags dd  ?                   ; V86 flags
PassVirt_Vsp   dd  ?                   ; V86 sp register
PassVirt_Vss   dd  ?                   ; V86 ss register
PassVirt_Ves   dd  ?                   ; V86 es register
PassVirt_Vds   dd  ?                   ; V86 ds register
PassVirt_Vfs   dd  ?                   ; V86 fs register
PassVirt_Vgs   dd  ?                   ; V86 gs register
PassVirt_Frame Ends                    ; End of stack structure


PassV86        proc                    ; Non-interrupt portion that invokes int

       push    ebp                     ; Push ebp to form standard stack frame
       mov     ebp, esp                ; Form the frame
       pushfd                          ; Push eflags
       cli                             ; Disable any interruption
       pushad                          ; Push all registers

;       mov     [v86nest], 1            ; Set the nesting level

       lea     eax, tss                ; Get the address of tss
       mov     [eax + 4], esp          ; Store current esp to tss ring 0 esp

;       mov     eax, [v86request]       ; Get the interrupt number
       mov     ecx, dword ptr fs:[eax*4]; Get the v86 mode address vector

; Start building the v86 type stack so that we can pass control using
; the iretd instruction from the kernel
       xor     ebx, ebx                ; We will need zero
       push    ebx                     ; 00:gs - set v86 gs to zero
       push    ebx                     ; 00:fs - set v86 fs to zero
       push    ebx                     ; 00:ds
       push    ebx                     ; 00:es
       push    dword V86_SSEG          ; 00ss
       push    dword V86_SPTR          ; 00sp

       ;
       ; Set the flag register:
       ;    V86 mode bit is 1
       ;    IOPL is 0 meaning monitor intercepts sensitive instructions
       ;    IF is 1 to allow interrupts
       ;
       ; 17 16 15 14 13 12 11 10 9  8  7  6  5  4  3  2  1  0   <- Bit
       ; VM RF .. NT IO-PL OF DF IF TF SF ZF .. AF .. PF .. CF  <- Name
       ; 1  0  0  0  0  0  0  0  1  0  0  0  0  0  0  0  1  0   <= Value
       ;      |           |           |           |
       ;  2   |     0     |     2     |     0     |     2       <- Hex
       ;
       push    dword 00020202h         ; Push eflags

       push    bx                      ; 00:
       ror     ecx, 16                 ; Get the cs part of the real vector
       push    cx                      ;    cs

       push    bx                      ; 00:
       ror     ecx, 16                 ; Get the offset part of the real vector
       push    cx                      ;    ip

push offset msgi1
call printk
mov ax, 7130h
mov fs:[0b8002h], ax
die2: jmp die2
       iretd                           ; Jump into a v86 mode interrupt routine

PassV86        endp                    ; End of non-interrupt portion

;****************************************

PassV86Interrupt   proc

       test    word ptr [v86int], 1    ; Test if v86 intr is enabled
       jnz     @DoV86Intr              ; Jump and do it if the bit was set
;       ret                             ; Return if interrupts were disabled
@DoV86Intr:                            ; Make virtual interrupt
       test    [ebp.PassEbp_eflags], 020000h; Test V86 bit in eflags of the stack
       jnz     @FromV86                ; Jump forward if it was set

ret

; Virtual 86 mode was not set in the eflags on the stack. Kernel process
; was interrupted.  We can do simple PassV86() call emulation on the stack
; and that will require moving the bottom of the stack to make space,
; we need 1 dword to invoke our PassV86() helper function that will call the
; interrupt
       mov     esi, esp                ; Source is current esp
       mov     edi, esi                ; Destination is 1 dword lower
       sub     edi, 4                  ; Subtract a dword
       mov     ecx, ebp                ; We need to know the size of the locals
       sub     ecx, esp                ; Subtract esp
       add     ecx, size PassEbp_Frame ; Add the rest of the block
       shr     ecx, 2                  ; make it in dwords

       cld                             ; Positive direction
       rep     movsd                   ; Move a block
       sub     esp, 4                  ; Position new esp down a dword
       sub     ebp, 4                  ; and ebp pointer as well

; Now fill in the new dword with the original interrupted address, and set
; set the returning address to the PassV86() that will call the interrupt
; from the out of this interrupt routine
       mov     eax, [esp.PassV86_intr] ; Get the interrupt number
;       mov     [v86request], eax       ; Store the interrupt number to execute
       mov     eax, [ebp.PassEbp_eip]  ; Get the original caller eip
       mov     [ebp.NewEbp_Caller], eax; Store in the caller field
       mov     eax, offset PassV86     ; Get the address of the function
       mov     [ebp.PassEbp_eip], eax  ; Store to return there

; Now we can return.  Upon exit, CPU will continue into PassV86() function
; that will start virtual process that will finish and our original
; interrupted kernel will continue.
       ret


; The process being interrupted was V86 process.  We can simulate the interrupt
; by placing the curent address/flags on the v86 stack and returning to the
; v86 interrupt handler
@FromV86:

inc byte ptr fs:[0b8000h + 78*2]

int 1
       mov     eax, [esp.PassV86_intr] ; Get the interrupt number in eax
       mov     eax, dword ptr fs:[eax*4]; Get the address from the BIOS table in

       ; Form the linear address of the stack in virtual mode
       mov     ebx, [ebp.PassVirt_Vss] ; Get the stack segment
       and     ebx, 0ffffh             ; Isolate the low 16 bits (SS)
       shl     ebx, 4                  ; Shift by 4 to form the linear address
       add     ebx, [ebp.PassVirt_Vsp] ; Add the stack pointer

       ; Push flags, cs and ip on the virtual stack
       mov     ecx, [ebp.PassVirt_flags]; Get the flags
       mov     word ptr fs:[ebx-0], cx ; Push the flags word
       mov     ecx, [ebp.PassVirt_cs]  ; Get the cs
       mov     word ptr fs:[ebx-2], cx ; Push the code segment
       mov     ecx, [ebp.PassVirt_eip] ; Get the ip
       mov     word ptr fs:[ebx-4], cx ; Push the instruction pointer
       sub     word ptr [ebp.PassVirt_Vsp], 6; Decrement the virtual stack

       ; Store the interrupt function address as a new cs:ip
       mov     word ptr [ebp.PassVirt_eip], ax ; Store the ip portion
       shr     eax, 16                         ; Rotate to get the cs portion
       mov     word ptr [ebp.PassVirt_cs], ax  ; Store the cs portion

       ; Increment the nesting level of the virtual task
;       inc     [v86nest]               ; Add 1 to the nesting level

       ret                             ; Return to the caller

PassV86Interrupt   endp


;**********************************************************************
;
; V86_Terminate - handler for v86 termination
;
; Used to switch back from the V86 mode to ring 0 pmode when V86's iret
; tries to return to address 0000:0000.
;
;
;   Stack on entry:
; ebp+64  *REGS  (ebp+12) (PARM2)  | On return
; ebp+60  intr   (ebp+8 ) (PARM1)  |
; ebp+5C  caller (ebp+4 )
; ebp+58  ebp    (ebp+0 )
; ebp+54   eflags
; ebp+50   eax (from pushad)
; ebp+4c   ecx
; ebp+48   edx
; ebp+44   ebx
; ebp+40   esp
; ebp+3c   ebp
; ebp+38   esi
; ebp+34   edi            <- esp from tss -+- stored on v86 call
; ebp+30  -- old GS
; ebp+2c  -- old FS
; ebp+28  -- old DS
; ebp+24  -- old ES
; ebp+20  -- old SS
; ebp+1c  -- old SP
; ebp+18  old eflags
; ebp+14  -- old CS
; ebp+10  -- old IP         |
; ebp+0c  ErrorCode       <-+- stored by the CPU on interrupt
; ebp+08  eax
; ebp+04  ebx
; ebp+00  ecx             <- current esp, stored by the interrupt routine
;
;**********************************************************************
V86_Terminate:

;       mov     ecx, [v86request]       ; Get the possible request interrupt
;       or      ecx, ecx                ; Is it zero?
;       jnz     @PassTerm               ; Just passed interrupt terminates

;cli
;push offset msgi1
;call printk
;mov ax, 7130h
;mov fs:[0b8002h], ax
;die4: jmp die4

        mov     ecx, [esp+64h]          ; Get the addr of the regs structure

        ; Store registers from the v86 stack into the v86_regs structure
        mov   [ ecx+REGS_DX ], dx          ; Store dx
        mov   [ ecx+REGS_SI ], si          ; Store si
        mov   [ ecx+REGS_DI ], di          ; Store di

        mov   ax, [ esp+VIRT_ECX ]         ; Get cx
        mov   [ ecx+REGS_CX ], ax
        mov   ax, [ esp+VIRT_EBX ]         ; Get bx
        mov   [ ecx+REGS_BX ], ax
        mov   ax, [ esp+VIRT_EAX ]         ; Get ax
        mov   [ ecx+REGS_AX ], ax

        mov   ax, [ esp+VIRT_EFLAGS ]      ; Get flags
        mov   [ ecx+REGS_FLAGS ], ax

        mov   ax, [ esp+VIRT_DS ]          ; Get ds
        mov   [ ecx+REGS_DS ], ax

        mov   ax, [ esp+VIRT_ES ]          ; Get es
        mov   [ ecx+REGS_ES ], ax

@PassTerm:
        add   esp, 13 * 4                  ; Adjust esp for the frame
        popad                              ; Restore all registers from the stack
        popfd                              ; Restore eflags
        pop   ebp                          ; Restore base register
        ret                                ; Return to the pmode caller

;@PassTerm:
;mov ax, 7132h
;mov fs:[0b8004h], ax
;die3: jmp die3

;**********************************************************************
;
; Interrupt 13 - General Protection Handler
;
; This handler is called in two cases:
;  1) In protected mode, GP fault occurred
;  2) In v86 mode, a sensitive instruction is executed
;      (iret,sti,cli,pushf,popf,int n)
;
;**********************************************************************
Int13_Handler:

        push  eax                ; Store some registers
        push  ebx
        push  ecx

; Supervisor stack now contains:
;
VIRT_GS      equ  48     ;  virt GS          (+48)
VIRT_FS      equ  44     ;  virt FS          (+44)
VIRT_DS      equ  40     ;  virt DS          (+40)
VIRT_ES      equ  36     ;  virt ES          (+36)
VIRT_SS      equ  32     ;  virt SS          (+32)
VIRT_ESP     equ  28     ;  virt ESP         (+28)
VIRT_EFLAGS  equ  24     ;  virt EFLAGS      (+24)
VIRT_CS      equ  20     ;  virt CS          (+20)
VIRT_EIP     equ  16     ;  virt EIP         (+16)
VIRT_ERROR   equ  12     ;  virt Error Code  (+12)
VIRT_EAX     equ  8      ;  eax              (+ 8)
VIRT_EBX     equ  4      ;  ebx              (+ 4)
VIRT_ECX     equ  0      ;  ecx              (+ 0) <- esp

        call  __GETDS                      ; Set up ds, es, fs and gs selectors

inc byte ptr fs:[0b8000h + 79*2 + 22*80*2]

        test  dword ptr [esp+VIRT_EFLAGS], 020000h ; VM flag set?
        jz    GP_ProtMode                  ; No, go to the pmode handler

        ; Obtain the address of the v86 mode sensitive instruction
        movzx eax, word ptr [ esp+VIRT_CS ]
        shl   eax, 4                       ; Real segment
        add   eax, [ esp+VIRT_EIP ]        ; Add the IP

        mov   ax, word ptr fs:[ eax ]      ; Get the opcode ( 2 bytes )

        movzx ecx, word ptr [ esp+VIRT_SS ]; Get SS
        shl   ecx, 4                       ; Real segment
        add   ecx, [ esp+VIRT_ESP ]        ; Add the ESP

        ;---------------------------------------------------------
        ;
        ; Examine the opcode and emulate it:
        ;  ah  - second byte of the opcode
        ;  al  - opcode of the instruction
        ;  ecx - v86 stack pointer
        ;
        ;---------------------------------------------------------

        cmp   al, 09Ch                     ; PUSHF ?
        jne   @GP_13a

        ; pushf here
        mov   bx, [ esp+VIRT_EFLAGS ]      ; Get flags
        test  word ptr[ v86int ], 1        ; Is v86 interrupt disabled?
        jnz   @inten                       ; skip if enabled
        and   bx, 0FDFFh                   ; Clear int in flags
@inten:
        sub   word ptr[ esp+VIRT_ESP ], 2  ; Decrement sp
        mov   fs:[ ecx ], bx               ; Push flags

        jmp   @GP_13common                 ; Done

        ;---------------------------------------------------------
@GP_13a:
        cmp   al, 09Dh                     ; POPF ?
        jne   @GP_13b

        ; popf here
        mov   bx, [ fs:ecx ]               ; Get flags
        add   word ptr[ esp+VIRT_ESP ], 2  ; Adjust v86 sp

        or    bx, 0200h                    ; Enable interrupts
        mov   [ esp+VIRT_EFLAGS ], bx      ; Store on the stack to be popped

        jmp   @GP_13common

        ;---------------------------------------------------------
@GP_13b:
        cmp   al, 0CDh                     ; INT X ?
        jne   @GP_13c

        ; int x here:
        ;  pushf
        ;  push cs
        ;  push ip+2
        ;  cs:ip = interrupt table (int x * 4)

        mov   bx, [ esp+VIRT_EFLAGS ]      ; Get flags
        mov   fs:[ ecx-2 ], bx             ; Push it
        mov   bx, [ esp+VIRT_CS ]          ; Get CS
        mov   fs:[ ecx-4 ], bx             ; Push it
        mov   bx, [ esp+VIRT_EIP ]         ; Get IP
        add   bx, 2                        ; Next instruction after int x
        mov   fs:[ ecx-6 ], bx             ; Push it
        sub   word ptr [ esp+VIRT_ESP ], 6 ; Adjust sp

;        inc   dword ptr[ v86nest ]         ; Increase v86 nesting level

        movzx eax, ah                      ; Get the int number in eax
        mov   ebx, fs:[ eax*4 ]            ; Get the v86 mode cs:ip
        mov   [ esp+VIRT_EIP ], bx         ; Set new eIP
        shr   ebx, 16                      ; CS is now in bx
        mov   [ esp+VIRT_CS ], bx          ; Set CS

        jmp   @GP_13commonNoInc            ; Jump to that location

        ;---------------------------------------------------------
@GP_13c:
        cmp   al, 0CFh                     ; IRET ?
        jne   @GP_13d

        ; iret here
 ;       dec   dword ptr [v86nest]          ; Decrease nesting level
;        jz    V86_Terminate                ; If level has reached 0, terminate
                                           ; this v86 call
        mov   bx, fs:[ ecx ]               ; Get IP from the v86 stack
        mov   [ esp+VIRT_EIP ], bx         ; Store it

        or    bx, fs:[ ecx+2 ]             ; OR it with the CS value
        jz    V86_Terminate                ; If address is 0000:0000, terminate

        mov   bx, fs:[ ecx+2 ]             ; Get CS
        mov   [ esp+VIRT_CS ], bx          ; Store it
        mov   bx, fs:[ ecx+4 ]             ; Get flags
        or    bx, 0200h                    ; Enable interrupts
        mov   [ esp+VIRT_EFLAGS ], bx      ; Store it
        add   word ptr [ esp+VIRT_ESP ], 6 ; Adjust SP for iret

        jmp   @GP_13commonNoInc            ; Jump to that location

        ;---------------------------------------------------------
@GP_13d:
        cmp   al, 0FAh                     ; CLI ?
        jne   @GP_13e

        ; cli here
        mov   word ptr [v86int], 0         ; Disable virtual interrupts

        jmp   @GP_13common

        ;---------------------------------------------------------
@GP_13e:
        cmp   al, 0FBh                     ; STI ?
        jne   @GP_13common

        ; sti here
        mov   word ptr [v86int], 1         ; Enable virtual interrupts

        ; ... proceeds
        ;---------------------------------------------------------
@GP_13common:
        inc   word ptr [ esp+VIRT_EIP ]    ; Advance ip

@GP_13commonNoInc:

        ; Restore registers and return to v86 code
        pop   ecx
        pop   ebx
        pop   eax
        add   esp, 4                       ; Adjust esp for error code

        iretd


;**********************************************************************
;
; General Protection Error in Protected Mode
;
;**********************************************************************
GP_ProtMode:

        ; Print a message using the registers from the stack
        push  edi
        push  esi
        push  edx
        push  offset GP_Msg1
        call  printk
        add   esp, 52+4*4                  ; Skip all the data

        int   1                            ; Call the debugger

;**********************************************************************
;
; Debugger Single Step Trap: INT 1
;
;**********************************************************************
Int1_Frame     Struct                  ; Stack frame of the virtual interceptor
Int1_edi       dd  ?                   ; edi from pusha
Int1_esi       dd  ?                   ; esi from pusha
Int1_ebp       dd  ?                   ; ebp from pusha
Int1_esp       dd  ?                   ; esp from pusha
Int1_ebx       dd  ?                   ; ebx from pusha
Int1_edx       dd  ?                   ; edx from pusha
Int1_ecx       dd  ?                   ; ecx from pusha
Int1_eax       dd  ?                   ; eax from pusha
Int1_eip       dd  ?                   ; Originally interrupted eip
Int1_cs        dd  ?                   ; Originally interrupted code segment
Int1_eflags    dd  ?                   ; Originally interrupted flags
Int1_Vsp       dd  ?                   ; Possibly V86 sp register
Int1_Vss       dd  ?                   ; Possibly V86 ss register
Int1_Ves       dd  ?                   ; Possibly V86 es register
Int1_Vds       dd  ?                   ; Possibly V86 ds register
Int1_Vfs       dd  ?                   ; Possibly V86 fs register
Int1_Vgs       dd  ?                   ; Possibly V86 gs register
Int1_Frame     Ends                    ; End of stack structure


DebugINT1_Handler:

        pushad                         ; Push all registers on the stack

        mov     ax, ds                 ; Store old data selector
        mov     bx, DATASEL            ; Get new data sel
        mov     ds, bx                 ; Set up new data sel

        mov     [Db_int], byte 1       ; Set int 1 state message
@debug:
        lea     ecx, TSS               ; In the TSS structure
        sub     dword ptr [ecx+4], 256 ; Give more stack space

; Test if the V86 mode was interrupted and jump in appropriate section
        test    [esp.Int1_eflags], 020000h
        jz      @pmode

; If V86 mode was interrupted, store segment registers
        lea     ebx, Sel               ; Get the address of the selector struct
        mov     ax, word ptr [esp.Int1_cs]
        mov     word ptr [ebx+0], ax
        mov     ax, word ptr [esp.Int1_Vds]
        mov     word ptr [ebx+2], ax
        mov     ax, word ptr [esp.Int1_Vss]
        mov     word ptr [ebx+4], ax
        mov     ax, word ptr [esp.Int1_Ves]
        mov     word ptr [ebx+6], ax
        mov     ax, word ptr [esp.Int1_Vfs]
        mov     word ptr [ebx+8], ax
        mov     ax, word ptr [esp.Int1_Vgs]
        mov     word ptr [ebx+10], ax

        mov     ebx, 0                 ; 16 bits arguments

        jmp     @skip1

@pmode:
        lea     ebx, Sel               ; Get the address of the selector struct
        mov     cx, word ptr [esp.Int1_cs]
        mov     word ptr [ebx+0], cx
        mov     word ptr [ebx+2], ax   ; Store old ds
        mov     word ptr [ebx+4], ss   ; Store old ss
        mov     word ptr [ebx+6], es   ; Store old es
        mov     word ptr [ebx+8], fs   ; Store old fs
        mov     word ptr [ebx+10], gs  ; Store old gs

        mov     ebx, 1                 ; 32 bits arguments

@skip1:
        call  __GETDS                  ; Set all the selectors
        mov     eax, esp               ; Get the current stack pointer
        push    ebx                    ; Push the operands size
        push    eax                    ; push the address of the registers
        call    Debugger               ; Call the single-step debugger
        add     esp, 8                 ; Clean the arguments

        lea     ecx, TSS
        add     dword ptr [ecx+4], 256 ; Restore taken space

        popad                          ; Get back all the registers
        iretd                          ; Proceed with execution


;**********************************************************************
;
; Debugger Trap: INT 3
;
;**********************************************************************
DebugINT3_Handler:

        pushad                              ; Push all registers on the stack
        mov     ax, ds                      ; Store old data selector
        mov     bx, DATASEL                 ; Get new data sel
        mov     ds, bx                      ; Set up new data sel
        mov     [Db_int], byte 3            ; Set interrupt 3 state message
        jmp     @debug                      ; jump to common point


;**********************************************************************
_TEXT        ends


             end
