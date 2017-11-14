/******************************************************************************
*                                                                             *
*   Module:     Break.h                                                       *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       11/17/97                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This is a header file for the breakpoint module.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 11/17/97   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _BREAK_H_
#define _BREAK_H_


/******************************************************************************
*                                                                             *
*   Include Files                                                             *
*                                                                             *
******************************************************************************/

#include "types.h"                      // Include basic data types

#include "Queue.h"                      // Include queue manager header file

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

//----------------------------------------------------------------------------
// Action codes for `ControlBP()' function
//----------------------------------------------------------------------------

#define ACT_PRINT           0           // Print a specific breakpoint
#define ACT_DISABLE         1           // Disable a breakpoint
#define ACT_ENABLE          2           // Enable a breakpoint
#define ACT_CLEAR           3           // Clear a breakpoint
#define ACT_SET             4           // Physically set a breakpoint
#define ACT_UNSET           5           // Physically unset a breakpoint
#define ACT_FIND            6           // Find bp with specified address


extern TQueue qBP;                      // Breakpoint linked list


/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern int BreakClear( char *pArg );
extern int BreakDisable( char *pArg );
extern int BreakEnable( char *pArg );
extern int BreakList( char *pArg );
extern int BreakSet( char *pArg );

extern int BreakSetup();
extern int BreakCleanup();
extern int ControlBP( int num, int action );


#endif //  _BREAK_H_
