/*********************************************************************
*                                                                    *
*   Module:     tss.h
*                                                                    *
*   Revision:   1.00                                                 *
*                                                                    *
*   Date:       07/07/96                                             *
*                                                                    *
*   Author:     Goran Devic                                          *
*                                                                    *
**********************************************************************
*                                                                    *
*   Module Description:                                              *

          This is the header file for tss.c, task-state segment
          operations.
          
**********************************************************************
*                                                                    *
*   Changes:                                                         *
*                                                                    *
*   DATE     REV   DESCRIPTION OF CHANGES                 AUTHOR     *
* --------   ----  -----------------------------------   -----------
  07/07/96   1.00  Original                              Goran Devic

* --------   ----  -----------------------------------   ----------- *
**********************************************************************
*   Important defines/undefines
**********************************************************************/
#ifndef _TSS_H_
#define _TSS_H_

/*********************************************************************
*   Include Files
**********************************************************************/
#include "types.h"              // Include basic data types


/*********************************************************************
*   Variables and defines
**********************************************************************/
#define NUMIOPL        8192     // Length of iopl in bytes in TSS
                                        

// Task state segment structure
//
typedef struct
{
   WORD      back;              // Backlink
   WORD      dummy1;            // filler
   DWORD     esp0;              // ESP for level 0
   WORD      ss0;               // Stack selector for level 0
   WORD      dummy2;            // filler
   DWORD     esp1;              // ESP for level 1
   WORD      ss1;               // Stack selector for level 1
   WORD      dummy3;            // filler
   DWORD     esp2;              // ESP for level 2
   WORD      ss2;               // Stack selector for level 2
   WORD      dummy4;            // filler
   DWORD     cr3;               // PDPR, page directory address
   DWORD     eip;               // current EIP
   DWORD     eflags;            // current flags register
   DWORD     eax;               // current eax register
   DWORD     ecx;               // current ecx register
   DWORD     edx;               // current edx register
   DWORD     ebx;               // current ebx register
   DWORD     esp;               // current esp register
   DWORD     ebp;               // current ebp register
   DWORD     esi;               // current esi register
   DWORD     edi;               // current edi register
   WORD      es;                // current es selector
   WORD      dummy5;            // filler
   WORD      cs;                // current cs selector
   WORD      dummy6;            // filler
   WORD      ss;                // current ss selector
   WORD      dummy7;            // filler
   WORD      ds;                // current ds selector
   WORD      dummy8;            // filler
   WORD      fs;                // current fs selector
   WORD      dummy9;            // filler
   WORD      gs;                // current gs selector
   WORD      dummyA;            // filler
   WORD      ldt;               // local descriptor table
   WORD      dummyB;            // filler
   WORD      debug;             // bit 0 is T, debug trap
   WORD      ioperm;            // io permission offset
   BYTE      iopl[ NUMIOPL ];   // io permission map
   BYTE      iopl_last;         // last byte should contain 255

} TSS_Struct;


#endif // _TSS_H_
