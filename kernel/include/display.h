/******************************************************************************
*                                                                             *
*   Module:     Display.h                                                     *
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

        This is a header file for the direct display access functions.
        It is used mainly from the debugger.


    This is how you use the dprintf() function:

    dprintf("Standard printf() %d %08X %s %c...\n", -2, 5, "abc", 'X' );
    dprintf("New attrib %c%cNEW\n", DP_SETWRITEATTR, GREY COLOR_ON BLUE );
    dprintf("Scroll %c%c%c\n", DP_SCROLL, 0, 24 );
    ...
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
*   Important Defines                                                         *
******************************************************************************/
#ifndef _DISPLAY_H_
#define _DISPLAY_H_

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

#define MAX_LINES       50              // Maximum number of lines on the scrn

#define SCREEN_SIZE     (80*2*MAX_LINES)

#define LIN_SCREEN      0xB8000         // Linear address of it


// The additional bytes to the following command codes are:
//
// DP_SETWRITEATTR, Attribute
// DP_SETCURSOR, xLoc, yLoc
// DP_SETLOCATTR, xLoc, yLoc, Attribute
// DP_SETLINES, bTotalLines
// DP_SCROLL, yTop, bLines   (1 >= bLines >= bTotalLines)
// DP_SETSCROLLREGION, yTop, yBottom   (1 >= yTop >= yBottom >= bTotalLines)

#define DP_SETWRITEATTR     1           // Sets the const. writing attribute
#define DP_SETCURSOR        2           // Sets the location
#define DP_SETLOCATTR       3           // Sets an arbitrary attribute
#define DP_CLS              4           // Clears the screen
#define DP_SETLINES         5           // Set the total number of lines
#define DP_SCROLL           6           // Scrolls part of the screen up,
                                        // erasing the bottom line
#define DP_SETSCROLLREGION  7           // Sets auto-scroll region

// Define color attributes

#define BLACK               0
#define BLUE                1
#define GREEN               2
#define CYAN                3
#define RED                 4
#define MAGENTA             5
#define BROWN               6
#define GREY                7
#define DARKGREY            8
#define LIGHTBLUE           9
#define LIGHTGREEN          10
#define LIGHTCYAN           11
#define LIGHTRED            12
#define LIGHTMAGENTA        13
#define YELLOW              14
#define WHITE               15

// Using the following word you can combine colors into an attribute
// example:  YELLOW COLOR_ON BLUE

#define COLOR_ON            + 16 *


//-----------------------------------------------------------------------------
// Set some default screen attributes
//-----------------------------------------------------------------------------
#define ATTR_RESPONSE   (GREY COLOR_ON BLACK)
#define ATTR_RESPONSEHI (WHITE COLOR_ON BLACK)
#define ATTR_SCREAM     (RED COLOR_ON CYAN)
#define ATTR_SYSTEM     (WHITE COLOR_ON BLUE)
#define ATTR_SYSTEMHI   (WHITE COLOR_ON RED)


/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

int dprintf( const char *format, ... );


#endif //  _DISPLAY_H_
