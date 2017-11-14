/******************************************************************************
*                                                                             *
*   Module:     Debug.h                                                       *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       8/28/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This is the main header file for the kernel-level debugger.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 8/28/97    1.00  Original                                       Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _DEBUG_H_
#define _DEBUG_H_


/******************************************************************************
*                                                                             *
*   Include Files                                                             *
*                                                                             *
******************************************************************************/

#include "display.h"                    // Include display functions

#include "kernel.h"                     // Include kernel data structures

#include "types.h"                      // Include basic data types

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

//-----------------------------------------------------------------------------
// Set debugger default screen attributes
//-----------------------------------------------------------------------------
#define ATTR_REGS       GREY COLOR_ON BLUE
#define ATTR_REGSHI     WHITE COLOR_ON BLUE
#define ATTR_DIS        BLUE COLOR_ON CYAN
#define ATTR_DISHI      WHITE COLOR_ON CYAN
#define ATTR_ECHO       RED COLOR_ON BLACK
#define ATTR_CMDLINE    WHITE COLOR_ON BLUE


//-----------------------------------------------------------------------------
// Debugger state structure
//-----------------------------------------------------------------------------

typedef struct                          // Debugger state structure
{
    char sDebugeeScreen[ SCREEN_SIZE ]; // User text screen buffer
    int  nNumLines;                     // Number of lines on the screen
    int  nDumpLines;                    // Number of lines to dump
    int  nListLines;                    // Number of lines to list
    BOOL fEcho;                         // Echo command line to the screen
    BOOL fRegs;                         // Display registers at the top
    BOOL fCode32;                       // Disasm. code is 32-bit (1) or 16-bit
    BOOL fData32;                       // Disasm. data is 32-bit (1) or 16-bit

} TDebug;

extern TDebug Deb;                      // Actual TDebug global variable

/******************************************************************************
*                                                                             *
*   Register file as it comes from the int 3 stack                            *
*                                                                             *
******************************************************************************/

extern TIntStack * pDebFrame;           // Original interrupted stack frame
extern TStack * pDebStack;              // Pointer to interrupted program's
extern TCode  * pDebCode;               //  stack, code, register and segment
extern TReg   * pDebReg;                //  descriptor structures
extern TSeg   * pDebSeg;


/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/


#endif // _DEBUG_H_
