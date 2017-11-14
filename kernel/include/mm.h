/******************************************************************************
*                                                                             *
*   Module:     mm.h                                                          *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       04/20/96                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This is a header file for the basic memory management code

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 04/20/96   1.00  Original                                       Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _MM_H_
#define _MM_H_

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

// Attributes in a page record - use with MapPhysical()
//
#define PAGE_PRESENT       0x00000001   // Present bit
#define PAGE_NOT_PRESENT   0x00000000   //   Not present
#define PAGE_READWRITE     0x00000002   // Read/Write bit
#define PAGE_READ          0x00000000   //   Read only
#define PAGE_USER          0x00000004   // *User*/Supervisor bit
#define PAGE_SUPERVISOR    0x00000000   //   User/*Supervisor* bit
#define PAGE_ACCESSED      0x00000020   // Accessed bit
#define PAGE_DIRTY         0x00000040   // Dirty bit


/******************************************************************************
*
*  Memory access macros:
*      Abs* uses fs selector and acts on the absolute memory address
*      *b, *w, *d are the byte, word and dword forms of the instr.
*
*  Assumes: fs = global selector (base 0)
*
*     SelPeekd, Selpokeb use a given selector to access memory.  The macros
*       use gs selector to load it.
*
******************************************************************************/
void AbsMemcpyb( DWORD dwDest, DWORD dwSrc, DWORD dwBytes );
#pragma aux AbsMemcpyb =  \
"lp:"                     \
"      mov  dl, fs:[ebx]" \
"      mov  fs:[eax], dl" \
"      inc  ebx"          \
"      inc  eax"          \
"      loop lp"           \
parm caller [eax] [ebx] [ecx] modify [dx];

void AbsMemcpyw( DWORD dwDest, DWORD dwSrc, DWORD dwWords );
#pragma aux AbsMemcpyw =  \
"lp:"                     \
"      mov  dx, fs:[ebx]" \
"      mov  fs:[eax], dx" \
"      add  ebx, 2"       \
"      add  eax, 2"       \
"      loop lp"           \
parm caller [eax] [ebx] [ecx] modify [dx];

void AbsMemcpyd( DWORD dwDest, DWORD dwSrc, DWORD dwDwords );
#pragma aux AbsMemcpyd =  \
"lp:"                     \
"      mov  edx, fs:[ebx]"\
"      mov  fs:[eax], edx"\
"      add  ebx, 4"       \
"      add  eax, 4"       \
"      loop lp"           \
parm caller [eax] [ebx] [ecx] modify [edx];


void AbsMemsetb( DWORD dwDest, DWORD dwBytes, BYTE bFiller );
#pragma aux AbsMemsetb =  \
"lp:"                     \
"      mov  fs:[ebx+ecx-1], al "\
"      loop lp"           \
parm caller [ebx] [ecx] [al];

void AbsMemsetw( DWORD dwDest, DWORD dwWords, WORD wFiller );
#pragma aux AbsMemsetw =  \
"lp:"                     \
"      mov  fs:[ebx], ax "\
"      add  ebx, 2"       \
"      loop lp"           \
parm caller [ebx] [ecx] [ax];

void AbsMemsetd( DWORD dwDest, DWORD dwDwords, DWORD dwFiller );
#pragma aux AbsMemsetd =  \
"lp:"                     \
"      mov  fs:[ebx], eax "\
"      add  ebx, 4"       \
"      loop lp"           \
parm caller [ebx] [ecx] [eax];


void AbsPokeb( DWORD dwAddr, BYTE bByte );
#pragma aux AbsPokeb =    \
"      mov  fs:[ebx], al" \
parm caller [ebx] [al];

void AbsPokew( DWORD dwAddr, WORD wWord );
#pragma aux AbsPokew =    \
"      mov  fs:[ebx], ax" \
parm caller [ebx] [ax];

void AbsPoked( DWORD dwAddr, DWORD dwDword );
#pragma aux AbsPoked =     \
"      mov  fs:[ebx], eax" \
parm caller [ebx] [eax];


BYTE AbsPeekb( DWORD dwAddr );
#pragma aux AbsPeekb =    \
"      mov  al, fs:[ebx]" \
parm caller [ebx] value [al];

WORD AbsPeekw( DWORD dwAddr );
#pragma aux AbsPeekw =    \
"      mov  ax, fs:[ebx]" \
parm caller [ebx] value [ax];

WORD AbsPeekw( DWORD dwAddr );
DWORD AbsPeekd( DWORD dwAddr );
#pragma aux AbsPeekd =     \
"      mov  eax, fs:[ebx]" \
parm caller [ebx] value [eax];


DWORD SelPeekd( WORD wSelector, DWORD dwOffset );
#pragma aux SelPeekd =      \
"      mov  gs, ax"         \
"      mov  eax, gs:[ebx]"  \
parm caller [ax] [ebx] value [eax];

DWORD SelPeekd( WORD wSelector, DWORD dwOffset );
void SelPokeb( WORD wSelector, DWORD dwOffset, BYTE bByte );
#pragma aux SelPokeb =      \
"      mov  gs, ax"         \
"      mov  gs:[ebx], cl"   \
parm caller [ax] [ebx] [cl];


/******************************************************************************
*
*  Memory access macros:
*      k* kernel memory access
*      *b, *w, *d are the byte, word and dword forms of the function
*
*  Assumes: es = ds
*
******************************************************************************/
void kmemcpy( void * dest, const void * src, int len );
#pragma aux kmemcpy = \
"rep movsb"            \
parm caller [edi] [esi] [ecx];

void kmemset( void * dest, int c, int len );
#pragma aux kmemset = \
"rep stosb"            \
parm caller [edi] [eax] [ecx];


/******************************************************************************
*
*   FlushTLB()
*
*   Flushes the Translation-Lookaside Buffer.
*
******************************************************************************/
void FlushTLB();
#pragma aux FlushTLB = \
"  mov eax, cr3"       \
"  mov cr3, eax"       \
"  nop"                \
parm modify [eax];

/******************************************************************************
*
*   void EnablePaging( DWORD dwPhyPageDirectory )
*
*   Enables paging.
*
*   Where:
*       dwPhyPageDirectory is the physical address of the frame containing
*           the page directory table
*
******************************************************************************/
void EnablePaging( DWORD dwPhyPageDirectory );
#pragma aux EnablePaging = \
"  mov cr3, eax"       \
"  mov eax, cr0"       \
"  or  eax, 0x80000000"\
"  mov cr0, eax"       \
"  nop"                \
parm caller [eax];

/******************************************************************************
*
*   void DisablePaging()
*
*   Disables paging.
*
******************************************************************************/
void DisablePaging();
#pragma aux DisablePaging = \
"  mov eax, cr0"       \
"  and eax, 0x7fffffff"\
"  mov cr0, eax"       \
"  nop"                \
parm modify [eax];


/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/
extern void  AbsMemcpyb( DWORD dwDest, DWORD dwSrc, DWORD dwBytes );
extern void  AbsMemcpyw( DWORD dwDest, DWORD dwSrc, DWORD dwWords );
extern void  AbsMemcpyd( DWORD dwDest, DWORD dwSrc, DWORD dwDwords );

extern void  AbsMemsetb( DWORD dwDest, DWORD dwBytes, BYTE bFiller );
extern void  AbsMemsetw( DWORD dwDest, DWORD dwWords, WORD wFiller );
extern void  AbsMemsetd( DWORD dwDest, DWORD dwDwords, DWORD dwFiller );

extern void  AbsPokeb( DWORD dwAddr, BYTE bByte );
extern void  AbsPokew( DWORD dwAddr, WORD wWord );
extern void  AbsPoked( DWORD dwAddr, DWORD dwDword );

extern BYTE  AbsPeekb( DWORD dwAddr );
extern WORD  AbsPeekw( DWORD dwAddr );
extern DWORD AbsPeekd( DWORD dwAddr );

extern DWORD SelPeekd( WORD wSelector, DWORD dwOffset );
extern void  SelPokeb( WORD wSelector, DWORD dwOffset, BYTE bByte );

extern void  kmemcpy( void * dest, const void * src, int len );
extern void  kmemset( void * dest, int c, int len );

extern void  FlushTLB();
extern void  EnablePaging( DWORD dwPhyPageDirectory );
extern void  DisablePaging();

extern void  InitMemoryManager();

extern DWORD PageGetPhyAddr( DWORD dwLinearAddr );
extern DWORD MapPhysical( DWORD dwAbsLinearAddr, DWORD dwPhySim, DWORD dwSize, DWORD dwAttributes );
extern void  UnmapPhysical( DWORD dwAbsLinearAddr, DWORD dwSize );


#endif // _MM_H_
