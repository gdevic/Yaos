/******************************************************************************
*                                                                             *
*   Module:     PF.c                                                          *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       8/22/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

    This module contains the code for the page fault exception handling.
    It implements a copy-on-write demand paging.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 8/22/97    1.00  Original                                       Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "display.h"                    // print function

#include "inline.h"                     // Inline functions

#include "intel.h"                      // Include intel specific defines

#include "kernel.h"                     // Include kernel header file

#include "mm.h"                         // Include memory management header

#include "tty.h"                        // Include terminal file header

#include "device.h"                     // Include device header file

#include "process.h"                    // Include the process header

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

DWORD FaultyAddress;

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

//-----------------------------------------------------------------------------
//
// The following are the bits of error code for page fault:
//  Bit 0:  1 - access rights violation or use of a reserved bit
//          0 - page was not present
//
//  Bit 1:  1 - memory access was a write
//          0 - memory access was a read
//
//  Bit 2:  1 - processor was executing in user mode
//          0 - processor was executing in supervisor mode

#define FAULT_PAGE_PRESENT  1
#define FAULT_ON_WRITE      2
#define FAULT_IN_USER_MODE  4


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   void Page_Fault_Handler( TIntStack Stack )                                *
*                                                                             *
*******************************************************************************
*
*   This is handler for page faults.  When a process is interrupted
#   due to the write violation, this function implements copy-on-write
#   demand paging.
#
*
******************************************************************************/
void Page_Fault_Handler( TIntStack Stack )
{
    TPage Page, Shared, TmpPage;
    int index, dir, TmpIndex;


    FaultyAddress = GetFaultyAddress();

// dprintf("\nPF: %08X EC: %X ", FaultyAddress, Stack.ErrorCode & 0x7 );

    //------------------------------------------------------------------------
    // If a fault is due to a non-present page, get a page and map it
    //------------------------------------------------------------------------

    if( (Stack.ErrorCode & FAULT_PAGE_PRESENT) == 0 )
    {
        // Find a new physical page to map

        Page = GetPhysicalPage();

        // For now I assume there are enough physical pages in the page pool
        // but here needs to come some code for disk swapping etc.

        if( Page.fPresent == FALSE ) KERNEL_DIE;

        {};


        Page.fWrite = TRUE;
        Page.fUser  = TRUE;
        Page.Flags  = 1;

        // Map the physical page into the page arena where it faulted

        index = (FaultyAddress >> 12) & 0x3FF;
        dir   = (FaultyAddress >> 22) & 0x3FF;

        PageTable[ dir * 1024 + index ] = Page;

        // Flush the TLB

        FlushTLB();

// abs_memsetw( FaultyAddress & ~0xFFF, 0xCCCC, 4096/2 );

        // Return to the interrupted process

        return;
    }

    //------------------------------------------------------------------------
    // If the page fault is due to a write attempt on a read-only page,
    // do a normal copy-on-write demand paging stuff
    //------------------------------------------------------------------------

    if( (Stack.ErrorCode & FAULT_ON_WRITE) )
    {
        // Get the index of a page table that faulted

        index = (FaultyAddress >> 12) & 0x3FF;
        dir   = (FaultyAddress >> 22) & 0x3FF;

        Shared = PageTable[ dir * 1024 + index ];

        // If the shared page count for this page is 1, set it writable.
        // That means only one process owns it so we allow it to write
        // to that page.

//dprintf("  sh: %d", pPages[ Shared.Index ] );

        if( pPages[ Shared.Index ] == 1 )
        {
            Shared.fWrite = TRUE;

            PageTable[ dir * 1024 + index ] = Shared;

            // Flush the TLB

            FlushTLB();

            return;
        }

        // There was more than one process sharing the page read-only.  Since
        // this one wants to write to it, we need to give it a separate copy

        Page = GetPhysicalPage();

        // For now I assume there are enough physical pages in the page pool
        // but here needs to come some code for disk swapping etc.

        if( Page.fPresent == FALSE ) KERNEL_DIE;

        {};


        Page.fWrite = TRUE;
        Page.fUser  = TRUE;
        Page.Flags  = 2;

        // Decrement the shared count of the faulting page since we take
        // away one of the sharing processes

        pPages[ Shared.Index ]--;

        // We need to copy the content of the faulty page into the new page,
        // but the new page must be mapped somewhere in order to be `visible'
        // for a copy.  We will map it temporarily at the next successive page
        // or at the previous page if the page is the last one in a 4MB
        // address space

        TmpIndex = ((FaultyAddress >> 12) & 0x3FF) + 1;
        if( TmpIndex == 0x400 ) TmpIndex = 0x3FE;

        TmpPage = PageTable[ dir * 1024 + TmpIndex ];
        PageTable[ dir * 1024 + TmpIndex ] = Page;

        FlushTLB();

        // Copy the old page to a new page

        abs_memcpy( (dir << 22) + TmpIndex * 4096, FaultyAddress & ~0xFFF, 4096 );

        // Remap the new page at its proper location and restore what we
        // have saved

        PageTable[ dir * 1024 + index ] = Page;
        PageTable[ dir * 1024 + TmpIndex ] = TmpPage;

        FlushTLB();

        // Return to the interrupted process

        return;
    }


    // This is something unexpected...

    dprintf("\nUnexpected page fault type of %d at %08Xh",
        Stack.ErrorCode & 0x7, FaultyAddress );

    KERNEL_DIE;
}

