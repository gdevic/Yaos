/******************************************************************************
*                                                                             *
*   Module:     kSysCall.h                                                    *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       10/12/97                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This is a header file for the kernel system call module.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 10/12/97   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _KSYSCALL_H_
#define _KSYSCALL_H_


/******************************************************************************
*                                                                             *
*   Include Files                                                             *
*                                                                             *
******************************************************************************/

#include "types.h"                      // Include basic data types


/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

//.-
//----------------------------------------------------------------------------
// Aliases to access the client process registers for argument passing
//----------------------------------------------------------------------------

#define SYS_NUM     pSysRegs->eax       // System call number
#define SYS_ARG1    pSysRegs->ebx       //  First argument
#define SYS_ARG2    pSysRegs->ecx       //  Second argument
#define SYS_ARG3    pSysRegs->edx       //  Third argument

// System call functions that return a value to a process that called them
// use register `eax' of a client

#define SYS_RET     pSysRegs->eax       // Returning value or error code

//----------------------------------------------------------------------------
// System call functions return the following constants
//----------------------------------------------------------------------------

#define SYS_END             0           // System call accomplished
#define SYS_BLOCKED         1           // Process was blocked on a call and
                                        // rescheduling needs to be done

// Use this macro to return from a system call by setting the returning
// argument of a calling process in a single line and ending a non-blocking
// system call                                        ---------------------

#define sys_end(ret)     { pSysRegs->eax = ret; return(SYS_END); }

// If no arguments should be passed back to the calling process, use `return'
// with argument SYS_END (non-blocked) or SYS_BLOCKED (blocked).

//----------------------------------------------------------------------------
// Global pointer to a current client PM process register structure (stack)
//----------------------------------------------------------------------------

extern TPMStack *pSysRegs;
//.-

/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern int Sys_Null();
extern int Sys_Fork();
extern int Sys_Exec();
extern int Sys_Wait();
extern int Sys_Exit();
extern int Sys_Kill();

extern int Sys_Creat();
extern int Sys_Open();
extern int Sys_Close();
extern int Sys_Read();
extern int Sys_Write();
extern int Sys_LSeek();
extern int Sys_Ioctl();

extern int Sys_Stat();
extern int Sys_Fstat();

extern int Sys_Opendir();
extern int Sys_Readdir();
extern int Sys_Closedir();

//.-
//----------------------------------------------------------------------------
// Helper functions for system call routines (defined in SysCall.c)
//----------------------------------------------------------------------------

extern char *CheckProcessBuf( DWORD dwSrc, int nSize, int nMaxSize );
extern char *CheckProcessString( char *pStr, int nMaxSize );
extern int   CheckProcessAddress( DWORD dwAddress );
extern int   IsBlocked( BYTE bMajor, BYTE bMinor );
extern void  DevUnblock( int pid, int n );

//-.

#endif //  _KSYSCALL_H_
