/*********************************************************************
*                                                                    *
*   Module:     display.c 
*                                                                    *
*   Revision:   1.00                                                 *
*                                                                    *
*   Date:       06/29/96                                             *
*                                                                    *
*   Author:     Goran Devic                                          *
*                                                                    *
**********************************************************************
*                                                                    *
*   Module Description:                                               *

          This module implements the basic display output code
          
**********************************************************************
*                                                                    *
*   Changes:                                                         *
*                                                                    *
*   DATE     REV   DESCRIPTION OF CHANGES                 AUTHOR     *
* --------   ----  -----------------------------------   -----------
  06/29/96   1.00  Original                              Goran Devic

* --------   ----  -----------------------------------   ----------- *
**********************************************************************
*   Important defines/undefines
**********************************************************************/

/*********************************************************************
*   Include Files
**********************************************************************/
#include "mm.h"                 // Include basic memory management
#include "ports.h"
#include "ttys.h"               // Include terminal structures
#include "types.h"

/*********************************************************************
*   Defines
**********************************************************************/
#define VGA_CRTC_REG    0x3d4   // VGA CRTC index register
#define VGA_CRTC_VAL    0x3d5   // VGA CRTC value register

/*********************************************************************
*   External Functions
**********************************************************************/

/*********************************************************************
*   Global variables
**********************************************************************/

/*********************************************************************
*   Local Functions
**********************************************************************/
/*********************************************************************
*
*   void set_cursor( tty_no )
*
*       Sets the VGA hardware cursor if the given terminal number is
*       the controlling terminal.
*
*   Where:
*       tty_no is the terminal number
*
**********************************************************************/
static void set_cursor( tty_no )
{
    TTY_Struct *ptty = &TTY[tty_no];
    int offset;
    BYTE hi, lo;

    // Check that the terminal is the controlling
    if( tty_no == ctty )
    {
        // Calculate the cursor address from the coordinates
        //
        offset = ptty->y * 80 + ptty->x;
        lo = offset & 0xff;
        hi = offset >> 8;

        // Program VGA CRTC registers to set the cursor position
        //
        Outpb( VGA_CRTC_REG, 14 );  // Cursor location high byte
        Inpb(0x80);
        Outpb( VGA_CRTC_VAL, hi );
        Inpb(0x80);
        Outpb( VGA_CRTC_REG, 15 );;  // Cursor location low byte
        Inpb(0x80);
        Outpb( VGA_CRTC_VAL, lo );
        Inpb(0x80);
    }
}


/*********************************************************************
*
*   static void scroll_up( BYTE tty_no )
*
*       Scrolls the screen buffer of the given terminal structure
*       one line up.  If the terminal is controlling, the display
*       will be scrolled as well.
*
*       The emerging line will be set to spaces.
*
*   Where:
*       tty_no is the terminal number to scroll
*
**********************************************************************/
static void scroll_up( BYTE tty_no )
{
    TTY_Struct *ptty = &TTY[tty_no];


    // Scroll the terminal buffer
    //
    kmemcpy( ptty->screen, ptty->screen + 80, 80*(TTY_LINES-1) );
    kmemset( ptty->screen + 80*(TTY_LINES-1), 80, ' ' );

    // Scroll the display only if this is the controlling terminal
    //
    if( tty_no == ctty )
    {
        AbsMemcpyw( 0xB8000, 0xB8000 + 80*2, 80*2*(TTY_LINES-1) );
        AbsMemsetw( 0xB8000+80*2*(TTY_LINES-1), 80, 0x700 | ' ' );
    }
}


/*********************************************************************
*
*   void display_out_char( BYTE tty_no, char c )
*
*       This function is the low-level output character function.
*       The character will be output to the specified terminal number,
*       and sent to the display only if that is currently controlling
*       terminal.
*
*       If the stop flag is set, no output will be performed.
*
*       The tabulator is expanded and '\n' performs CR/LF
*       function.
*
*       TTY_CURSOR_LEFT code is processed here.
*
*       VGA hardware cursor is used.
*
*
*   Where:
*       tty_no  - terminal structure to which to print
*       c       - character to be printed
*
**********************************************************************/
void display_out_char( BYTE tty_no, char c )
{
    TTY_Struct *ptty = &TTY[tty_no];
    int i;


    // Avoid any kind of output if stop flag is set
    //
    if( ptty->stop )
        return;


    // Look for the LF character and insert CR before itself
    //
    if( c == '\n' )
        display_out_char( tty_no, '\r' );


    // Expand the tabulator code into spaces
    //
    if( c == '\t' )
    {
        // Calculate the number of spaces to pad
        i = ptty->tabs - (ptty->x % ptty->tabs);

        // Call itself to fill in the tab spaces
        for( ; i>0; i--)
            display_out_char( tty_no, ' ' );

        return;
    }


    // Evaluate CR character
    //
    if( c == '\r' )
    {
        ptty->x = 0;
        set_cursor( tty_no );
        return;
    }

    // Evaluate LF character
    //
    if( c == '\n' )
    {
        if( ++ptty->y == TTY_LINES )
        {
             ptty->y--;
             scroll_up( tty_no );
        }

        set_cursor( tty_no );
        return;
    }

    // Evaluate TTY_CURSOR_LEFT code
    if( c == TTY_CURSOR_LEFT )
    {
        if( ptty->x == 0 )
        {
            // Go to the preceeding line, if possible
            if( ptty->y > 0 )
            {
                ptty->y--;
                ptty->x = 80;
            }
        }
        else
            ptty->x--;

        set_cursor( tty_no );
        return;
    }


    // Here we have a printable character.  Stuff it in the buffer first
    //
    ptty->screen[ ptty->y ][ ptty->x ] = (BYTE)c;


    // If the terminal is controlling, print to the screen as well
    //
    if( tty_no == ctty )
         AbsPokew( 0xb8000 + ptty->y*80*2 + ptty->x*2, (ptty->attrib << 8) | c );
    

    // Adjust the screen position
    //
    if( ++ptty->x == 80 )
    {
         ptty->x = 0;

         // See if scrolling is needed
         if( ++ptty->y == TTY_LINES )
         {
             ptty->y--;
             scroll_up( tty_no );
         }
    }

    set_cursor( tty_no );
}

