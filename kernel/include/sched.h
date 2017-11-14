/******************************************************************************
*                                                                             *
*   Module:     sched.h                                                       *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       09/07/96                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This is a header file of the scheduler module

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 09/07/96   1.00  Original                                       Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _SCHED_H_
#define _SCHED_H_

/******************************************************************************
*                                                                             *
*   Include Files                                                             *
*                                                                             *
******************************************************************************/
#include "types.h"                      // Include basic types


/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

// Process Control Block structure
typedef struct
{
    WORD    wUmask;                     // Process umask value
    BYTE   *sPwd;                       // Process working directory
    BYTE   *sRootDir;                   // Process root directory
    WORD    wUID;                       // Real user ID
    WORD    wEUID;                      // Effective user ID
    WORD    wGID;                       // Real group ID
    WORD    wEGID;                      // Effective group ID

    BYTE    bMajorTTY;                  // Major number of controlling terminal
    BYTE    bMinorTTY;                  // Minor number of controlling terminal

    WORD    wPID;                       // Process ID

    // Register section: state of the registers and memory
    DWORD   dwEIP;                      // Program counter
    DWORD   dwEFlags;                   // Eflags register
    DWORD   dwEAX;                      // eax register
    DWORD   dwEBX;                      // ebx register
    DWORD   dwECX;                      // ecx register
    DWORD   dwEDX;                      // edx register
    DWORD   dwESP;                      // esp register
    DWORD   dwEBP;                      // ebp register
    DWORD   dwEDI;                      // edi register
    DWORD   dwESI;                      // esi register

    // Selectors
    WORD    wDS;                        // Data selector
    WORD    wCS;                        // Code selector
    WORD    wSS;                        // Stack selector
    WORD    wES;                        // Extra selector
    WORD    wGS;                        // Extra selector
    WORD    wFS;                        // Extra selector

    DWORD   dwAddress;                  // Kernel address of the start of the
                                        // process address space
    DWORD   dwSize;                     // Size of the process image in bytes

    struct TPCB *next;                  // Make a linked list
} TPCB;


// Linked lists of the process queues:
//   Ready queue
//   Waiting queue
//   Zombie queue
//   Running process
//
TPCB *pReady;
TPCB *pWaiting;
TPCB *pZombie;
TPCB *pRunning;


/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern void InitSched();



// Linked list of process structures
extern TPCB *Proc;

// Defines for the wStatus field of a process structure
#define PROC_RUNNING        1           // Running process status
#define PROC_READY          2           // Process ready status
#define PROC_WAITING        3           // Waiting process status
#define PROC_NEW            4           // Process just launched
#define PROC_ZOMBIE         5           // Process has been terminated




#endif // _SCHED_H_
