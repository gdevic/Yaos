/******************************************************************************
*                                                                             *
*   Module:     mm.c                                                          *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       09/14/96                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This module implements the memory management mechanism via paging.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 04/20/96   1.00  Original                                       Goran Devic *
* 09/13/96   1.10  Every process is mapped individually           Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/
#include "kernel.h"                     // Include kernel header
#include "mm.h"                         // Include its own header file
#include "types.h"                      // Include basic types

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

// Define a page entry
typedef struct
{
    union
    {
        DWORD dwEntry;                  // dword access
        struct
        {
            DWORD fPresent      : 1;    // Present bit
            DWORD fWritable     : 1;    // Page is writable
            DWORD fUser         : 1;    // User page
            DWORD Res1          : 2;    // Reserved
            DWORD fAccessed     : 1;    // Page has been accessed
            DWORD fDirty        : 1;    // Dirty page
            DWORD Res2          : 2;    // Reserved
            DWORD fAvailable    : 3;    // Available
            DWORD Address       :20;    // Page frame address
        };
    };
} TPage;


// The page memory structure
typedef struct
{
    DWORD   dwTotalMemory;              // Total RAM memory found
    DWORD   dwFreeMemory;               // Free RAM memory above kernel heap
    DWORD   dwHeapStart;                // Start of the kernel heap
    DWORD   dwHeapSize;                 // Size of the kernel heap

    DWORD   dwNumPages;                 // Total number of pages
    BYTE   *pbFrameTable;               //   Array of free pages

    TPage  *pPageDir;                   // Page directory table
} Tmem;

static Tmem mem;

// Each frame in a pbFrameTable array is defined as a bit in the corresponding
// bit vector such that:
#define FRAME_FREE         1            // Frame is free
#define FRAME_TAKEN        0            // Frame is taken   
#define ALL_FRAMES_FREE    0xff         // Used by init to fill pbFrameTable
#define ALL_FRAMES_TAKEN   0x00         // Used by init to occupy the ROM range
#define NO_PAGES           0xffffffff   // GetFreePhyPage() failed

#define PAGE_EMPTY         0x06         // New page table filler

// Access macros
#define FRAME(n)           ((mem.pbFrameTable[(n)/8] >> ((n)&7)) & 1)
#define TAKE_FRAME(n)      (mem.pbFrameTable[(n)/8] &= ~(1<<((n)&7)))
#define FREE_FRAME(n)      (mem.pbFrameTable[(n)/8] |= 1<<((n)&7))


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   void InitMemoryManager( DWORD dwHeapStart )                               *
*                                                                             *
*******************************************************************************
*
*   Initializes memory manager:
*   1)  Kernel heap management - the available memory from the
*       start of the kernel heap to the top of the kernel heap (from the
*       dwHeapStart to KERNEL_HEAP_TOP) is managed by kmalloc() and kfree() 
*       calls.
*
*   2)  Paging - The memory from the address 0, the first MB and the
*       kernel up to the top of the kernel heap is mapped 1-1 to the
*       corresponding physical memory.  Next follow the page directory
*       table and page tables as needed.
*
*   Where:
*       dwHeapStart is the address of the start of the kernel heap
*       
******************************************************************************/
void InitMemoryManager( DWORD dwHeapStart )
{
    int count;
    volatile DWORD * pdwMem;


    // First, find out how much memory this system has; we start probing from
    // the top of the kernel heap
    //
    pdwMem = (DWORD *) KERNEL_HEAP_TOP;
    count = 0;
    do
    {
        // Store two dwords to avoid floating bus echo
        pdwMem[0] = 0x5555BABE;
        pdwMem[1] = 0xBEEFAAAA;

        // We have found the top of the memory (data was not stored)
        if( pdwMem[0] != 0x5555BABE  ||  pdwMem[1] != 0xBEEFAAAA )
            break;

        // Count free memory in pages
        count++;
        pdwMem += 4096/4;

    } while( 1 );
                        
    mem.dwFreeMemory = count * 4 * 1024;
    mem.dwTotalMemory = mem.dwFreeMemory + ABS_KERNEL_HEAP_TOP;


    // Check if enough free memory is left for the pages and user processes.
    // There is no much sense running if at least MIN_FREE_MEMORY bytes is
    // not left after the kernel basic needs
    //
    if( mem.dwFreeMemory < MIN_FREE_MEMORY )
        KernelPanic("Not enough memory");


    // Initialize kernel heap dynamic memory manager: kmalloc(), kfree()
    //
    mem.dwHeapStart = dwHeapStart;
    mem.dwHeapSize = KERNEL_HEAP_TOP - mem.dwHeapStart;
    init_kmem( (BYTE *)mem.dwHeapStart, mem.dwHeapSize );


    // Allocate the array of free pages.  The amount of free memory will
    // determine the array size - each page has a corresponding bit in this
    // array that is "1" if a page is free and "0" if a page is taken.
    // This array will always be divisible by 8 (bits in a byte) since the
    // memory installed is always in multiple of megabyte (is it?) and this
    // takes whole 32 bytes (8*32 pages in a megabyte).
    //
    // Array pbFrameTable has exactly dwNumPages entries.
    //
    mem.dwNumPages = mem.dwTotalMemory / 4096;
    mem.pbFrameTable = (BYTE *) kmalloc( mem.dwNumPages / 8 );
    kmemset( mem.pbFrameTable, ALL_FRAMES_FREE, mem.dwNumPages / 8 );


    // Prepare the page directory table at the top of the kernel heap and
    // the corresponding page table.  The kernel will take the first entry
    // in the directory table since it resides in the first 4Mb.
    // Make the rest of the pages not present.
    //
    mem.pPageDir = (TPage *) KERNEL_HEAP_TOP;
    kmemset( mem.pPageDir, PAGE_EMPTY, 2*4096 );

    // The page table that maps the page directory and the first 4Mb 
    // is set up here.  We have to allocate two frames from the frame table
    // and set them up manually so that the subsequent allocations will yield
    // the 1-1 mapping.
    //
    // First, the page directory is 1-1 mapped at ABS_KERNEL_HEAP_TOP:
    //
    TAKE_FRAME( ABS_KERNEL_HEAP_TOP/4096 );

    // The first page is also 1-1 mapped and is following page directory:
    TAKE_FRAME( ABS_KERNEL_HEAP_TOP/4096 + 1 );

    // Now link the first page table in the directory
    mem.pPageDir[0].dwEntry = PAGE_PRESENT + PAGE_READWRITE + PAGE_USER;
    mem.pPageDir[0].Address = ABS_KERNEL_HEAP_TOP/4096 + 1;


    // Map kernel address space in the linear order to preserve the 
    // one-to-one correspondence for kernel physical/linear addresses
    //
    MapPhysical( 0,                     // Virtual 86 mode pages, first Mb
                0,                      // Real allocation
                ABS_KERNEL_LIN,         // So many bytes
                PAGE_PRESENT + PAGE_READWRITE + PAGE_USER );

    MapPhysical( ABS_KERNEL_LIN,        // Kernel pages
                0,                      // Real allocation
                KERNEL_HEAP_TOP,        // Kernel needs so many bytes
                PAGE_PRESENT + PAGE_READWRITE + PAGE_SUPERVISOR );

    // Map the page directory and the first page table to be accessible
    // by the kernel.  Frames were already commited earlier
    //
    MapPhysical( ABS_KERNEL_HEAP_TOP,   // Page directory + page 0
                ABS_KERNEL_HEAP_TOP,    // 1-1 correspondence
                4096 * 2,               // Two frames
                PAGE_PRESENT + PAGE_READWRITE + PAGE_SUPERVISOR );

    // Now release 256 Kb from C0000 to 100000 to map ROM there
    UnmapPhysical( 0xC0000, 0x40000 );

    // In the place of the ROM, map the real ROM code but do not commit any
    // physical page to that area - just adjust page tables
    //
    MapPhysical( 0xC0000,               // ROM address range
                0xC0000,                // Simulate physical range
                0x40000,                // So many bytes
                PAGE_PRESENT + PAGE_READ + PAGE_USER );

    // It does not seem that we can use the 256 Kb of RAM there since the ROM 
    // code likes to show there
    kmemset( mem.pbFrameTable + 0xC0000/(4096*8), ALL_FRAMES_TAKEN, 0x40000/(4096*8) );


    // Finally, enable paging
    //
    EnablePaging( ABS_KERNEL_HEAP_TOP );
    printk("Paging Enabled\n");

    printk("Free memory: %d Kb\n", mem.dwFreeMemory / 1024 );
    printk("Kernel heap @%08X for %08X\n", mem.dwHeapStart, 
            KERNEL_HEAP_TOP - mem.dwHeapStart );
}


/******************************************************************************
*                                                                             *
*   DWORD GetFreePhyPage()                                                    *
*                                                                             *
*******************************************************************************
*
*   Returns the physical address of the free page using an incremental linear
*   search through the free pages array.
*
*   Returns:
*       Physical address of the free page
*       NO_PAGES (0xffffffff) if there was no free page available
*       
******************************************************************************/
static DWORD GetFreePhyPage()
{
    int i;

    // Traverse free pages array and find a free page
    //
    for( i=0; i < mem.dwNumPages; i++ )
    {
        if( FRAME(i)==FRAME_FREE )
        {
            // Reserve the page and return the physical address
            TAKE_FRAME(i);
            return( i * 4096 );
        }
    }

    // Could not find a free page
    return( NO_PAGES );
}


/******************************************************************************
*                                                                             *
*   DWORD MapPhysical                                                         *
*  ( DWORD dwAbsLinearAddr, DWORD dwPhySim, DWORD dwSize, DWORD dwAttibutes)  *
*                                                                             *
*******************************************************************************
*
*   This function maps physical pages to linear addresses.  The pages are taken
*   from the pool of free pages using linear ascending search.
*   The corresponding page table may need to be allocated as well.  The block
*   has to be within one 4Mb segment..
*
*   Where:
*       dwAbsLinearAddr is the starting address in the linear space that
*           requires mapping
*       dwPhySim, if not NULL, is the physical address to be simulated.  In
*           that case no physical frames will be commited, but only page tables
*           will be built.  Anyway, the page table will be allocated if
*           necessary.
*       dwSize is the size in bytes of the address range
*       dwAttributes are page attributes: they may include
*           PAGE_PRESENT, PAGE_READWRITE, PAGE_SUPERVISOR, ...
*
*   Returns:
*       NO_PAGES if there was not enough free pages in the free pool,
*       Zero is operation completed
*
******************************************************************************/
DWORD MapPhysical( 
    DWORD dwAbsLinearAddr, DWORD dwPhySim, DWORD dwSize, DWORD dwAttributes )
{
    DWORD dwPhyAddr;
    DWORD dwDir, dwPage;
    int i;

    // Calculate number of pages to allocate
    //
    if( dwSize & 0xfff )
        dwSize = (dwSize >> 12) + 1;
    else
        dwSize >>= 12;

    // Find the directory and page entry of the address to be mapped
    dwDir = dwAbsLinearAddr >> 22;
    dwPage = (dwAbsLinearAddr >> 12) & 0x3ff;

    // Check if the page table needs to be allocated as well
    if( ! mem.pPageDir[ dwDir ].fPresent )
    {
        if( (dwPhyAddr = GetFreePhyPage())==NO_PAGES )
            return( NO_PAGES );

        // Link the page table frame with the first page so that the kernel
        // may access it from the page tables above the heap
        mem.pPageDir[ 1024 + ABS_KERNEL_HEAP_TOP/4096 + dwDir + 1 ].dwEntry = PAGE_PRESENT + PAGE_READWRITE + PAGE_USER;
        mem.pPageDir[ 1024 + ABS_KERNEL_HEAP_TOP/4096 + dwDir + 1 ].Address = dwPhyAddr >> 12;

        // Clear the new page table
        kmemset( &mem.pPageDir[ 1024 + dwDir*1024 ], PAGE_EMPTY, 4096 );

        // Link the page directory to actually give the new memory some
        // place in address space
        mem.pPageDir[ dwDir ].dwEntry = PAGE_PRESENT + PAGE_READWRITE + PAGE_USER;
        mem.pPageDir[ dwDir ].Address = dwPhyAddr >> 12;
    }

    // For each page loop and map it
    //
    for( i=0; i<dwSize; i++ )
    {
        // If the physical frames should not be commited, use given addresses
        if( dwPhySim != 0 )
        {
            dwPhyAddr = dwPhySim;
            dwPhySim += 4096;

        }   // Get the physical page address to map and map it
        else if( (dwPhyAddr = GetFreePhyPage())==NO_PAGES )
        {
            // Undo the previous allocations
            UnmapPhysical( dwAbsLinearAddr, i*4096 );

            // Could not allocate free page
            return( NO_PAGES );
        }

        // Set the page table entry
        mem.pPageDir[ 1024 + dwDir*1024 + dwPage ].dwEntry =
           (dwPhyAddr & ~0xfff) + dwAttributes;

        // Advance address to the next page
        dwPage++;
    }

    return( 0 );
}


/******************************************************************************
*                                                                             *
*   void UnmapPhysical( DWORD dwAbsLinearAddr, DWORD dwSize )                 *
*                                                                             *
*******************************************************************************
*
*   This function returns physical frames to the free frames pool.  The page
*   table entries referencing the frames are marked not present.  The page
*   table itself is not freed.
*
*   Where:
*       dwAbsLinearAddr is the starting address to free
*       dwSize is the number of bytes of the block to be freed
*
******************************************************************************/
void UnmapPhysical( DWORD dwAbsLinearAddr, DWORD dwSize )
{
    DWORD dwPhyFrame;
    DWORD dwDir, dwPage;

    // Calculate number of pages to free
    //
    if( dwSize & 0xfff )
        dwSize = (dwSize >> 12) + 1;
    else
        dwSize >>= 12;

    // Why not ?
    if( dwSize==0 ) return;

    dwDir = dwAbsLinearAddr >> 22;
    dwPage = (dwAbsLinearAddr >> 12) & 0x3ff;

    // For each page loop and release it
    //
    for( ; dwSize; dwSize-- )
    {
        // Get the physical address of the frame
        dwPhyFrame = mem.pPageDir[ 1024 + dwDir*1024 + dwPage ].Address;

        // Free the frame
        FREE_FRAME( dwPhyFrame );

        // Set the page table entry to not present
        mem.pPageDir[ 1024 + dwDir*1024 + dwPage ].fPresent = PAGE_NOT_PRESENT;

        // Advance the page number
        dwPage++;
    }
}
