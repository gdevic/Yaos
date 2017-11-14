/******************************************************************************
*                                                                             *
*   Module:     VM.c                                                          *
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

        This module contains the code for the Virtual Machine process
        management.

        Virtual machines are processes that are executed in the address
        context of the first Mb.

        Process Management

        Variable pProc[ PROCESS_ID_MAX ] holds the array to process structures
        that are dynamically allocated and freed.  pCurProc points to the
        current VM process.  pCurProcPM points to the current PM process.

        Global variable sys_pid holds the pid of the system VM process in
        which all PM processes must run.

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
*   Include Files                                                             *
******************************************************************************/

#include "bios.h"                       // Include Bios defines

#include "display.h"                    // Include display print functions

#include "inline.h"                     // Include inline macros

#include "intel.h"                      // Include intel specific defines

#include "kernel.h"                     // Include kernel header file

#include "mm.h"                         // Include memory management header

#include "tty.h"                        // Include terminal file header

#include "device.h"                     // Include device header file

#include "process.h"                    // Include its own header

#include "v86.h"                        // Include virtual 86 machine header

#include "assertk.h"                    // Include kernel assert macro

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

extern WORD VM_SS;
extern DWORD VM_SP;


TProcess *pProc[ PROCESS_ID_MAX ];      // Array of pointers to process structs

TProcess *pCurProc;                     // Pointer to a current process

TProcess *pCurProcPM;                   // Pointer to a current PM process

BOOL VM_RefreshScreen = FALSE;          // Copy the screen on VM switch

// The following array holds the process numbers that should be activated
// on a hot key with the ASCII code that indexes it

int ProcessHotKey[256] =
{
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,

    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
};


/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

char VM_Banner[160] =
"  Y A O S   V i r t u a l   M a c h i n e                                                                                                 p i d :              ";

extern int Loader_V86;

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   void Init_Process()                                                       *
*                                                                             *
*******************************************************************************
*
*   This function initializes the process module.
#
#   It creates a VM process template out of the current loader process.
*
******************************************************************************/
void Init_Process()
{
    TPage Page, PageArena;
    TProcess *pProcess;
    int i;


    dprintf("\nInit process subsystem");

    // Reset the process array to NULL

    memset( pProc, 0, sizeof(pProc) );

    //-------------------------------------------------------------------------
    // Set up the VM template pseudo-process
    //-------------------------------------------------------------------------

    // Commit a physical page for the page table of the template process

    PageArena = GetPhysicalPage();

    PageArena.fWrite = TRUE;
    PageArena.fUser  = TRUE;
    PageArena.Flags  = PROCESS_FLAG_USED;

    // Store the physical address of the page tables into the page directory
    // This makes the template visible at the process appertures

    PageDir[ PROCESS_ID_TEMPLATE ] = PageArena;

    // Store the physical address of the page tables into the page table
    // arena.  This makes the page table visible in the page tables apperture

    PageTableMap[ PROCESS_ID_TEMPLATE ] = PageArena;

    // Commit 640K of the physical memory to buffer the template and build the
    // page table of the template pseudo-process

    for( i=0; i<1024; i++ )
    {
        if( i < 0xA0000 / 4096 )
        {
            Page = GetPhysicalPage();

            Page.fWrite = TRUE;
            Page.fUser  = TRUE;
            Page.Flags  = PROCESS_FLAG_USED;

            // Set the new page into the template page table

            PageTable[ PROCESS_ID_TEMPLATE * 1024 + i ] = Page;

            FlushTLB();

            // Copy the new page into the template space

            abs_memcpy( (PROCESS_ID_TEMPLATE << 22) +  i * 4096,
                        i * 4096,
                        4096 );
        }
        else
        {
            // The rest of pages are not committed but just copied from the
            // first Mb that is identity mapped

            PageTable[ PROCESS_ID_TEMPLATE * 1024 + i ] = PageTable[ i ];

            FlushTLB();

            // Increase shared page count since these are shared

            if( PageTable[i].fPresent )   pPages[ PageTable[i].Index ]++;
        }
    }


    // Create the template pseudo-process structure

    pProcess = (TProcess *) MALLOC( sizeof(TProcess) );
    memset( pProcess, 0, sizeof(TProcess) );

    pProcess->pid      = PROCESS_ID_TEMPLATE;
    pProcess->Flags    = PROCESS_VM;
    pProcess->Reg.ss   = VM_SS;
    pProcess->Reg.esp  = VM_SP;
    pProcess->Reg.cs   = Loader_V86 >> 16;
    pProcess->Reg.eip  = Loader_V86 & 0xFFFF;
    pProcess->Reg.eflags = VM_MASK | IF_MASK | 2;
    pProcess->Image    = (PROCESS_ID_TEMPLATE << 22) - LIN_KERNEL;
    pProcess->VM_pid   = PROCESS_ID_TEMPLATE;

    // Create the terminal and copy the screen into it

    pProcess->pTTY     = CreateTTY();

    // We should be able to create the template display

    assertk( pProcess->pTTY != NULL, "Cannot create template terminal");

    abs_memcpy( LIN_KERNEL + (DWORD)pProcess->pTTY->screen, LIN_SCREEN, SCREEN_SIZE );

    // Insert the template pseudo-process into the process array and
    // set the current process to the template pseudo-process

    pCurProcPM = pCurProc = pProc[ PROCESS_ID_TEMPLATE ] = pProcess;

    dprintf(".");
}



/******************************************************************************
*                                                                             *
*   int CreateVM()                                                            *
*                                                                             *
*******************************************************************************
*
*   This powerful function creates a virtual machine from the template VM.
#   A terminal is created and attached to a process, but its capabilities
#   are not used - only the screen buffer is used with VMs.
*
#   Returns:
#       -1 on error
#       pid of the new process
#
******************************************************************************/
int CreateVM()
{
    TTY_Struct *pTTY;
    TProcess *pProcess;
    TPage Page, PageArena;
    int pid, i;
    char sPid[4];


    //------------------------------------------------------------------------
    // Find a free process ID number (slot)
    //------------------------------------------------------------------------

    for( pid=PROCESS_ID_MIN; pid<PROCESS_ID_MAX; pid++ )
        if( pProc[pid] == NULL )
            break;

    // Return if it cannot find an empty slot

    if( pid==PROCESS_ID_MAX )
        return( -1 );

    //------------------------------------------------------------------------
    // Create a terminal structure for this process
    //------------------------------------------------------------------------

    pTTY = CreateTTY();

    // Return if the terminal cannot be created

    if( pTTY==NULL )
        return( -1 );

    //------------------------------------------------------------------------
    // Now create a process structure
    //------------------------------------------------------------------------

    pProcess = (TProcess *) MALLOC( sizeof(TProcess) );

    if( pProcess != NULL )
    {
        //--------------------------------------------------------------------
        // Get the physical page to map the page arena
        //--------------------------------------------------------------------

        PageArena = GetPhysicalPage();

        if( PageArena.fPresent == TRUE )
        {
            // Set the page to map into the process apperture

            PageArena.fWrite = TRUE;
            PageArena.fUser  = TRUE;
            PageArena.Flags  = PROCESS_FLAG_USED;

            PageDir[ pid ] = PageArena;

            // Set the page to map the page table arena and invalidate TLB
            // This makes the page table visible

            PageTableMap[ pid ] = PageArena;

            FlushTLB();

            // Now we can access new memory, so let's copy the page template
            // from the template VM

            for( i=0; i<1024; i++ )
            {
                Page = PageTable[ PROCESS_ID_TEMPLATE * 1024 + i ];

                // Set all pages to read-only

                Page.fWrite = FALSE;



                PageTable[ pid * 1024 + i ] = Page;

                // Increase the shared count

                if( Page.fPresent )  pPages[ Page.Index ]++;
            }

            // Copy the process structure template from the template process

            memcpy( (void *) pProcess,
                    (void *) pProc[ PROCESS_ID_TEMPLATE ],
                    sizeof(TProcess) );

            // Set the fields that are specific to the process

            pProcess->pTTY  = pTTY;
            pProcess->pid   = pid;
            pProcess->VM_pid = pid;
            pProcess->Image = (pid << 22) - LIN_KERNEL;

            // Copy the contents of the template screen

            memcpy( pProcess->pTTY->screen,
                    pProc[ PROCESS_ID_TEMPLATE ]->pTTY->screen,
                    SCREEN_SIZE );

            // Set up the screen banner

            memcpy( pProcess->pTTY->screen, VM_Banner, 160 );

            sprintf( sPid, "%2d", pid );

            pProcess->pTTY->screen[0][75] = sPid[0] + 0x2000;
            pProcess->pTTY->screen[0][76] = sPid[1] + 0x2000;
            pProcess->pTTY->screen[0][77] = sPid[2] + 0x2000;

            // Add the new process to the process list

            pProc[ pid ] = pProcess;

            // Successfully return with the pid

            return( pid );
        }

        FREE( pProcess );
    }

    // Destroy the terminal structure

    DestroyTTY( pTTY );

    return( -1 );
}


/******************************************************************************
*                                                                             *
*   int DestroyProcess( pid_t pid )                                           *
*                                                                             *
*******************************************************************************
*
*   This function destroys a process with the given pid.  The process may be
#   VM process or a PM process.
*
#   If the process id equals the current VM process, process VM template will
#   be set.
#   If the process id equals the current PM process, pCurProcPM will be set
#   to NULL.
#
*   Where:
*       pid of a process to be destroyed (PROCESS_ID_MIN <= pid < PROCESS_ID_MAX)
*
*   Returns:
*       0 if successful
#       -1 if failed
#
******************************************************************************/
int DestroyProcess( pid_t pid )
{
    static const TPage PageClean = { 0 };
    TPage Page;
    int i;

    // Check if a pid to be destroyed is valid

    if( (pid < PROCESS_ID_MIN) || (pid >= PROCESS_ID_MAX) )
        return( -1 );

    // Check if a pid contains a valid process

    if( pProc[pid]->pid != pid )
        return( -1 );

    // If the process to be destroyed is the current one and VM, set the
    // template process as the current VM process

    if( pid == pCurProc->pid )
        SetCurrentVM( PROCESS_ID_TEMPLATE, NULL );
    else
    if( pid == pCurProcPM->pid )
        pCurProcPM = NULL;

    // We have to free all physical pages that the process had acquired

    for( i=0; i<1024; i++ )
    {
        Page = PageTable[ pid * 1024 + i ];

        // If a page is present, free it

        if( Page.fPresent )
            ReleasePhysicalPage( Page );
    }

    // Free the page containing the page table

    Page = PageTableMap[ pid ];

    ReleasePhysicalPage( Page );

    // Clean the page directory entry and page table entry (they point
    // to the same page containing the page table)

    PageDir[ pid ]      = PageClean;
    PageTableMap[ pid ] = PageClean;

    // Destroy the terminal structure

    DestroyTTY( pProc[pid]->pTTY );

    // Free the LDT table memory

    FreeLDT( pProc[pid]->LDT );

    // Free the memory occupied by the process structure

    FREE( pProc[pid]->pName );
    FREE( pProc[pid] );

    // Free the process slot

    pProc[ pid ] = NULL;

    return( 0 );
}


/******************************************************************************
*                                                                             *
*   int SetCurrentVM( int pid, TIntStack *pStack )                            *
*                                                                             *
*******************************************************************************
*
*   This function sets the current VM to a process pid.  pCurProc should
#   point to the current (valid) process, or be NULL.  That process will be
#   flushed.
#
#   If the new process is a system VM process, the PM process that is
#   currently active will be set up.
*
*   Where:
*       pid - a process id of the process that is to be set
#       pStack - pointer to a stack structure of the interrupted process
#           if NULL, stack will not be stored
*
*   Returns:
*       pid if successful
#       other pid if the new pid can not be set
*
******************************************************************************/
int SetCurrentVM( int pid, TIntStack *pStack )
{
    TPage Page;
    int i, text_page, text_xy, text_mode;
    int old_pid = 0;
    char sPid[4];


    // Check if a new pid is valid

    if( pid == PROCESS_ID_TEMPLATE || (pid >= PROCESS_ID_MIN && pid < PROCESS_ID_MAX) )
    {
        // Check if a new pid has a valid process

        if( pProc[pid]->pid == pid )
        {
            if( pCurProc != NULL )
            {
                old_pid = pCurProc->pid;

                // We need to flush the old process memory from the running VM
                // memory to the process arena

                // Clean up all the pages that are different due to the current
                // process execution

                for( i=0; i<0xA0000 / 4096; i++ )
                {
                    if( PageCompare( i, pCurProc->Image ) != 0 )
                    {
                        // Value has changed.  Check the page entry if it needs
                        // to be copied or a new page allocated

                        Page = PageTable[ old_pid * 1024 + i ];

                        if( Page.fWrite==FALSE )
                        {
                            // If a page is not writable, allocate new page

                            Page = GetPhysicalPage();
                            Page.fWrite = TRUE;

                            PageTable[ old_pid * 1024 + i ] = Page;

                            FlushTLB();
                        }
                        else
                        {
                            // If a page is already there, just copy
                        }

                        // Copy the changed content to a process page

                        abs_memcpy( LIN_KERNEL + pCurProc->Image + i * 4096,
                                    i * 4096,
                                    4096 );
                    }
                }

                // Return if the same process is being flushed

                if( old_pid == pid )
                    return( pid );

                // Store the old screen content

                if( pStack != NULL )
                {
                    memcpy( &pCurProc->Reg, pStack, sizeof(TIntStack) );
                    abs_memcpy( LIN_KERNEL + (DWORD)pCurProc->pTTY->screen, LIN_SCREEN, SCREEN_SIZE );
                }
            }

            // Store the old stack structure into the old process and restore
            // new stack structure from the new process.

            if( pStack != NULL )
                memcpy( pStack, &pProc[pid]->Reg, sizeof(TIntStack) );

            // Set the new current process

            pCurProc = pProc[ pid ];

            // Copy the new process VM arena to the running VM arena

            abs_memcpy( 0, LIN_KERNEL + pCurProc->Image, 1024 * 640 );

            // Print the VM pid

            sprintf( sPid, "%2d", pid );

            pCurProc->pTTY->screen[0][75] = sPid[0] + 0x2000;
            pCurProc->pTTY->screen[0][76] = sPid[1] + 0x2000;
            pCurProc->pTTY->screen[0][77] = sPid[2] + 0x2000;

            // Restore new screen

            if( VM_RefreshScreen )
                abs_memcpy( LIN_SCREEN, LIN_KERNEL + (DWORD)pCurProc->pTTY->screen, SCREEN_SIZE );

            // We will now restore some video parameters.  Since we really do
            // only partial virtualization, we can chose the degree of it.
            // Right now, let it at least look good, so virtualize the cursor

            // Set the cursor position

            text_page = *(BYTE *)( pCurProc->Image + BIOS_VIDEO_PAGE );
            text_xy   = *(WORD *)( pCurProc->Image + BIOS_VIDEO_CURSORS + text_page*2 );

            Reg.eax = 0x0200;
            Reg.ebx = text_page << 8;
            Reg.edx = text_xy;

            V86_Int( 0x10, &Seg, &Reg, NULL );

            // Set the cursor shape

            text_mode = *(BYTE *)( pCurProc->Image + BIOS_VIDEO_MODE );
            text_xy   = *(WORD *)( pCurProc->Image + BIOS_VIDEO_CURSORTYPE );

            Reg.eax = 0x0100 + text_mode;
            Reg.ecx = text_xy;

            V86_Int( 0x10, &Seg, &Reg, NULL );

            // Set the current terminal to point to this one

            pCurTTY = pCurProc->pTTY;

            // Set the process LDT.  This will affect only PM processes that are
            // running under the system VM process or DPMI processes

            SetLDT( pCurProcPM->LDT );
        }
    }

    // Return the current process id

    return( pCurProc->pid );
}

