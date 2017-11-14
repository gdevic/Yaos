/******************************************************************************
*                                                                             *
*   Module:     Debug.c                                                       *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       8/28/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

    This file contains the main source for the debugger.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 8/28/97    1.00  Original                                       Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include <string.h>                     // String functions

#include <ctype.h>                      // Include character types header

#include "command.h"                    // Command line editor

#include "break.h"                      // Include breakpoints header file

#include "debug.h"                      // Include debugger header file

#include "dis386.h"                     // Include disassember code header

#include "dump.h"                       // Include value fetch and dump header

#include "eval.h"                       // Include expression evaluator

#include "info.h"                       // Include information servces header

#include "inline.h"                     // Inline functions

#include "int.h"                        // Include interrupts definitions

#include "intel.h"                      // memory access functions

#include "keyboard.h"                   // Include keyboard defines

#include "kernel.h"                     // Include kernel defines

#include "map.h"                        // Include symbol map header file

#include "mm.h"                         // Include memory manager header

#include "pc.h"                         // Include hardware constants

#include "tty.h"                        // Include terminal file header

#include "device.h"                     // Include device header file

#include "process.h"                    // Include process header

#include "register.h"                   // Include register display header

#include "set.h"                        // Include set command header file

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

// Set the following define to 1 to use local debugger stack.  Use it to
// debug kernel stack problems

#define USE_LOCAL_STACK      0

//-----------------------------------------------------------------------------
// Local debugger stack
//-----------------------------------------------------------------------------

#if USE_LOCAL_STACK
BYTE MemDeb[HEAP_DEBUG_LEN];
#endif


//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

extern DWORD FaultyAddress;             // Page fault faulty address
DWORD CPU_ErrorCode;                    // Possible error code pushed on stack

BYTE *pMemDeb;                          // Address of the debugger stack

TDebug Deb;                             // Debugger state structure

TIntStack * pDebFrame;                  // Original interrupted stack frame
TStack * pDebStack;                     // Pointer to interrupted program's
TCode  * pDebCode;                      //  stack, code, register and segment
TReg   * pDebReg;                       //  descriptor structures
TSeg   * pDebSeg;

int context;                            // Interrupted process context:
                                        //  0 - kernel was interrupted
                                        //  1 - V86 process was interrupted
                                        //  2 - PM process was interrupted

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

static char sCmd[80];                   // Command line
static char *pCmd;                      // Pointer to command itself
static char *pArg;                      // Pointer to command argument


typedef enum { SEG_NULL, SEG_CS, SEG_DS, SEG_ES, SEG_FS, SEG_GS, SEG_SS, SEG_GLOBAL } TSEG;

static char *sSeg[] = {"", "cs:", "ds:", "es:", "fs:", "gs:", "ss:", "@" };
static TSEG iSeg;                       // Current overriden segment/selector

static TSEG  iSegCode;                  // Current code segment/selector index
static DWORD dwOffsetCode;              // Current code offset
static TSEG  iSegData;                  // Current data segment/selector index
static DWORD dwOffsetData;              // Current data offset


//-----------------------------------------------------------------------------
// Structure containing the commands and function pointers
//-----------------------------------------------------------------------------

typedef struct
{
    char * sName;
    int    nNameLen;
    BOOL   (*fnCmd)();
    char * sHelp;

} TCmd;

//-----------------------------------------------------------------------------
// Define command functions
//-----------------------------------------------------------------------------

static BOOL fnEval();
static BOOL fnAddr();
static BOOL fnBreak();
static BOOL fnCls();
static BOOL fnDump();
static BOOL fnFn();
static BOOL fnGDT();
static BOOL fnGo();
static BOOL fnIDT();
static BOOL fnHelp();
static BOOL fnMap();
static BOOL fnMem();
static BOOL fnProc();
static BOOL fnReboot();
static BOOL fnRehash();
static BOOL fnReg();
static BOOL fnSet();
static BOOL fnSym();
static BOOL fnPoke();
static BOOL fnStep();
static BOOL fnTerm();
static BOOL fnTrace();
static BOOL fnUnload();
static BOOL fnUnassemble();


//-----------------------------------------------------------------------------
// Define command structure
//-----------------------------------------------------------------------------

static const TCmd Cmd[] =
{
    { "?",      1, fnEval,      "[expression]    - Evaluates an expression and lists the closest symbols" },
    { "addr",   4, fnAddr,      "[pid | 0]    - Displays or changes process address context" },
    { "bc",     2, fnBreak,     "list | *       - Clears one or more breakpoints" },
    { "bd",     2, fnBreak,     "list | *       - Disables one or more breakpoints" },
    { "be",     2, fnBreak,     "list | *       - Enables one or more breakpoints" },
    { "bl",     2, fnBreak,     "               - Lists all breakpoints" },
    { "bp",     2, fnBreak,     "<address>      - Sets a breakpoint" },
    { "cls",    3, fnCls,       "              - Clears the debugger screen" },
    { "db",     2, fnDump,      "<address>      - Dumps memory as a stream of bytes" },
    { "dw",     2, fnDump,      "<address>      - Dumps memory as a stream of words" },
    { "dd",     2, fnDump,      "<address>      - Dumps memory as a stream of dwords" },
    { "dr",     2, fnDump,      "               - Displays the debug registers" },
    { "d",      1, fnDump,      "                - Continues previous memory dump" },
    { "fn",     2, fnFn,        "[name][args ..]- Executes a kernel function with up to 3 optional arguments" },
    { "gdt",    3, fnGDT,       "              - Dumps the global descriptor table and embedded LDTs" },
    { "idt",    3, fnIDT,       "              - Dumps the interrupt descriptor table" },
    { "g",      1, fnGo,        "<break addr>    - Runs the program up to an optional break address" },
    { "h",      1, fnHelp,      "                - Displays this list of commands" },
    { "map",    3, fnMap,       "[file] [pid|0]- Loads process symbol information for a given process id" },
    { "map",    3, fnMap,       "[-] <pid>     - Clears the symbol table, all or for specific process id" },
    { "mem",    3, fnMem,       "              - Displays memory info" },
    { "proc",   4, fnProc,      "<pid>        - Displays process table or a specific process information" },
    { "reboot", 6, fnReboot,    "           - Reboots the computer" },
    { "rehash", 6, fnRehash,    "           - Reloads debugger system variables" },
    { "r",      1, fnReg,       "<reg>=<val>     - Sets a CPU register to a given value" },
    { "set",    3, fnSet,       "<key>[=<val>] - Sets or deletes a debugger variable" },
    { "sym",    3, fnSym,       "<patterm>     - Lists symbols that match search pattern" },
    { "sb",     2, fnPoke,      "[adr] [val ..] - Sets stream of bytes" },
    { "sw",     2, fnPoke,      "[adr] [val ..] - Sets stream of words" },
    { "sd",     2, fnPoke,      "[adr] [val ..] - Sets stream of dwords" },
    { "step",   4, fnStep,      "             - Executes one program step" },
    { "tty",    3, fnTerm,      "[tty0 | tty?] - Shows the content of a terminal and sets it controlling" },
    { "t",      1, fnTrace,     "                - Executes current instruction" },
    { "unload", 6, fnUnload,    "           - Unloads OS" },
    { "u",      1, fnUnassemble,"<address>       - Disassembles code" },
    { NULL }
};


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

extern void Debug_Trace_Handler();

extern void Debug_Breakpoint_Handler();

extern void DebugRun();

static DWORD GetLinAddress( TSEG iSeg, DWORD dwOffset );

/******************************************************************************
*                                                                             *
*   BOOL fnEval()                                                             *
*                                                                             *
*******************************************************************************
*
*   This function evaluates given expression.
*
******************************************************************************/
static BOOL fnEval()
{
    int nResult, i, j, d;
    DWORD dwAddr, dwVal, dwVal2;
    char *pSym;
    static char sChr[5] = "0123";

    // If the function does not have arguments, display help

    if( *pArg=='\0' )
    {
        fnHelp();
    }
    else
    {
        nResult = nEvaluate( pArg, NULL );

        // Setup the character form

        if( (sChr[0] = (nResult >> 24) & 0xFF) < 32) sChr[0] = '.';
        if( (sChr[1] = (nResult >> 16) & 0xFF) < 32) sChr[1] = '.';
        if( (sChr[2] = (nResult >> 8)  & 0xFF) < 32) sChr[2] = '.';
        if( (sChr[3] = (nResult >> 0)  & 0xFF) < 32) sChr[3] = '.';

        // Finally, print the number in all of its forms except binary

        dprintf("\n%10d  0x%08X  %s    ", nResult, nResult, sChr );

        // Print the binary form

        for( i=31; i>=0; i-- )
        {
            if( (nResult >> i) & 1 )
                dprintf("1");
            else
                dprintf("0");

            if( i==16 )
                dprintf(" - ");
            else
                if( (i & 3)==0 && i)
                    dprintf("-");
        }

        // Go through all the process appertures

        if( nResult > 0 )
        {
            nResult = nResult & 0x3FFFFF;

            for( i=0; i<256; i++ )
            {
                dwAddr = nResult | (i << 22);

                pSym = GetClosestSymbolName( &d, dwAddr );

                if( pSym != NULL )
                {
                    dwAddr -= d;

                    dprintf("\nSym: %08X  %s", dwAddr, pSym );

                    if( d )
                        dprintf(" (%+d)", d );
                    else
                    {
                        // Print a dword from that address

                        for( dwVal = j = 0; j<4; j++ )
                        {
                            dwVal2 = GetByte( dwAddr + j );

                            if( dwVal2==BYTE_NOT_PRESENT )
                                break;

                            dwVal >>= 8;
                            dwVal |= dwVal2 << 24;
                        }

                        if( j==4 ) dprintf(" -> %08X", dwVal );
                    }
                }
            }
        }
    }

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL fnAddr()                                                             *
*                                                                             *
*******************************************************************************
*
*   This function displays address context.  If the optional pid is given,
#   it switches to the given process address context.
*
******************************************************************************/
static BOOL fnAddr()
{
    static const char *sContext[3] = { "Kernel", "V86 process", "PM process" };
    int pid;


    // Display current address context

    dprintf("\n%s interrupted.  Current VM: %d  Active PM: %d",
        sContext[context], pCurProc->pid, pCurProcPM->pid );

    // Get the optional process id of a new context

    if( *pArg != '\0' )
    {
        pid = nEvaluate( pArg, NULL );

        // Kernel address context is 0

        if( pid==0 )
        {
            pDebSeg->ds = pDebSeg->es = SEL_DATA;
            pDebSeg->fs = pDebSeg->gs = SEL_GLOBAL;
            pDebStack->ss = SEL_DATA;
            pDebCode->cs = SEL_CODE;
        }
        else
        {
            if( context != 0 )
            {
                // If we are not in the kernel process, we can switch
                // first switch the base VM process

                SetCurrentVM( pProc[pid]->VM_pid, pDebFrame );

                // Now switch to the PM process

                if( (context==2) && (pProc[pid]->Flags & PROCESS_PM) )
                {
                    // Load the LDT and copy the registers

                    SetLDT( pProc[pid]->LDT );

                    memcpy( pDebFrame, &pProc[pid]->Reg, sizeof(TPMStack) );

                    pCurProcPM = pProc[pid];
                }
            }

            dprintf("\nNew VM: %d  New PM: %d", pCurProc->pid, pCurProcPM->pid );
        }
    }

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL fnBreak()                                                            *
*                                                                             *
*******************************************************************************
*
*   This function calls the breakpoint module
*
******************************************************************************/
static BOOL fnBreak()
{
    // Call breakpoint function depending on the command

    switch( tolower(pCmd[1]) )
    {
        case 'c':       return( BreakClear( pArg ) );
        case 'd':       return( BreakDisable( pArg ) );
        case 'e':       return( BreakEnable( pArg ) );
        case 'l':       return( BreakList( pArg ) );
        case 'p':       return( BreakSet( pArg ) );
    }

    return( FALSE );
}


/******************************************************************************
*                                                                             *
*   BOOL fnCls()                                                              *
*                                                                             *
*******************************************************************************
*
*   This function clears the debugger screen
*
******************************************************************************/
static BOOL fnCls()
{
    dprintf("\n%c", DP_CLS );

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL fnDump()                                                             *
*                                                                             *
*******************************************************************************
*
*   This function dumps the memory contents from the given location
*
******************************************************************************/
static BOOL fnDump()
{
    DWORD dwLinAddress;
    static int size = 4;

    // Make the second character of a command lowercased

    pCmd[1] = tolower(pCmd[1]);

    // See if the second char is `r' ro display debug registers

    if( pCmd[1] == 'r' )
    {
        dprintf("\nDR0=%08X", GetDebugReg(0) );
        dprintf("\nDR1=%08X", GetDebugReg(1) );
        dprintf("\nDR2=%08X", GetDebugReg(2) );
        dprintf("\nDR3=%08X", GetDebugReg(3) );
        dprintf("\nDR6=%08X", GetDebugReg(6) );
        dprintf("\nDR7=%08X", GetDebugReg(7) );

        return( TRUE );
    }

    // Get the value size from the command: byte, word or double word

    if( pCmd[1]=='b' )
        size = 1;
    else
        if( pCmd[1]=='w' )
            size = 2;
        else
            if( pCmd[1]=='d' )
                size = 4;

    // If there were no additional parameters, continue where stopped last time

    if( *pArg != '\0' )
    {
        // Get the address value

        iSeg = SEG_NULL;
        dwOffsetData = nEvaluate( pArg, NULL );

        // Check if a segment/selector is explicitly given

        if( iSeg != SEG_NULL )
            iSegData = iSeg;
    }

    // Compute the linear address from the segment/selector:offset pair

    dwLinAddress = GetLinAddress( iSegData, dwOffsetData );

    // Dump the memory block

    Dump( dwLinAddress, dwOffsetData, Deb.nDumpLines, size, sSeg[iSegData] );

    // Advance the default address

    dwOffsetData += Deb.nDumpLines * 16;

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL fnFn()                                                               *
*                                                                             *
*******************************************************************************
*
*   This function executes a kernel function with given arguments that must be
#   integers.
#
#   For this command to work, kernel map file must be loaded.  Optional
#   argument may be supplied.
*
******************************************************************************/
static BOOL fnFn()
{
    char *pVal;
    int Args[3], nResult;
    int funct, i;

    // Get the address of a kernel function

    funct = nEvaluate( pArg, &pVal );

    // If the address cannot be found, return

    if( (funct<LIN_KERNEL) || (funct>LIN_KERNEL+ 4 * 1024 * 1024))
    {
        dprintf("\nFunction `%s' cannot be found.  Did you load a kernel map file?", pArg);
        return( TRUE );
    }

    funct = funct - LIN_KERNEL;

    // Get optional arguments

    for( i=0; i<3; i++ )
    {
        Args[i] = nEvaluate( pVal, &pVal );
    }

    dprintf("\nExecuting at CS:%Xh with (%d,%d,%d)",
        funct, Args[0], Args[1], Args[2] );

    nResult = ExecFunction(&funct, Args[0], Args[1], Args[2] );

    dprintf("\nFunction returns: %d (%08Xh)", nResult, nResult );

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL fnGDT()                                                              *
*                                                                             *
*******************************************************************************
*
*   This function dumps the Global Descriptor Table
*
******************************************************************************/
static BOOL fnGDT()
{
    PrintGDT();

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL fnIDT()                                                              *
*                                                                             *
*******************************************************************************
*
*   This function dumps the Interrupt Descriptor Table
*
******************************************************************************/
static BOOL fnIDT()
{
    PrintIDT();

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL fnGo()                                                               *
*                                                                             *
*******************************************************************************
*
*   This function executes a program
*
******************************************************************************/
static BOOL fnGo()
{
    DWORD dwLinAddress, dr7;

    // If the additional parameter is given use it to set the hardware debug
    // breakpoint

    if( *pArg != '\0' )
    {
        // Get the break address

        dwLinAddress = nEvaluate( pArg, NULL );

        // If the offset is above 4Mb, we got request to stop in the another
        // process, so do not do local address translation

        if( dwLinAddress < 4*1024*1024 )
        {
            // Compute the linear address from the segment/selector:offset pair

            dwLinAddress = GetLinAddress( SEG_CS, dwLinAddress );
        }

        // Set the debuger breakpoint 2

        SetDebugReg(2, dwLinAddress);

        dr7 = GetDebugReg(7);
        dr7 &= ~DR7_RW2_MASK;               // Set a break on excute
        dr7 |=  DR7_L2_MASK;                // Enable local breakpoint
                                            // (We dont do a task switch)
        SetDebugReg(7, dr7);

        // Clear the debug status register

        SetDebugReg(6, 0);
    }

    // Set up all software breakpoints
    // Set the interrupt flag on the client eflags and disable traps

    pDebCode->eflags = (pDebCode->eflags | IF_MASK)
                     &~(NT_MASK | RF_MASK | TF_MASK);

    // Set up the default keyboard handler

    Register_Interrupt_Handler( 0x1, Int_Keyboard_Handler );

    // Set up the software breakpoints

    BreakSetup();

    // Jump into the interrupted code

    DebugRun();

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL fnHelp()                                                             *
*                                                                             *
*******************************************************************************
*
*   This function displays the help
*
******************************************************************************/
static BOOL fnHelp()
{
    int i;

    // Traverse the list of functions and display their associated help

    for( i=0; Cmd[i].sName != NULL; i++ )
    {
        dprintf("\n%c%c%s%c%c %s",
            DP_SETWRITEATTR, ATTR_RESPONSEHI,
            Cmd[i].sName,
            DP_SETWRITEATTR, ATTR_RESPONSE,
            Cmd[i].sHelp );
    }

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL fnMap()                                                              *
*                                                                             *
*******************************************************************************
*
*   This function loads process symbol map.  If the parameter given is only
#   a minus sign, it clears the symbol table.
*
******************************************************************************/
static BOOL fnMap()
{
    int i, pid, fixup;
    char *pVal;

    // If the function does not have arguments, display help

    if( *pArg != '\0' )
    {
        // Clear the symbol table if the parameter is `-'

        if( *pArg == '-' )
        {
            // Get the optional process id

            pid = nEvaluate( pArg + 1, NULL );

            ClearSymbols( pid );

            return( TRUE );
        }

        // Skip the file name parameter

        pVal = strchr(pArg, ' ');

        if( pVal != NULL )
        {
            *pVal = '\0';

            // Get the process id number

            pid = nEvaluate( pVal + 1, NULL );

            // Kernel is a special case

            if( pid == 0 )
                fixup = LIN_KERNEL;
            else
                fixup = pid << 22;

            i = LoadMap( pArg, fixup );

            if( i == 0 )
                dprintf("\nUnable to load map file %s", pArg );
            else
                dprintf("\nLoaded %d symbols from file %s fixed up by %08X for pid %d",
                    i, pArg, fixup, pid );

            return( TRUE );
        }

        dprintf(" - What pid ?");
    }

    return( FALSE );
}


/******************************************************************************
*                                                                             *
*   BOOL fnMem()                                                              *
*                                                                             *
*******************************************************************************
*
*   This function displays the memory information
*
******************************************************************************/
static BOOL fnMem()
{
    DisplayMemoryInfo();

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL fnProc()                                                             *
*                                                                             *
*******************************************************************************
*
*   This function displays the process information.  If an optional process ID
#   is given it displays extended process information.
*
******************************************************************************/
static BOOL fnProc()
{
    int pid;


    // If there were no additional parameters, display all processes

    if( *pArg != '\0' )
    {
        // Get the specific process ID

        pid = nEvaluate( pArg, NULL );

        DisplayProcessInfo( pid, TRUE );
    }
    else
    {
        for( pid=0; pid<PROCESS_ID_MAX; pid++ )
        {
            if( pProc[pid] != 0 )
            {
                DisplayProcessInfo( pid, FALSE );
            }
        }
    }

    return( TRUE );
}



/******************************************************************************
*                                                                             *
*   BOOL fnReboot()                                                           *
*                                                                             *
*******************************************************************************
*
*   This function (hopefully) reboots the computer
*
******************************************************************************/
static BOOL fnReboot()
{
    dprintf("\nPlease wait while the system is rebooting...");

    // Short & effective

    outp( 0x64, 0xFE );

    return( TRUE );
}



/******************************************************************************
*                                                                             *
*   BOOL fnRehash()                                                           *
*                                                                             *
*******************************************************************************
*
*   Reads the system setting from the system variables and set the state
*
#   Dump = [1|49]  - Default number of lines to dump in a dump command (d?)
#   List = [1|49]  - Default number of lines to list in a list command (ldt..)
#   Code32 = <0|1> - Default code size for unassemble command
#   Data32 = <0|1> - Default data size for unassemble command
#   Lines = [25,50]- Number of lines on the debugger screen
#   Echo = <0|1>   - Echoes the command line to the screen
#   Regs = <0|1>   - Displays the registers on the top of the screen
*
******************************************************************************/
static BOOL fnRehash()
{
    // Get the default number of lines for the dump command (db, dw, dd)

    Deb.nDumpLines = SetGetInteger("Dump");

    // Get the default number of lines for the list commands (ldt, sym)

    Deb.nListLines = SetGetInteger("List");

    // Get the default code and data sizes for the unassemble command (u)

    Deb.fCode32 = SetGetInteger("Code32");
    Deb.fData32 = SetGetInteger("Data32");

    // Get the number of lines from a system variables

    Deb.nNumLines = SetGetInteger("Lines");

    // Check for sanity

    if( Deb.nNumLines < 5 || Deb.nNumLines > 50 )
        Deb.nNumLines = 25;

    dprintf("%c%c", DP_SETLINES, Deb.nNumLines );

    // Get and set other system parameters

    Deb.fEcho = SetGetInteger("Echo");
    Deb.fRegs = SetGetInteger("Regs");

    return( TRUE );
}



/******************************************************************************
*                                                                             *
*   BOOL fnReg()                                                              *
*                                                                             *
*******************************************************************************
*
#   Sets a given register to a specified value.  This command also resets
#   code segment/offset to cs:eip value.
*
******************************************************************************/
static BOOL fnReg()
{
    char * pVal;

    // Reset the code segment/offset pair

    iSegCode = SEG_CS;
    dwOffsetCode = pDebCode->eip;

    // Find the separator of register/value

    pVal = pArg + strcspn( pArg, "=" );

    if( pVal != pArg )
    {
        // Set the register with the given value

        if( RegSet( pArg, nEvaluate(pVal+1, NULL) ) == FALSE )
            return( FALSE );
    }

    // Display registers at the normal, response line

    RegDisplay( FALSE );

    return( TRUE );
}



/******************************************************************************
*                                                                             *
*   BOOL fnSym()                                                              *
*                                                                             *
*******************************************************************************
*
*   This function prints all symbols that match search pattern.
#
******************************************************************************/
static BOOL fnSym()
{
    SymPrint( pArg );

    return( TRUE );
}



/******************************************************************************
*                                                                             *
*   BOOL fnSet()                                                              *
*                                                                             *
*******************************************************************************
*
*   This function sets or deletes a system variable.
#
#   There are a number of predefined system variables and they are rehashed
#   (re-read) every time a set command is invoked:
#
******************************************************************************/
static BOOL fnSet()
{
    char * pVal, *pTrail;


    // `Set' command alone lists all variables

    if( *pArg=='\0' )
    {
        // Traverse the set linked list and print all key/value pairs

        SetPrintAll();
    }
    else
    {
        // Find the separator of key/value

        pVal = pArg + strcspn( pArg, "=" );
        pTrail = pVal - 1;

        // Strip any trailing spaces of a key

        while( pTrail>=sCmd && *pTrail==' ') *pTrail-- = '\0';

        // Make sure the '=' is reset to a zero-terminator

        if( *pVal!='\0' )
            *pVal++ = '\0';

        // Skip possible spaces in the value field

        pVal = pVal + strspn( pVal, " " );

        // Add the new string

        if( SetValue( pArg, pVal ) == FALSE )
            dprintf("\nValue could not be set!");
    }

    // Do some internal checking

    if( QCheck( &qSet ) != 0 )
        dprintf("\nWarning: Internal queue structure error!");

    // Rehash the set variables into internal data structures

    fnRehash();

    return( TRUE );
}



/******************************************************************************
*                                                                             *
*   BOOL fnPoke()                                                             *
*                                                                             *
*******************************************************************************
*
*   This function sets a stream of bytes, words or dwords
*
******************************************************************************/
static BOOL fnPoke()
{
    DWORD dwLinAddress;
    char * pVal;
    DWORD dwValue;


    // If there were no additional parameters, return

    if( *pArg == '\0' )
        return( FALSE );

    // Get the address value

    iSeg = SEG_NULL;
    dwOffsetData = nEvaluate( pArg, &pVal );

    // Check if a segment/selector is explicitly given

    if( iSeg != SEG_NULL )
        iSegData = iSeg;

    // Compute the linear address from the segment/selector:offset pair

    dwLinAddress = GetLinAddress( iSegData, dwOffsetData );

    // There should be at least one value

    if( *pVal == '\0' )
        return( FALSE );

    // Make the second character of a command lowercased

    pCmd[1] = tolower(pCmd[1]);

    // Loop through the additional values and set them

    while( *pVal != '\0' )
    {
        dwValue = nEvaluate( pVal, &pVal );

        // Get the value size from the command: byte, word or double word

        switch( pCmd[1] )
        {
            case 'b':
                //                 Set a byte

                dprintf("\n%08X: %02X -> %02X",
                    dwLinAddress, abs_peekb(dwLinAddress), dwValue );

                if( dwValue & ~0xFF )
                {
                    dprintf(" invalid value");
                    return( TRUE );
                }

                abs_pokeb( dwLinAddress, (BYTE) dwValue );
                dwLinAddress += 1;

                break;

            case 'w':
                //                 Set a word

                dprintf("\n%08X: %04X -> %04X",
                    dwLinAddress, abs_peekw(dwLinAddress), dwValue );

                if( dwValue & ~0xFFFF )
                {
                    dprintf(" invalid value");
                    return( TRUE );
                }

                abs_pokew( dwLinAddress, (WORD) dwValue );
                dwLinAddress += 2;

                break;

            default:
                //                 Set a dword

                dprintf("\n%08X: %08X -> %08X",
                    dwLinAddress, abs_peekdw(dwLinAddress), dwValue );

                abs_pokedw( dwLinAddress, dwValue );
                dwLinAddress += 4;

                break;
        }
    }

    // Always flush TLB since we may have update page tables

    FlushTLB();

    return( TRUE );
}



/******************************************************************************
*                                                                             *
*   BOOL fnStep()                                                             *
*                                                                             *
*******************************************************************************
*
*   This function executes one program step.  It uses debug register 3 to place
#   a breakpoint after the current instruction.
#
#   If the current instruction is a call, interrupt or repeated instruction,
#   the entire call or iteration is repeated.
#
#   If the current instruction is jump, return or loop, default trap command
#   is executed, so that the execution may be traced and not `lost'.
*
******************************************************************************/
static BOOL fnStep()
{
    DWORD dwEIPAddress, dr7;
    char sDis[80];
    TDis386 Dis;


    // Find the address to set up a breakpoint

    // Get the absolute CS:EIP address

    dwEIPAddress = GetLinAddress( SEG_CS, pDebCode->eip );

    // Set the Dis structure that defines the disasembly line

    Dis.bpTarget = (BYTE *)dwEIPAddress;
    Dis.bDataSize = Deb.fData32;
    Dis.bAddressSize = Deb.fCode32;
    Dis.szDisasm = sDis;

    // Disassemble from memory at the given address

    Dis386( &Dis );

    // If the current instruction is non-returnable change of flow,
    // use the trace command instead

    if( (sDis[0]=='j')                  // Any jump instruction: j*
     || (!strnicmp(sDis,"loop",4))      // Loops: loop*
     || (!strnicmp(sDis+1,"ret",3))     // Returns: ?ret*
     || (!strnicmp(sDis,"ret",3)) )     // Returns: ret*
    {
        fnTrace();
    }

    // Add the instruction len to the address

    dwEIPAddress += Dis.bInstrLen;

    // Set the debuger breakpoint 3

    SetDebugReg(3, dwEIPAddress);

    dr7 = GetDebugReg(7);
    dr7 &= ~DR7_RW3_MASK;               // Set a break on excute
    dr7 |=  DR7_L3_MASK;                // Enable local breakpoint
                                        // (We dont do a task switch)
    SetDebugReg(7, dr7);

    // Clear the debug status register

    SetDebugReg(6, 0);

    // Set the interrupt flag on the client eflags and disable traps

    pDebCode->eflags = (pDebCode->eflags | IF_MASK)
                     &~(NT_MASK | RF_MASK | TF_MASK);

    // Set up the default keyboard handler

    Register_Interrupt_Handler( 0x1, Int_Keyboard_Handler );

    // Jump into the interrupted code

    DebugRun();

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL fnTerm()                                                             *
*                                                                             *
*******************************************************************************
*
*   This function shows the content of a terminal
#
******************************************************************************/
static BOOL fnTerm()
{
    int n;

    n = nEvaluate( pArg, NULL );

    if( TTY[n] != NULL )
    {
        abs_memcpy( LIN_SCREEN, (DWORD)TTY[n]->screen + LIN_KERNEL, SCREEN_SIZE );

        // Set the terminal controlling

        pCurTTY = TTY[n];
    }
    else
        dprintf("\nTerminal tty%d undefined", n );

    return( TRUE );
}



/******************************************************************************
*                                                                             *
*   BOOL fnTrace()                                                            *
*                                                                             *
*******************************************************************************
*
*   This function executes one program instruction. It does that by setting
#   the trap bit in the flags register to trap back to debugger after a
#   single instruction has been executed.
*
******************************************************************************/
static BOOL fnTrace()
{
    // Set the trace flag on the client eflags, so we execute a single
    // instruction before generating an int 1.

    pDebCode->eflags = (pDebCode->eflags | TF_MASK | IF_MASK)
                     &~(NT_MASK | RF_MASK);

    // Set up the default keyboard handler

    Register_Interrupt_Handler( 0x1, Int_Keyboard_Handler );

    // Jump into the interrupted program code

    DebugRun();

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL fnUnload()                                                           *
*                                                                             *
*******************************************************************************
*
*   This function unloads the OS if it was started from the DOS command line.
*
******************************************************************************/
static BOOL fnUnload()
{
    pCurProc = NULL;
    SetCurrentVM( PROCESS_ID_TEMPLATE, NULL );


    // Disable interrupts during the unload process

    DisableInterrupts();

    // Restore the state of the main interrupt controller mask

    outp(PIC1_MASK, bSaved8259);

    // Unload the kernel

    Unload();

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL fnUnassemble()                                                       *
*                                                                             *
*******************************************************************************
*
*   This function disassembles the code from the given location
*
******************************************************************************/
static BOOL fnUnassemble()
{
    DWORD dwLinAddress, dwEIPAddress;
    char sDis[80], sSymbol[80];
    char *psLen, *pSym;
    TDis386 Dis;
    int nLines, i, bytes;
    static const char attr[2][2] =
    { {ATTR_RESPONSE, ATTR_RESPONSEHI },
      {ATTR_DIS, ATTR_DISHI } };
    int index = 0;
    int nOldOffset = -1;


    // Get the number of lines to dump

    nLines = Deb.nDumpLines;

    // If there were no additional parameters, continue where stopped last time
    // If there are some parameters, set the new address up

    if( *pArg != '\0' )
    {
        // If the pArg[0] is '^', we should use register attributes for the top
        // and also set the number of lines to disassemble to a constant value

        if( pArg[0] == '^' )
            index = 1, nLines = Deb.nDumpLines, nOldOffset = dwOffsetCode;

        // Get the address value

        iSeg = SEG_NULL;
        dwOffsetCode = nEvaluate( pArg + index, &psLen );

        // Check if a segment/selector is explicitly given

        if( iSeg != SEG_NULL )
            iSegCode = iSeg;

        // Get possible number of lines parameter

        if( *psLen != '\0' )
        {
            nLines = nEvaluate( psLen, NULL );

            if( nLines < 1 || nLines > 30 )
                nLines = Deb.nDumpLines;
        }
    }

    // Get the absolute CS:EIP address

    dwEIPAddress = GetLinAddress( SEG_CS, pDebCode->eip );


    for( i=0; i<nLines; i++ )
    {
        // Compute the linear address from the segment/selector:offset pair

        dwLinAddress = GetLinAddress( iSegCode, dwOffsetCode );

        // Set the Dis structure that defines the disasembly line

        Dis.bpTarget = (BYTE *)dwLinAddress;
        Dis.bDataSize = Deb.fData32;
        Dis.bAddressSize = Deb.fCode32;
        Dis.szDisasm = sDis;

        // Disassemble from memory at the given address

        Dis386( &Dis );

        // Look for a symbol name with the specified address and print it

        pSym = GetSymbolName( dwLinAddress );

        if( pSym != NULL )
        {
            strcpy( sSymbol, pSym );
            strcat( sSymbol, ":");
            dprintf("\n%c%c%-79s", DP_SETWRITEATTR, attr[index][1], sSymbol );

            if( ++i == nLines )
                break;
        }

        // If the segment is used, print the seg:offset.  Otherwise, print
        // only the linear address (iSegCode would be SEG_NULL)

        dprintf("\n%c%c%s%08X ", DP_SETWRITEATTR,
            attr[index][dwEIPAddress == dwLinAddress? 1:0],
            sSeg[iSegCode], dwOffsetCode );

        // Print instruction bytes, up to 6

        for( bytes=0; bytes<6; bytes++ )
        {
            // Print the instruction bytes

            if( bytes < Dis.bInstrLen )
                dprintf(" %02X", Dis.sbInstr[bytes] );
            else
                dprintf("   ");
        }

        // If there were more than 6 bytes, print '..'.  After it,
        // print the disassembled instruction string

        dprintf("%s %c%c%s\r",
            Dis.bInstrLen>6?"..":"  ",
            DP_SETWRITEATTR, attr[index][1], sDis );

        // Advance the default address

        dwOffsetCode += Dis.bInstrLen;
    }

    // If we have printed on the top, restore old offset variable

    if( nOldOffset != -1 )
        dwOffsetCode = nOldOffset;

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   void Debug_Int1()                                                         *
*                                                                             *
*******************************************************************************
*
*   This is the int 1, instruction trace handler.
*
#   On entry, interrupts are disabled and the stack frame is set in Int.asm.
#
******************************************************************************/
void Debug_Int1()
{
//  dprintf("\nInt 1");

    Debug();
}


/******************************************************************************
*                                                                             *
*   void Debug_Int3()                                                         *
*                                                                             *
*******************************************************************************
*
*   This is the int 3, breakpoint handler.
*
#   On entry, interrupts are disabled and the stack frame is set in Int.asm.
#
******************************************************************************/
void Debug_Int3()
{
    int bp;
    DWORD dwAddress;

    dprintf("\nINT 3 Breakpoint");

    // Look into the debugger software breakpoints to find if the one was
    // placed there by us or it was there from the app.  If it was placed
    // by us, we will decrement EIP since that INT 3 will be cleared and
    // the original opcode will be placed there

    dwAddress = GetLinAddress( SEG_CS, pDebCode->eip ) - 1;

    if( (bp = ControlBP(dwAddress, ACT_FIND) != FALSE ) )
    {
        dprintf(" - debugger breakpoint # %d", bp );
        pDebCode->eip -= 1;
    }

    Debug();
}


/******************************************************************************
*                                                                             *
*   void Debug_TSS_Handler()                                                  *
*                                                                             *
*******************************************************************************
*
*   This is the int 1, instruction trace handler.
*
#   On entry, interrupts are disabled and the stack frame is set in Int.asm.
#
******************************************************************************/
void Debug_TSS_Handler()
{
    // Now we can enable interrupts

    EnableInterrupts();

    dprintf("\nInvalid TSS Fault" );

    Int3();
}


/******************************************************************************
*                                                                             *
*   void Debug()                                                              *
*                                                                             *
*******************************************************************************
*
*   This is the main debug function.  It can be entered ONLY FROM
#   Int3 and Int1 handlers.
*
******************************************************************************/
void Debug()
{
    int i;
    char *psValue;
    BOOL fHush;


    // Set up the debugger keyboard handler

    Register_Interrupt_Handler( 0x1, Deb_Keyboard_Handler );

    // Copy the process context

    context = CopyContext();

    // Now we can enable interrupts

    EnableInterrupts();

    // Disable hardware breakpoints

    SetDebugReg(7, GetDebugReg(7) & ~0xFF );

    // Reset all software breakpoints

    BreakCleanup();

    // Set the default code and data segment and offset

    iSegCode = SEG_CS;
    dwOffsetCode = pDebCode->eip;

    iSegData = SEG_DS;
    dwOffsetData = 0;

    // If the code being interrupted is in V86 mode, use code32=data32=0
    // values, else use 32-bit PM values

    if( pDebCode->eflags & VM_MASK )
    {
        SetValue( "Code32", "0" );
        SetValue( "Data32", "0" );
    }
    else
    {
        SetValue( "Code32", "1" );
        SetValue( "Data32", "1" );
    }

    fnRehash();

    // Display the register and current line only if Deb.fRegs is False

    if( !Deb.fRegs )
    {
        RegDisplay( FALSE );

        // Disassemble one line of code right after the registers

        pArg = "eip 1";
        fnUnassemble();
    }


    while( 1 )
    {
        // Display the register settings only if Deb.fRegs is True

        if( Deb.fRegs )
        {
            RegDisplay( TRUE );

            // Disassemble several lines of code right under the registers

            pArg = "^eip";
            dprintf("%c%c%c", DP_SETCURSOR, 0, 2 );
            fnUnassemble();
        }


        // Set the color attributes for the command line

        dprintf("%c%c", DP_SETWRITEATTR, ATTR_CMDLINE );

        // Get the command from the user

        GetCommand( Deb.nNumLines - 1, sCmd );

        // Set up the display unit with autoscroll range and the cursor location

        dprintf("%c%c%c", DP_SETSCROLLREGION, Deb.fRegs? 3 + Deb.nDumpLines: 0, Deb.nNumLines - 2 );
        dprintf("%c%c%c", DP_SETCURSOR, 0, Deb.nNumLines - 2);
        dprintf("%c%c",   DP_SETWRITEATTR, ATTR_RESPONSE );

    NewCommand:
        // Find the beginning of the command (non-space character)

        pCmd = pArg = sCmd + strspn( sCmd, " " );

        // If the line is empty, ignore it

        if( *pArg=='\0' )
            continue;

        // If the first character is `@', do not echo

        if( *pCmd=='@' )
            *pCmd++ = ' ', pArg++, fHush = TRUE;
        else
            fHush = FALSE;

        // Find the command in the list of supported commands

        for( i=0; Cmd[i].sName != NULL; i++ )
        {
            if( !strnicmp( Cmd[i].sName, pArg, Cmd[i].nNameLen) )
                break;
        }

        if( Cmd[i].sName == NULL )
        {
            // Command is not recognized.  Check if it is a function key or macro

            psValue = SetGetValue( pArg );

            if( psValue != NULL )
            {
                // Copy the command in the command buffer and recurse

                strcpy( pCmd, psValue );

                goto NewCommand;
            }

            dprintf("\n%s - Unrecognized, press `h' for help", pArg );
        }
        else
        {
            // Echo the command (if `Echo' was True) and the first char's not `@'

            if( Deb.fEcho && !fHush )
                dprintf("\n%c%c%s%c%c", DP_SETWRITEATTR, ATTR_ECHO,
                        pCmd, DP_SETWRITEATTR, ATTR_RESPONSE );

            // Set the index of the first possible argument

            pArg += Cmd[i].nNameLen;
            pArg += strspn(pArg, " ");

            // Execute the function that is associated with the command

            if( Cmd[i].fnCmd() == FALSE )
                dprintf("\n%s - Syntax error, press `h' for help", pArg );
        }
    }
}


/******************************************************************************
*                                                                             *
*   DWORD GetLinAddress( TSEG iSeg, DWORD dwOffset )                          *
*                                                                             *
*******************************************************************************
*
*   Returns the linear access address computed from the given segment/selector
#   and offset.  This function takes into account the mode of operation:
#   for PM it will use iSeg as an selector, for V86 it will use iSeg as an
#   segment.
*
*   Where:
*       iSeg is the segment index
#       dwOffset is the offset to add
*
*   Returns:
*       Linear address
*
******************************************************************************/
static DWORD GetLinAddress( TSEG iSeg, DWORD dwOffset )
{
    DWORD pLDT;
    WORD wSel, wLDT;
    TDesc *pSel;

    // Assign segment/selector values depending on the index.  For SEG_NULL and
    // SEG_GLOBAL, just return with the dwOffset (global) value

    switch( iSeg )
    {
        case SEG_CS:  wSel = pDebCode->cs;  break;
        case SEG_DS:  wSel = pDebSeg->ds;   break;
        case SEG_ES:  wSel = pDebSeg->es;   break;
        case SEG_FS:  wSel = pDebSeg->fs;   break;
        case SEG_GS:  wSel = pDebSeg->gs;   break;
        case SEG_SS:  wSel = pDebStack->ss;  break;

        default: return( dwOffset );
    };

    // Depending upon the program mode, use segment or selector

    if( pDebCode->eflags & VM_MASK )
    {
        // V86 mode - iSeg is a segment

        return( (wSel << 4) + dwOffset );
    }

    // In PM, we can be in the kernel mode or in the application mode
    // using a LDT table

    if( wSel & 4 )
    {
        // Selector indexes a current local descriptor table

        wLDT = GetLDT();
        pLDT = GET_DESCBASE(&GDT[wLDT >> 3]) - LIN_KERNEL;

        pSel = (TDesc *)(pLDT + (wSel >> 3) * sizeof(TDesc));
    }
    else
    {
        // Selector indexes a global descriptor table

        pSel = &GDT[ wSel >> 3 ];
    }

    return( GET_DESCBASE(pSel) + dwOffset );
}


/******************************************************************************
*                                                                             *
*   int fnLiteralHandler( char * sName )                                      *
*                                                                             *
*******************************************************************************
*
*   This function is an extension to the expression evaluator.  It handles
#   all literals and constants (non-number substrings).
*
#   The strings are matched with the names of CPU registers first, then
#   with the symbol names and lastly with the debugger variables that may
#   further recurse the search.
#
*   Where:
*       sName is the literal name
*
*   Returns:
*       integer that represent the literal
*
******************************************************************************/

#define SUB_LIT_LEN     24              // Maximum length of a sub-literal

static int LiteralRecurse( char * pSubStr )
{
    char sSub[SUB_LIT_LEN];
    int  nSubLen;

    // Copy the sub-string into the local buffer because we will call
    // nEvaluare recursively and we need a separate copy of that string

    nSubLen = strlen(pSubStr);

    if( nSubLen < SUB_LIT_LEN )
        strcpy( sSub, pSubStr );
    else
    {
        strncpy( sSub, pSubStr, SUB_LIT_LEN-1 );
        sSub[SUB_LIT_LEN-1] = '\0';
    }

    // Return the integer that is recursively evaluated pSubStr

    return( nEvaluate(sSub, NULL) );
}

static int fnLiteralHandler( char * sName )
{
    char * pLit;
    int i;
    static const char *sPrefix[] =
    {
        "@", "cs:", "ss:", "ds:", "es:", "fs:", "gs:",
        "eax","ebx","ecx","edx","ebp","esi","edi",
        "ax","bx","cx","dx","bp","si","di",
        "al","ah","bl","bh","cl","ch","dl","dh",
        "ss","esp","sp","gs","fs","ds","es","cs","eip","ip","efl","fl",
        NULL
    };

    // Find the string

    i = 0;
    while( sPrefix[i] != NULL )
    {
        if( !strnicmp( sName, sPrefix[i], strlen(sPrefix[i]) ) )
            break;
        i++;
    }

    if( sPrefix[i] != NULL )
    {
        switch( i )
        {
            // Check the special form of <segment/selector>:<offset>

            case 0:  iSeg = SEG_GLOBAL; return( LiteralRecurse( &sName[1]) );
            case 1:  iSeg = SEG_CS;     return( LiteralRecurse( &sName[3]) );
            case 2:  iSeg = SEG_SS;     return( LiteralRecurse( &sName[3]) );
            case 3:  iSeg = SEG_DS;     return( LiteralRecurse( &sName[3]) );
            case 4:  iSeg = SEG_ES;     return( LiteralRecurse( &sName[3]) );
            case 5:  iSeg = SEG_FS;     return( LiteralRecurse( &sName[3]) );
            case 6:  iSeg = SEG_GS;     return( LiteralRecurse( &sName[3]) );

            // Check the register names

            case 7 : return( pDebReg->eax );
            case 8 : return( pDebReg->ebx );
            case 9 : return( pDebReg->ecx );
            case 10: return( pDebReg->edx );
            case 11: return( pDebReg->ebp );
            case 12: return( pDebReg->esi );
            case 13: return( pDebReg->edi );

            case 14: return( pDebReg->eax & 0xFFFF );
            case 15: return( pDebReg->ebx & 0xFFFF );
            case 16: return( pDebReg->ecx & 0xFFFF );
            case 17: return( pDebReg->edx & 0xFFFF );
            case 18: return( pDebReg->ebp & 0xFFFF );
            case 19: return( pDebReg->esi & 0xFFFF );
            case 20: return( pDebReg->edi & 0xFFFF );

            case 21: return( pDebReg->eax & 0xFF );
            case 22: return( (pDebReg->eax >> 8) & 0xFF );
            case 23: return( pDebReg->ebx & 0xFF );
            case 24: return( (pDebReg->ebx >> 8) & 0xFF );
            case 25: return( pDebReg->ecx & 0xFF );
            case 26: return( (pDebReg->ecx >> 8) & 0xFF );
            case 27: return( pDebReg->edx & 0xFF );
            case 28: return( (pDebReg->edx >> 8) & 0xFF );

            // Stack record fields

            case 29: return( pDebStack->ss );
            case 30: return( pDebStack->esp );
            case 31: return( pDebStack->esp & 0xFFFF );

            // Segment registers/selectors

            case 32: return( pDebSeg->gs );
            case 33: return( pDebSeg->fs );
            case 34: return( pDebSeg->ds );
            case 35: return( pDebSeg->es );

            // Code record fields

            case 36: return( pDebCode->cs );
            case 37: return( pDebCode->eip );
            case 38: return( pDebCode->eip & 0xFFFF );

            // Flags

            case 39: return( pDebCode->eflags );
            case 40: return( pDebCode->eflags & 0xFFFF );
        }
    }

    // Look for the symbol name that matches a literal

    i = GetSymbolAddress( sName );

    if( i != -1 ) return( i );

    // Finally, look if a system variable is defined that matches

    pLit = SetGetValue( sName );

    if( pLit != NULL )  return( LiteralRecurse(pLit) );

    // Return 0 if the literal could not be matched

    return( 0 );
}



/******************************************************************************
*                                                                             *
*   void Init_Debug()                                                         *
*                                                                             *
*******************************************************************************
*
*   This function initializes the debugger.
*
******************************************************************************/
void Init_Debug()
{
#if USE_LOCAL_STACK
    // Use local stack for the debugger

    dprintf("\nDebugger local heap at ");

    pMemDeb = MemDeb;
#else
    // Initialize memory pool for the debugger from the kernel heap

    dprintf("\nDebugger global heap at ");

    pMemDeb = HeapCommitMap( HEAP_DEBUG_LEN );
#endif

    dprintf("%08Xh of %Xh", pMemDeb, HEAP_DEBUG_LEN );

    // Clear the debug enable register and status

    SetDebugReg( 7, 0 );
    SetDebugReg( 6, 0 );

    // Init memory allocation on the debugger memory pool

    _Init_Alloc( pMemDeb, HEAP_DEBUG_LEN );

    // Initialize `set' linked list for debuger variables

    QCreate( &qSet );

    // Initialize `sym' linked list for symbol names

    QCreate( &qSym );

    // Initialize `breakpoint' linked list for debuger breakpoints

    QCreate( &qBP );

    // Register literal evaluator

    pfnEvalLiteralHandler = fnLiteralHandler;

    // Add basic system variables

    // The number of lines on the text display will be set to 50
    // later on after VM86 module is initialized

    SetValue( "Lines","50" );
    SetValue( "Dump", "8" );
    SetValue( "List", "25" );
    SetValue( "Echo", "1" );

    // Add some debugging keys

    SetValue( "F1", "@h" );             // F1 - Help command
    SetValue( "F2", "@u cs:eip" );      // F2 - Desassemble from cs:eip
    SetValue( "F3", "@dd ss:esp");      // F3 - Show stack as dwords
    SetValue( "CF3","@dw ss:esp");      // Ctrl+F3 - Show stack as words
    SetValue( "F5", "g" );              // F5 - Go command
    SetValue( "F8", "@t" );             // F8 - Trace command
    SetValue( "F10", "@step" );         // F10 - Program step command

    SetValue( "F4", "set regs=1" );     // F4 - Show registers
    SetValue( "CF4","set regs=0" );     // Ctrl+F4 - Hide registers

    SetValue( "CF1", "unload" );        // Ctrl+F1 - Unload

    // Set some temp constants

    SetValue( "pd", "@0800000h" );
    SetValue( "pt", "@0C00000h" );


    // Rehash the state into the debugger system variable

    fnRehash();

    // Register exception handlers for debug: interrupts 1 and 3

    Register_Exception_Handler( 0x1, Debug_Trace_Handler );

    Register_Exception_Handler( 0x3, Debug_Breakpoint_Handler );

    dprintf(".");
}

