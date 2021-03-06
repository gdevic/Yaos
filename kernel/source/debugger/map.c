/******************************************************************************
*                                                                             *
*   Module:     Map.c                                                         *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       10/25/97                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This module contains the code for manipulating with process map files.
        Map files generated by the Watcom (c) Linker 10.6 are supported.

        Symbols are recognized that begin line with a character `0'.
        Then, first 8 characters comprise the zero-based address,
        next 8 characters are skipped, and
        the characters following up to the end of line are a symbol name.

    Note: Map file has to have lines terminated with 0x0D/0x0A characters
          in that order!
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 10/25/97   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include <ctype.h>                      // Include character types header

#include "debug.h"                      // Include debugger header file

#include "display.h"                    // Include display header file

#include "dosx.h"                       // Include DOS extender header

#include "Intel.h"                      // MM needs this include

#include "keyboard.h"                   // Include keyboard support

#include "MM.h"                         // Include memory management

#include "queue.h"                      // Include queue support

#include "map.h"                        // Include its own header

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

extern BYTE *pMemDeb;

TQueue qSym;                            // Linked list of sybmol names


/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

typedef struct
{
    DWORD dwAddress;
    char sName;

} TSymbol;


#define SYMBOL_LEN      32              // Maximum symbol length

#define BUF_LEN         1024            // Input file buffer length

static int handle = -1;                 // Map file handle

static char Buffer[ BUF_LEN ];          // File input buffer
static size = 0;                        // Useful buffer content size
static int pos = 0;                     // Reading position in a buffer


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   int fnSymCmp( void * s1, const void *s2 )                                 *
*                                                                             *
*******************************************************************************
*
*   This is a compare function for the symbol address
*
******************************************************************************/
static int fnSymCmp( void * s1, const void *s2 )
{
    return( ((TSymbol *)s1)->dwAddress <= ((TSymbol *)s2)->dwAddress );
}


/******************************************************************************
*                                                                             *
*   char GetMapChar()                                                         *
*                                                                             *
*******************************************************************************
*
*   This function gets an character from the file whose handle was set to
#   `handle'.  Input is buffered.
#
*   Where:
*       no args
*
*   Returns:
*       next available character from a file
#       0 if there are no more characters in a file
*
******************************************************************************/
static char GetMapChar()
{
    // See if we have to reload new chunk from the file

    if( pos >= size )
    {
        size = DOS_read( handle, BUF_LEN, Buffer );

        if( size <= 0 ) return( 0 );    // Read operation may fail or EOF

        pos = 0;                        // Reset the position back to zero
    }

    return( Buffer[pos++] );
}

/******************************************************************************
*                                                                             *
*   int LoadMap( char *sFileName, DWORD dwBaseAddress )                       *
*                                                                             *
*******************************************************************************
*
*   This function loads a symbol map.
*
*   Where:
*       sFileName is the name of the symbol map file.
#       dwBaseAddress is the fixup address (base address to add to all symbols)
*
*   Returns:
*       0 if could not load map
#       > 0 ok
*
******************************************************************************/
int LoadMap( char *sFileName, DWORD dwBaseAddress )
{
    DWORD dwAddress;
    int i, n, nSymbols = 0;
    char ch;
    char sName[ SYMBOL_LEN + 1 ];
    TSymbol *pSym;


    // Open the map file

    if( (handle = DOS_open( sFileName, 0 )) < 0 )  return( 0 );

    // Loop and read map symbol entries

    do
    {
        ch = GetMapChar();

        // Look for the leading character of `0'

        if( ch == '0' )
        {
            // Found it!  Calculate the address

            dwAddress = 0;

            do
            {
                dwAddress <<= 4;
                n = (int)tolower(ch) - '0';
                if( n > 9 )  n -= 'a' - '9' - 1;
                dwAddress |= n;

                ch = GetMapChar();
            }
            while( isxdigit(ch) );

            // Add the address fixup

            dwAddress += dwBaseAddress;

            // Skip the next 8 characters (one is already loaded)

            for( i=0; i<6; i++ )  GetMapChar();

            // Load symbol name, SYMBOL_LEN or up to the end of line, whatever
            // is less

            for( i=0; i<SYMBOL_LEN; i++ )
            {
                sName[i] = GetMapChar();

                // Break if the end of line is reached

                if( sName[i] == 0x0D )
                    break;
            }

            sName[i] = 0;

            // Allocate memory for a new symbol record

            pSym = _kMalloc(pMemDeb, sizeof(TSymbol) + i );
            if( pSym==NULL ) return( 0 );

            // Set up the symbol record

            pSym->dwAddress = dwAddress;
            strcpy( &pSym->sName, sName );

            // Add a name/address record to a sorted list of names

            QPriorityEnqueue( pMemDeb, &qSym, fnSymCmp, pSym );

            nSymbols++;
        }
        else
        {
            // Need to skip the whole line

            while( (ch != 0) && (ch != 0x0A) ) ch = GetMapChar();
        }
    }
    while( ch != 0 );

    // Close a file

    DOS_close( handle );


    return( nSymbols );
}



/******************************************************************************
*                                                                             *
*   void SymPrint( char *sPattern )                                           *
*                                                                             *
*******************************************************************************
*
*   This function prints symbols that satisfy search patterm.
#
*   Where:
*       sPattern is a search pattern.  Allowed symbols are:
#           `?' - replaces any character
*
*   Returns:
*       void
*
******************************************************************************/
void SymPrint( char *sPattern )
{
    TSymbol * pSym;
    int len = strlen(sPattern), i;
    BOOL fOK;
    int nLines = 0;


    pSym = QFirst( &qSym );

    while( pSym != NULL )
    {
        // Check if a search pattern can allow this symbol

        for( fOK = TRUE, i=0; i<len; i++ )
        {
            if( sPattern[i] == '*' )
                break;

            if( sPattern[i] != '?' )
                if( tolower(sPattern[i]) != tolower(*(&pSym->sName+i)) )
                    fOK = FALSE;
        }

        // Print the address and the name

        if( fOK == TRUE )
        {
            dprintf("\n%08X  %s", pSym->dwAddress, &pSym->sName );

            if( nLines++ == Deb.nListLines )
            {
                nLines = 0;

                if( GetKey( 1 )==ESC )
                    return;
            }
        }

        // Next node in a list

        pSym = QNext( &qSym );
    }
}


/******************************************************************************
*                                                                             *
*   DWORD GetSymbolAddress( char *sName )                                     *
*                                                                             *
*******************************************************************************
*
*   This function searches for the symbol sName and returns its address.
*
*   Where:
*       sName is the name of the symbol to find (case insensitive)
*
*   Returns:
*       Symbol address
#       0xFFFFFFFF if a symbol cannot be found
*
******************************************************************************/
DWORD GetSymbolAddress( char *sName )
{
    TSymbol * pSym;

    pSym = QFirst( &qSym );

    while( pSym != NULL )
    {
        if( stricmp(&pSym->sName, sName)==0 )
            return( pSym->dwAddress );

        // Next node in a list

        pSym = QNext( &qSym );
    }

    return( 0xFFFFFFFF );
}



/******************************************************************************
*                                                                             *
*   char *GetSymbolName( DWORD dwAddress )                                    *
*                                                                             *
*******************************************************************************
*
*   This function returns the name of a symbols at the specific address.
*
*   Where:
*       dwAddress is the address of a symbol
*
*   Returns:
*       Pointer to a symbol name
#       NULL if there was no symbol at the specified address
*
******************************************************************************/
char *GetSymbolName( DWORD dwAddress )
{
    TSymbol * pSym;

    pSym = QFirst( &qSym );

    while( pSym != NULL )
    {
        if( pSym->dwAddress == dwAddress )
            return( &pSym->sName );

        // We can return early since symbols are sorted starting at smallest
        // one so if the current one is larger, we have missed it

        if( pSym->dwAddress > dwAddress )
            return( NULL );

        // Next node in a list

        pSym = QNext( &qSym );
    }

    return( NULL );
}


/******************************************************************************
*                                                                             *
*   char *GetClosestSymbolName( int *pDelta, DWORD dwAddress )                *
*                                                                             *
*******************************************************************************
*
*   This function returns the name of a symbols at the specific address.
#   If there is no symbols at that address, the first symbol that has the
#   smaller address will be returned.
*
*   Where:
*       dwAddress is the address of a symbol
#       pDelta is the address to store the delta address value
*
*   Returns:
*       Pointer to a symbol name
#       NULL if symbols are not loaded or the delta is unreasonably large
*
******************************************************************************/
char *GetClosestSymbolName( int *pDelta, DWORD dwAddress )
{
    TSymbol * pSym;


    pSym = QLast( &qSym );

    while( pSym != NULL )
    {
        if( pSym->dwAddress <= dwAddress )
        {
            // If the delta is "really large", we certainly dont have a hit,
            // so return error

            if( (dwAddress - pSym->dwAddress) > 128*1024  )
                return( NULL );

            if( pDelta != NULL )
                *pDelta = dwAddress - pSym->dwAddress;

            return( &pSym->sName );
        }

        // Next node in a list

        pSym = QPrev( &qSym );
    }

    return( NULL );
}



/******************************************************************************
*                                                                             *
*   void ClearSymbols( int pid )                                              *
*                                                                             *
*******************************************************************************
*
*   This function clears the symbol table or a portion of symbol table
#   pertaining to a process pid.  The address is pid << 22.
*
*   Where:
*       pid is the specific process id whose symbol table needs to be cleared
#           0 if all symbols needs clear
*
*   Returns:
*       Symbol table or a portion of it is clean
*
******************************************************************************/
void ClearSymbols( int pid )
{
    TSymbol * pSym;

    pSym = QFirst( &qSym );

    while( pSym != NULL )
    {
        // See if we need to erase this symbold

        if( (pid == 0) || ((pid << 22)==(pSym->dwAddress & ~0xFFFFF) ) )
        {
            _kFree( pMemDeb, pSym );

            // Free the current node in a queue manager

            pSym = QDelete( pMemDeb, &qSym );
        }
        else
            pSym = QNext( &qSym );
    }

    dprintf("\nSymbols cleared");

    if( pid != 0 )
        dprintf(" for process %d (apperture %08X)", pid, pid << 22 );

    // Do some internal checking

    if( QCheck( &qSym ) != 0 )
        dprintf("\nWarning: Internal symbol queue structure error!");
}

