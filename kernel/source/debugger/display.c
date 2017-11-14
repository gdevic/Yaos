/******************************************************************************
*                                                                             *
*   Module:     Display.c                                                     *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       9/4/97                                                        *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

    This module contains the code for the direct display output.

    dprintf() performs the functionality of printf() with some extra control
    characters that are defined in Display.h file.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 9/4/97     1.00  Original                                       Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include <stdarg.h>                     // Variable number of arguments

#include "display.h"                    // Include its own header file

#include "inline.h"                     // Inline functions

#include "printf.h"                     // The main _printf function

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

#define INIT_LINES          50          // Initial number of lines
#define INIT_SCROLL_TOP     0           // Initial autoscroll top line
#define INIT_SCROLL_BOT     INIT_LINES  // Initial autoscroll bottom line
#define ATTR_DEFAULT  (WHITE COLOR_ON BLUE)  // Default screen attribute

// Structure holding all the device values

typedef struct                          // Text screen structure
{
    int nLines;                         // Number of lines on the screen
    BYTE bScrollTop;                    // Top line of the autoscroll region
    BYTE bScrollBottom;                 // Bottom line of the autoscroll rgn
    WORD wScrollPos;                    // Critical position for autoscroll

    WORD wOffPos;                       // Current write offset
    WORD wWriteAttrib;                  // Current write attribute (high)

    WORD wMaxPos;                       // Maximum position

} TDisplay;


static TDisplay Disp = { INIT_LINES,
                         INIT_SCROLL_TOP,
                         INIT_SCROLL_BOT,
                         INIT_SCROLL_BOT * 80 * 2,
                         0,
                         ATTR_DEFAULT * 256,
                         INIT_LINES * 80 * 2 };

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   void Scroll( BYTE bTop, BYTE bBottom )                                    *
*                                                                             *
*******************************************************************************
*
*   This function performs a screen scroll up on line.
*
*   Where:
*       bTop - first line to scroll
#       bBottom - last line to scroll
*
******************************************************************************/
static void Scroll( BYTE bTop, BYTE bBottom )
{
    int nLines;

    // Calculate the number of lines to scroll

    nLines = bBottom - bTop;

    if( nLines > 0 )
    {
        // Scroll up nLines lines

        abs_memcpy( LIN_SCREEN + bTop * 160,
                    LIN_SCREEN + bTop * 160 + 160,
                    nLines * 160 );
    }

    // Clear the last line

    abs_memsetw( LIN_SCREEN + bBottom * 160,
                 ' ' + Disp.wWriteAttrib,
                 80 );
}


/******************************************************************************
*                                                                             *
*   dprintf() helper function that prints a character on the screen           *
*                                                                             *
******************************************************************************/
static void dputchar( char c )
{
    static int nRetainCount = 0;
    static BYTE bRetain[4];

    WORD wChar;
    WORD wTemp;


    // If the retained count is not zero, decrement it, and if it reached
    // zero now, we can execute special code command

    if( nRetainCount )
    {
        // Store additional character in a buffer

        bRetain[ nRetainCount-- ] = c;

        if( nRetainCount==0 )
        {
            switch( bRetain[0] )
            {
                case DP_SETWRITEATTR:

                    // Set the attribute for the writing

                    Disp.wWriteAttrib = bRetain[1] * 256;

                    // Little safety never hurts...

                    if( Disp.wWriteAttrib == 0 )
                        Disp.wWriteAttrib = (RED COLOR_ON BLACK) * 256;

                    break;

                case DP_SETCURSOR:

                    // Set the write cursor location

                    Disp.wOffPos = (bRetain[2] + bRetain[1] * 80) * 2;
                    if( Disp.wOffPos >= Disp.wMaxPos )
                        Disp.wOffPos = 0;

                    break;

                case DP_SETLOCATTR:

                    // Set an attribute at the given coordinates

                    wTemp = (bRetain[3] + bRetain[2] * 80) * 2;
                    if( wTemp < Disp.wMaxPos )
                        abs_pokeb( LIN_SCREEN + wTemp + 1, bRetain[1] );

                    break;

                case DP_SETLINES:

                    // Set the number of lines of a text display

                    if( bRetain[1]==0 || bRetain[1] > 50 )
                        return;

                    Disp.nLines = bRetain[1];
                    Disp.wMaxPos = Disp.nLines * 80 * 2;

                    break;

                case DP_SCROLL:

                    // Scroll a portion of the screen up and clear bottom line
                    // Check for valid arguments

                    if( bRetain[2] + bRetain[1] > Disp.nLines )
                        return;

                    if( bRetain[1] == 0 )
                        return;

                    Scroll( bRetain[2], bRetain[2] + bRetain[1] );

                    break;

                case DP_SETSCROLLREGION:

                    // Set an autoscroll region for the print output.  Whenever
                    // the last character is on the bScrollBottom line and it
                    // causes the next char to slide to a new line, this
                    // region is scrolled up by one line.

                    if( bRetain[1] > Disp.nLines || bRetain[2] > Disp.nLines )
                        return;

                    if( bRetain[1] < bRetain[2] )
                        return;

                    Disp.bScrollTop = bRetain[2];
                    Disp.bScrollBottom = bRetain[1];
                    Disp.wScrollPos = (bRetain[1] + 1) * 160;

                    break;
            }
        }

        return;
    }

    // Evaluate any character that may be a special sequence or code

    switch( c )
    {
        case DP_SETWRITEATTR:
        case DP_SETLINES:

            // We need an additional byte to set the working attribute (a)
            // Set the number of lines - need additional byte

            nRetainCount = 1;
            bRetain[0] = c;

            break;

        case DP_SETCURSOR:
        case DP_SCROLL:
        case DP_SETSCROLLREGION:

            // Additional 2 bytes are needed for cursor coordinates (x,y)
            // Additional 2 bytes are needed for scroll (yTop, nLines)
            // Additional 2 bytes are needed to set the autoscroll region

            nRetainCount = 2;
            bRetain[0] = c;

            break;

        case DP_SETLOCATTR:

            // We need 3 additional bytes to set arbitrary attribute (x,y,a)

            nRetainCount = 3;
            bRetain[0] = c;

            break;

        case DP_CLS:

            // Clear the screen - hopefuly this way it won't be so slow

            for( wTemp=0; wTemp<Disp.wMaxPos; wTemp += 2 )
                abs_pokew( LIN_SCREEN + wTemp, ' ' | Disp.wWriteAttrib );

            break;

        case '\n':

            // Go to a new line

            Disp.wOffPos /= 80 * 2;
            Disp.wOffPos = (Disp.wOffPos * 80 * 2) + 160;

            // Check for the autoscroll of a region

            if( Disp.wOffPos == Disp.wScrollPos )
            {
                Scroll( Disp.bScrollTop, Disp.bScrollBottom );

                // Keep the write position on the last line

                Disp.wOffPos -= 160;
            }

            // If the cursor moved over the screen bottom, move it back up

            if( Disp.wOffPos >= Disp.wMaxPos )
                Disp.wOffPos = Disp.wMaxPos - 160;

            nCharsWritten++;

            break;

        case '\r':

            // Clear to the end of a current line

            while( Disp.wOffPos % 160 )
            {
                abs_pokew( LIN_SCREEN + Disp.wOffPos, ' ' | Disp.wWriteAttrib );

                Disp.wOffPos += 2;
            }

            // Go back a character

            Disp.wOffPos -= 2;

            break;

        default:

            // All printable characters

            wChar = c | Disp.wWriteAttrib;

            abs_pokew( LIN_SCREEN + Disp.wOffPos, wChar );

            // Advance the write head location

            if( Disp.wOffPos < Disp.wMaxPos )
                Disp.wOffPos += 2;

            // Check for the autoscroll of a region

            if( Disp.wOffPos == Disp.wScrollPos )
            {
                Scroll( Disp.bScrollTop, Disp.bScrollBottom );

                // Keep the write position on the last line

                Disp.wOffPos -= 160;
            }

            // If the cursor moved over the screen bottom, move it back up

            if( Disp.wOffPos >= Disp.wMaxPos )
                Disp.wOffPos = Disp.wMaxPos - 2;

            nCharsWritten++;

            break;
    }
}


/******************************************************************************
*                                                                             *
*   int dprintf( const char *format, ... )                                    *
*                                                                             *
*******************************************************************************
*
*   This function performs as printf(), but with some extra functionality
#   added to it.  Refer to the Display.h for specifics on using this function.
*
*   Where:
*       format is the standard printf() format string
#       ... standard printf() list of arguments.
*
*   Returns:
*       number of characters actually printed.
*
******************************************************************************/
int dprintf( const char *format, ... )
{
    va_list arg;


    // Set the output function

    fnPutChar = dputchar;

    va_start( arg, format );

    return _print( format, arg );
}

