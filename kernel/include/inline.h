/******************************************************************************
*                                                                             *
*   Module:     Inline.h                                                      *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       9/19/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This is a header file containing the Intel-inline macros that are
        done this way for performance reasons.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 9/19/97    Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _INLINE_H_
#define _INLINE_H_

/******************************************************************************
*                                                                             *
*   Include Files                                                             *
*                                                                             *
******************************************************************************/

#include "types.h"                      // Include basic data types

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   Inline Functions                                                          *
*                                                                             *
******************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Intel system services
///////////////////////////////////////////////////////////////////////////////

#pragma aux FlushTLB =   \
" mov eax, cr3"          \
" mov cr3, eax"          \
parm value [eax];

///////////////////////////////////////////////////////////////////////////////
// Input-Output inline functions
///////////////////////////////////////////////////////////////////////////////

#pragma aux inp =   \
" in al, (dx)"      \
parm caller [dx] value [al];


#pragma aux outp =  \
" out (dx), al"     \
parm caller [dx][al];


///////////////////////////////////////////////////////////////////////////////
// Memory access inline functions
///////////////////////////////////////////////////////////////////////////////

#pragma aux abs_pokedw =    \
" mov gs:[edx], eax"        \
parm caller [edx][eax];

#pragma aux abs_pokew =     \
" mov gs:[edx], ax"         \
parm caller [edx][ax];

#pragma aux abs_pokeb =     \
" mov gs:[edx], al"         \
parm caller [edx][al];


#pragma aux abs_peekdw =    \
" mov eax, gs:[edx]"        \
parm caller [edx] value [eax];

#pragma aux abs_peekw =     \
" mov ax, gs:[edx]"         \
parm caller [edx] value [ax];

#pragma aux abs_peekb =     \
" mov al, gs:[edx]"         \
parm caller [edx] value [al];


///////////////////////////////////////////////////////////////////////////////
// Memory move and copy functions
///////////////////////////////////////////////////////////////////////////////

#pragma aux abs_memcpy =    \
" or   ecx, ecx"            \
" jz   done"                \
" test ecx, 3"              \
" jnz  slow_path"           \
" shr  ecx, 2"              \
" push esi"                 \
" xor  esi, esi"            \
"copy_loop:"                \
" mov  eax, gs:[ebx+esi]"   \
" mov  gs:[edx+esi], eax"   \
" add  esi, 4"              \
" loop copy_loop"           \
" pop  esi"                 \
" jmp  done"                \
"slow_path:"                \
" mov  al, gs:[ebx]"        \
" mov  gs:[edx], al"        \
" inc  ebx"                 \
" inc  edx"                 \
" loop slow_path"           \
"done:"                     \
parm caller [edx][ebx][ecx] modify [eax];

#pragma aux abs_memsetw =   \
"set_loop:"                 \
" mov gs:[edx], ax"         \
" inc edx"                  \
" inc edx"                  \
" loop set_loop"            \
parm caller [edx][ax][ecx];


#pragma aux memsetw =       \
"set_loop:"                 \
" mov ds:[edx], ax"         \
" inc edx"                  \
" inc edx"                  \
" loop set_loop"            \
parm caller [edx][ax][ecx];


#pragma aux GetFaultyAddress =  \
" mov  eax, cr2"                \
parm value [eax];


///////////////////////////////////////////////////////////////////////////////
// Interrupt enable/disable
///////////////////////////////////////////////////////////////////////////////

#pragma aux EnableInterrupts =  \
" sti"                          \
parm;

#pragma aux DisableInterrupts = \
" cli"                          \
parm;


///////////////////////////////////////////////////////////////////////////////
// Debug interrupts inline
///////////////////////////////////////////////////////////////////////////////

#pragma aux Int1 =  \
" int 1"            \
parm;

#pragma aux Int3 =  \
" int 3"            \
parm;

#pragma aux LoadIDT =       \
" lidt fword ptr ds:[eax]"  \
parm caller [eax];

#pragma aux SetLDT = \
" lldt ax"           \
parm caller [ax];

#pragma aux GetLDT = \
" sldt ax"           \
parm value [ax];


///////////////////////////////////////////////////////////////////////////////
// Compare memory pages for VM.c
///////////////////////////////////////////////////////////////////////////////

#pragma aux PageCompare =   \
"    mov  ecx, 1024"        \
"    shl  ebx, 12"          \
"lp: mov  eax, gs:[ebx]"    \
"    cmp  eax, ds:[edx+ebx]"\
"    jnz  not_equal"        \
"    add  ebx, 4"           \
"    loop lp"               \
"    xor  eax, eax"         \
"    jmp  return"           \
"not_equal:"                \
"    mov  eax, 1"           \
"return:"                   \
parm caller [ebx][edx] modify [ecx] value [eax];


///////////////////////////////////////////////////////////////////////////////
// Debug registers
///////////////////////////////////////////////////////////////////////////////

#pragma aux GetDebugReg =       \
"    or eax, eax"               \
"    jz _r0"                    \
"    dec eax"                   \
"    jz _r1"                    \
"    dec eax"                   \
"    jz _r2"                    \
"    dec eax"                   \
"    jz _r3"                    \
"    dec eax"                   \
"    dec eax"                   \
"    dec eax"                   \
"    jz _r6"                    \
"_r7:mov eax, dr7"              \
"    jmp end"                   \
"_r0:mov eax, dr0"              \
"    jmp end"                   \
"_r1:mov eax, dr1"              \
"    jmp end"                   \
"_r2:mov eax, dr2"              \
"    jmp end"                   \
"_r3:mov eax, dr3"              \
"    jmp end"                   \
"_r6:mov eax, dr6"              \
"End:"                          \
parm caller [eax] value [eax];


#pragma aux SetDebugReg =       \
"    or ebx, ebx"               \
"    jz _r0"                    \
"    dec ebx"                   \
"    jz _r1"                    \
"    dec ebx"                   \
"    jz _r2"                    \
"    dec ebx"                   \
"    jz _r3"                    \
"    dec ebx"                   \
"    dec ebx"                   \
"    dec ebx"                   \
"    jz _r6"                    \
"_r7:mov dr7, eax"              \
"    jmp end"                   \
"_r0:mov dr0, eax"              \
"    jmp end"                   \
"_r1:mov dr1, eax"              \
"    jmp end"                   \
"_r2:mov dr2, eax"              \
"    jmp end"                   \
"_r3:mov dr3, eax"              \
"    jmp end"                   \
"_r6:mov dr6, eax"              \
"End:"                          \
parm caller [ebx][eax];


///////////////////////////////////////////////////////////////////////////////
// Executes function
///////////////////////////////////////////////////////////////////////////////

#pragma aux ExecFunction =      \
"   push edi"                   \
"   push esi"                   \
"   push edx"                   \
"   push ecx"                   \
"   push ebx"                   \
"   push eax"                   \
"   call [edx]"                 \
"   add  esp, 4"                \
"   pop  ebx"                   \
"   pop  ecx"                   \
"   pop  edx"                   \
"   pop  esi"                   \
"   pop  edi"                   \
parm caller [edx][eax][ebx][ecx] value [eax];


///////////////////////////////////////////////////////////////////////////////
// Supports terminal to quickly positions the cursor on VGA
///////////////////////////////////////////////////////////////////////////////

#pragma aux SetCursor =     \
"   mov dx, 3D4h"           \
"   mov al, 0Eh"            \
"   out dx, al"             \
"   inc edx"                \
"   mov al, ch"             \
"   out dx, al"             \
"   dec edx"                \
"   mov al, 0Fh"            \
"   out dx, al"             \
"   mov al, cl"             \
"   inc edx"                \
"   out dx, al"             \
parm caller [ecx] modify [edx ecx] value [eax];


///////////////////////////////////////////////////////////////////////////////
// Supports initialization
///////////////////////////////////////////////////////////////////////////////

#pragma aux SubESP =        \
"   sub  esp, eax"          \
parm caller [eax];


#pragma aux GetESP =        \
"   mov  eax, esp"          \
parm value [eax];


#pragma aux LiftOff =       \
"   pop  eax"               \
"   mov  es, ax"            \
"   pop  eax"               \
"   mov  ds, ax"            \
"   pop  eax"               \
"   mov  fs, ax"            \
"   pop  eax"               \
"   mov  gs, ax"            \
"   popad"                  \
"   add  esp, 4"            \
"   iretd"                  \
parm;


/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern DWORD FlushTLB();

extern BYTE inp( WORD wReg );
extern void outp( WORD wReg, BYTE bByte );

extern void abs_pokedw( DWORD dwLinAddress, DWORD dwStore );
extern void abs_pokew( DWORD dwLinAddress, WORD wStore );
extern void abs_pokeb( DWORD dwLinAddress, BYTE bStore );
extern DWORD abs_peekdw( DWORD dwLinAddress );
extern WORD abs_peekw( DWORD dwLinAddress );
extern BYTE abs_peekb( DWORD dwLinAddress );

extern DWORD GetFaultyAddress();

extern void abs_memcpy( DWORD dwDest, DWORD dwSrc, DWORD dwLen );
extern void abs_memsetw( DWORD dwDest, WORD wWord, DWORD dwCount );
extern void memsetw( DWORD dwDest, WORD wWord, DWORD dwCount );

extern void EnableInterrupts();
extern void DisableInterrupts();

extern void Int1();
extern void Int3();
extern void LoadIDT( void * IDT_Descriptor );
extern void SetLDT( WORD LDT_Descriptor );
extern WORD GetLDT();

extern int PageCompare( int page_no, DWORD base2 );

extern DWORD GetDebugReg(int index);
extern void  SetDebugReg(int index, DWORD value);

extern int ExecFunction(int *pAddress, int Arg1, int Arg2, int Arg3 );

extern int SetCursor( int offset );

extern void SubESP( int Amount );
extern DWORD GetESP( void );
extern void LiftOff(void);


#endif //  _INLINE_H_
