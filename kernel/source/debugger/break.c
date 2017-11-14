/******************************************************************************
*                                                                             *
*   Module:     Break.c                                                       *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       11/17/97                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This module contains the code for breakpoints.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 11/17/97   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "debug.h"                      // Include debugger colors

#include "break.h"                      // Include its own header

#include "inline.h"                     // Include inline macros

#include "intel.h"                      // Include intel specific defines

#include "kernel.h"                     // Include page directory address

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


#define OPCODE_INT3         0xCC        // Int 3 debug opcode instruction value

//----------------------------------------------------------------------------
// Breakpoint structure
//----------------------------------------------------------------------------

#define BP_DEFAULT          0x0000      // Default flag settings
#define BP_DISABLE          0x0001      // Breakpoint is disabled
#define BP_DEAD             0x0002      // Do not use this bp but destroy it
#define BP_PLACED           0x0004      // Int 3 has been placed


typedef struct
{
    DWORD dwAddress;                    // Linear address of a breakpoint
    DWORD dwFlags;                      // Breakpoint flags
    char * pArg;                        // Pointer to argument string
    BYTE bCovered;                      // Byte that is covered by the `int 3'

} TBP;


TQueue qBP;                             // Breakpoint linked list


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   int ControlBP( int num, int action )                                      *
*                                                                             *
*******************************************************************************
*
*   Performs action over a specific breakpoint.
*
*   Where:
*       num is the breakpoint number in a list.
*
*   Returns:
*       True if bp is valid
#       False if bp is not valid
*
******************************************************************************/
int ControlBP( int num, int action )
{
    TBP *pBP;
    int count = 1;

    pBP = QFirst( &qBP );

    while( pBP != NULL )
    {
        // Do a specific address search command code first

        if( (action==ACT_FIND) && (pBP->dwAddress==num) )
            return( count );
        else
        if( num == count )
        {
            // Do specific action on this breakpoint

            switch( action )
            {
                case ACT_PRINT:     // Print a breakpoint content

                    dprintf("\n%2d. %s  %08X  %s",
                            num,
                            (pBP->dwFlags & BP_DISABLE)? "d":" ",
                            pBP->dwAddress,
                            pBP->pArg );
                    break;

                case ACT_DISABLE:   // Disable a breakpoint

                    pBP->dwFlags |= BP_DISABLE;

                    break;

                case ACT_ENABLE:    // Enable a breakpoint

                    pBP->dwFlags &= ~BP_DISABLE;

                    break;

                case ACT_CLEAR:     // Clear a breakpoint

                    pBP->dwFlags |= BP_DEAD;

                    break;

                case ACT_SET:       // Physically set a breakpoint
                    // Only not disabled breakpoints that were not placed yet

                    if( !(pBP->dwFlags & BP_DISABLE) && !(pBP->dwFlags & BP_PLACED))
                    {
                        pBP->bCovered = abs_peekb( pBP->dwAddress );
                        abs_pokeb( pBP->dwAddress, OPCODE_INT3 );
                        pBP->dwFlags |= BP_PLACED;
                    }

                    break;

                case ACT_UNSET:     // Physically unset a breakpoint
                    // Only not disabled breakpoints that were placed

                    if( !(pBP->dwFlags & BP_DISABLE) && (pBP->dwFlags & BP_PLACED))
                    {
                        abs_pokeb( pBP->dwAddress, pBP->bCovered );
                        pBP->dwFlags &= ~BP_PLACED;
                    }

                    break;
            }

            // Normal return since we have found a good record

            return( TRUE );
        }

        count++;
        pBP = QNext( &qBP );
    }

    // Did not find a breakpoint to act upon

    return( FALSE );
}


/******************************************************************************
*                                                                             *
*   int BreakClear( char *pArg )                                              *
*                                                                             *
*******************************************************************************
*
*   Clears one or more breakpoints.
*
*   Where:
*       pArg is the command's argument string
*
*   Returns:
*       True if command ok
#       False if command syntax wrong
*
******************************************************************************/
int BreakClear( char *pArg )
{
    int Num, count;
    char *pVal;
    TBP *pBP;

    pVal = pArg;

    if( *pVal=='*' )
    {
        // Clear all breakpoints

        for( count=1; ;count++ )
            if( ControlBP(count, ACT_CLEAR)==FALSE )
                break;
    }
    else
    while( pVal != NULL )
    {
        // Get a breakpoint number

        Num = nEvaluate( pVal, &pVal );

        if( ControlBP( Num, ACT_CLEAR )==FALSE )
            break;
    }

    // After we have tagged all the record that need clearing, we can loop
    // and actually delete them

    pBP = QFirst( &qBP );

    while( pBP != NULL )
    {
        if( pBP->dwFlags & BP_DEAD )
        {
            // Free all the dynamic memory used by this node

            FREE( pBP->pArg );
            FREE( pBP );

            // Free the current node

            QDelete( pMemDeb, &qBP );

            pBP = QCurrent( &qBP );
        }
        else
            pBP = QNext( &qBP );
    }

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   int BreakDisable( char *pArg )                                            *
*                                                                             *
*******************************************************************************
*
*   Disables one or more breakpoints.
*
*   Where:
*       pArg is the command's argument string
*
*   Returns:
*       True if command ok
#       False if command syntax wrong
*
******************************************************************************/
int BreakDisable( char *pArg )
{
    int Num, count;
    char *pVal;

    pVal = pArg;

    if( *pVal=='*' )
    {
        // Disable all breakpoints

        for( count=1; ;count++ )
            if( ControlBP(count, ACT_DISABLE)==FALSE )
                break;
    }
    else
    while( pVal != NULL )
    {
        // Get a breakpoint number

        Num = nEvaluate( pVal, &pVal );

        if( ControlBP( Num, ACT_DISABLE )==FALSE )
            break;
    }

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   int BreakEnable( char *pArg )                                             *
*                                                                             *
*******************************************************************************
*
*   Enables one or more breakpoints.
*
*   Where:
*       pArg is the command's argument string
*
*   Returns:
*       True if command ok
#       False if command syntax wrong
*
******************************************************************************/
int BreakEnable( char *pArg )
{
    int Num, count;
    char *pVal;

    pVal = pArg;

    if( *pVal=='*' )
    {
        // Enable all breakpoints

        for( count=1; ;count++ )
            if( ControlBP(count, ACT_ENABLE)==FALSE )
                break;
    }
    else
    while( pVal != NULL )
    {
        // Get a breakpoint number

        Num = nEvaluate( pVal, &pVal );

        if( ControlBP( Num, ACT_ENABLE )==FALSE )
            break;
    }

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   int BreakList( char *pArg )                                               *
*                                                                             *
*******************************************************************************
*
*   Lists all breakpoints.
*
*   Where:
*       pArg is the command's argument string
*
*   Returns:
*       True if command ok
#       False if command syntax wrong
*
******************************************************************************/
int BreakList( char *pArg )
{
    int count;

    for( count=1; ;count++ )
        if( ControlBP(count, ACT_PRINT)==FALSE )
            break;

    if( count==1 )
        dprintf("\nNo breakpoint defined");

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   int BreakSet( char *pArg )                                                *
*                                                                             *
*******************************************************************************
*
*   Sets a breakpoint to the given address (expression).
*
*   Where:
*       pArg is the command's argument string
*
*   Returns:
*       True if command ok
#       False if command syntax wrong
*
******************************************************************************/
int BreakSet( char *pArg )
{
    DWORD dwAddress;
    TBP *pBP;
    char *pArgument;
    int res;


    // Get the address value

    if( (dwAddress = nEvaluate( pArg, NULL ))==0 )  return( FALSE );

    // Make sure that the address is not duplicated

    if( (res = ControlBP(dwAddress, ACT_FIND) != FALSE ) )
    {
        dprintf("\nDuplicated breakpoint (%d)", res );
        return( TRUE );
    }

    // Allocate memory for the breakpoint structure and for the argument line

    pBP = (TBP *) MALLOC( sizeof(TBP) );
    pArgument = (char *) MALLOC( strlen(pArg) + 1 );

    if( (pBP != NULL) && (pArgument != NULL) )
    {
        strcpy( pArgument, pArg );

        pBP->dwAddress = dwAddress;
        pBP->dwFlags   = BP_DEFAULT;
        pBP->pArg      = pArgument;

        // Link a new node at the end of the list

        QLast( &qBP );
        res = QAdd( pMemDeb, &qBP, pBP );

        if( res==0 )
            dprintf("\nUnable to insert a breakpoint");
    }
    else
        dprintf("\nUnable to allocate memory for a breakpoint!");

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   int BreakSetup()                                                          *
*                                                                             *
*******************************************************************************
*
*   This function sets up all the breakpoints into the memory.  This call
#   is used before leaving the debugger.
*
*   Where:
*       no args
*
*   Returns:
*       0
*
******************************************************************************/
int BreakSetup()
{
    int count;

    for( count=1; ;count++ )
        if( ControlBP(count, ACT_SET)==FALSE )
            break;

    return( 0 );
}



/******************************************************************************
*                                                                             *
*   int BreakCleanup()                                                        *
*                                                                             *
*******************************************************************************
*
#   This function cleans up all the breakpoints from the code.  This is used
#   immediately after entering the debugger.
*
*   Where:
*       no args
*
*   Returns:
*       0
*
******************************************************************************/
int BreakCleanup()
{
    int count;

    for( count=1; ;count++ )
        if( ControlBP(count, ACT_UNSET)==FALSE )
            break;

    return( 0 );
}

