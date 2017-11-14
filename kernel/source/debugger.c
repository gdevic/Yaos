/*********************************************************************
*                                                                    *
*   Module:     debugger.c                                           *
*                                                                    *
*   Revision:   1.00                                                 *
*                                                                    *
*   Date:       04/04/96                                             *
*                                                                    *
*   Author:     Goran Devic                                          *
*                                                                    *
**********************************************************************
*                                                                    *
*   Module Description:                                              *

        This code implements the kernel-level debugger

**********************************************************************
*                                                                    *
*   Changes:                                                         *
*                                                                    *
*   DATE     REV   DESCRIPTION OF CHANGES                 AUTHOR     *
* --------   ----  -----------------------------------   -----------
  04/04/96   1.00  Original                              Goran Devic

* --------   ----  -----------------------------------   ----------- *
**********************************************************************/

#define _TEST_         0

/*********************************************************************
*   Include Files
**********************************************************************/
#if _TEST_

#include "..\include\kernel.h"
#include "..\include\dis386.h"

#define printk printf
#define GetLine gets

BYTE GDT;
void  DebugINT1_Handler(){}
void  DebugINT3_Handler(){}
DWORD Peek( DWORD a, DWORD b){ return 0; }
void  Poke( DWORD a, DWORD b, BYTE c ){}
void  Dis386( TDis386 *d){ printf(" %08X ",d->bpTarget);}
void  SetIDTEntry( BYTE bNum, void (*fn)(void), WORD Type ){};

#else

#include "interrpt.h"           // Include interrupt header
#include "ttys.h"               // Include terminal types

#include "types.h"
#include "debugger.h"
#include "mm.h"
#include "kernel.h"
#include "stdlib.h"
#include "dis386.h"

#include "ports.h"              // Include basic ports I/O

#endif
/*********************************************************************
*   Global Functions
**********************************************************************/

/*********************************************************************
*   External Functions
**********************************************************************/

/*********************************************************************
*   Local Variables and defines
**********************************************************************/
#define MAX_BK     20            // Maximum number of breakpoints

TBreakpoint Bk[MAX_BK];          // Array of breakpoints
TBreakpoint BkGo;                // Breakpoint for "Go" function

TAddr Data;                      // Current data address
TAddr Code;                      // Current code address

char *sParse;                    // Parse pointer to a command string
char buf[256];                   // Command line string
TDebug *regs;                    // Global registers holder
Tsel Sel;                        // Global selector registers
DWORD Db_int;                    // Int 1 or 3 message

TDis386 Dis;                     // Disassembler structure
char szDisasm[80];               // Space for disasm to disassemble

int fExitAsked;                  // Flag to exit debugger

#define NUMREGS  11
static char *sRegs[] =
{
    "edi",
    "esi",
    "ebp",
    "esp",
    "ebx",
    "edx",
    "ecx",
    "eax",
    "eip",
    "cs",
    "eflags"
};

struct
{
    WORD entries;
    DWORD address;

} Db_6;

void GetGDT( void * );
#pragma aux GetGDT = \
"sgdt fword ptr [eax]"      \
parm caller [eax];

void GetIDT( void * );
#pragma aux GetIDT = \
"sidt fword ptr [eax]"      \
parm caller [eax];


void DisablePaging();
#pragma aux DisablePaging = \
"  mov  eax, cr0"          \
"  and  eax, 0x7fffffff"   \
"  mov  cr0, eax"          \
parm modify [eax];


/*********************************************************************
*   Local Functions
**********************************************************************/

/*********************************************************************
*
*   void InitDebugger()
*
*   Initializes debugger.
*
**********************************************************************/
void InitDebugger()
{
    // Init debugger single-trace vector (INT 1)
    //
    if( !RegisterInterruptHandler( INT_DEBUG, (void(*)()) DebugINT1_Handler, IDT_TRAP ) )
       Halt("Unable to register INT1 handler");
    if( !RegisterInterruptHandler( INT_BREAKPOINT, (void(*)()) DebugINT3_Handler, IDT_TRAP ) )
       Halt("Unable to register INT3 handler");

    // Set disassembling buffer
    Dis.szDisasm = szDisasm;
    Dis.bDataSize = DIS_32_BIT;
    Dis.bAddressSize = DIS_32_BIT;

    // Init all software breakpoints
    ResetBreakpoints();
    BkGo.fUsed = 0;

    printk("Kernel Debugger\n");
}


/*********************************************************************
*
*   Command line processing functions
*
**********************************************************************/
// Skips blanks starting at buf, returns first address of nonblank
// character or NULL if end of line
//
char *SkipBlanks( char *buf )
{
    while(1)
    {
        if( *buf==13 || *buf=='\0' )
            return( NULL );
        if( *buf!=' ')
            return( buf );

        buf++;
    }
}

// Compares two strings and returns either NULL (strings dont match)
// or pointer to a suffix of a second string when the first one
// reached end.
//
char *StrCmp( const char *s1, char *s2 )
{
    while( *s1 )
    {
        if( *s1++ != *s2++ )
            return( NULL );
    }
    return( s2 );
}

/*********************************************************************
*
*   GetRegister will try to parse the given string and return
*   the number of the specified register, or -1 if no register
*   was recognized.
*
**********************************************************************/
char *GetRegister( int *r, char *buf )
{
    int i;
    char *tmp;

    *r = -1;
    for( i=0; i<NUMREGS; i++ )
        if( (tmp = StrCmp(sRegs[i], buf)) != NULL )
        {
            *r = i;
            return( tmp );
        }

    return( buf );
}


// String to hex - returns pointer to a suffix of a given string
// and a hex number that was evaluated (or 0).
// Returns a regster content if a register was given (prefix "/")
//
char * StrToHex( DWORD *hex, char *str )
{
    char c;
    int r;

    // Reset to 0
    *hex = 0;

    // Skip leading spaces
    //
    while( *str==' ' ) str++;

    // If a register was specified
    //
    if( *str=='/' )
    {
        str = GetRegister( &r, str+1 );    // Try with the register
        if( r==-1 )
            return( str-1 );               // It's not a register either

        *hex = *(DWORD *)((DWORD *)regs+r); // Register-store contents
        return( str );
    }

    while(1)
    {
        c = *str++;
        if( (c<'0') || (c>'f') || (c>'9' && c<'a') )
            break;

        if( c>'9' )
            c -= 'a' - '9' - 1;

        c -= '0';

        *hex = (*hex)*16 + c;
    }

    return( str-1 );
}

/*********************************************************************
*
*   GetAddr - parse the buffer and stores the address if present
*             Address may be written as selector:offset or just offset
*             A dot (.) will keep the old address
*             A slash "/" will use specified register content
*
**********************************************************************/
char *GetAddr( TAddr *Addr, char *buf )
{
    DWORD dwAddr;
    char *tmp;

    tmp = StrToHex( &dwAddr, buf );        // Get the hex number

    if( *tmp=='.' )                        // To keep old address?
        return( tmp+1 );                   // Yes.

    if( tmp != buf )
        Addr->addr = dwAddr;               // Store offset if valid

    if( *tmp==':' )                        // Was that a selector?
    {
        Addr->sel = Addr->addr;            // Yes - store it as such

        tmp = StrToHex( &dwAddr, &tmp[1] );// Get the offset
        Addr->addr = dwAddr;               // Store the offset
    }

    return( tmp );
}


/*********************************************************************
*
*   Peek functions: take segment:offset address and add offs to it
*   If segment is DATASEG, use global ds selector, otherwise use
*   global fs selector and pretend segment:offset
*
**********************************************************************/
DWORD dwPeek( TAddr *Addr, DWORD offs )
{
    return( SelPeekd(Addr->sel, Addr->addr + 4*offs) );
}

WORD wPeek( TAddr *Addr, DWORD offs )
{
    return( SelPeekd(Addr->sel, Addr->addr + 2*offs) & 0xffff );
}

BYTE bPeek( TAddr *Addr, DWORD offs )
{
    return( SelPeekd(Addr->sel, Addr->addr + offs) & 0xff );
}

void bPoke( TAddr *Addr, BYTE b )
{
    SelPokeb(Addr->sel, Addr->addr, b);
}

/*********************************************************************
*
*   SetSpecificBreakpoint will set a breakpoint INT 3 (0xCC) at a given
*   address.  It only sets Bk array, not the code 0xCC.
*
**********************************************************************/
void SetSpecificBreakpoint( TAddr *Addr )
{
    int i;

    for( i=0; i<MAX_BK; i++ )
        if( Bk[i].fUsed == 0 )
        {
            Bk[i].Addr.sel = Addr->sel;
            Bk[i].Addr.addr = Addr->addr;
            Bk[i].fUsed = 1;

            return;
        }

    printk("Failed: too many breakpoints!\n");
}


/*********************************************************************
*
*   ClearSpecificBreakpoint will clear a breakpoint INT 3 at a given
*   address.  0xCC code should not be set.
*
**********************************************************************/
void ClearSpecificBreakpoint( TAddr *Addr )
{
    int i;

    for( i=0; i<MAX_BK; i++ )
        if( Bk[i].fUsed )
            if( (Bk[i].Addr.sel == Addr->sel) &&
                (Bk[i].Addr.addr== Addr->addr))
            {
                Bk[i].fUsed = 0;

                return;
            }

    printk("Failed: non-existent breakpoint!\n");
}


/*********************************************************************
*
*   ClearBreakpoint will clear a breakpoint INT 3 (0xCC) at a given
*   address (virtually)
*
**********************************************************************/
void ClearBreakpoint()
{
    TAddr Addr;

    Addr.sel = Data.sel;
    Addr.addr = Data.addr;

    if( sParse != GetAddr(&Addr, sParse) )
        ClearSpecificBreakpoint( &Addr );
}


/*********************************************************************
*
*   SetAllBreakpoints sets all memorized breakpoints.
*   This should be called before program is to be run.
*
**********************************************************************/
void SetAllBreakpoints()
{
    int i;

    for( i=0; i<MAX_BK; i++ )
        if( Bk[i].fUsed )
        {
            Bk[i].bByte = bPeek(&Bk[i].Addr,0);
            bPoke(&Bk[i].Addr,0xCC);
        }
}


/*********************************************************************
*
*   SetOrShowAllBreakpoints will display all memorized breakpoints
*   or set a new one if address was specified
*
**********************************************************************/
void SetOrShowAllBreakpoints()
{
    int i;
    TAddr Addr;

    Addr.sel = Data.sel;
    Addr.addr = Data.addr;

    if( sParse != GetAddr(&Addr, sParse) )
        SetSpecificBreakpoint( &Addr );

    for( i=0; i<MAX_BK; i++ )
        if( Bk[i].fUsed )
        {
            printk(" %04X:%08X\n", Bk[i].Addr.sel, Bk[i].Addr.addr );
        }
}


/*********************************************************************
*
*   ResetBreakpoints will reset (clear) all breakpoints
*
**********************************************************************/
void ResetBreakpoints()
{
    int i;

    for( i=0; i<MAX_BK; i++ )
        Bk[i].fUsed = 0;
}


/*********************************************************************
*
*   ClearAllBreakpoints resets all memorized breakpoints
*   This should be called when debugger gets control
*
**********************************************************************/
void ClearAllBreakpoints()
{
    int i;

    for( i=0; i<MAX_BK; i++ )
        if( Bk[i].fUsed )
            bPoke(&Bk[i].Addr,Bk[i].bByte);
}


/*********************************************************************
*
*   ClearOneBreakpoint is used in INT 3 when the current eip
*   breakpoint must be skipped, if set.
*
**********************************************************************/
void ClearOneBreakpoint( TAddr *Addr )
{
    int i;

    for( i=0; i<MAX_BK; i++ )
        if( Bk[i].fUsed )
            if( (Bk[i].Addr.sel == Addr->sel) &&
                (Bk[i].Addr.addr== Addr->addr))
                    bPoke(&Bk[i].Addr,Bk[i].bByte);
}


/*********************************************************************
*
*   IsBreakpoint returns 1 if the current address contains a
*   breakpoint, 0 otherwise. (Only addr fields are compared, not
*   selector).
*
**********************************************************************/
int IsBreakpoint( TAddr *Addr )
{
    int i;

    for( i=0; i<MAX_BK; i++ )
        if( Bk[i].fUsed )
            if( Bk[i].Addr.addr==Addr->addr )
                    return( 1 );
    return( 0 );
}


/*********************************************************************
*
*   DumpData will dump dwLines worth of data of size bSize (1/2/4)
*   from the Addr address
*
**********************************************************************/
DumpData( TAddr *Addr, BYTE bSize, DWORD dwLines )
{
    int i, j;

    if( dwLines > 50 )
        dwLines = 50;

    for( i=0; i < dwLines; i++ )
    {
        printk("%04X:%08X ", Addr->sel, Addr->addr );

        switch( bSize )
        {
            case 1:
                 for( j=0; j<16; j++ )
                      printk("%02X ", bPeek(Addr, j) );
                 break;

            case 2:
                 for( j=0; j<8; j++ )
                      printk("%04X ", wPeek(Addr, j) );
                 break;

            case 4:
                 for( j=0; j<4; j++ )
                      printk("%08X ", dwPeek(Addr, j) );
                 break;

        };

        printk(" ");

        for( j=0; j<16; j++ )
        {
            printk("%c", (bPeek(Addr,j)) > 31? bPeek(Addr,j):'.' );
            if( j==7 )
                printk(" ");
        }

        Addr->addr += 16;
        if( bSize != 1 )
            printk("\n");
    }
}


/*********************************************************************
*
*   Set of dump functions for dumping bytes, words and dwords
*
**********************************************************************/
void DumpB()
{
    DWORD dwNum;

    sParse = GetAddr(&Data, sParse);
    sParse = StrToHex(&dwNum, sParse);

    DumpData( &Data, 1, dwNum? dwNum:6 );
}

void DumpW()
{
    DWORD dwNum;

    sParse = GetAddr(&Data, sParse);
    sParse = StrToHex(&dwNum, sParse);

    DumpData( &Data, 2, dwNum? dwNum:6 );
}

void DumpD()
{
    DWORD dwNum;

    sParse = GetAddr(&Data, sParse);
    sParse = StrToHex(&dwNum, sParse);

    DumpData( &Data, 4, dwNum? dwNum:6 );
}


/*********************************************************************
*
*   Enter function - change bytes in memory
*
**********************************************************************/
void Enter()
{
    TAddr Addr;
    DWORD dwNew;
    char *tmp;

    Addr.sel = Data.sel;
    Addr.addr = Data.addr;

    sParse = GetAddr(&Addr, sParse);

    while(1)
    {
        printk("%04X:%08X %02X ?", Addr.sel, Addr.addr, bPeek(&Addr,0) );
        tty_read(ctty, &buf, 256);

        tmp = StrToHex(&dwNew, &buf);

        if( tmp!=buf && dwNew<=255 )
            bPoke(&Addr,(BYTE)dwNew);
        else
            break;

        Addr.addr++;
    }
}


/*********************************************************************
*
*   Prints all the registers, flags and last two stack values
*
**********************************************************************/
void PrintRegisters()
{
    int r;
    char *tmp;
    DWORD dwNew;
    TAddr Stack;
    Teflags *f = (Teflags *)&regs->eflags;

    Stack.sel = DATASEL;
    Stack.addr = regs->Temp;

    if( (tmp=SkipBlanks(sParse)) != NULL )
    {
        GetRegister( &r, tmp );

        if( r != -1 )
        {
            printk(" %s = %08X ?", sRegs[r], *(DWORD *)((DWORD *)regs+r) );
            tty_read(ctty, &buf, 256);

            tmp = StrToHex(&dwNew, &buf);

            if( tmp!=buf )
                *(DWORD *)((DWORD *)regs+r) = dwNew;
        }
    }

    printk("EAX:%08X EBX:%08X ECX:%08X EDX:%08X    %08X\n",
        regs->eax, regs->ebx, regs->ecx, regs->edx, dwPeek(&Stack,1) );
    printk("ESI:%08X EDI:%08X EBP:%08X ESP:%08X -> %08X\n",
        regs->esi, regs->edi, regs->ebp, regs->Temp, dwPeek(&Stack,0) );
    printk("CS:%04X EIP:%08X IOPL:%X EFLAGS:%08X %s %c %s %c%c %s %c %s %c %s %s\n",
        regs->cs & 0xffff, regs->eip, f->iopl, regs->eflags,
        f->vm? "VM":"  ",
        f->r?  'R' :' ',
        f->nt? "NT":"  ",
        f->o?  'O' : ' ',
        f->d?  'D' : ' ',
        f->i?  "EI": "DI",
        f->s?  'S' : ' ',
        f->z?  " Z": "NZ",
        f->a?  'A' : ' ',
        f->p?  "PE": "PO",
        f->c?  " C": "NC" );
}


/*********************************************************************
*   Prints a single selector, called only from PrintSelectors()
**********************************************************************/
void PrintSingleSelector( char *name, WORD value, TSelector *sel )
{
    DWORD dwBase;
    DWORD dwLimit;

    dwBase = sel->base15_0 | (sel->base23_16 << 16) | (sel->base31_24 << 24);
    dwLimit = sel->limit15_0 | (sel->limit19_16 << 16);

    printk("%s:%04X Base %8X [%5X %s] %s type %X dpl:%X (avl:%X) %s\n",
        name, value, dwBase, dwLimit,
        (sel->g)?"pages":"bytes",
        (sel->dt)?"app":"sys",
        sel->type, sel->dpl, sel->avl,
        (sel->p)?"present":"not present" );
}

/*********************************************************************
*   Prints all the selectors and their description
**********************************************************************/
void PrintSelectors()
{
    PrintSingleSelector( "CS", regs->cs, (TSelector *)(&GDT + regs->cs ) );
    PrintSingleSelector( "DS", Sel.ds, (TSelector *)(&GDT + Sel.ds ) );
    PrintSingleSelector( "SS", Sel.ss, (TSelector *)(&GDT + Sel.ss ) );
    PrintSingleSelector( "ES", Sel.es, (TSelector *)(&GDT + Sel.es ) );
    PrintSingleSelector( "FS", Sel.fs, (TSelector *)(&GDT + Sel.fs ) );
    PrintSingleSelector( "GS", Sel.gs, (TSelector *)(&GDT + Sel.gs ) );
}

/*********************************************************************
*
*   Unassembles lines of code starting at the target address for
*   so many lines
*
**********************************************************************/
void DisassembleLines( DWORD dwLines )
{
    BYTE bOverflow;
    int i, j, k;

    for( k=0; k < dwLines; k++ )
    {
        Dis.bpTarget = (BYTE *)Code.addr;
        Dis386( &Dis );

        printk("%04X:%08X%c", Code.sel, Code.addr, IsBreakpoint(&Code)?'*':' ' );

        Code.addr += Dis.bInstrLen;

        j = (Dis.bInstrLen <= 5)?
          5-Dis.bInstrLen : 0;

        if( Dis.bInstrLen > 5 )
            Dis.bInstrLen = 5,
            bOverflow = 1;
        else
            bOverflow = 0;

        for( i=0; i<Dis.bInstrLen; i++ )
           printk(" %02X", bPeek(&Code,i) );
        for( i=0; i<j; i++ )
           printk("   ");

        printk("%c %s\n",
           bOverflow? '+':' ',
           Dis.szDisasm );
    }
}


/*********************************************************************
*
*   Unassemble command
*
**********************************************************************/
void Disassemble()
{
    DWORD dwNum;

    sParse = GetAddr(&Code, sParse);
    if( StrToHex(&dwNum, sParse) == sParse )
        dwNum = 8;
    else
        if( dwNum > 50 )
            dwNum = 50;

    DisassembleLines( dwNum );
}


/*********************************************************************
*
*   ShowStack function will display so many lines of stack data
*
**********************************************************************/
void ShowStack()
{
    TAddr Stack;
    DWORD dwNum;
    int i;


    Stack.sel = DATASEL;
    Stack.addr = regs->Temp;

    if( StrToHex(&dwNum, sParse)==sParse )
        dwNum = 5;

    for( i=(dwNum&31)*4-4; i>0; i-=4 )
    {
        printk("%04X:%08X  %08X     +%2d (%2Xh)\n",
            Stack.sel, Stack.addr+i, dwPeek(&Stack,i/4), i, i );
    }

    printk("%04X:%08X  %08X  <- esp\n",
        Stack.sel, Stack.addr+i, dwPeek(&Stack,i/4) );
}


/*********************************************************************
*
*   Trace function
*
**********************************************************************/
void Trace()
{
    regs->eflags |= TRAP_FLAG;

    fExitAsked = 1;
}


/*********************************************************************
*
*   Go freely to some optional address
*
**********************************************************************/
void Go()
{
    TAddr Addr;

    regs->eflags &= ~TRAP_FLAG;

    fExitAsked = 1;

    Addr.sel = Data.sel;
    Addr.addr = Data.addr;

    // If user did not specify an address, go freely
    if( sParse == GetAddr(&Addr, sParse) )
        return;

    // Otherwise, set up a breakpoint
    BkGo.Addr.sel = Addr.sel;
    BkGo.Addr.addr = Addr.addr;
    BkGo.bByte = bPeek(&BkGo.Addr,0);
    bPoke(&BkGo.Addr,0xCC);
    BkGo.fUsed = 1;
}

/*********************************************************************
*
*   Ignore function - when user only presses Enter
*
**********************************************************************/
void Ignore(){}


/*********************************************************************
*
*   ShowGDT will show the gdt register contents
*
**********************************************************************/
void ShowGDT()
{
    GetGDT( (void *)&Db_6 );

    printk("    %04X  entries=%d\n", Db_6.entries, (Db_6.entries+1)/8 );
    printk("%08X  physical address\n", Db_6.address );
}


/*********************************************************************
*
*   ShowIDT will show the idt register contents
*
**********************************************************************/
void ShowIDT()
{
    GetIDT( (void *)&Db_6 );

    printk("    %04X  entries=%d\n", Db_6.entries, (Db_6.entries+1)/8 );
    printk("%08X  physical address\n", Db_6.address );
}


/*********************************************************************
*
*   Reboot will, hopefully, reboot the system
*
**********************************************************************/
void Reboot()
{
    DisablePaging();
    Outpb(0x64, 0xfe );
}


/*********************************************************************
*
*   Evaluates an expression
*
**********************************************************************/
void Evaluate()
{
    int value;

    value = nEvaluate( sParse );

    printk("%08Xh  %d", value, value );
    if( value>31 && value<128 )
        printk("  '%c'", value );
    printk("\n");
}


/*********************************************************************
*
*   OutPort function will out a number to a port
*
**********************************************************************/
void OutPort()
{
    DWORD dwPort, dwValue;

    sParse = StrToHex(&dwPort, sParse);
    sParse = StrToHex(&dwValue, sParse);

    printk("outpb %4x -> %2x\n", dwPort, dwValue );
    Outpb((WORD)dwPort, (BYTE)dwValue );
}


/*********************************************************************
*
*   Runs a custom process
*
**********************************************************************/
void Run()
{
    REGS regs;

#if 1
    do
    {
        printk("FDD Access Attempted\n");
        // Load fdd sector
        //
        regs.w.ax = 0x0201;
        regs.w.cx = 0x0001;
        regs.w.dx = 0x0000;
        regs.w.es = 0x5000;
        regs.w.bx = 0x0000;
        Intv86( 0x13, &regs );

        Outpb( 0x3f2, 0 );
        printk("On return: eflags: %08X\n", regs.w.flags );

        if( !(regs.w.flags & 0x01) )
            return;

        printk("FDD Access failed\n");

        // Reset fdd controller
        //
        regs.w.ax = 0x0000;
        regs.w.dx = 0x0000;
        Intv86( 0x13, &regs );

    } while(1);
#endif

//   Intv86( 0x13, &regs );
//   INT1();
}


/*********************************************************************
*
*   Help message
*
**********************************************************************/
void Help()
{
    printk("YAOS Integrated Debugger (c) Goran Devic\n");
    printk("b [address]                    Set/show breakpoints\n");
    printk("bc [address]                   Clear a breakpoint\n");
    printk("bx                             Clear all breakpoints\n");
    printk("db[d]/dw/dd [address] [lines]  Dump data\n");
    printk("e [address]                    Enter new values\n");
    printk("g [address]                    Execute (up to)\n");
    printk("gdt/idt                        Show\n");
    printk("k [lines]                      Show stack data\n");
    printk("o [port] [value]               Output a byte to a port\n");
    printk("r [reg]                        Print/edit registers\n");
    printk("s                              Print selectors\n");
    printk("t                              Instruction trace\n");
    printk("u [address] [lines]            Disassemble\n");
    printk("Where address   = seg:offs | offs | '.'\n");
    printk("      seg, offs = hexnum | '/'register\n");
}


/*********************************************************************
*
*   Main parse and function distribute loop
*
**********************************************************************/
void * Parse( char *buf )
{
#define MENUS   21
    static TMenu menu[] = {
    { "bc",      ClearBreakpoint },
    { "bx",      ResetBreakpoints },
    { "b",       SetOrShowAllBreakpoints },
    { "db",      DumpB },
    { "dw",      DumpW },
    { "dd",      DumpD },
    { "d",       DumpB },
    { "e",       Enter },
    { "gdt",     ShowGDT },
    { "g",       Go },
    { "h",       Help },
    { "idt",     ShowIDT },
    { "k",       ShowStack },
    { "o",       OutPort },
    { "reboot",  Reboot },
    { "run",     Run },
    { "r",       PrintRegisters },
    { "s",       PrintSelectors },
    { "t",       Trace },
    { "u",       Disassemble },
    { "?",       Evaluate },
    };

    int i;
    char *tmp;

    sParse = buf;

    if( (sParse=SkipBlanks(sParse))==NULL )
        return( Ignore );

    for( i=0; i<MENUS; i++ )
    {
        if( (tmp=StrCmp(menu[i].cmd, sParse)) != NULL )
        {
            sParse = tmp;
            return( menu[i].fn );
        }
    }

    return( Ignore );
}


/*********************************************************************
*
*   Main debugger entry
*
**********************************************************************/
void Debugger( TDebug *registers, int OpSize )
{
    Dis.bDataSize = OpSize;
    Dis.bAddressSize = OpSize;

    EnableInterrupts();
    regs = registers;

    if( Db_int == 3 )
        regs->eip -= 1;

    // Clear Go breakpoint
    if( BkGo.fUsed )
    {
        bPoke(&BkGo.Addr,BkGo.bByte);
        BkGo.fUsed = 0;
    }

    if( OpSize == 1 )
    {
       // Kernel code
       //
       Code.sel = CODESEL;
       Code.addr = regs->eip;

       Data.sel = DATASEL;
       Data.addr = regs->eip;
    }
    else
    {
       // V86 code
       //
       Code.sel = GLOBALSEL;
       Code.addr = regs->cs*16 + regs->eip;

       Data.sel = GLOBALSEL;
       Data.addr = regs->cs*16 + regs->eip;
    }

    ClearAllBreakpoints();

    // Old stack address on the supervisor stack with same
    // privilege level int, no error code
    //
    regs->Temp = (DWORD)regs + 44;

    buf[0] = '\0';
    PrintRegisters();
    DisassembleLines( 1 );

    fExitAsked = 0;
    while( !fExitAsked )
    {
        printk(">");
        tty_read(ctty, &buf, 256);

        ((void(*)())Parse(&buf))();
    }

    SetAllBreakpoints();

    Data.sel = DATASEL;
    Data.addr = regs->eip;

    ClearOneBreakpoint( &Data );
}




#if _TEST_

/*********************************************************************
*
*   Testing...
*
**********************************************************************/
main()
{
    TDebug r;

    InitDebugger();

    Debugger(&r, 1);
}


#endif // _TEST_




