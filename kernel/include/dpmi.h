/******************************************************************************
*                                                                             *
*   Module:     dpmi.h                                                        *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       10/9/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This is a header file for the DPMI module.  DPMI is not (yet)
        implemented although there are some hooks in place (2F/31).
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 10/9/97    Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _DPMI_H_
#define _DPMI_H_


/******************************************************************************
*                                                                             *
*   Include Files                                                             *
*                                                                             *
******************************************************************************/

#include "types.h"                      // Include basic data types

#include "kernel.h"                     // Include kernel header file

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

// Address of the DPMI mode switch function

extern DWORD dwDPMISegOffs;


/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern BOOL Int21_Handler( TIntStack *pStack );
extern BOOL Int2F_Handler( TIntStack *pStack );
extern BOOL Int31_Handler( TIntStack *pStack );


#endif //  _DPMI_H_
