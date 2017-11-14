/******************************************************************************
*                                                                             *
*   Module:     Info.c                                                        *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       10/2/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This module contains the code for displaying different
        process information.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 10/2/97    Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "debug.h"                      // Include debugger defines

#include "display.h"                    // Include display file header

#include "dpmi.h"                       // Include dpmi file header

#include "kernel.h"                     // Include kernel defines

#include "keyboard.h"                   // Include keyboard support

#include "info.h"                       // Include its own header

#include "inline.h"                     // Inline functions

#include "intel.h"                      // memory access functions

#include "mm.h"                         // Include memory manager header

#include "tty.h"                        // Include terminal file header

#include "device.h"                     // Include device header file

#include "process.h"                    // Include process header

#include "v86.h"                        // Include v86 header file

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

extern DWORD GDT_Address;               // GDT Base address
extern WORD GDT_Descriptor;             // GDT Limit

extern DWORD IDT_Address;               // IDT Base address
extern WORD IDT_Descriptor;             // IDT Limit

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   void dMemory( int nBytes )                                                *
*                                                                             *
*******************************************************************************
*
*   This function prints the given number as amount of memory and appends
#   the Mb/Kb variation.
*
*   Where:
*       nBytes - amount of memory to print
*
*   Returns:
*       void
*
******************************************************************************/
static void dMemory( int nBytes )
{
    int nMb, nKb;

    nMb = nBytes / (1024*1024);
    nKb = (nBytes % (1024*1024)) / 1024;

    dprintf("%d bytes", nBytes );

    if( nBytes >= 1024 )
    {
        dprintf(" or ");

        if( nMb != 0 )
        {
            dprintf("%d Mb ", nMb );
            if( nKb != 0 )
            dprintf("+ %d Kb", nKb );
        }
        else
            dprintf("%d Kb", nKb );
    }
}



/******************************************************************************
*                                                                             *
*   void DisplayProcessInfo( int pid, BOOL fExtended )                        *
*                                                                             *
*******************************************************************************
*
*   This function displays the process information
*
*   Where:
*       pid of the process to display
#       fExtended if True, displays extended process information
*
*   Returns:
*       void
*
******************************************************************************/
void DisplayProcessInfo( int pid, BOOL fExtended )
{
    int nPages, nShared, i;
    TProcess *p;
    char cStat1 = 'R', cStat2 = ' ';


    p = pProc[pid];

    if( p != 0 )
    {
        // Set some flags

        if( p->Flags & PROCESS_BLOCKED )
        {
            if( p->Flags & PROCESS_WAITING_PID ) cStat1 = 'W', cStat2 = 'P';
            if( p->Flags & PROCESS_WAITING_DEV ) cStat1 = 'W', cStat2 = 'D';
            if( p->Flags & PROCESS_ZOMBIE )      cStat1 = 'Z';
        }

        dprintf("\n%c%c%2d/%2d %s PCB: %08X  Im: %08X  %c%c %c%c",
                DP_SETWRITEATTR, ATTR_RESPONSEHI,
                pid,
                p->gid,
                p == pCurProc ? "VM": p == pCurProcPM ? "PM":"  ",
                p,
                p->Image,
                cStat1, cStat2,
                DP_SETWRITEATTR, ATTR_RESPONSE );

        if( pid==PROCESS_ID_TEMPLATE )
            dprintf("Template process");
        else
        {
            if( p->Flags & PROCESS_VM )
                dprintf("VM process ");

            if( p->Flags & PROCESS_SYSTEM_VM )
                dprintf("(System VM)");

            if( p->Flags & PROCESS_PM )
                dprintf("32 bit PM proc: %s", p->pName );
        }
    }

    // That is all for non-extended information

    if( !fExtended )  return;

    // Traverse through the process page table and add up the total number
    // of pages that it has allocated (present pages)

    for( nShared = nPages = i = 0; i<1024; i++ )
    {
        if( PageTable[ pid * 1024 + i ].fPresent == TRUE )
        {
            nPages++;

            if( PageTable[ pid * 1024 + i ].fWrite == FALSE )
                nShared++;
        }
    }

    dprintf("\n  Time slices: %u", p->slices );

    // Print the flags

    dprintf("\n  nWait: %04X, status: %04X, ptr: %08X, len: %d",
        p->nWait, p->status, p->Rq.p, p->Rq.n );

    dprintf("\n  Total pages:  %d  ", nPages );
    dMemory( nPages * 4096 );

    dprintf("\n  Pages used:   %d  ", nPages - nShared );
    dMemory( (nPages - nShared) * 4096 );

    dprintf("\n  Pages shared: %d  ", nShared );
    dMemory( nShared * 4096 );

    // Display extended process information

    if( fExtended )
    {
        dprintf("\n  pTTY: %08X  LDT: %04X\n  fd: ", p->pTTY, p->LDT );

        // Display the file descriptor indices

        for( i=0; i<FOPEN_MAX; i++ )
            dprintf(" %d", p->iFile[i] );

        // Display the registers saved

        PrintStackFrame( &p->Reg );
    }
}



/******************************************************************************
*                                                                             *
*   void DisplayMemoryInfo()                                                  *
*                                                                             *
*******************************************************************************
*
*   This function displays memory information
*
*   Returns:
*       void
*
******************************************************************************/
void DisplayMemoryInfo()
{
    int i, nFree;

    // Loop and count free pages

    for( nFree=i=0; i<nPages; i++ )
    {
        if( pPages[i] == 0 )
            nFree++;
    }

    // Print the global nPages that is set up at the init.c

    dprintf("\nTotal amount of memory  "); dMemory( nPages * 4096 );
    dprintf("\nUsed physical memory    %d pages or ", nPages - nFree ); dMemory( (nPages - nFree) * 4096 );
    dprintf("  (%d%%)", 100 - (100 * nFree / nPages) );
    dprintf("\nFree physical memory    %d pages or ", nFree ); dMemory( nFree * 4096 );
    dprintf("\nPhysical pages array at %08X [%d]", pPages, nPages );
    dprintf("\nARPL Break for VM at    %04X:%04X", dwBreakSegOffs >> 16, dwBreakSegOffs & 0xFFFF );
    dprintf("\nARPL Break for DPMI at  %04X:%04X", dwDPMISegOffs >> 16, dwDPMISegOffs & 0xFFFF );
}



/******************************************************************************
*                                                                             *
*   BOOL PrintDesc( int index, TDesc *pDesc, BOOL fRecurse )                  *
*                                                                             *
*******************************************************************************
*
*   This function prints out one descriptor.  Descriptor must be present.
*
*   Where:
#       index is the index of that descriptor from the beginning of a table
*       pDesc is the descriptor to be decoded
#       fRecurse is True if the function lists LDTs
*
*   Returns:
#       True if the descriptor is an LDT
#       False if the descriptor is not an LDT
#
******************************************************************************/
BOOL PrintDesc( int index, TDesc *pDesc, BOOL fRecurse )
{
    static const char *App1[] = { "Data16", "Code16", "Data32", "Code32" };
    static const char *App2[] = { "RO", "RW", "ROD", "RWD", "EO", "ER", "EOC", "ERC" };
    static const char *Sys1[] =
    {
        "Reserved", "TSS16", "LDT",      "TSS16", "Call16", "Task16",   "Int16", "Trap16",
        "Reserved", "TSS32", "Reserved", "TSS32", "Call32", "Reserved", "Int32", "Trap32"
    };

    DWORD Base, Limit;
    int type;
    BOOL fLDT = 0;
    int i;


    // Descriptor must be present

    if( pDesc->Present == 1 )
    {
        // Index must match the descriptor privilege level.  If the table is an
        // LDT (fRecurse is True), bit 2 of a selector is also set

        dprintf("\n%04X ", (index<<3) | pDesc->DPL | (fRecurse << 2) );

        // Decompose a descriptor

        if( pDesc->CodeData == 0 )
        {
            // System descriptor

            dprintf("%-10s ", Sys1[pDesc->Type] );

            // If the descriptor is an LDT, return TRUE

            if( pDesc->Type == 2 )
                fLDT = TRUE;
        }
        else
        {
            // Application code/data descriptor

            type = (pDesc->Size << 1) + (pDesc->Type >> 3);

            dprintf("%-7s ", App1[type] );

            dprintf("%-3s ", App2[pDesc->Type >> 1] );
        }

        // Print the common base and limit fields

        Base  = GET_DESCBASE(pDesc);
        Limit = GET_DESCLIMIT(pDesc);

        if( pDesc->Granularity == 1 )
            Limit = (Limit << 12) | 0xFFF;

        dprintf("%08X ", Base );

        if( Base >= LIN_KERNEL )
            dprintf("%08X  %08X",
                Base - LIN_KERNEL, Limit );
        else
            dprintf("          %08X", Limit );
    }

    // If the descriptor was an LDT, call itself recursively to list all
    // entries of an LDT table

    if( fLDT==TRUE )
    {
        for( i=0; i<(Limit+1)/8; i++ )
        {
            dprintf("%c%c", DP_SETWRITEATTR, ATTR_RESPONSEHI );

            PrintDesc( i, (TDesc *)(Base - LIN_KERNEL + i*8), TRUE );

            dprintf("%c%c", DP_SETWRITEATTR, ATTR_RESPONSE );
        }

        return( FALSE );
    }

    return( fLDT );
}


/******************************************************************************
*                                                                             *
*   void PrintGDT()                                                           *
*                                                                             *
*******************************************************************************
*
*   This function prints the global descriptor table.
*
******************************************************************************/
void PrintGDT()
{
    int nDesc, i;

    // Calculate the legal number of GDT entries

    nDesc = (GDT_Descriptor + 1) / sizeof(TDesc);

    dprintf("\nGDT Base Address: @%08X   Limit: %04X (%d entries)  LDT: %04X",
            GDT_Address, GDT_Descriptor, nDesc, GetLDT() );

    dprintf("\nIndex Type       @Base    Kernel    Limit");
    dprintf("\n--------------------------------------------");

    // GDT 0 is never loaded, so we skip it

    for( i=1; i<nDesc; i++ )
    {
        PrintDesc( i, &GDT[i], FALSE );
    }
}


/******************************************************************************
*                                                                             *
*   void PrintIDT()                                                           *
*                                                                             *
*******************************************************************************
*
*   This function prints the interrupt descriptor table.
*
******************************************************************************/
void PrintIDT()
{
    static const char *Idt1[] =
    {
        "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Task16", "Int16", "Trap16",
        "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Task32", "Int32", "Trap32"
    };

    int nDesc, i, nLines;

    // Calculate the legal number of IDT entries

    nDesc = (IDT_Descriptor + 1) / sizeof(TIntDesc);

    dprintf("\nIDT Base Address: @%08X   Limit: %04X (%d entries)",
            IDT_Address, IDT_Descriptor, nDesc );

    dprintf("\nIndex Type         Sel:Offset    DPL");
    dprintf("\n-----------------------------------------");

    for( nLines = i = 0; i<nDesc; i++ )
    {
        dprintf("\n%04X %-10s  %04X:%08X  DPL=%d  %c",
            i,
            Idt1[ IDT[i].Type + (IDT[i].Size << 3) ],
            IDT[i].Selector,
            GET_INTOFFSET( &IDT[i] ),
            IDT[i].DPL,
            IDT[i].Present==1 ? 'P':' ' );

        if( nLines++ == Deb.nListLines )
        {
            nLines = 0;

            if( GetKey( 1 )==ESC )
                return;
        }
    }
}
