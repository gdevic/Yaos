/*********************************************************************
*                                                                    *
*   Module:     ttys.c                                               *
*                                                                    *
*   Revision:   1.00                                                 *
*                                                                    *
*   Date:       04/18/96                                             *
*                                                                    *
*   Author:     Goran Devic                                          *
*                                                                    *
**********************************************************************
*                                                                    *
*   Module Description:                                              *

        This module contains the code for the teletype terminal
        (tty).
       
**********************************************************************
*                                                                    *
*   Changes:                                                         *
*                                                                    *
*   DATE     REV   DESCRIPTION OF CHANGES                 AUTHOR     *
* --------   ----  -----------------------------------   -----------
  04/18/96   1.00  Original                              Goran Devic

* --------   ----  -----------------------------------   ----------- *
**********************************************************************
*   Important defines/undefines
**********************************************************************/

/*********************************************************************
*   Include Files
**********************************************************************/
#include "interrpt.h"           // Include interrupt header
#include "ports.h"              // Include basic ports I/O
#include "ttys.h"               // Include terminal types

/*********************************************************************
*   Global Variables
**********************************************************************/
BYTE ctty;                                // Controlling terminal number
TTY_Struct TTY[ NR_TTYS ];                // Terminal structure

/*********************************************************************
*   Local Variables and defines
**********************************************************************/

/*********************************************************************
*   Local Functions
**********************************************************************/

/*********************************************************************
*
*   InitTTYs()
*
*   Initializes ttys: keyboard, display.  Sets the controlling terminal
*   number (for the foreground process).
*   
**********************************************************************/
void InitTTYs()
{
    int i;


    // Init each tty structure
    //
    for( i=0; i<NR_TTYS; i++ )
    {
        // Set the initial flags
        TTY[i].c_iflag = ICRNL + IXON;
        TTY[i].c_oflag = 0;
        TTY[i].c_lflag = ECHO + ECHOE + ECHOK + ECHONL + ICANON;
        TTY[i].c_cflag = 0;

        // Fill in the control characters in the c_cc array
        TTY[i].c_cc[VEOF]  = EOF_CHAR;
        TTY[i].c_cc[VEOL]  = EOL_CHAR;
        TTY[i].c_cc[VERASE]= ERASE_CHAR;
        TTY[i].c_cc[VINTR] = INTR_CHAR;
        TTY[i].c_cc[VKILL] = KILL_CHAR;
        TTY[i].c_cc[VQUIT] = QUIT_CHAR;
        TTY[i].c_cc[VSUSP] = SUSP_CHAR;
        TTY[i].c_cc[VSTART]= START_CHAR;
        TTY[i].c_cc[VSTOP] = STOP_CHAR;
        
        // Set private fields of the terminal structure
        TTY[i].x = 0;
        TTY[i].y = 0;
        TTY[i].stop = 0;
        TTY[i].tabs = 8;
        TTY[i].attrib = 7;

        // Set the input and output queue indices
        TTY[i].in.head = 0;
        TTY[i].in.tail = 0;
        TTY[i].out.head = 0;
        TTY[i].out.tail = 0;

        // Set the input and output functions
        TTY[i].out_fn = display_out_char;
        TTY[i].in_fn = keyboard_in_char;
    }
    
    // Set console controlling tty number to terminal 0
    ctty = 0;

    // Register and enable keyboard handler function
    //
    if( !RegisterIRQHandler( IRQ_KEYBOARD, (void (*)()) ProcessKeyboard ) )
       Halt("Unable to register keyboard handler!");

    IRQControl( IRQ_ENABLE, IRQ_KEYBOARD );

//    printk("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
//    printk("TTY/Keyboard\n");
}


 /*********************************************************************
*
*   char dummy_in_char( BYTE tty_no )
*
*       Dummy input character function
*
*   Where:
*       tty_no is the terminal number
*
*   Returns:
*       SIGINT character for a particular terminal
*
**********************************************************************/
char dummy_in_char( BYTE tty_no )
{
    return( TTY[tty_no].c_cc[ VINTR ] );
}


/*********************************************************************
*
*   void tty_read( BYTE tty_no, char * buf, int max_len )
*
*       This function reads from a terminal.  It uses the terminal's
*       current setting (raw/cooked mode etc)
*
*       If INLCR and '\n', translate to '\r'
*       If IGNCR and '\r', ignore it
*       If ICRNL and '\r', translate to '\n'
*
*       If ICANON
*           if ERASE_CHAR, erase the last character
*           if KILL_CHAR, erase the last line
*
*       If ECHO
*           character is output back to the terminal via tty_write
*       else
*           if EOL_CHAR and ECHONL, output EOL_CHAR
*
*
*   Where:
*       tty_no  - number of terminal to read from
*       buf     - destination buffer to be filled in
*       max_len - maximal string length
*
**********************************************************************/
void tty_read( BYTE tty_no, char * buf, int max_len )
{
    TTY_Struct *ptty = &TTY[tty_no];
    char c;
    int real_len = 0;


AnotherChar:

    // Get the next character
    //
    c = ptty->in_fn( tty_no );


    // If '\n' and INLCR, set CR instead
    if( (c == '\n') && (ptty->c_iflag & INLCR) )
        c = '\r';

    // If '\r' and IGNCR, ignore character
    if( (c == '\r') && (ptty->c_iflag & IGNCR) )
        goto AnotherChar;

    // If '\r' and ICRNL, set LF instead
    if( (c == '\r' ) && (ptty->c_iflag & ICRNL) )
        c = '\n';


    // Check the raw/cooked mode
    //
    if( ptty->c_lflag & ICANON )
    {
        // Cooked mode - we have to do everything here :-(
        // But we do not return until EOF or CR character
        //
        // Backspace?
        if( (c == ptty->c_cc[VERASE]) && (real_len>0) )
        {
             real_len--;
        }
        else
        // Erase the whole line?
        if( c == ptty->c_cc[VKILL] )
        {
             real_len = 0;
        }
        else
        // Store the character in the user buffer
            buf[real_len++] = c;
    }
    else
        // In non-canonical mode, just store the character
        buf[real_len++] = c;
    

    // Check if echo is enabled
    //
    if( ptty->c_lflag & ECHO )
    {
        tty_write( tty_no, c );
    }
    else
    // If '\n' and ECHONL, echo \n even if echo is off
    if( (c == '\n') && (ptty->c_lflag & ECHONL) )
         tty_write( tty_no, '\n' );
        

    // In caconical mode loop until a sensitive character is not encountered
    // or maximum characters is read.
    //
    if( ptty->c_lflag & ICANON )
    {
        if( (c != ptty->c_cc[VEOF])  && (c != ptty->c_cc[VEOL]) && 
            (c != ptty->c_cc[VINTR]) && (c != ptty->c_cc[VSUSP]) && 
            (real_len < max_len ) )
             goto AnotherChar;

        // Set the terminating zero at the end of the input string
        //
        if( real_len == max_len )
            real_len--;

        buf[real_len] = (char)0;
    }
}


/*********************************************************************
*
*   void tty_write( BYTE tty_no, char c )
*
*       This function writes to a terminal.  It uses the terminal's
*       current setting (raw/cooked mode etc)
*
*
*       If ICANON
*           if ERASE_CHAR, erase the last char from the display
*           if KILL_CHAR, erase the last line from the display
*
*
*   Where:
*         tty_no - number of the terminal to write to
*         c - character to be written
*
**********************************************************************/
void tty_write( BYTE tty_no, char c )
{
    TTY_Struct *ptty = &TTY[tty_no];


    // If in cooked mode, process erase and kill characters
    //
    if( ptty->c_lflag & ICANON )
    {
        if( c == ptty->c_cc[VERASE] )
        {
            // Erase the last character
            // Left, space, left
            ptty->out_fn( tty_no, TTY_CURSOR_LEFT );
            ptty->out_fn( tty_no, ' ');
            ptty->out_fn( tty_no, TTY_CURSOR_LEFT );
        }
        else
        if( c == ptty->c_cc[VKILL] )
        {
            // Kill line character, simulated with a CR code
            ptty->out_fn( tty_no, '\n');
        }
        else
            // Printable character
            ptty->out_fn( tty_no, c );
    }
    else
        // Raw mode: output characters as they are
        ptty->out_fn( tty_no, c );
}

