/******************************************************************************
*                                                                             *
*   Module:     MM.h                                                          *
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

        This is a header file for the memory management module.
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
*   Important Defines                                                         *
******************************************************************************/
#ifndef _MM_H_
#define _MM_H_

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

extern const int nPages;                // Total number of memory pages

extern BYTE * pPages;                   // Array of shared page counters

//.-
// The following variables are "C" aliases for:
//
//  PageDir - address of the page directory page, 4K long, index [0,1023]
//
//  PageTable - address of the page tables (second level paging), 4Mb apperture
//              index [0,1024*1024]
//
//  PageTableMap - address of a page table mapping page tables, 4K long,
//                 index [0,1023]
//-.

extern const TPage * PageDir;
extern const TPage * PageTable;
extern const TPage * PageTableMap;

//.-
//-----------------------------------------------------------------------------
// Macros for kernel heap allocation - use MALLOC() and FREE() in the kernel
// and all device drivers!  Those macros will use the kernel heap `pHeap' by
// default.
//-----------------------------------------------------------------------------
//-.

extern BYTE * pHeap;                    // Pointer to a heap memory buffer

extern BYTE * _Init_Alloc( BYTE *pRamStart, DWORD dwRamSize );
extern void * _kMalloc( BYTE *pHeap, DWORD size );
extern void _kFree( BYTE *pHeap, void *pMem );
extern int _Alloc_Check( BYTE *pHeap, unsigned init_size );


#define MALLOC(size)        _kMalloc(pHeap,size)
#define FREE(ptr)           _kFree(pHeap,ptr)

extern BYTE *pMemDeb;                   // Debugger memory heap


/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern void Init_MM();
extern BYTE *HeapCommitMap( DWORD dwSize );

extern TPage GetPhysicalPage();
extern void ReleasePhysicalPage( TPage Page );


#endif //  _MM_H_
