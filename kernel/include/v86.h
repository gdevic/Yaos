/******************************************************************************
*                                                                             *
*   Module:     V86.h                                                         *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       9/17/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This is a header file for the Virtual 86 Machine module
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 9/17/97    Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _V86_H_
#define _V86_H_


/******************************************************************************
*                                                                             *
*   Include Files                                                             *
*                                                                             *
******************************************************************************/

#include "kernel.h"                     // Include kernel data structures

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

extern DWORD dwBreakSegOffs;            // that address in segment:offset form

extern TReg Reg;                        // Global register and segment structs
extern TSeg Seg;


// Array of pointers to functions to handle VM interrupt calls (INT XX).
// Client can hook these to simulate V86 interrupt handler.

extern BOOL (*IntXX_Handler[256])(TIntStack *pStack);


/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern void V86_Int( int nIntNum, TSeg *pSeg, TReg *pRegs, TIntStack *pStack );
extern BOOL Reflect_V86_Int( int nIntNum, TIntStack *pStack );
extern DWORD GetVMBreakAddress();


#endif //  _V86_H_
