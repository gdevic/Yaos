/******************************************************************************
*                                                                             *
*   Module:     gdt.c                                                         *
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

        This module contains the code for the management of the Global
        Descriptor Table and Local Descriptor tables.

        GDT is an array of 256 entries.  First 16 are reserved by the kernel,
        and then entries PROCESS_ID_MIN to PROCESS_ID_MAX are free for LDTs.

        Therefore, process id directly indexes into GDT where its LDT reside.
        That LDT describe a data structure allocated by the kernel containing
        3 descriptor entries:
            for NULL selector,
            SEL_APP_DATA and
            SEL_APP_CODE

        Code and Data descriptors are also set up to describe base address of
        the process image with the named process id using the formula

        Absolute_Image = pid << 22,
        Image = Absolute_Image - LIN_KERNEL

        Where `Absolute_Image' is for the CPU access starting with the base 0,
        and Image is addressable by the kernel since kernel data does not
        start at absolute address of 0.
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

#include <sys/types.h>                  // Include standard clib types

#include "inline.h"                     // Include inline macros

#include "intel.h"                      // Include intel specific defines

#include "kernel.h"                     // Include kernel header file

#include "mm.h"                         // Include memory management header

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

#define MAX_LDT             3           // Number of LDT entries per process

#define LDT_LIMIT     (4*1024*1024-1)   // 4Mb limit


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   void SetupSelector( TDesc *pDesc, DWORD dwBase, DWORD dwLimit,            #
#                       BYTE Type, BOOL fSystem, BYTE DPL )                   *
*                                                                             *
*******************************************************************************
*
*   Sets up the selector fields.  If the fSystem bit is set, the descriptor
#   has system type (LDT, TSS etc.) and the granularity is set to 0 (byte
#   granular).  Otherwise, the dwLimit field is scaled down and granularity
#   is set to page granular.
*
*   Where:
*       pDesc is a pointer to a descriptor to be set up
#       dwBase is the base address
#       dwLimit is the limit
#       Type is the selector type (4 bits wide)
#       fSystem is the system bit
#       DPL is the dpl field
*
*   Returns:
*       void
*
******************************************************************************/
void SetupSelector( TDesc *pDesc, DWORD dwBase, DWORD dwLimit,
                           BYTE Type, BOOL fSystem, BYTE DPL )
{
//  dprintf("\n%08X -> %08X %08X %X %d %d", pDesc, dwBase, dwLimit, Type, fSystem, DPL );

    // If the system flag is TRUE, we have a system descriptor

    if( fSystem )
    {
        pDesc->CodeData = 0;            // System selector
        pDesc->Granularity = 0;         // Byte granular
    }
    else
    {
        pDesc->CodeData = 1;            // Code/Data selector
        pDesc->Granularity = 1;         // Page granular

        // Shift down limit because we use 4K granularity

        dwLimit >>= 12;
    }

    pDesc->Limit0   = dwLimit & 0xFFFF;
    pDesc->Base0    = dwBase & 0xFFFF;
    pDesc->Base1    = (dwBase >> 16) & 0xFF;
    pDesc->Type     = Type;
    pDesc->DPL      = DPL;
    pDesc->Present  = 1;
    pDesc->Limit1   = (dwLimit >> 16) & 0xF;
    pDesc->Res      = 0;
    pDesc->Size     = 1;
    pDesc->Base2    = (dwBase >> 24) & 0xFF;
}


/******************************************************************************
*                                                                             *
*   WORD AllocLDT( pid_t pid )                                                *
*                                                                             *
*******************************************************************************
*
*   This function allocates an LDT for the given process pid.  Process number
#   is used to directly map the ldt entry in the gdt table.  All the selectors
#   of the LDT table have DPL of 3.
*
*   Where:
*       pid is a process number
*
*   Returns:
*       LDT or 0 if there was an error and ldt could not be set up
*
******************************************************************************/
WORD AllocLDT( pid_t pid )
{
    TDesc *pLDT;

    // Allocate memory for the LDT table

    pLDT = (TDesc *) MALLOC( sizeof(TDesc) * MAX_LDT );

    if( pLDT != NULL )
    {
        // Set up the Local descriptors:
        //  LDT0 - null
        //  LDT1 - code 32 bit executable, limit 4Mb
        //  LDT2 - data 32 bit, limit 4Mb

        memset( pLDT, 0, sizeof(TDesc) * MAX_LDT );

        SetupSelector( &pLDT[1], pid << 22, LDT_LIMIT, DESC_TYPE_EXEC, FALSE, 3 );
        SetupSelector( &pLDT[2], pid << 22, LDT_LIMIT, DESC_TYPE_DATA, FALSE, 3 );

        // Insert the ldt pointer into the gdt at the pid position

        SetupSelector( &GDT[pid], (DWORD)(LIN_KERNEL + (int)pLDT), MAX_LDT * 8 - 1, DESC_TYPE_LDT, TRUE, 3 );

        // The returning LDT is in fact the pid, that is the index into GDT.

        return( (pid << 3) | 3 );
    }

    return( 0 );
}


/******************************************************************************
*                                                                             *
*   int FreeLDT( WORD wSel )                                                  *
*                                                                             *
*******************************************************************************
*
#   This function frees an LDT whose selector is given.
#
*   Where:
*       wSel is the LDT selector
*
*   Returns:
*       0 on success
#       -1 on failure; invalid pid to free
*
******************************************************************************/
int FreeLDT( WORD wSel )
{
    TDesc *pLDT;
    TDesc DescBlank = { 0 };
    void * pBase;

    // It's ok to try to free 0 selector

    if( wSel==0 ) return( 0 );

    pLDT = &GDT[wSel >> 3];

    if( pLDT->Type != DESC_TYPE_LDT )
    {
        dprintf("\nTrying to free a non-LDT descriptor!");
        return( -1 );
    }

    pBase = (void *)( GET_DESCBASE(pLDT) - LIN_KERNEL);

    // Free the process LDT array

    FREE( pBase );

    // Reset the LDT entry in the GDT table

    *pLDT = DescBlank;

    return( 0 );
}


