/******************************************************************************
*                                                                             *
*   Module:     Process.h                                                     *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       9/23/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This is a header file for the process module.  The main process
        structure, `TProcess' is defined here.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 9/23/97    Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _PROCESS_H_
#define _PROCESS_H_


/******************************************************************************
*                                                                             *
*   Include Files                                                             *
*                                                                             *
******************************************************************************/

#include <stdio.h>                      // Number of files opened

#include <sys/types.h>                  // Include standard clib types

#include "types.h"                      // Include basic data types

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

// First process is a pseudo-process that is a template for each VM process.
// The maximum pid is 255 since we use direct indexing into GDT to get a
// process' LDT. (That's the reason we start with pid of 16 since we need first
// few entries of GDT for the kernel - keep simplicity).

#define PROCESS_ID_TEMPLATE     15      // Template VM pseudo-process ID

#define PROCESS_ID_MIN          16      // Minimum process ID
#define PROCESS_ID_MAX          255     // Maximum process ID


//-----------------------------------------------------------------------------
// Process flags
//-----------------------------------------------------------------------------

#define PROCESS_VM              0x0001  // Process is a DOS VM process
#define PROCESS_SYSTEM_VM       0x0002  // Process is a DOS VM and system based
#define PROCESS_PM              0x0004  // Process is a PM process running under a system VM
                                        // VM_pid contains the system VM pid

#define PROCESS_BLOCKED         0x0070  // PM process is waiting (blocked) mask
#define  PROCESS_WAITING_PID    0x0010  // .. on a wait() call
#define  PROCESS_WAITING_DEV    0x0020  // .. on a device read info
#define PROCESS_ZOMBIE          0x0040  // is actually waiting


//-----------------------------------------------------------------------------
//
//  Process Descriptor
//
//-----------------------------------------------------------------------------

typedef struct
{
    TIntStack   Reg;                    // CPU Registers: * must be the first *

    int         pid;                    // Process ID number
    int         gid;                    // Process group ID number
    int         ppid;                   // Process parent ID number
    WORD        Flags;                  // Process flaqs, status and type
    WORD        LDT;                    // Process LDT mapped into gdt by pid
    DWORD       slices;                 // Count of number ot time-slices given
    DWORD       Image;                  // Address of the process image
    TTY_Struct *pTTY;                   // Pointer to a terminal structure
    int         VM_pid;                 // pid of the context VM
    int         iFile[ FOPEN_MAX ];     // Index into process file table
    char       *pName;                  // Pointer to process name

    int         nWait;                  // Wait parameter: Device number/pid
    TDevRq      Rq;                     // Device request buffer when blocked
    int         status;                 // Process exit status (for zombies)

} TProcess;


//-----------------------------------------------------------------------------
// Array of pointers to process structures
//-----------------------------------------------------------------------------

extern TProcess *pProc[ PROCESS_ID_MAX ];

extern TProcess *pCurProc;              // Pointer to a current process

extern TProcess *pCurProcPM;            // Pointer to a current PM process


//-----------------------------------------------------------------------------
// Array of ASCII hot keys holding respective pid's
//-----------------------------------------------------------------------------

extern int ProcessHotKey[256];

//-----------------------------------------------------------------------------
// Process next usabe excution group id number
//-----------------------------------------------------------------------------

extern int next_gid;

//-----------------------------------------------------------------------------
// System VM pid
//-----------------------------------------------------------------------------

extern int sys_pid;

/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern void Init_Process();

extern int CreateVM();
extern int SetCurrentVM( int pid, TIntStack *pStack );

extern int DestroyProcess( pid_t pid );


#endif //  _PROCESS_H_
