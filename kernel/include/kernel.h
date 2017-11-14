/******************************************************************************
*                                                                             *
*   Module:     kernel.h                                                      *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       09/14/96                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This is a header file for the kernel.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 03/30/96   1.00  Original                                       Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _KERNEL_H_
#define _KERNEL_H_

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

#define ABS_KERNEL_LIN      0x110000    // Global Linear address of the kernel
#define ABS_KERNEL_HEAP_TOP 0x200000    // Global Top of the kernel heap
#define MIN_FREE_MEMORY     1023*1024   // Min amount of free memory to run

#define KERNEL_HEAP_TOP     (ABS_KERNEL_HEAP_TOP - ABS_KERNEL_LIN)



//#define ABS_HEAP_TOP       2*1024*1024     // Start of user memory
//#define HEAP_TOP   (ABS_HEAP_TOP - KERNEL_LIN) // kernel's view of that
                                                                      
#define NULLSEL        0x0000       // Null selector value
#define CODESEL        0x0008       // Kernel code selector value
#define DATASEL        0x0010       // Kernel data selector value
#define GLOBALSEL      0x0018       // Global data selector value

struct BYTEREGS
{
    WORD flags;
    BYTE al, ah;
    BYTE bl, bh;
    BYTE cl, ch;
    BYTE dl, dh;

    WORD si;
    WORD di;
    
    WORD ds;
    WORD es;
};

struct WORDREGS
{
    WORD flags;
    WORD ax;
    WORD bx;
    WORD cx;
    WORD dx;

    WORD si;
    WORD di;
    
    WORD ds;
    WORD es;
};


typedef union
{
    struct BYTEREGS b;
    struct WORDREGS w;

}REGS;


typedef struct
{
    DWORD  c       : 1;     // Carry
    DWORD  res1    : 1;
    DWORD  p       : 1;     // Parity
    DWORD  res2    : 1;
    DWORD  a       : 1;     // Aux carry
    DWORD  res3    : 1;
    DWORD  z       : 1;     // Zero
    DWORD  s       : 1;     // Sign
    DWORD  t       : 1;     // Trap
    DWORD  i       : 1;     // Interrupt
    DWORD  d       : 1;     // Direction
    DWORD  o       : 1;     // Overflow
    DWORD  iopl    : 2;     // IO Privilege
    DWORD  nt      : 1;     // Nested task
    DWORD  res4    : 1;
    DWORD  r       : 1;     // Resume
    DWORD  vm      : 1;     // Virtual mode

} Teflags;


extern BYTE  GDT;
extern DWORD TopOfStack;
extern BYTE   HeapStart;
extern DWORD TaskTopStack;


/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern void KernelPanic( char *pszMessage, ... );
extern void Halt( char *pszMessage, ... );

extern void  Int14_Handler();
extern void  Intv86( BYTE, REGS * );
extern void  PassV86Interrupt( BYTE );

extern void  GetLine( BYTE * );

//
// From printk.c
//
extern int printk( const char *format, ... );


#endif // _KERNEL_H_
