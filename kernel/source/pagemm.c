/******************************************************************************
*                                                                             *
*   Module:     PageMM.c                                                      *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       9/16/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This module contains the code for the kernel memory management
        at the page level.

        Pages are stored in an array of bytes, each entry holding the reference
        sharing count.  If a page is unused, the number is 0.  If a page is
        used only once, the number is 1.  If a page is shared, the number is
        more than 1.  You get the idea?


        Memory / Paging

        Total number of memory pages is stored in nPages, and that number does
        not change.  Pages are counted in the loader and passed as a parameter
        to Init.asm.  Since at the startup all allocated pages are contigous,
        nFreePage is also passed to init.asm as a parameter and it contains
        the index of the first unused physical page.

        Kernel heap is allocated and the global pHeap points to it.
        For all kernel allocations and frees macros MALLOC() and FREE()
        should be used.  They are set up to use pHeap as the default heap.

        pPages[] is an array holding the shared page counts.  It has exactly
        nPages elements.  All available pages have entry 0.  Other pages have
        shared count.

        Since the pages array consists of BYTEs, maximum theoretical number
        of processes that share pages is 255.  But since the first pid is 16
        and GDT is used to address each process's LDT, we can have max
        256-16=240 processes.

        Memory is partitioned in segments (arenas) of 4Mb each.  First arena,
        LIN_VM, contains the V86 process to be executed.  Next, LIN_KERNEL,
        contains the image of the kernel.  Next, LIN_PD, contains page
        directory table and only uses starting 4K for it.  Next arena,
        LIN_PT, contains page tables.  After page tables there is a heap
        arena, LIN_HEAP.  Each process is mapped in the successive arena
        after the heap with the (process id << 22)– LIN_KERNEL determining
        the process image address.

        Page tables arena contains the second-level page tables.  Each
        allocated page is 4K in size and they map: VM arena (LIN_PT_VM),
        kernel arena (LIN_PT_KERNEL), page directory arena (LIN_PT_PD),
        page tables itself – that makes it visible – (LIN_PT_PT), heap
        arena (LIN_PT_HEAP).

        Special pointers are defined to access page tables from "C":

        PageDir[] is an array pointing to the page directory.  Its indices
        are [0-1023] that address page directory entries (page arenas).
        PageTable[] points to the beginning of the page table arena and can
        be used to index a page of a process as in [pid * 1024 + page].
        Its index run in the range [0-1024*1024].

        PageTableMap[] is set up to point to the mapping page of the page
        table.  It is used to map a new page table so it is visible.
        Its index range [0-1023].  A new page table mapped within PageTableMap
        is visible in the LIN_PT arena, and then you can use
        PageTable[pid * 1024+()] to set it up.

-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 9/16/97    Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "intel.h"                      // Page tables defines

#include "kernel.h"                     // Include kernel defines

#include "mm.h"                         // Include its own header

#include "assertk.h"                    // Include kernel assert macro

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

const int nPages;                       // Total number of memory pages

BYTE * pPages = NULL;                   // Array of shared page counters

// Define base addresses of the page directory and page tables arrays.

const TPage * PageDir = (TPage *) (LIN_PD - LIN_KERNEL);
const TPage * PageTable = (TPage *) (LIN_PT - LIN_KERNEL);
const TPage * PageTableMap = (TPage *) (LIN_PT_PT - LIN_KERNEL);


//-----------------------------------------------------------------------------
// Variables private to this module
//-----------------------------------------------------------------------------

BYTE * pHeap = NULL;                    // Pointer to a heap memory buffer

// nFreePage is used only at the init time.  It holds the first consecutive
// free page number.

int nFreePage;                          // First free page

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

static const TPage * PageTableHeap = (TPage *) (LIN_PT_HEAP - LIN_KERNEL);


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   void Init_MM()                                                            *
*                                                                             *
*******************************************************************************
*
*   This function is called at the very beginning to initialize memory
#   structures.
*
******************************************************************************/
void Init_MM()
{
    dprintf("\nKernel heap at ");

    // Commit memory for the kernel heap

    pHeap = HeapCommitMap( HEAP_LEN );

    dprintf("%08Xh of %Xh", pHeap, HEAP_LEN );

    // Initialize kernel heap memory

    _Init_Alloc( pHeap, HEAP_LEN );

    // Allocate memory for the free pages array

    pPages = MALLOC( nPages );

    // Make all the pages free except first `nFreePage's

    memset( pPages, 0, nPages );
    memset( pPages, 1, nFreePage );

    dprintf(".");
}


/******************************************************************************
*                                                                             *
*   BYTE *HeapCommitMap( DWORD dwSize )                                       *
*                                                                             *
*******************************************************************************
*
*   This function sequentionally commits physical memory and maps it on the
#   kernel heap.  Before every allocation, one page is left not present to
#   help catch invalid references.
#
#   The address returned is based on the kernel data selector.
#
#   This function should be used only on initialization while the pages are
#   linearly uncommitted.  It will update free page array if it exists.
*
*   Where:
#       dwSize - size of a memory block (must be a multiple of a page size)
*
*   Returns:
*       address of a memory block based on the kernel data selector
*
******************************************************************************/
BYTE *HeapCommitMap( DWORD dwSize )
{
    static DWORD dwEntry = 0;
    DWORD dwStart, dwPages;
    TPage Page = { 0 };

    // Leave one page uncommitted: "++dwEntry".  This is done to catch the
    // reference that may traverse outside the committed bounds.

    dwStart = ++dwEntry;
    dwPages = dwSize / 4096;

    Page.fPresent = TRUE;
    Page.fWrite   = TRUE;

    for(; dwPages!=0; dwPages-- )
    {
        // Set the page table entry

        Page.Index = nFreePage;

        PageTableHeap[ dwEntry ] = Page;

        // Update free pages array if it was allocated

        if( pPages != 0 )
            pPages[nFreePage]++;

        dwEntry++;
        nFreePage++;
    }

    return( (void *)(dwStart * 4096 + LIN_HEAP - LIN_KERNEL) );
}


/******************************************************************************
*                                                                             *
*   TPage GetPhysicalPage()                                                   *
*                                                                             *
*******************************************************************************
*
*   Returns the descriptor of a free physical page.  If a free physical page
#   could not be found, the descriptor will have fPresent flag set to FALSE.
*
*   Returns:
*       TPage descriptor of a physical page with the index set up
*
******************************************************************************/
TPage GetPhysicalPage()
{
    static int LastPage = 0;
    int i;
    TPage Page = { 0 };


    // Sequential search through the free page bitfield and return the first
    // free page after marking it used.  To speed up a search, we remember
    // where we stopped last time and start from there

    for( i=LastPage; i<nPages; i++ )
    {
        if( pPages[i] == 0 )
        {
            // We have found a free page, so return a TPage structure

            Page.Index = i;
            Page.fPresent = TRUE;

            pPages[i]++;

            LastPage = i;

            return( Page );
        }
    }

    // We did not find a free page from the LastPage to the end, so repeat
    // the search from the page 0.  Iteratively, if the LastPage is already 0,
    // that means this is our second unsuccessful pass.

    if( LastPage==0 )
    {
        Page.fPresent = FALSE;

        return( Page );
    }

    LastPage = 0;

    return( GetPhysicalPage() );
}


/******************************************************************************
*                                                                             *
*   void ReleasePhysicalPage( TPage Page )                                    *
*                                                                             *
*******************************************************************************
*
*   This function returns a physical page to the free page pool.
*
*   Where:
*       Page is a physical page descriptor
*
******************************************************************************/
void ReleasePhysicalPage( TPage Page )
{
    // This should be very simple

    assertk( Page.Index < nPages, "Released page index out of bounds" );
    assertk( pPages[Page.Index] != 0, "Trying to release a free physical page" );

    pPages[Page.Index]--;
}


