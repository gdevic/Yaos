/******************************************************************************
*                                                                             *
*   Module:     Map.h                                                         *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       10/25/97                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This is a header file for the symbol file module.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 10/25/97   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _MAP_H_
#define _MAP_H_


/******************************************************************************
*                                                                             *
*   Include Files                                                             *
*                                                                             *
******************************************************************************/

#include "types.h"                      // Include basic data types

#include "queue.h"                      // Include queue header file

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

extern TQueue qSym;                     // Linked list of sybmol names


/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern int LoadMap( char *sFileName, DWORD dwBaseAddress );
extern void SymPrint( char *sPattern );
extern DWORD GetSymbolAddress( char *sName );
extern char *GetSymbolName( DWORD dwAddress );
extern char *GetClosestSymbolName( int *pDelta, DWORD dwAddress );
extern void ClearSymbols( int pid );


#endif //  _MAP_H_
