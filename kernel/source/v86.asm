
       Page    58, 132
       Title   V86.asm                                       Copyright (c) 1997

;******************************************************************************
;                                                                             *
;   Module:     V86.asm                                                       *
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
;        This module contains the assembly code for the virtual 86 mode
;      management.
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

       Public V86_Int
       Public Reflect_Int

       Public Int13_VM_Handler
       Public V86_Int_Ret

       Public VM_SS
       Public VM_SP

;******************************************************************************
;                                                                             *
;      External Declarations                                                  *
;                                                                             *
;******************************************************************************

       Extern GPF_Handler:near

       Extern tss:dword

       Extern IntXX_Handler:dword

       Extern dwBreakSegOffs:dword

       Extern Loader_V86:dword

       Extern pCurProc:dword

;******************************************************************************
;                                                                             *
;      Local Constants and Macros                                             *
;                                                                             *
;******************************************************************************

OPCODE_PUSHF   Equ   09Ch
OPCODE_POPF    Equ   09Dh
OPCODE_INTX    Equ   0CDh
OPCODE_IRET    Equ   0CFh
OPCODE_CLI     Equ   0FAh
OPCODE_STI     Equ   0FBh
OPCODE_INT3    Equ   0CCh


; These bits are zeroed in the V86 eflags register - NT flag is left as set
; by the VM process to allow detection of a "386 processor or better" by
; Watcom and some other software.  Note that this is not recommended by Intel !

V86_FLAGS_MASK   Equ   Not (RES3_MASK+RES5_MASK+TF_MASK+IOPL_MASK+RES22_MASK)

; These bits are set in the V86 eflags register by default

V86_FLAGS_SETIF  Equ   (RES1_MASK + IF_MASK + VM_MASK)

;******************************************************************************
;                                                                             *
;      Data Segment                                                           *
;                                                                             *
;******************************************************************************
_DATA          segment use32 para public 'DATA'    ; 32 bit data segment


;------------------------------------------------------------------------------
; Virtual Machine Stack Segment
;------------------------------------------------------------------------------

VM_SS          dd    ?
VM_SP          dd    ?

;------------------------------------------------------------------------------
; Virtual Machine State
;------------------------------------------------------------------------------

VM_IF          dd    IF_MASK

;------------------------------------------------------------------------------
; Privileged instructions
;------------------------------------------------------------------------------

Privileged_Instr_Handler:

       Opcode = 0
       Rept    256

         If Opcode Eq OPCODE_PUSHF
dd  Opcode_PUSHF_Handler
         Else
           If Opcode Eq OPCODE_POPF
dd  Opcode_POPF_Handler
           Else
             If Opcode Eq OPCODE_INTX
dd  Opcode_INTX_Handler
             Else
               If Opcode Eq OPCODE_IRET
dd  Opcode_IRET_Handler
               Else
                 If Opcode Eq OPCODE_CLI
dd  Opcode_CLI_Handler
                 Else
                   If Opcode Eq OPCODE_STI
dd  Opcode_STI_Handler
                   Else
                     If Opcode Eq OPCODE_INT3
dd  Opcode_INT3_Handler
                     Else
dd  Opcode_Invalid_Handler
                     Endif
                   Endif
                 Endif
               Endif
             Endif
           Endif
         Endif

         Opcode = Opcode + 1
       Endm

; This table contains the addresses of the routines for handlers of opcodes
; that have opcode override `0x66'.

Privileged_Instr_Handler_32:

       Opcode = 0
       Rept    256

         If Opcode Eq OPCODE_PUSHF
dd  Opcode_PUSHFD_Handler
         Else
           If Opcode Eq OPCODE_POPF
dd  Opcode_POPFD_Handler
           Else
             If Opcode Eq OPCODE_IRET
dd  Opcode_IRETD_Handler
             Else
dd  Opcode_Invalid_Handler
             Endif
           Endif
         Endif

         Opcode = Opcode + 1
       Endm

_DATA          ends


;******************************************************************************
;                                                                             *
;      Code Segment                                                           *
;                                                                             *
;******************************************************************************
_TEXT          segment use32 para public 'CODE'    ; 32 bit code segment


       Subttl  Opcode_PUSHF_Handler
       Page    +
;******************************************************************************
;
;      Opcode_PUSHF_Handler
;
;******************************************************************************
;
;      Registers on entry:
;              gs: selector for global memory
;              edx - Linear address of the faulty instruction
;              ecx - EIP of the next instruction byte (must be preserved)
;
;      Registers on exit:
;              ecx - EIP of the next instruction byte (must be preserved)
;
;      Note: Real V86_EFLAGS interrupt bit is always set.
;
;******************************************************************************
Opcode_PUSHF_Handler proc

       movzx   ebx, ds:[ebp+Int_SS]    ; Get the stack segment
       shl     ebx, 4                  ; Shift it up by four
       movzx   edx, word ptr ds:[ebp+Int_ESP]   ; Get the stack pointer
       sub     dx, 2                   ; Lower 2 bytes for the flags
       add     ebx, edx                ; Linear address of the stack
       mov     ds:[ebp+Int_ESP], edx   ; Store new stack pointer

       mov     eax, ds:[ebp+Int_EFLAGS]; Get the V86 flags register
       btr     eax, IF_BIT             ; Reset the interrupt bit
       or      eax, ds:[VM_IF]         ; Add the virtual interrupt bit

       mov     gs:[ebx], ax            ; Store the flags register
       ret                             ; Return control to the caller

Opcode_PUSHF_Handler endp


       Subttl  Opcode_PUSHFD_Handler
       Page    +
;******************************************************************************
;
;      Opcode_PUSHFD_Handler
;
;******************************************************************************
;
;      Registers on entry:
;              gs: selector for global memory
;              edx - Linear address of the faulty instruction
;              ecx - EIP of the next instruction byte (must be preserved)
;
;      Registers on exit:
;              ecx - EIP of the next instruction byte (must be preserved)
;
;      Note: Real Int_EFLAGS interrupt bit is always set.
;
;******************************************************************************
Opcode_PUSHFD_Handler proc

       movzx   ebx, ds:[ebp+Int_SS]    ; Get the stack segment
       shl     ebx, 4                  ; Shift it up by four
       mov     edx, ds:[ebp+Int_ESP]   ; Get the stack pointer
       sub     edx, 4                  ; Lower 4 bytes for the eflags
       add     ebx, edx                ; Linear address of the stack
       mov     ds:[ebp+Int_ESP], edx   ; Store new stack pointer

       mov     eax, ds:[ebp+Int_EFLAGS]; Get the flags register
       btr     eax, IF_BIT             ; Reset the interrupt bit
       or      eax, ds:[VM_IF]         ; Add the virtual interrupt bit

       mov     gs:[ebx], eax           ; Store the eflags register
       ret                             ; Return control to the caller

Opcode_PUSHFD_Handler endp


       Subttl  Opcode_POPF_Handler
       Page    +
;******************************************************************************
;
;      Opcode_POPF_Handler
;
;******************************************************************************
;
;      Registers on entry:
;              gs: selector for global memory
;              edx - Linear address of the faulty instruction
;              ecx - EIP of the next instruction byte (must be preserved)
;
;      Registers on exit:
;              ecx - EIP of the next instruction byte (must be preserved)
;
;      Note: Real Int_EFLAGS interrupt bit is always set.
;
;******************************************************************************
Opcode_POPF_Handler proc

       movzx   ebx, ds:[ebp+Int_SS]    ; Get the stack segment
       shl     ebx, 4                  ; Shift it up by four
       movzx   edx, word ptr ds:[ebp+Int_ESP]   ; Get the stack pointer
       add     ebx, edx                ; Linear address of the stack
       mov     ax, gs:[ebx]            ; Fetch the flags from the VM stack
       add     dx, 2                   ; Add 2 bytes for the flags
       mov     ds:[ebp+Int_ESP], edx   ; Store new stack pointer

       and     ax, V86_FLAGS_MASK      ; Clear the VM reserved bits
       or      ax, V86_FLAGS_SETIF     ; Add all the constant bits + IF

       mov     word ptr ds:[ebp+Int_EFLAGS], ax ; Store new flags
       ret                             ; Return control to the caller

Opcode_POPF_Handler endp


       Subttl  Opcode_POPFD_Handler
       Page    +
;******************************************************************************
;
;      Opcode_POPFD_Handler
;
;******************************************************************************
;
;      Registers on entry:
;              gs: selector for global memory
;              edx - Linear address of the faulty instruction
;              ecx - EIP of the next instruction byte (must be preserved)
;
;      Registers on exit:
;              ecx - EIP of the next instruction byte (must be preserved)
;
;      Note: Real Int_EFLAGS interrupt bit is always set.
;
;******************************************************************************
Opcode_POPFD_Handler proc

       movzx   ebx, ds:[ebp+Int_SS]    ; Get the stack segment
       shl     ebx, 4                  ; Shift it up by four
       mov     edx, ds:[ebp+Int_ESP]   ; Get the stack pointer
       add     ebx, edx                ; Linear address of the stack
       mov     eax, gs:[ebx]           ; Fetch the eflags from the VM stack
       add     edx, 4                  ; Add 4 bytes for the eflags
       mov     ds:[ebp+Int_ESP], edx   ; Store new stack pointer

       and     eax, V86_FLAGS_MASK     ; Clear the VM reserved bits
       or      ax, V86_FLAGS_SETIF     ; Add all the constant bits + IF

       mov     ds:[ebp+Int_EFLAGS],eax ; Store new eflags
       ret                             ; Return control to the caller

Opcode_POPFD_Handler endp


       Subttl  Opcode_CLI_Handler
       Page    +
;******************************************************************************
;
;      Opcode_CLI_Handler
;
;******************************************************************************
;
;      Registers on entry:
;              gs: selector for global memory
;              edx - Linear address of the faulty instruction
;              ecx - EIP of the next instruction byte (must be preserved)
;
;      Registers on exit:
;              ecx - EIP of the next instruction byte (must be preserved)
;
;
;******************************************************************************
Opcode_CLI_Handler proc
       mov     ds:[VM_IF], 0           ; Reset the interrupt enable flag bit
       ret                             ; Return control to the caller
Opcode_CLI_Handler endp


       Subttl  Opcode_STI_Handler
       Page    +
;******************************************************************************
;
;      Opcode_STI_Handler
;
;******************************************************************************
;
;      Registers on entry:
;              gs: selector for global memory
;              edx - Linear address of the faulty instruction
;              ecx - EIP of the next instruction byte (must be preserved)
;
;      Registers on exit:
;              ecx - EIP of the next instruction byte (must be preserved)
;
;
;******************************************************************************
Opcode_STI_Handler proc
       mov     ds:[VM_IF], IF_MASK     ; Set the interrupt enable flag bit
       ret                             ; Return control to the caller
Opcode_STI_Handler endp


       Subttl  Opcode_INTX_Handler
       Page    +
;******************************************************************************
;
;      Opcode_INTX_Handler
;
;******************************************************************************
;
;      Interrupt may be hooked by the kernel by setting the handler into the
;      slot of IntXX_Handler[].
;
;      Registers on entry:
;              gs: selector for global memory
;              edx - Linear address of the faulty instruction
;              ecx - EIP of the next instruction byte (must be preserved)
;
;      Registers on exit:
;              ecx - EIP of the next instruction byte (must be preserved)
;
;              well, this function changes ecx by the virtue of changing
;              the execution flow.
;
;******************************************************************************
Opcode_INTX_Handler proc

       movzx   edi, byte ptr gs:[edx+1]; Get the interrupt number in edi
       inc     ecx                     ; Next instruction after the int number

Opcode_INT3_Entry:
       mov     ebx, [ds:IntXX_Handler + edi*4] ; Get the handler function ptr
       or      ebx, ebx                        ; Is it NULL?
       jz      DoInt                           ; Do interrupt as normal

       push    ecx                     ; Store the registers
       push    edx
       push    edi
       push    ebp                     ; Argument is a pointer to TIntStack
       call    [ds:IntXX_Handler + edi*4]
       pop     ebp
       pop     edi
       pop     edx                     ; Restore the registers
       pop     ecx

       ; If the hook function returned zero in eax, we will not send this
       ; interrupt to execution in V86 mode.

       or      eax, eax                ; FALSE ?
       jz      Int_Ret                 ; Return if it was
DoInt:
       movzx   ebx, ds:[ebp+Int_SS]    ; Get the stack segment
       shl     ebx, 4                  ; Shift it up by four
       movzx   edx, word ptr ds:[ebp+Int_ESP]   ; Get the stack pointer
       sub     dx, 6                   ; Lower 6 bytes for the stack frame
       add     ebx, edx                ; Linear address of the stack
       mov     ds:[ebp+Int_ESP], edx   ; Store new stack pointer

       mov     dx, ds:[ebp+Int_CS]     ; Get the current CS
       shl     edx, 16
       mov     dx, cx                  ; Get the current IP
       mov     gs:[ebx], edx           ; Store it on the V86 stack

       mov     eax, ds:[ebp+Int_EFLAGS]; Get the V86 flags register
       btr     eax, IF_BIT             ; Reset the interrupt bit
       or      eax, ds:[VM_IF]         ; Add the virtual interrupt bit
       mov     gs:[ebx+4], ax          ; Store the V86 flags on the stack

       mov     eax, gs:[edi*4]         ; Get the new segment:offset from IT
       mov     ecx, eax                ; copy it to ecx for return (eip)
       and     ecx, 0FFFFh             ; Mask EIP portion
       shr     eax, 16
       mov     ds:[ebp+Int_CS], ax     ; Store the new CS
Int_Ret:
       ret                             ; Return control to the caller

Opcode_INTX_Handler endp


       Subttl  Opcode_INTX_Handler
       Page    +
;******************************************************************************
;
;      Opcode_INT3_Handler
;
;******************************************************************************
;
;      Debug interrupt 3 handler
;
;      Registers on entry:
;              gs: selector for global memory
;              edx - Linear address of the faulty instruction
;              ecx - EIP of the next instruction byte (must be preserved)
;
;      Registers on exit:
;              ecx - EIP of the next instruction byte (must be preserved)
;
;******************************************************************************
Opcode_INT3_Handler proc

       mov     edi, 3                  ; Interrupt number 3
       jmp     Opcode_INT3_Entry       ; Jump to a common interrupt routine

Opcode_INT3_Handler endp


       Subttl  Opcode_IRET_Handler
       Page    +
;******************************************************************************
;
;      Opcode_IRET_Handler
;
;******************************************************************************
;
;      Registers on entry:
;              gs: selector for global memory
;              edx - Linear address of the faulty instruction
;              ecx - EIP of the next instruction byte (must be preserved)
;
;      Registers on exit:
;              ecx - EIP of the next instruction byte (must be preserved)
;
;              well, this function changes ecx by the virtue of changing
;              the execution flow.
;
;******************************************************************************
Opcode_IRET_Handler proc

       movzx   ebx, ds:[ebp+Int_SS]    ; Get the stack segment
       shl     ebx, 4                  ; Shift it up by four
       movzx   edx, word ptr ds:[ebp+Int_ESP]   ; Get the stack pointer
       add     ebx, edx                ; Linear address of the stack
       add     edx, 6                  ; Add 6 bytes for the FL:CS:IP
       mov     ds:[ebp+Int_ESP], edx   ; Store new stack pointer

       mov     eax, gs:[ebx]           ; Fetch new CS:IP from the VM stack
       mov     ecx, eax                ; Set it as a next instruction
       and     ecx, 0FFFFh             ; Only low 16 bits
       shr     eax, 16
       mov     ds:[ebp+Int_CS], ax     ; Store new CS

       mov     ax, gs:[ebx+4]          ; Fetch FLAGS from the VM stack
       and     ax, V86_FLAGS_MASK      ; Clear the VM reserved bits
       or      ax, V86_FLAGS_SETIF     ; Add all the constant bits + IF
       mov     word ptr ds:[ebp+Int_EFLAGS], ax ; Store new flags

       ret                             ; Return control to the caller

Opcode_IRET_Handler endp


       Subttl  Opcode_IRETD_Handler
       Page    +
;******************************************************************************
;
;      Opcode_IRETD_Handler
;
;******************************************************************************
;
;      Registers on entry:
;              gs: selector for global memory
;              edx - Linear address of the faulty instruction
;              ecx - EIP of the next instruction byte (must be preserved)
;
;      Registers on exit:
;              ecx - EIP of the next instruction byte (must be preserved)
;
;              well, this function changes ecx by the virtue of changing
;              the execution flow.
;
;******************************************************************************
Opcode_IRETD_Handler proc

       movzx   ebx, ds:[ebp+Int_SS]    ; Get the stack segment
       shl     ebx, 4                  ; Shift it up by four
       mov     edx, ds:[ebp+Int_ESP]   ; Get the stack pointer
       add     ebx, edx                ; Linear address of the stack
       add     edx, 12                 ; Add 12 bytes for the EFL:__CS:EIP
       mov     ds:[ebp+Int_ESP], edx   ; Store new stack pointer

       mov     ecx, gs:[ebx]           ; Fetch new EIP from the VM stack
       mov     ax, gs:[ebx+4]          ; Get the cs register value
       mov     ds:[ebp+Int_CS], ax     ; Store new CS
       mov     eax, gs:[ebx+8]         ; Fetch FLAGS from the VM stack

       and     eax, V86_FLAGS_MASK     ; Clear the VM reserved bits
       or      ax, V86_FLAGS_SETIF     ; Add all the constant bits + IF
       mov     ds:[ebp+Int_EFLAGS],eax ; Store new flags

       ret                             ; Return control to the caller

Opcode_IRETD_Handler endp


       Subttl  Opcode_Invalid_Handler
       Page    +
;******************************************************************************
;
;      Opcode_Invalid_Handler
;
;******************************************************************************
;
;      Registers on entry:
;              gs: selector for global memory
;              edx - Linear address of the faulty instruction
;              ecx - EIP of the next instruction byte (must be preserved)
;
;      Registers on exit:
;              ecx - EIP of the next instruction byte (must be preserved)
;
;
;******************************************************************************
Opcode_Invalid_Handler proc

       pop     eax                     ; Pop the returning address
       jmp     GPF_Handler

Opcode_Invalid_Handler endp


;******************************************************************************
; End of opcode handlers
;******************************************************************************

       Subttl  Reflect_V86_Int
       Page    +
;******************************************************************************
;
;      Reflect_Int( int nIntNum, TIntStack *pStack )
;
;******************************************************************************
;
;      This function reflects the interrupt number nIntNum to the current VM.
;
;      By default, interrupts and exceptions that occurred in V86 mode are not
;      reflected as such unless this function is called to set up the VM stack
;      and the returning address so the execution can continue from the V86
;      mode handler.
;
;      Registers on entry:
;              Arg1 - interrupt number to reflect
;              Arg2 - pointer to a stack of the interrupted V86 code
;
;      Registers on exit:
;              V86 stack set up and PM returning address changed
;
;******************************************************************************
Reflect_Int    proc

       push    ebp                     ; Standard stack frame
       mov     ebp, esp
       pushad                          ; Store all registers

       mov     ecx, ss:[ebp+Arg2]      ; Get the pointer to a stack structure

       movzx   ebx, ds:[ecx+Int_SS]    ; Get the stack segment
       shl     ebx, 4                  ; Shift it up by four
       mov     edx, ds:[ecx+Int_ESP]   ; Get the stack pointer
       sub     edx, 6                  ; Lower 6 bytes for the stack frame
       add     ebx, edx                ; Linear address of the V86 stack
       mov     ds:[ecx+Int_ESP], edx   ; Store new stack pointer

       mov     dx, ds:[ecx+Int_CS]     ; Get the current CS
       shl     edx, 16
       mov     dx, word ptr ds:[ecx+Int_EIP] ; Get the current IP
       mov     gs:[ebx], edx           ; Store it on the V86 stack

       mov     eax, ds:[ecx+Int_EFLAGS]; Get the V86 flags register
       mov     gs:[ebx+4], ax          ; Store the V86 flags on the stack

       mov     edx, ds:[ebp+Arg1]      ; Get the interrupt number
       mov     eax, gs:[ edx * 4 ]     ; Get the new segment:offset from IT

       mov     word ptr ds:[ecx+Int_EIP], ax
       shr     eax, 16
       mov     ds:[ecx+Int_CS], ax     ; Store new CS:IP

       popad                           ; Restore all registers
       pop     ebp                     ; Restore stack frame
       ret                             ; Return control to the caller

Reflect_Int    endp



       Subttl  V86_Int
       Page    +
;******************************************************************************
;
;      V86_Int( int nIntNum, TSeg *pSeg, TReg *pRegs )
;
;******************************************************************************
;
;      This function issues and completes a V86 interrupt call in the context
;      of the current VM.
;
;      Registers on entry:
;
;
;      Registers on exit:
;
;
;******************************************************************************
V86_Int        proc

       push    ebp                     ; Standard stack frame
       mov     ebp, esp
       pushad                          ; Store all registers

       cli                             ; Disable ints during this setup process

       ; We will build a PM stack frame that will load all the registers during
       ; the popad/iretd sequence. Since the execution is exclusively in V86
       ; mode until return, we set up the TSS->esp to handle all the interrupts
       ; without messing up previous stack (including our returning address)

       push    dword ptr ds:[tss+TSS_ESP0] ; Push the old ESP from tss
       mov     ds:[tss+TSS_ESP0], esp      ; Store the current esp pointer

       ; Set the arguments on the stack

       mov     ebx, ss:[ebp+Arg2]      ; Get the address of the seg structure

       push    dword ptr ds:[ebx.Seg_GS]   ; Get segment registers and
       push    dword ptr ds:[ebx.Seg_FS]   ;  push them on the stack
       push    dword ptr ds:[ebx.Seg_DS]
       push    dword ptr ds:[ebx.Seg_ES]

       ; Store the returning address on the V86 mode stack and
       ; push the VM stack segment and pointer

       mov     edx, ss:[ebp+Arg4]      ; Get the pointer to a stack structure
       or      edx, edx                ; Is it NULL?
       jz      v86_setup_stack         ; Use the default stack

       mov     edx, ds:[pCurProc]      ; Get the address of the stack structure
                                       ; of the current process
       movzx   eax, ds:[edx+Int_SS]
       push    eax                     ; Push ss on the stack
       shl     eax, 4
       mov     ebx, ds:[edx+Int_ESP]   ; Get the esp
       jmp     v86_stack               ; jump to a common code
v86_setup_stack:
       mov     eax, ds:[VM_SS]         ; Get the VM stack segmnt
       push    eax                     ; Push ss on the stack
       shl     eax, 4
       mov     ebx, ds:[VM_SP]         ; Get the VM stack pointer
v86_stack:
       sub     ebx, 6                  ; Return segment:offset and flags
       push    ebx
       add     eax, ebx
       mov     edx, ds:[dwBreakSegOffs]
       mov     dword ptr gs:[eax], edx
       mov     word ptr gs:[eax+4], (2 or IF_MASK)

       ; Push the eflags

       push    dword ptr (2 or IF_MASK or VM_MASK)

       ; Set the address of the interrupt function in V86 mode

       movzx   eax, byte ptr ss:[ebp+Arg1] ; Get the interrupt number
       shl     eax, 2                  ; times 4 bytes per address
       mov     ebx, gs:[eax]           ; Get the seg:offset of the routine
       mov     eax, ebx
       shr     eax, 16                 ; Get the segment word
       push    eax
       and     ebx, 0FFFFh             ; Get the offset word
       push    ebx

       ; Set up the general registers

       mov     ebx, ss:[ebp+Arg3]

       mov     eax, ds:[ebx].Regs_EAX
       mov     ecx, ds:[ebx].Regs_ECX
       mov     edx, ds:[ebx].Regs_EDX
       mov     ebp, ds:[ebx].Regs_EBP
       mov     esi, ds:[ebx].Regs_ESI
       mov     edi, ds:[ebx].Regs_EDI
       mov     ebx, ds:[ebx].Regs_EBX

       ; Enter the function in V86 mode

       iretd                           ; Jump to the V86 code

V86_Int        endp


       Subttl  V86_Int_Ret
       Page    +
;******************************************************************************
;
;      V86_Int_Ret( TIntStack * pStack )
;
;******************************************************************************
;
;      This routine quits the V86 mode execution and returns to the PM caller
;      that initiated the VM execution.
;
;      The pStack points to the stack frame that was set up based on the
;      TSS.esp0.  That constitutes the current interrupt frame.
;
;******************************************************************************
V86_Int_Ret    proc

       push    ebp                     ; Simulate standard stack frame
       mov     ebp, esp

       mov     ebx, ss:[ebp+Arg1]      ; Get the address of the int stack frame
       mov     esp, ebx                ; Next to be popped
       mov     ecx, ebx                ; stack frame in ecx
       add     ecx, size Int_Stack     ; Skip over the stack frame
       add     ecx, 4 * 8              ; Skip over the pushad registers
       add     ecx, 4                  ; Skip over the tss.esp value
       mov     edi, ss:[ecx+Arg3]      ; Get the address of the register struct
       mov     esi, ss:[ecx+Arg2]      ; Get the address of the segment struct

       add     esp, 4 * 4              ; Skip over the PM selectors on the stack
       pop     dword ptr ds:[edi].Regs_EDI
       pop     dword ptr ds:[edi].Regs_ESI
       pop     dword ptr ds:[edi].Regs_EBP
       add     esp, 4
       pop     dword ptr ds:[edi].Regs_EBX
       pop     dword ptr ds:[edi].Regs_EDX
       pop     dword ptr ds:[edi].Regs_ECX
       pop     dword ptr ds:[edi].Regs_EAX

       add     esp, 4 * 3              ; Skip over the error code, eip and cs
       pop     dword ptr ds:[edi].Regs_ESP ; Store EFLAGS into ESP !!!!! (Hack?)

       add     esp, 4 * 2              ; Skip over the esp, ss

       pop     dword ptr ds:[esi].Seg_ES
       pop     dword ptr ds:[esi].Seg_DS
       pop     dword ptr ds:[esi].Seg_FS
       pop     dword ptr ds:[esi].Seg_GS

       pop     dword ptr ds:[tss+TSS_ESP0] ; Pop the previous tss.ESP into the tss

       popad                           ; Restore all registers
       pop     ebp                     ; Standard stack frame
       sti                             ; Be sure to enable interrupts now
       ret

V86_Int_Ret    endp


       Subttl  Int13_Handler
       Page    +
;******************************************************************************
;
;      Int13_VM_Handler
;
;******************************************************************************
;
;      This routine is a GP Falut exception handler.
;      For now, we are concerned about a case where:
;
;      1) V86 code tried to execute a privileged instruction
;
;      Registers on entry:
;              gs - selector for global memory
;              ds, es - data selectors
;
;      Registers on exit:
;
;
;******************************************************************************
Int13_VM_Handler  proc

       mov     ebp, esp                ; Set the base pointer
       add     ebp, 4                  ; Skip the returning address, so now ebp
                                       ; points to a Int_Stack structure
       movzx   edx, ds:[ebp+Int_CS]    ; Get the current CS
       shl     edx, 4                  ; Shift it up by four
       mov     ecx, ds:[ebp+Int_EIP]   ; Get the current EIP
       add     edx, ecx                ; Get the linear address
       inc     ecx                     ; Next opcode byte
       movzx   eax, byte ptr gs:[edx]  ; Get the faulty instruction opcode

       lea     ebx, ds:[Privileged_Instr_Handler]
       cmp     al, OPERAND_SIZE        ; Is the opcode 32-bit?
       jnz     Int13_16bit

       inc     edx                     ; Next linear byte of the opcode
       inc     ecx                     ; Next opcode byte
       mov     al, gs:[edx]            ; Get the opcode value
       lea     ebx, ds:[Privileged_Instr_Handler_32]

Int13_16bit:
       call    [ebx + eax*4]
       mov     ds:[ebp+Int_EIP], ecx   ; Store the next instr. opcode address

       or      dword ptr ds:[ebp+Int_EFLAGS], RF_MASK

       ret                             ; Return to the default intr. handler

Int13_VM_Handler  endp


_TEXT          ends

       End                             ; End of the module

