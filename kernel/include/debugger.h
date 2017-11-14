/*********************************************************************
*                                                                    *
*   Module:     debugger.h                                           *
*                                                                    *
*   Revision:   1.00                                                 *
*                                                                    *
*   Date:       04/20/96                                             *
*                                                                    *
*   Author:     Goran Devic                                          *
*                                                                    *
**********************************************************************
*                                                                    *
*   Module Description:                                              *

        Header file for the debugger
       
**********************************************************************
*                                                                    *
*   Changes:                                                         *
*                                                                    *
*   DATE     REV   DESCRIPTION OF CHANGES                 AUTHOR     *
* --------   ----  -----------------------------------   -----------
  04/20/96   1.00  Original                              Goran Devic

* --------   ----  -----------------------------------   ----------- *
**********************************************************************
*   Important defines/undefines
**********************************************************************/
#ifndef _DEBUGGER_H_
#define _DEBUGGER_H_

/*********************************************************************
*   Include Files
**********************************************************************/
#include "types.h"


/*********************************************************************
*   Types and Defines
**********************************************************************/
#define TRAP_FLAG         0x100

extern void INT1();
#pragma aux INT1 = "int 1" parm;

typedef struct
{
    DWORD edi;
    DWORD esi;
    DWORD ebp;
    DWORD Temp;
    DWORD ebx;
    DWORD edx;
    DWORD ecx;
    DWORD eax;

    DWORD eip;
    DWORD cs;
    DWORD eflags;

} TDebug;


typedef struct
{
    WORD cs;
    WORD ds;
    WORD ss;
    WORD es;
    WORD fs;
    WORD gs;

} Tsel;


typedef struct                   // Menu structure
{
    const char *cmd;             // Contains command
    void *fn;                    // And a pointer to a function

} TMenu;


typedef struct                   // Address structure
{
    WORD sel;                    // Selector part
    DWORD addr;                  // Offset part

} TAddr;


// Descriptor structure
//
typedef struct
{
    DWORD limit15_0    : 16;  // Limit bits 15-0
    DWORD base15_0     : 16;  // Base address bits 15-0
    DWORD base23_16    : 8 ;  // Base address bits 23-16
    DWORD type         : 4 ;  // Type of the segment (pg 5-13)
    DWORD dt           : 1 ;  // Application set, system, gates clr
    DWORD dpl          : 2 ;  // Descriptor privilege level
    DWORD p            : 1 ;  // Segment present
    DWORD limit19_16   : 4 ;  // Limit bits 19-16
    DWORD avl          : 1 ;  // Available
    DWORD res          : 2 ;  // Reserved
    DWORD g            : 1 ;  // Granularity bit
    DWORD base31_24    : 8 ;  // Base address bits 31-24

} TSelector;

typedef struct                   // Breakpoint structure
{
    TAddr Addr;                  // Location
    BYTE bByte;                  // Byte that was there
    BYTE fUsed;                  // 1 if breakpoint is used, 0 if not

} TBreakpoint;


/*********************************************************************
*   Global Functions
**********************************************************************/
extern void  Int13_Handler(void);
extern void  DebugINT1_Handler(void);
extern void  DebugINT3_Handler(void);

extern void  InitDebugger();
extern DWORD dwPeek( TAddr *, DWORD );
extern WORD  wPeek( TAddr *, DWORD );
extern BYTE  bPeek( TAddr *, DWORD );
extern void  bPoke( TAddr *, BYTE );

extern void  Debugger( TDebug *, int OpSize );


#endif // _DEBUGGER_H_
