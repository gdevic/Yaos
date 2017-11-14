/******************************************************************************
*                                                                             *
*   Module:     Exec.c                                                        *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       10/8/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This module contains the code for the process creation and preparing
        to execute it.

        System calls fork() and exec() are also here.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 10/8/97    Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include <sys/syscall.h>                // Include system call header file

#include <errno.h>                      // Include error header file

#include <stdio.h>                      // Include standard io header file

#include "dosx.h"                       // Include dos extender functions

#include "inline.h"                     // Include inline macros

#include "intel.h"                      // Include intel specific defines

#include "kernel.h"                     // Include kernel header file

#include "ksyscall.h"                   // Include kernel system call header

#include "mm.h"                         // Include memory management header

#include "tty.h"                        // Include terminal file header

#include "device.h"                     // Include device header file

#include "process.h"                    // Include its own header

#include "break.h"                      // Include debugger breakpoint header

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

#define ARG_MAX         3072            // Maximum size of arguments


#define EXP_SIGNATURE   0x504D          // EXP file signature value

typedef struct
{
    WORD    Signature;                  // Signature - "MP"
    WORD    SizeRem;                    // Image size mod 512
    WORD    Size;                       // Image size in 512b pages
    WORD    Reloc;                      // Number of relocation entries
    WORD    HeaderSize;                 // Header size in pages (16b)
    WORD    MinPages;                   // Minimum extra pages (4K)
    WORD    MaxPages;                   // Maximum extra pages (4K)
    DWORD   ESP;                        // Initial ESP value
    WORD    Checksum;                   // Word checksum
    DWORD   EIP;                        // Initial EIP value
    WORD    RelocOff;                   // Offset of the first reloc item
    WORD    Overlay;                    // Overlay number
    WORD    Reserved;                   // Reserved word
    WORD    Reserved2;                  // Reserved word

} TExpHeader;                           // Phar Lap Old EXP-file header


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   int Sys_Fork()                                                            *
*                                                                             *
*******************************************************************************
*
*   This function performs the fork() system call.  pCurProcPM must point to
#   a process that forks.
*
*   Where:
#       no args
*
*   Returns:
#       Error code if it cannot fork
#       Parent process is returned 0
*
#   Note:
#       Child process is set up to return child pid on execution.
*
******************************************************************************/
int Sys_Fork()
{
    TProcess *pProcess;
    TPage PageArena, Page;
    int pid, i;


    //-----------------------------------------------------------------
    // Get all the indices and all that can fail right
    // here, at the beginning, so we are sure to complete call later on
    //-----------------------------------------------------------------

    for( pid=PROCESS_ID_MIN; pid<PROCESS_ID_MAX; pid++ )
        if( pProc[pid] == NULL )
            break;

    // Return if it cannot find a new process slot

    if( pid == PROCESS_ID_MAX ) sys_end( ENOMEM );

    // Allocate a process structure
    // ------------------------------

    pProcess = (TProcess *) MALLOC( sizeof(TProcess) );

    if( pProcess == NULL ) sys_end( ENOMEM );

    // Get the physical page to map the page arena
    // -------------------------------------------

    PageArena = GetPhysicalPage();

    // Return if it cannot alocate memory after freeing the process struct

    if( PageArena.fPresent == FALSE )
    {
        FREE( pProcess );
        sys_end( ENOMEM );
    }

    // Here we need to do some little hack to support easier debug.
    // We will clean up all software breakpoints and at the end of this
    // call we will place them back.  If we dont do that, all the
    // int 3 codes will be duplicated and we will lose track of them

    BreakCleanup();

    //=================================================================
    // Can't fail now...
    //-----------------------------------------------------------------
    // Set up the page tables
    //-----------------------------------------------------------------

    PageArena.fWrite = TRUE;
    PageArena.fUser  = TRUE;
    PageArena.Flags  = PROCESS_FLAG_USED;

    PageDir[ pid ] = PageArena;

    // Set the page to map the page table arena and invalidate TLB
    // This makes the page table visible

    PageTableMap[ pid ] = PageArena;

    FlushTLB();

    // Now we can access new memory, so let's set the new page table with
    // non-writable copy of parent pages.  We will set our pages as read
    // only and the parent pages as read only, so they will all be copy
    // on write.

    for( i=0; i<1024; i++ )
    {
        Page = PageTable[ pCurProcPM->pid * 1024 + i ];
        Page.fWrite = FALSE;
        PageTable[ pCurProcPM->pid * 1024 + i ] = Page;
        PageTable[ pid * 1024             + i ] = Page;

        // Increase the shared page count

        if( Page.fPresent == TRUE )   pPages[ Page.Index ]++;
    }

    FlushTLB();

    //-----------------------------------------------------------------
    // Set up the process structure - copy from the parent process block
    //-----------------------------------------------------------------

    memcpy( pProcess, pCurProcPM, sizeof(TProcess) );

    // Set the fields that are specific to the child process

    pProcess->pid    = pid;
    pProcess->ppid   = pCurProcPM->pid;
    pProcess->Image  = (pid << 22) - LIN_KERNEL;

    // Get the LDT for the task

    pProcess->LDT = AllocLDT( pid );

    // Store the parent registers off the stack into the child registers record

    memcpy( &pProcess->Reg, pSysRegs, sizeof(TPMStack) );

    // Set up registers so that the child will return with its pid on execution

    pProcess->Reg.eax = pid;

    // Add the new process to the process list

    pProc[ pid ] = pProcess;

    // End of debuger hack... put back all the breakpoints (that may also
    // trigger some page creation since pages will not be the same any more

    BreakSetup();

    // Return 0 to the parent process

    sys_end( 0 );
}


/******************************************************************************
*                                                                             *
*   int Sys_Exec()                                                            *
*                                                                             *
*******************************************************************************
*
*   This function performs exec system call.  It executes a specified file
#   and destroys the process that started it.
*
*   Where:
*       SYS_ARG1 is the address of the file pathname
#       SYS_ARG2 is the address of the array of pointers to argument list
#       SYS_ARG3 is the address of the array of pointers to enviroment strings
*
*   Returns:
*       New process is created
#       Error code if cannot exec
*
******************************************************************************/
int Sys_Exec()
{
#define MAX_ARG  16
#define MAX_ENV  32

    char *path, **argp, **envp;
    char *Arg[ MAX_ARG ];
    char *Env[ MAX_ENV ];
    TPage Page, PageArena;
    int i, new_pid, old_pid;


    // First, check all pointers and assign the kernel working ones

    if( (SYS_ARG1==NULL) || (SYS_ARG2==NULL) || (SYS_ARG3==NULL) )  sys_end( EFAULT );

    path = (char *)  CheckProcessAddress( SYS_ARG1 );
    argp = (char **) CheckProcessAddress( SYS_ARG2 );
    envp = (char **) CheckProcessAddress( SYS_ARG3 );

    if( (path == -1) || (argp == -1) || (envp == -1) )  sys_end( EFAULT );

    // We will make a temporary arrays of pointers for arguments and environs
    // during the address translation and pointer checking.

    // Traverse the argument list and copy modified pointers

    for( i=0; i<MAX_ARG-1; i++ )
    {
        Arg[i] = (char *) CheckProcessAddress( (DWORD) *argp++ );
        if( Arg[i] == -1 )  sys_end( EFAULT );
        if( Arg[i] == NULL )  break;
    }

    Arg[i] = NULL;

    // Traverse the environment list and copy modified pointers

    for( i=0; i<MAX_ENV-1; i++ )
    {
        Env[i] = (char *)CheckProcessAddress( (DWORD) *envp++ );
        if( Env[i] == -1 )  sys_end( EFAULT );
        if( Env[i] == NULL )  break;
    }

    Env[i] = NULL;

    //------------------------------------------------------------------------
    // Call the execve function - that will create a new process with a
    // distinctive pid
    //------------------------------------------------------------------------

    new_pid = do_execve( path, Arg, Env );

    // If the function succedded, we need to destroy old process

    if( new_pid > 0 )
    {
        // Store the old pid - we will use it to relocate new process

        old_pid = pCurProcPM->pid;

        // Copy all file descriptors - they are inherited by the child process

        memcpy( pProc[new_pid]->iFile, pCurProcPM->iFile, FOPEN_MAX * sizeof(int) );

        // Copy process group id number and pid that are inherited

        pProc[new_pid]->pid   = old_pid;
        pProc[new_pid]->gid   = pCurProcPM->gid;
        pProc[new_pid]->ppid  = pCurProcPM->ppid;
        pProc[new_pid]->Image = pCurProcPM->Image;

        // Free the old LDT and form a new LDT for the new location

        FreeLDT(pProc[new_pid]->LDT);

        // Destroy the old process

        DestroyProcess( old_pid );

        // Copy the new process to the old slot and destroy new info

        pCurProcPM = pProc[old_pid] = pProc[new_pid];
        pProc[new_pid] = NULL;

        pCurProcPM->LDT = AllocLDT(old_pid);

        // Exchange page directory entries to relocate process image

        Page = PageDir[ old_pid ];
        PageDir[ old_pid ] = PageDir[ new_pid ];
        PageDir[ new_pid ] = Page;

        // Exchange page tables to make process visible

        PageArena = PageTableMap[ old_pid ];
        PageTableMap[ old_pid ] = PageTableMap[ new_pid ];
        PageTableMap[ new_pid ] = PageArena;

        FlushTLB();

        // Copy the new process registers to the stack so we return into it

        memcpy( pSysRegs, &pCurProcPM->Reg, sizeof(TPMStack) );

        return( SYS_END );
    }

    // Return the error from the execve function

    sys_end( new_pid );
}


/******************************************************************************
*                                                                             *
*   int do_execve( const char *path, char *const argp[], char *const *envp )  *
*                                                                             *
*******************************************************************************
*
*   This function creates a new PM process.  It loads the process image and
#   prepares it to run, setting the arguments and environment.  It does not
#   make a process current - it only sets the ready state.
*
*   Where:
*       path is the path/name of the file to be executed
#       argp[] is an array of pointers to arguments
#       envp is the pointer to a string of environment strings
*
*   Returns:
*       pid of a new process (>0)
#       error code (<0)
*
#   It is allowed to have argp or envp NULL, but the pointers validity is
#   not checked.
#
******************************************************************************/
int do_execve( const char *path, char *const argp[], char *const *envp )
{
    TExpHeader Head;
    TProcess *pProcess;
    TPage Page, PageArena;
    DWORD *pArgs;
    char *pStrings, *pEnviron, *pName;
    int pid, handle, read, image_size, i;
    int num_args, total_args, envc;


    //-----------------------------------------------------------------
    // Get all the necessary memory and everything that can fail right
    // here at the beginning so we are sure to complete call later on
    //-----------------------------------------------------------------

    for( pid=PROCESS_ID_MIN; pid<PROCESS_ID_MAX; pid++ )
        if( pProc[pid] == NULL )
            break;

    // Return if it cannot find a new process slot

    if( pid == PROCESS_ID_MAX )
        return( ENOMEM );

    // Open a file that is to be loaded
    // --------------------------------

    if( (handle = DOS_open( path, 0 )) < 0 )
        return( ENOENT );

    // Check that the file is in the executable format - load a header

    read = DOS_read( handle, sizeof(TExpHeader), (char *) &Head );

    if( Head.Signature != EXP_SIGNATURE )
    {
        // Close a file handle and return with the error

        DOS_close( handle );
        return( ENOEXEC );
    }

    // Allocate a process structure
    // ------------------------------

    pProcess = (TProcess *) MALLOC( sizeof(TProcess) );

    if( pProcess != NULL )
    {
        // Allocate memory for the process name

        pName = (char *) MALLOC(strlen(path)+1);

        if( pName != NULL )
        {
            strcpy(pName, path);

            // Get the physical page to map the page arena
            // -------------------------------------------

            PageArena = GetPhysicalPage();

            // Return if it cannot alocate memory after freeing the process struct
            // and closing the file

            if( PageArena.fPresent == FALSE )
            {
                DOS_close( handle );
                FREE( pProcess );
                return( ENOMEM );
            }
        }
        else
        {
            FREE(pProcess);
            DOS_close( handle );
            return( ENOMEM );
        }
    }
    else
    {
        // Couldn`t allocate process - Close a file handle and return

        DOS_close( handle );
        return( ENOMEM );
    }

    //-----------------------------------------------------------------
    // Set up the page tables
    //-----------------------------------------------------------------

    PageArena.fWrite = TRUE;
    PageArena.fUser  = TRUE;
    PageArena.Flags  = PROCESS_FLAG_USED;

    PageDir[ pid ] = PageArena;

    // Set the page to map the page table arena and invalidate TLB
    // This makes the page table visible

    PageTableMap[ pid ] = PageArena;

    FlushTLB();

    // Now we can access new memory, so let's set the new page table with
    // the non-present pages

    Page.fPresent = FALSE;

    for( i=0; i<1024; i++ )
    {
        PageTable[ pid * 1024 + i ] = Page;
    }

    FlushTLB();

    //-----------------------------------------------------------------
    // Set up the process structure
    //-----------------------------------------------------------------

    memset( pProcess, 0, sizeof(TProcess) );

    // Set the fields that are specific to the process

    pProcess->pid    = pid;
    pProcess->VM_pid = sys_pid;
    pProcess->Image  = (pid << 22) - LIN_KERNEL;
    pProcess->pName  = pName;

    // Add the new process to the process list

    pProc[ pid ] = pProcess;

    //-----------------------------------------------------------------
    // LOAD A PROCESS IMAGE
    //-----------------------------------------------------------------

    image_size = (Head.Size - 1) * 512 + Head.SizeRem - sizeof(TExpHeader);

    // Load the image of the process into the process image arena

    read = DOS_read( handle, image_size, (char *) pProcess->Image );

    // Close a file

    DOS_close( handle );

    //-----------------------------------------------------------------
    // PREPARE A PROCESS TO RUN
    //-----------------------------------------------------------------

    // Get the LDT for the task

    pProcess->LDT = AllocLDT( pid );

    pProcess->Reg.pm_es = SEL_APP_DATA;
    pProcess->Reg.pm_ds = SEL_APP_DATA;
    pProcess->Reg.pm_fs = SEL_APP_DATA;
    pProcess->Reg.pm_gs = SEL_APP_DATA;
    pProcess->Reg.eip   = Head.EIP;
    pProcess->Reg.cs    = SEL_APP_CODE;
    pProcess->Reg.eflags= IF_MASK | 2;
    pProcess->Reg.ss    = SEL_APP_DATA;

    // Process is a PM process

    pProcess->Flags = PROCESS_PM;

    // Assign the next available process group id number

    pProcess->gid = next_gid;

    //-----------------------------------------------------------------
    // Build the process' argument list and environment block
    //-----------------------------------------------------------------
    /*
       (The following is also shown in Startup.asm)

       The loader sets up the stack as follows:

       ========================== 4Mb <- top of the process memory
       .
       .  This block is reserved for argument and environment strings
       .  and is defined as ARG_MAX.
       .
       .                                     (open top end)
       (environment variable n)`0     <- ASCIIZ strings (env. variables)
       (environment variable 2)`0
       (environment variable 1)`0
       (argument string 0)`0          <- ASCIIZ strings (arguments)
       (argument string 1)`0
       (argument string 2)`0
       (argument string m)`0                                     ========
       -------------------------- 4Mb - ARG_MAX virtual limit <- Strings !
       NULL                                                      ========
       p->(argument string m)         <- Array of pointers to arguments
       p->(argument string 2)
       p->(argument string 1)
   +-> p->(argument string 0)
   |   envc                           <- Number of environment strings
   |   p->(environment variable 1)    <- Pointer to a first env. string
   +-- argp    <-- ESP                <- Pointer to the arguments array
                  - | -
                  - V -  Stack grows down

    */

    // Copy the environment strings and the argument strings.
    // Strings will grow up ('stack up') and pArgs will grow down ('stack down')

    pStrings = (char *)pProcess->Image + 4 * 1024 * 1024 - ARG_MAX;
    pArgs = (DWORD *)pStrings - 1;

    // Stack down the trailing NULL on the argument list

    *pArgs-- = NULL;

    // Find the end of argument strings so we can stack them up backward

    total_args = 0;

    if( argp != NULL )
    {
        for( ; argp[total_args] != NULL; total_args++ );

        // Stack up arguments strings

        for( num_args=total_args-1; num_args >= 0; num_args-- )
        {
            // Copy the argument string

            strcpy( pStrings, argp[num_args] );

            // Stack down the pointer to it on the stack that we are building

            *pArgs-- = (DWORD)pStrings - pProcess->Image;

            // Advance strings up

            pStrings += strlen(argp[num_args]) + 1;
        }
    }

    // Loop for each environment string, stack them up and count them

    envc = 0;
    pEnviron = pStrings;

    if( envp != NULL )
    {
        while( envp[envc] != NULL )
        {
            // Copy the environment string

            strcpy( pStrings, envp[envc] );

            // Advance strings up

            pStrings += strlen(envp[envc]) + 1;

            // Increase the count of strings

            envc++;
        }
    }

    // Stack down the number of env. strings

    *pArgs-- = envc;

    // Stack down the address of the first environment string

    *pArgs-- = (DWORD) pEnviron - pProcess->Image;

    // Stack down a pointer to the argument array

    *pArgs-- = ((DWORD) pArgs) + (4 * 3) - pProcess->Image;

    // Stack down the argument count

    *pArgs = total_args;

    // Set the stack pointer to the bottom of our structure

    pProcess->Reg.esp = ((DWORD) pArgs) - pProcess->Image;


    // Return a new process number

    return( pid );
}

