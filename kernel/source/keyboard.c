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
.-
    Module Description:

          This module contains the low-level keyboard handler code.
          Two keyboard handlers are defined - one for the normal mode of
          operation and another for the debugger.
-.
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

#include <ctype.h>                      // Character types

#include <limits.h>                     // Include limits header file

#include "display.h"

#include "inline.h"                     // Port operations, interrupts

#include "intel.h"                      // Include processor defines

#include "kernel.h"                     // Include kernel data structures

#include "keyboard.h"                   // Include its own header file

#include "pc.h"                         // Include hardware specific defines

#include "tty.h"                        // Include terminal file header

#include "device.h"                     // Include device header file

#include "process.h"                    // Include its own header

/******************************************************************************
*   Global Variables
******************************************************************************/

extern int sys_pid;

/******************************************************************************
*   Local Defines, Variables and Macros
******************************************************************************/

#define NEXT_KQUEUE(i) (((i)+1 >= MAX_INPUT)? 0 : (i)+1)


static const BYTE DefaultASCIITerm[4][128] = {
{// Normal keys
    '?',  ESC,  '1',  '2',  '3',  '4',  '5',  '6',       '7',  '8',  '9',  '0',  '-',  '=',  '\b',  '\t',
    'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',       'o',  'p',  '[',  ']',  ENTER,'?',  'a',   's',
    'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',       '\'', '`',  '?',  '\\', 'z',  'x',  'c',   'v',
    'b',  'n',  'm',  ',',  '.',  '/',  '?',  '*',       '?',  ' ',  '?',  F1,   F2,   F3,   F4,   F5,
    F6,   F7,   F8,   F9,   F10, NUMLOCK, SCROLL, HOME,  UP,  PGUP,  '?',  LEFT, '5', RIGHT, '?',   END,
    DOWN, PGDN, INS,  DEL,  '?',  '?',  '?',  F11,       F12,
},
{// Shifted keys
    '?',  ESC,  '!',  '@',  '#',  '$',  '%',  '^',       '&',  '*',  '(',  ')',  '_',  '+',  '\b', '\t',
    'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',       'O',  'P',  '{',  '}',  ENTER,'?',  'A',  'S',
    'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',       '"',  '~',  '?',  '|',  'Z',  'X',  'C',  'V',
    'B',  'N',  'M',  '<',  '>',  '?',  '?',  '*',       '?',  ' ',  '?',  SF1,  SF2,  SF3,  SF4,  SF5,
     SF6,  SF7,  SF8,  SF9,  SF10, NUMLOCK, SCROLL, HOME, UP,   PGUP,'?',  LEFT, '5',  RIGHT,'?',  END,
     DOWN, PGDN, INS,  DEL, '?',  '?',  '?',  SF11,      SF12,
},
{// Ctrl keys
    '?',        ESC,        '1',        '2',        '3',        '4',        '5',        '6',
    '7',        '8',        '9',        '0',        '-',        '=',        '\b',       '\t',
    CONTROL_Q,  CONTROL_W,  CONTROL_E,  CONTROL_R,  CONTROL_T,  CONTROL_Y,  CONTROL_U,  CONTROL_I,
    CONTROL_O,  CONTROL_P,  '[',        ']',        ENTER,      '?',        CONTROL_A,  CONTROL_S,
    CONTROL_D,  CONTROL_F,  CONTROL_G,  CONTROL_H,  CONTROL_J,  CONTROL_K,  CONTROL_L,  ';',
    '\'',       '`',        '?',        '\\',       CONTROL_Z,  CONTROL_X,  CONTROL_C,  CONTROL_V,
    CONTROL_B,  CONTROL_N,  CONTROL_M,  ',',        '.',        '/',        '?',        '*',
    '?',        ' ',        '?',        CF1,        CF2,        CF3,        CF4,        CF5,
    CF6,        CF7,        CF8,        CF9,        CF10,       NUMLOCK,    SCROLL,     HOME,
    UP,         PGUP,       '?',        LEFT,       '5',        RIGHT,      '?',        END,
    DOWN,       PGDN,       INS,        DEL,        '?',        '?',        '?',        CF11,
    CF12,
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

static volatile char kQueue[ MAX_INPUT ];  // Keyboard circular queue
static volatile int head = 0, tail = 0;     // Head and Tail of the queue


/******************************************************************************
*   Functions
******************************************************************************/


/******************************************************************************
*                                                                             *
*   static void AckKeyboard()                                                 *
*                                                                             *
*******************************************************************************
*
*   This helper function acknowledges the keyboard controller and the
#   interrupt controller.
*
******************************************************************************/
static void AckKeyboard()
{
    BYTE ScanCode;

    // Acknowledge keyboard controller

    ScanCode = inp( KBD_CONTROL );
    outp( ScanCode | 0x80, KBD_CONTROL );
    inp( PORT_DUMMY );
    outp( ScanCode, KBD_CONTROL );
    inp( PORT_DUMMY );

    // Ack the interrupt controller

    outp( PIC1, PIC_ACK );
}


/******************************************************************************
*                                                                             *
*   void Deb_Key_Handler( TIntStack Stack )                                   *
*                                                                             *
*******************************************************************************
*
#   This handler is used only when the debugger has control
#   -------------------------------------------------------
#
*   This is a low-level keyboard handler.  It translates hardware key codes
#   into ASCII and stores them in a circular keyboard buffer for the use by
#   the debugger.
*
******************************************************************************/
void Deb_Keyboard_Handler( TIntStack Stack )
{
    BYTE AsciiCode, bNext;
    BYTE ScanCode;
    BYTE Code, Pressed;
    int pid;


    AsciiCode = 0;
    ScanCode = inp( KBD_DATA );

    // On key press, bit 7 of the scan code is 0.  When the key is being
    // released, bit 7 is 1.

    Code = ScanCode & 0x7F;
    Pressed = (ScanCode >> 7) ^ 1;

    // Check for shift keys
    //
    if( (Code == SC_LEFT_SHIFT) || (Code == SC_RIGHT_SHIFT) )
    {
        // Determine if shift was pressed or depressed (bit 7):
        // fShift is 1 if shift has been pressed
        //           0 if shift has been released
        fShift = Pressed;
    }
    else

    // Check for control keys
    //
    if( Code == SC_CONTROL )
    {
        // Determine if control was pressed or depressed (bit 7)
        // fControl is 1 if control key has been pressed
        //             0 if control key has been released
        fControl = Pressed;
    }
    else

    // Check for caps lock key
    //
    if( Code == SC_CAPS_LOCK )
    {
        // Toggle caps lock state on press
        fCapsLock ^= Pressed;

    }
    else

    // Now return if key was released.  Nothing to do.
    // Store the code in a queue if a key was pressed.
    //
    if( Pressed )
    {
        // Map a scancode to an ASCII code
        //
        AsciiCode = DefaultASCIITerm[ (fControl<<1) | fShift ][ Code ];




        // DEBUGGER BREAK

        if( AsciiCode==SCROLL )
        {
            // Acknowledge keyboard controller and PIC

            AckKeyboard();

            // The way we will `exit' to debugger is by setting the trap
            // bit in the eflags on the current stack record.  That will
            // cause INT1 trap into debugger the very next instruction.
            // Cool, huh?

            Stack.eflags |= TF_MASK;

            return;
        }






        // Caps Lock key inverts the caps state of the alphabetical characters
        if( fCapsLock && isalpha(AsciiCode) )
            AsciiCode ^= 0x20;

        // Find if that key should show a terminal

        pid = ProcessHotKey[ AsciiCode ];

        if( pid != -1 )
        {
            // If the current VM is the system VM, set the controlling term

            if( pid == sys_pid )
                pCurTTY = TTY[ AsciiCode - SF1];
            else
                pCurTTY = pProc[pid]->pTTY;

            if( pCurTTY != NULL )
                abs_memcpy( LIN_SCREEN, LIN_KERNEL + (DWORD)pCurTTY->screen, SCREEN_SIZE );

            AckKeyboard();

            return;
        }
    }

    if( AsciiCode != 0 )
    {
        // If a key was pressed (as opposed of released), queue it in
        // the kernel keyboard queue if it is not full

        bNext = NEXT_KQUEUE( tail );

        if( bNext != head )
        {
            kQueue[ tail ] = AsciiCode;
            tail = bNext;
        }
    }

    // Ack the interrupt controller

    AckKeyboard();
}



/******************************************************************************
*                                                                             *
*   void Int_Key_Handler( TIntStack Stack )                                   *
*                                                                             *
*******************************************************************************
*
#   This is the working kernel keyboard handler
#   -------------------------------------------
#
*   This is a low-level keyboard handler.  It translates hardware key codes
#   into ASCII and stores them in a controlling terminal buffer or reflects
#   them into the current VM.
*
******************************************************************************/
void Int_Keyboard_Handler( TIntStack Stack )
{
    BYTE AsciiCode;
    BYTE ScanCode;
    BYTE Code, Pressed;
    int new_pid;


    AsciiCode = 0;
    ScanCode = inp( KBD_DATA );

    // On key press, bit 7 of the scan code is 0.  When the key is being
    // released, bit 7 is 1.

    Code = ScanCode & 0x7F;
    Pressed = (ScanCode >> 7) ^ 1;

    // Check for shift keys
    //
    if( (Code == SC_LEFT_SHIFT) || (Code == SC_RIGHT_SHIFT) )
    {
        // Determine if shift was pressed or depressed (bit 7):
        // fShift is 1 if shift has been pressed
        //           0 if shift has been released
        fShift = Pressed;
    }
    else

    // Check for control keys
    //
    if( Code == SC_CONTROL )
    {
        // Determine if control was pressed or depressed (bit 7)
        // fControl is 1 if control key has been pressed
        //             0 if control key has been released
        fControl = Pressed;
    }
    else

    // Check for caps lock key
    //
    if( Code == SC_CAPS_LOCK )
    {
        // Toggle caps lock state on press
        fCapsLock ^= Pressed;

    }
    else

    // Now return if key was released.  Nothing to do.
    // Store the code in a queue if a key was pressed.
    //
    if( Pressed )
    {
        // Map a scancode to an ASCII code
        //
        AsciiCode = DefaultASCIITerm[ (fControl<<1) | fShift ][ Code ];

        // DEBUGGER BREAK

        if( AsciiCode==SCROLL )
        {
            // Acknowledge keyboard controller and PIC

            AckKeyboard();

            // The way we will `exit' to debugger is by setting the trap
            // bit in the eflags on the current stack record.  That will
            // cause INT1 trap into debugger the very next instruction.
            // Cool, huh?

            Stack.eflags |= TF_MASK;

            return;
        }

        // Caps Lock key inverts the caps state of the alphabetical characters

        if( fCapsLock && isalpha(AsciiCode) )
            AsciiCode ^= 0x20;


        // Find if that key should activate a process

        new_pid = ProcessHotKey[ AsciiCode ];

        if( new_pid != -1 )
        {
            // Activate the process if the process is not the current one

            if( pCurProc->pid != new_pid )
            {
                // Acknowledge keyboard controller and PIC

                AckKeyboard();

                SetCurrentVM( new_pid, &Stack );

                // If the current VM is the system VM, set the controlling term

                if( new_pid == sys_pid )
                {
                    if( TTY[ AsciiCode - SF1] != NULL )
                    {
                        pCurTTY = TTY[ AsciiCode - SF1];
                        abs_memcpy( LIN_SCREEN, LIN_KERNEL + (DWORD)&pCurTTY->screen, SCREEN_SIZE );
                    }
                }

                return;
            }
        }

        // Reflect the character to the controlling terminal input if the
        // current terminal is defined

        if( Pressed && (pCurTTY != NULL) )
        {
            (pCurTTY->fnIn)(pCurTTY, AsciiCode );
        }
    }

    // If we have interrupted V86 mode execution, we need to reflect this
    // interrupt `down' so that the current VM would get it

    if( Reflect_V86_Int( 9, &Stack )==FALSE )
    {
        // Keyboard interrupts has not been sent to a VM

        AckKeyboard();
    }

    // Ack the interrupt controller

    outp( PIC1, PIC_ACK );
}


/******************************************************************************
*                                                                             *
*   char GetKey( BOOL fBlock )                                                *
*                                                                             *
*******************************************************************************
*
*   This function returns a key from the keyboard buffer.
#   If a key is not available and the fBlock argument is True, it polls until
#   a key becomes available.  Otherwise, it returns 0 (False).
*
*   Where:
*       fBlock is a blocking request.  If set to True, the function polls
#       the keyboard until a key is available.
*
*   Returns:
*       0 If no key was available
#       ASCII code of a next key in a queue
*
******************************************************************************/
char GetKey( BOOL fBlock )
{
    char c;


    // If the blocking is False, return 0 if a key is not available

    if( fBlock==FALSE && head==tail )
        return( 0 );

    // Poll for the input character

    while( head == tail ) {;}

    // Get a character from the keyboard queue - make it uninterruptible

    DisableInterrupts();

    c = kQueue[ head ];

    head = NEXT_KQUEUE( head );

    EnableInterrupts();

    return( c );
}


