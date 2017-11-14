/*********************************************************************
*                                                                    *
*   Module:     tss.c
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

          This module implements the task-state segment
          manipulations.
          
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

/*********************************************************************
*   Include Files
**********************************************************************/
#include "tss.h"               // Include tss structure header
#include "kernel.h"            // Include basic kernel defines

/*********************************************************************
*   Global Functions
**********************************************************************/

/*********************************************************************
*   External Functions
**********************************************************************/
extern DWORD TopOfStack;

/*********************************************************************
*   Global variables
**********************************************************************/
TSS_Struct tss;

/*********************************************************************
*   Local Functions
**********************************************************************/

/*********************************************************************
*
*  void init_tss()
*
*  Initializes Task State Segment data structure.
*
**********************************************************************/
void init_tss()
{
    int i;

    tss.esp0 = (DWORD)&TopOfStack;
    tss.ss0  = DATASEL;
    tss.cr3  = 0;
    tss.ldt  = 0;
    tss.ioperm = 104;
    tss.debug = 0;

    // Enable all I/O operation within that task
    for( i=0; i<NUMIOPL; i++ )
    {
        tss.iopl[i] = 0;
    }

    tss.iopl_last = 255;
}

