/******************************************************************************
*                                                                             *
*   Module:     Register.c                                                    *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       9/13/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

    This module contains the code for the displaying of register file.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 9/13/97    1.00  Original                                       Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/
#include <stdio.h>                      // Include standard I/O header file

#include "debug.h"                      // Include debugger hader file

#include "intel.h"                      // Include register flags defines

#include "tty.h"

#include "device.h"                     // Include device header file

#include "process.h"                    // Include process header file

#include "register.h"                   // Include its own header

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

static char sEflags[] = "vn3odi szapc";


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   void PrintStackFrame( TIntStack *pStack )                                 *
*                                                                             *
*******************************************************************************
*
*   This function prints all the registers from the given stack frame.  It is
#   used for info after a fault.
*
*   Where:
*       pStack is the stack frame containing the registers
*
*   Returns:
*       void
*
******************************************************************************/
void PrintStackFrame( TIntStack *pStack )
{
    dprintf("\nEAX:%08X EBX:%08X ECX:%08X EDX:%08X ESI:%08X EDI:%08X",
        pStack->eax, pStack->ebx, pStack->ecx, pStack->edx, pStack->esi, pStack->edi );

    dprintf("\nEBP:%08X ESP:%08X", pStack->ebp, pStack->esp );

    dprintf("\nDS:%04X ES:%04X FS:%04X GS:%04X SS:%04X CS:%04X EIP:%08X     ",
        pStack->pm_ds, pStack->pm_es, pStack->pm_fs, pStack->pm_gs, pStack->ss, pStack->cs, pStack->eip );

    // Set the eflags string

    strcpy( sEflags, "vn3odi szapc" );

    if( pStack->eflags & CF_MASK ) sEflags[11] = 'C';
    if( pStack->eflags & PF_MASK ) sEflags[10] = 'P';
    if( pStack->eflags & AF_MASK ) sEflags[9] = 'A';
    if( pStack->eflags & ZF_MASK ) sEflags[8] = 'Z';
    if( pStack->eflags & SF_MASK ) sEflags[7] = 'S';

    if( pStack->eflags & IF_MASK ) sEflags[5] = 'I';
    if( pStack->eflags & DF_MASK ) sEflags[4] = 'D';
    if( pStack->eflags & OF_MASK ) sEflags[3] = 'O';
    if((pStack->eflags & IOPL_MASK) == 0 ) sEflags[2] = '0'; else
    if((pStack->eflags & IOPL_MASK) == (1 << IOPL_BIT0) ) sEflags[2] = '1'; else
    if((pStack->eflags & IOPL_MASK) == (2 << IOPL_BIT0) ) sEflags[2] = '2';
    if( pStack->eflags & NT_MASK ) sEflags[1] = 'N';
    if( pStack->eflags & VM_MASK ) sEflags[0] = 'V';

    dprintf("%s  ", sEflags );

    dprintf("\nV86 DS:%04X ES:%04X FS:%04X GS:%04X",
            pStack->v86_ds, pStack->v86_es, pStack->v86_fs, pStack->v86_gs );
}



/******************************************************************************
*                                                                             *
*   void RegDisplay( BOOL fOnTop )                                            *
*                                                                             *
*******************************************************************************
*
*   Displays the register file.  If the Deb.fRegs is True, it will display
#   at the top of the screen.  Otherwise, it displays at the current location.
*
#   Where:
#       fOnTop - display registers at the top line
#
******************************************************************************/
void RegDisplay( BOOL fOnTop )
{
    if( fOnTop )
    {
        // Start at the top of the screen using the register attributes

        dprintf("%c%c%c%c%c", DP_SETCURSOR, 0, 0,
                              DP_SETWRITEATTR, ATTR_REGS );
    }
    else
        dprintf("\n");


    // Print all the registers

    dprintf("EAX=%08X EBX=%08X ECX=%08X EDX=%08X ESI=%08X EDI=%08X   ",
        pDebReg->eax, pDebReg->ebx, pDebReg->ecx, pDebReg->edx, pDebReg->esi, pDebReg->edi );

    dprintf("EBP=%08X ESP=%08X                                                       ",
        pDebReg->ebp, pDebStack->esp );

    dprintf("DS=%04X ES=%04X FS=%04X GS=%04X SS=%04X CS=%04X EIP=%08X     ",
        pDebSeg->ds, pDebSeg->es, pDebSeg->fs, pDebSeg->gs, pDebStack->ss, pDebCode->cs, pDebCode->eip );

    // Set the eflags string

    strcpy( sEflags, "vn3odi szapc" );

    if( pDebCode->eflags & CF_MASK ) sEflags[11] = 'C';
    if( pDebCode->eflags & PF_MASK ) sEflags[10] = 'P';
    if( pDebCode->eflags & AF_MASK ) sEflags[9] = 'A';
    if( pDebCode->eflags & ZF_MASK ) sEflags[8] = 'Z';
    if( pDebCode->eflags & SF_MASK ) sEflags[7] = 'S';

    if( pDebCode->eflags & IF_MASK ) sEflags[5] = 'I';
    if( pDebCode->eflags & DF_MASK ) sEflags[4] = 'D';
    if( pDebCode->eflags & OF_MASK ) sEflags[3] = 'O';
    if((pDebCode->eflags & IOPL_MASK) == 0 ) sEflags[2] = '0'; else
    if((pDebCode->eflags & IOPL_MASK) == (1 << IOPL_BIT0) ) sEflags[2] = '1'; else
    if((pDebCode->eflags & IOPL_MASK) == (2 << IOPL_BIT0) ) sEflags[2] = '2';
    if( pDebCode->eflags & NT_MASK ) sEflags[1] = 'N';
    if( pDebCode->eflags & VM_MASK ) sEflags[0] = 'V';

    dprintf("%s  ", sEflags );

    // Fill up the last space when printing on the top

    if( fOnTop ) dprintf(" ");
}



/******************************************************************************
*                                                                             *
*   BOOL RegSet( char *pRegName, DWORD dwValue )                              *
*                                                                             *
*******************************************************************************
*
*   Sets the named register with the value.
*
*   Where:
*       pRegName is a string containing the name of a register to be set
#       dwValue is dword, word, byte or a bit to be set
*
*   Returns:
*       TRUE if register is recognized and set
#       FALSE if the register name is invalid
*
******************************************************************************/

#define SET_REG_DWORD(Reg)  { Reg=dwValue; return( TRUE ); }
#define SET_REG_WORD(Reg)   { Reg=(Reg & ~0xFFFF) | (dwValue & 0xFFFF); return( TRUE ); }
#define SET_REG_BYTELO(Reg) { Reg=(Reg & ~0xFF) | (dwValue & 0xFF); return( TRUE ); }
#define SET_REG_BYTEHI(Reg) { Reg=(Reg & ~0xFF00) | ((dwValue & 0xFF)<<8); return( TRUE ); }
#define SET_REG_FLAG(Mask,Bit) { pDebCode->eflags=(pDebCode->eflags & ~Mask) | ((dwValue & 1)<<Bit); return( TRUE ); }
#define SET_REG_IOPL(Mask,Bit) { pDebCode->eflags=(pDebCode->eflags & ~Mask) | ((dwValue & 3)<<Bit); return( TRUE ); }

BOOL RegSet( char *pRegName, DWORD dwValue )
{
    // Check the register names

    if( !strnicmp( pRegName, "eax", 3 ) )  SET_REG_DWORD( pDebReg->eax );
    if( !strnicmp( pRegName, "ebx", 3 ) )  SET_REG_DWORD( pDebReg->ebx );
    if( !strnicmp( pRegName, "ecx", 3 ) )  SET_REG_DWORD( pDebReg->ecx );
    if( !strnicmp( pRegName, "edx", 3 ) )  SET_REG_DWORD( pDebReg->edx );
    if( !strnicmp( pRegName, "ebp", 3 ) )  SET_REG_DWORD( pDebReg->ebp );
    if( !strnicmp( pRegName, "esi", 3 ) )  SET_REG_DWORD( pDebReg->esi );
    if( !strnicmp( pRegName, "edi", 3 ) )  SET_REG_DWORD( pDebReg->edi );

    if( !strnicmp( pRegName, "ax", 2 ) )  SET_REG_WORD( pDebReg->eax);
    if( !strnicmp( pRegName, "bx", 2 ) )  SET_REG_WORD( pDebReg->ebx);
    if( !strnicmp( pRegName, "cx", 2 ) )  SET_REG_WORD( pDebReg->ecx);
    if( !strnicmp( pRegName, "dx", 2 ) )  SET_REG_WORD( pDebReg->edx);
    if( !strnicmp( pRegName, "bp", 2 ) )  SET_REG_WORD( pDebReg->ebp);
    if( !strnicmp( pRegName, "si", 2 ) )  SET_REG_WORD( pDebReg->esi);
    if( !strnicmp( pRegName, "di", 2 ) )  SET_REG_WORD( pDebReg->edi);

    if( !strnicmp( pRegName, "al", 2 ) )  SET_REG_BYTELO( pDebReg->eax );
    if( !strnicmp( pRegName, "ah", 2 ) )  SET_REG_BYTEHI( pDebReg->eax );
    if( !strnicmp( pRegName, "bl", 2 ) )  SET_REG_BYTELO( pDebReg->ebx );
    if( !strnicmp( pRegName, "bh", 2 ) )  SET_REG_BYTEHI( pDebReg->ebx );
    if( !strnicmp( pRegName, "cl", 2 ) )  SET_REG_BYTELO( pDebReg->ecx );
    if( !strnicmp( pRegName, "ch", 2 ) )  SET_REG_BYTEHI( pDebReg->ecx );
    if( !strnicmp( pRegName, "dl", 2 ) )  SET_REG_BYTELO( pDebReg->edx );
    if( !strnicmp( pRegName, "dh", 2 ) )  SET_REG_BYTEHI( pDebReg->edx );

    // Stack record fields

    if( !strnicmp( pRegName, "ss", 2 ) )  SET_REG_WORD( pDebStack->ss );
    if( !strnicmp( pRegName, "esp", 3 ) )  SET_REG_DWORD( pDebStack->esp );
    if( !strnicmp( pRegName, "sp", 2 ) )  SET_REG_WORD( pDebStack->esp );

    // Segment registers/selectors

    if( !strnicmp( pRegName, "gs", 2 ) )  SET_REG_WORD( pDebSeg->gs );
    if( !strnicmp( pRegName, "fs", 2 ) )  SET_REG_WORD( pDebSeg->fs );
    if( !strnicmp( pRegName, "ds", 2 ) )  SET_REG_WORD( pDebSeg->ds );
    if( !strnicmp( pRegName, "es", 2 ) )  SET_REG_WORD( pDebSeg->es );

    // Code record fields

    if( !strnicmp( pRegName, "cs", 2 ) )  SET_REG_WORD( pDebCode->cs );
    if( !strnicmp( pRegName, "eip", 3 ) )  SET_REG_DWORD( pDebCode->eip );
    if( !strnicmp( pRegName, "ip", 2 ) )  SET_REG_WORD( pDebCode->eip );

    // Flags

    if( !strnicmp( pRegName, "efl", 3 ) )  SET_REG_DWORD( pDebCode->eflags );
    if( !strnicmp( pRegName, "fl", 2 ) )  SET_REG_WORD( pDebCode->eflags );

    // Individual flags

    if( !strnicmp( pRegName, "CF", 2 ) )  SET_REG_FLAG( CF_MASK, CF_BIT );
    if( !strnicmp( pRegName, "PF", 2 ) )  SET_REG_FLAG( PF_MASK, PF_BIT );
    if( !strnicmp( pRegName, "AF", 2 ) )  SET_REG_FLAG( AF_MASK, AF_BIT );
    if( !strnicmp( pRegName, "ZF", 2 ) )  SET_REG_FLAG( ZF_MASK, ZF_BIT );
    if( !strnicmp( pRegName, "SF", 2 ) )  SET_REG_FLAG( SF_MASK, SF_BIT );
    if( !strnicmp( pRegName, "IF", 2 ) )  SET_REG_FLAG( IF_MASK, IF_BIT );
    if( !strnicmp( pRegName, "DF", 2 ) )  SET_REG_FLAG( DF_MASK, DF_BIT );
    if( !strnicmp( pRegName, "OF", 2 ) )  SET_REG_FLAG( OF_MASK, OF_BIT );
    if( !strnicmp( pRegName, "IOPL",4) )  SET_REG_IOPL( IOPL_MASK, IOPL_BIT0 );
    if( !strnicmp( pRegName, "NF", 2 ) )  SET_REG_FLAG( NT_MASK, NT_BIT );
    if( !strnicmp( pRegName, "VF", 2 ) )  SET_REG_FLAG( VM_MASK, VM_BIT );

    return( FALSE );
}



/******************************************************************************
*                                                                             *
*   int CopyContext()                                                         *
*                                                                             *
*******************************************************************************
*
*   This function copies the stack to a current process context:
#
#   1) If the stack represents the interrupted kernel, nothing is copied and
#       return 0
#   2) If the stack represents the interrupted V86 process, copy to pCurProc
#       and return 1
#   3) If the stack represents the interrupted PM process, copy to pCurProcPM
#       and return 2
*
*   Where:
#       no args
*
*   Returns:
*       0 - kernel is interrupted
#       1 - V86 is interrupted
#       2 - PM is interrupted
*
******************************************************************************/
int CopyContext()
{
    // Check for V86 mode process

    if( pDebCode->eflags & VM_MASK )
    {
        // Copy the stack into pCurProc and return 1

        memcpy( &pCurProc->Reg, pDebFrame, sizeof(TIntStack) );

        return( 1 );
    }

    // Check for the ring-3 PM application

    if( (pDebCode->cs & 3) == 3 )
    {
        // Copy the stack into pCurProcPM and return 2

        memcpy( &pCurProcPM->Reg, pDebFrame, sizeof(TIntStack) - sizeof(TSeg) );

        return( 2 );
    }

    // Finally, the kernel was interrupted.  Do not copy anything.

    return( 0 );
}

