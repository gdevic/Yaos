/******************************************************************************
*                                                                             *
*   Module:     TSS.c                                                         *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       8/5/97                                                        *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

    This module contains the code that manipulates the Task State Segment
    (TSS).

    The TSS itself is defined here.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 8/5/97     1.00  Original                                       Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include <string.h>                     // Include string functions

#include "display.h"                    // Include display defines

#include "int.h"                        // Include interrupt header

#include "inline.h"                     // Include inline macros

#include "kernel.h"                     // Include kernel header file

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

// Task state segment structure
//
typedef struct
{
   WORD      back;                      // Backlink
   WORD      dummy1;                    // filler
   DWORD     esp0;                      // ESP for level 0
   WORD      ss0;                       // Stack selector for level 0
   WORD      dummy2;                    // filler
   DWORD     esp1;                      // ESP for level 1
   WORD      ss1;                       // Stack selector for level 1
   WORD      dummy3;                    // filler
   DWORD     esp2;                      // ESP for level 2
   WORD      ss2;                       // Stack selector for level 2
   WORD      dummy4;                    // filler
   DWORD     cr3;                       // PDPR, page directory address
   DWORD     eip;                       // current EIP
   DWORD     eflags;                    // current flags register
   DWORD     eax;                       // current eax register
   DWORD     ecx;                       // current ecx register
   DWORD     edx;                       // current edx register
   DWORD     ebx;                       // current ebx register
   DWORD     esp;                       // current esp register
   DWORD     ebp;                       // current ebp register
   DWORD     esi;                       // current esi register
   DWORD     edi;                       // current edi register
   WORD      es;                        // current es selector
   WORD      dummy5;                    // filler
   WORD      cs;                        // current cs selector
   WORD      dummy6;                    // filler
   WORD      ss;                        // current ss selector
   WORD      dummy7;                    // filler
   WORD      ds;                        // current ds selector
   WORD      dummy8;                    // filler
   WORD      fs;                        // current fs selector
   WORD      dummy9;                    // filler
   WORD      gs;                        // current gs selector
   WORD      dummyA;                    // filler
   WORD      ldt;                       // local descriptor table
   WORD      dummyB;                    // filler
   WORD      debug;                     // bit 0 is T, debug trap
   WORD      ioperm;                    // io permission offset
   BYTE      iopl[ TSS_IOPL_LEN ];      // io permission map
   BYTE      iopl_last[4];              // last byte should contain 255

} TSS_Struct;


//-----------------------------------------------------------------------------
// Define a basic task structure
//-----------------------------------------------------------------------------

TSS_Struct tss;


extern DWORD TOS;

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

void Invalid_TSS_Handler();


/******************************************************************************
*                                                                             *
*   void Init_TSS()                                                           *
*                                                                             *
*******************************************************************************
*
*   This function initializes the TSS data structures.
*
******************************************************************************/
void Init_TSS()
{
    dprintf("\nInit TSS");

    // Clean up the task structure space -
    // Also enable all I/O operations within that task by having 0 in IO
    // permission bitmap.

    memset( &tss, 0, sizeof(TSS_Struct) );

    // Initialize task state structure

    tss.esp0   = (DWORD) &TOS - 1024;
    tss.ss0    = SEL_DATA;
    tss.ioperm = 104;
    tss.debug  = 0;
    tss.iopl_last[0] = 255;

    // These field are updated with each task switch

    tss.ldt  = SEL_NULL;
    tss.gs   = SEL_GLOBAL;
    tss.fs   = SEL_NULL;
    tss.ds   = SEL_DATA;
    tss.ss   = SEL_DATA;
    tss.cs   = SEL_CODE;
    tss.es   = SEL_DATA;
    tss.cr3  = PHY_PD;


    // Register the exception handler

    Register_Exception_Handler( 0xA, Invalid_TSS_Handler );

    dprintf(".");
}


/******************************************************************************
*                                                                             *
*   void Invalid_TSS_Handler( TIntStack Stack )                               *
*                                                                             *
*******************************************************************************
*
*   This function is an invalid TSS exception handler.
*
******************************************************************************/
void Invalid_TSS_Handler( TIntStack Stack )
{
    dprintf("\n%c%cInvalid TSS Fault %08X%c%c",
        DP_SETWRITEATTR, ATTR_SCREAM, Stack.ErrorCode,
        DP_SETWRITEATTR, ATTR_RESPONSE );

    PrintStackFrame( &Stack );

    Int3();
}

