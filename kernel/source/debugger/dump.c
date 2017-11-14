/******************************************************************************
*                                                                             *
*   Module:     Dump.c                                                        *
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

    This module contains the code for the memory dump commands.
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

#include "debug.h"                      // Include debugger colors

#include "dump.h"                       // Include its own header

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

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/



/******************************************************************************
*                                                                             *
*   DWORD GetPhysicalAddress( DWORD dwLinAddress )                            *
*                                                                             *
*******************************************************************************
*
*   This function returns physical address associated with the given linear
#   address.
*
*   Where:
*       dwLinAddress is the linear address
*
*   Returns:
*       Physical address
#       0 if it is unmapped or not present
*
******************************************************************************/
DWORD GetPhysicalAddress( DWORD dwLinAddress )
{
    DWORD dir, page, phy;

    // Cut the linear addres into directory and page

    dir = dwLinAddress >> 22;
    page = (dwLinAddress >> 12) & 0x3FF;

    // Check if the directory is assigned

    if( PageDir[ dir ].fPresent == 0 )
        return( 0 );

    // Get the page table and check if the page is present

    if( PageTable[ dir * 1024 + page ].fPresent ==0 )
        return( 0 );

    phy = (PageTable[ dir * 1024 + page ].Index << 12) + (dwLinAddress & 0xFFF);

    // Return the page index and the offset

    return( phy );
}


/******************************************************************************
*                                                                             *
*   WORD GetByte( DWORD dwLinAddress )                                        *
*                                                                             *
*******************************************************************************
*
*   This function returns a byte from a memory location specified as
#   the argument.  Since the address is given as linear, it may or may not be
#   a valid (present in memory).  It also returns the page attributes in the
#   upper byte of a result.  This byte is logical `and' of page directory
#   entry and page table entry.
*
*   Where:
*       dwLinAddress - linear address of the byte to fetch
*
*   Returns:
*       byte from the location dwLinAddress
#       BYTE_NOT_PRESENT if page is not present
*
******************************************************************************/
WORD GetByte( DWORD dwLinAddress )
{
    DWORD dir, page;

    // Cut the linear addres into directory and page

    dir = dwLinAddress >> 22;
    page = (dwLinAddress >> 12) & 0x3FF;

    // Check if the directory is assigned

    if( PageDir[ dir ].fPresent == 0 )
        return( BYTE_NOT_PRESENT );

    // Get the page table and check if the page is present

    if( PageTable[ dir * 1024 + page ].fPresent ==0 )
        return( BYTE_NOT_PRESENT );

    // Get the byte and return the byte and page descriptor

    return( abs_peekb( dwLinAddress ) );
}


/******************************************************************************
*                                                                             *
*   BOOL Dump( DWORD dwLinAddress, DWORD dwPrintAddress,                      #
#       int nLines, int nSize, char *sPrefix )                                *
*                                                                             *
*******************************************************************************
*
*   Dumps the memory block to the debug device in different size formats,
#   byte (1), word (2) or dword (4).
*
*   Where:
*       dwLinAddress - starting linear address to dump
#       dwPrintAddress - address to actually print
#       nLines - number of lines to dump
#       nSize - byte (1), word (2) or dword (4)
#       sPrefix - address string prefix (such as `ds:')
*
*   Returns:
#       void
*
******************************************************************************/
void Dump( DWORD dwLinAddress, DWORD dwPrintAddress, int nLines, int nSize, char *sPrefix )
{
    char sStr[24] = ".... .... .... ....";
    BYTE bLine[16];
    int i, index;
    WORD wValue;
    DWORD dwValue;


    for( ; nLines > 0; nLines--, dwLinAddress += 16, dwPrintAddress += 16 )
    {
        // Get 16 bytes from the source

        for( i=0, index=-1; i<16; i++, index++ )
        {
            // Calculate the correct index into the char array

            if( (i & 3)==0 )
                index++;

            wValue = GetByte( dwLinAddress + i );

            // Check for validity (presence)

            if( wValue == BYTE_NOT_PRESENT )
            {
                // A byte is not present - set it as 0xFF and blank the char

                bLine[i] = 0xFF;
                sStr[index] = ' ';
            }
            else
            {
                // Set a byte into the byte array and the char into the sStr[]

                bLine[i] = wValue;
                sStr[index] = wValue;
                if( sStr[index] < 32 )
                    sStr[index] = '.';
            }
        }

        // Print the address with the given prefix

        dprintf("\n%c%c%s%08X %c%c", DP_SETWRITEATTR, ATTR_RESPONSE,
            sPrefix, dwPrintAddress,
            DP_SETWRITEATTR, ATTR_RESPONSEHI );

        // Print those 16 bytes as bytes, words or dwords

        switch( nSize )
        {
            case 2:
                // Dump word values

                for( i=0; i<16; i+=2 )
                {
                    wValue = bLine[i] | (bLine[i+1]<<8);
                    dprintf(" %04X", wValue );
                    if( i==7 ) dprintf(" ");
                }

                // Advance to the character string

                dprintf("       ");

                break;

            case 4:
                // Dump dword values

                for( i=0; i<16; i+=4 )
                {
                    dwValue = bLine[i] | (bLine[i+1]<<8) | (bLine[i+2]<<16) | (bLine[i+3]<<24);
                    dprintf(" %08X", dwValue );
                }

                // Advance to the character string

                dprintf("           ");

                break;

            default:
                // Dump byte values

                for( i=0; i<16; i++ )
                {
                    dprintf("%02X ", bLine[i] );
                }

                break;
        }

        // Print the character string

        dprintf("%c%c%s", DP_SETWRITEATTR, ATTR_RESPONSE, sStr );
    }
}



