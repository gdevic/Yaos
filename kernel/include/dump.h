/******************************************************************************
*                                                                             *
*   Module:     Dump.c                                                        *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       9/14/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This is a header file for the debugger dump module.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 9/14/97    1.00  Original                                       Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _DUMP_H_
#define _DUMP_H_


/******************************************************************************
*                                                                             *
*   Include Files                                                             *
*                                                                             *
******************************************************************************/

#include "types.h"                      // Include basic data types
#include "intel.h"                      // Include memory access macros

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

#define BYTE_NOT_PRESENT    0x1FF       // Byte is not present value


/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern DWORD GetPhysicalAddress( DWORD dwLinAddress );
extern WORD GetByte( DWORD dwLinAddress );
extern void Dump( DWORD dwLinAddress, DWORD dwPrintAddress,
    int nLines, int nSize, char *sPrefix );


#endif //  _DUMP_H_
