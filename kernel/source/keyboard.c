/******************************************************************************
*
*   Module:     keyboard.c
*
*   Revision:   1.00
*
*   Date:       08/03/96
*
*   Author:     Goran Devic
*
*******************************************************************************

    Module Description:

          This module contains the code for the kernel keyboard handler.

*******************************************************************************
*
*   Changes:
*
*   DATE     REV   DESCRIPTION OF CHANGES                          AUTHOR
* --------   ----  ---------------------------------------------   -----------
* 08/03/96   1.00  Original                                        Goran Devic
* --------   ----  ---------------------------------------------   -----------
*******************************************************************************
*   Include Files
******************************************************************************/
#include <stdio.h>                     // Include standard header file

#include <debugger.h>                  // Include debugger types
#include <ctype.h>                     // Include character types
#include <kernel.h>
#include <ports.h>
#include <ttys.h>                      // Include terminal structures
#include <types.h>
#include <interrpt.h>

/******************************************************************************
*   Global Variables
******************************************************************************/

/******************************************************************************
*   Local Defines, Variables and Macros
******************************************************************************/

#define TTYIN       TTY[ctty].in       // Define a shortcut for tty access

#define SC_CONTROL          29         // Control key key scan code
#define SC_LEFT_SHIFT       42         // Left shift key scan code
#define SC_RIGHT_SHIFT      54         // Right shift key scan code
#define SC_CAPS_LOCK        58         // Caps lock key scan code

#define ESC           27
#define ENTER         '\r'
#define F1            128
#define F2            129
#define F3            130
#define F4            131
#define F5            132
#define F6            133
#define F7            134
#define F8            135
#define F9            136
#define F10           137
#define F11           138
#define F12           139
#define NUMLOCK       18
#define SCROLL        19
#define HOME          20
#define UP            21
#define PGUP          22
#define LEFT          23
#define RIGHT         24
#define END           25
#define DOWN          26
#define PGDN          28
#define INS           29
#define DEL           30


static const BYTE DefaultASCIITerm[4][256] = {
{// Normal keys
    '?',  ESC,  '1',  '2',  '3',  '4',  '5',  '6',       '7',  '8',  '9',  '0',  '-',  '=',  '\b',  '\t',
    'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',       'o',  'p',  '[',  ']',  ENTER,'?',  'a',   's',
    'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',       '\'', '`',  '?',  '\\', 'z',  'x',  'c',   'v',
    'b',  'n',  'm',  ',',  '.',  '/',  '?',  '*',       '?',  ' ',  '?',  F1,   F2,   F3,   F4,   F5,
     F6,   F7,   F8,   F9,  F10, NUMLOCK, SCROLL, HOME,  UP,  PGUP,  '?',  LEFT, '5', RIGHT, '?',   END,
     DOWN, PGDN, INS,  DEL, '?',  '?',  '?',  F11,       F12,
},
{// Shifted keys
    '?',  ESC,  '!',  '@',  '#',  '$',  '%',  '^',       '&',  '*',  '(',  ')',  '_',  '+',  '\b', '\t',
    'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',       'O',  'P',  '{',  '}',  ENTER,'?',  'A',  'S',
    'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',       '"',  '~',  '?',  '|', 'Z',  'X',  'C',   'V',
    'B',  'N',  'M',  '<',  '>',  '?',  '?',  '*',       '?',  ' ',  '?',  F1,  F2,   F3,   F4,   F5,
     F6,   F7,   F8,   F9,  F10, NUMLOCK, SCROLL, HOME,   UP, PGUP,  '?',  LEFT, '5', RIGHT, '?',  END,
     DOWN, PGDN, INS,  DEL, '?',  '?',  '?',  F11,       F12,
},
{// Ctrl keys
    '?',        ESC,        '1',        '2',        '3',        '4',        '5',        '6',
    '7',        '8',        '9',        '0',        '-',        '=',        '\b',       '\t',
    CONTROL_Q,  CONTROL_W,  CONTROL_E,  CONTROL_R,  CONTROL_T,  CONTROL_Y,  CONTROL_U,  CONTROL_I,
    CONTROL_O,  CONTROL_P,  '[',        ']',        ENTER,      '?',        CONTROL_A,  CONTROL_S,
    CONTROL_D,  CONTROL_F,  CONTROL_G,  CONTROL_H,  CONTROL_J,  CONTROL_K,  CONTROL_L,  ';',
    '\'',       '`',        '?',        '\\',       CONTROL_Z,  CONTROL_X,  CONTROL_C,  CONTROL_V,
    CONTROL_B,  CONTROL_N,  CONTROL_M,  ',',        '.',        '/',        '?',        '*',
    '?',        ' ',        '?',        F1,         F2,         F3,         F4,         F5,
    F6,         F7,         F8,         F9,         F10,        NUMLOCK,    SCROLL,     HOME,
    UP,         PGUP,       '?',        LEFT,       '5',        RIGHT,      '?',        END,
    DOWN,       PGDN,       INS,        DEL,        '?',        '?',        '?',        F11,
    F12,
},
{// Shift-ctrl keys
    '?',        ESC,        '1',        '2',        '3',        '4',        '5',        '6',
    '7',        '8',        '9',        '0',        '-',        '=',        '\b',       '\t',
    CONTROL_Q,  CONTROL_W,  CONTROL_E,  CONTROL_R,  CONTROL_T,  CONTROL_Y,  CONTROL_U,  CONTROL_I,
    CONTROL_O,  CONTROL_P,  '[',        ']',        ENTER,      '?',        CONTROL_A,  CONTROL_S,
    CONTROL_D,  CONTROL_F,  CONTROL_G,  CONTROL_H,  CONTROL_J,  CONTROL_K,  CONTROL_L,  ';',
    '\'',       '`',        '?',        '\\',       CONTROL_Z,  CONTROL_X,  CONTROL_C,  CONTROL_V,
    CONTROL_B,  CONTROL_N,  CONTROL_M,  ',',        '.',        '/',        '?',        '*',
    '?',        ' ',        '?',        F1,         F2,         F3,         F4,         F5,
    F6,         F7,         F8,         F9,         F10,        NUMLOCK,    SCROLL,     HOME,
    UP,         PGUP,       '?',        LEFT,       '5',        RIGHT,      '?',        END,
    DOWN,       PGDN,       INS,        DEL,        '?',        '?',        '?',        F11,
    F12,
}
};


static BYTE fShift = 0;
static BYTE fControl = 0;
static BYTE fCapsLock = 0;


/******************************************************************************
*   Functions
******************************************************************************/

/******************************************************************************
*
*   interrupt ProcessKeyboard()
*
*   Low-level keyboard handler
*
*   This function reads the code from the keyboard and stores the 
*   ASCII value in the keyboard buffer.  Codes are stored without any
*   processing, except that Start and Stop keys are processed and
*   they set the stop flag in the controlling tty.
*
*   Where:
*
*
*   Returns:
*
******************************************************************************/
interrupt ProcessKeyboard()
{
    BYTE AsciiCode, bNext;
    BYTE ScanCode = Inpb( 0x60 );


    // On key press, bit 7 of the scan code is 0.  When the key is being
    // released, bit 7 is 1.

    // Check for shift keys
    //
    if( ((ScanCode&0x7f) == SC_LEFT_SHIFT) || ((ScanCode&0x7f) == SC_RIGHT_SHIFT) )
    {
        // Determine if shift was pressed or depressed (bit 7):
        // fShift is 1 if shift has been pressed
        //           0 if shift has been released
        fShift = (ScanCode >> 7) ^ 1;
        goto End;
    }

    // Check for control keys
    //
    if( (ScanCode&0x7f) == SC_CONTROL )
    {
        // Determine if control was pressed or depressed (bit 7)
        // fControl is 1 if control key has been pressed
        //             0 if control key has been released
        fControl = (ScanCode >> 7) ^ 1;
        goto End;
    }

    // Check for caps lock key
    //
    if( (ScanCode&0x7f) == SC_CAPS_LOCK )
    {
        // Toggle caps lock state on press
        fCapsLock ^= (ScanCode >> 7) ^ 1;
        goto End;
    }


    // Now return if key was released.  Nothing to do.
    //
    if( ScanCode&0x80 )
        goto End;

    
    // Map a scancode to an ASCII code
    //
    AsciiCode = DefaultASCIITerm[ (fControl<<1) | fShift ][ ScanCode ];

    // Caps Lock key inverts the caps state of the alphabetical characters
    if( fCapsLock && isalpha(AsciiCode) )
        AsciiCode ^= 0x20;


    // If ESC was pressed, escape into debugger
    //
    if( AsciiCode==ESC )
    {
       EnableInterrupts();

       // Acknowledge keyboard controller
       //
       ScanCode = Inpb( 0x61 );
       Outpb( ScanCode | 0x80, 0x61 );
       Inpb( 0x80 );
       Outpb( ScanCode, 0x61 );
       Inpb( 0x80 );

       // Ack the interrupt controller
       //
       Outpb( 0x20, 0x20 );

       INT1();
    }

    // If stop/start character is pressed and the terminal is in cooked
    // mode, just set the flag
    //
    if( AsciiCode == TTY[ctty].c_cc[VSTOP] )
    {
        TTY[ctty].stop = 1;
        goto End;
    }

    if( AsciiCode == TTY[ctty].c_cc[VSTART] )
    {
        TTY[ctty].stop = 0;
        goto End;
    }


    // Now store the ascii code in the input queue of the current terminal
    //
    // Store a character only if buffer is not full
    bNext = TTYIN.tail + 1;

    if( bNext >= TTY_BUFLEN )
        bNext = 0;

    if( bNext != TTYIN.head )
    {
        TTYIN.buf[ TTYIN.tail ] = AsciiCode;
        TTYIN.tail = bNext;
    }


End:

#if 1
    // Acknowledge keyboard controller
    //
    ScanCode = Inpb( 0x61 );
    Outpb( ScanCode | 0x80, 0x61 );
    Inpb( 0x80 );
    Outpb( ScanCode, 0x61 );
    Inpb( 0x80 );

    // Ack the interrupt controller
    //
    Outpb( 0x20, 0x20 );
#else
    // Just call back keyboard BIOS handler
    //
    PassV86Interrupt( 9 );
#endif
}


/*********************************************************************
*
*   char keyboard_in_char( BYTE tty_no )
*
*       This is a low-level function for getting a key from the
*       terminal.
*
*   Where:
*       tty_no is the terminal number from which to get the character
*
*   Returns:
*       input character
*
**********************************************************************/
char keyboard_in_char( BYTE tty_no )
{
    TTY_Struct *ptty = &TTY[tty_no];
    BYTE c, bNext;


    // Poll for the input character
    //
    while( ptty->in.head == ptty->in.tail ) {;}

    // Get the character from the queue
    //
    c = ptty->in.buf[ ptty->in.head ];

    bNext = ptty->in.head + 1;
    if( bNext >= TTY_BUFLEN )
        bNext = 0;
    ptty->in.head = bNext;

    return( (char)c );    
}


