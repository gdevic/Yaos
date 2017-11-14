/*********************************************************************
*                                                                    *
*   Module:     ttys.h                                               *
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

        Header file for the teletype type terminal (tty)
       
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
#ifndef _TTYS_H_
#define _TTYS_H_

/*********************************************************************
*   Include Files
**********************************************************************/
#include "types.h"

/*********************************************************************
*   General Defines
**********************************************************************/
#define NR_TTYS         8       // Number of TTY structures
#define TTY_BUFLEN      256     // TTY input and output buffer lengths
#define TTY_LINES       50      // Terminal horizontal lines

#define TTY_CURSOR_LEFT 1       // Cursor left code

/*********************************************************************
*   Character Codes
**********************************************************************/

// Control-D  end-of-file character, break through cooked mode
// Control-J  CR character
// Control-H  erase character
// Control-U  erase whole line
// Control-C  stop signal, SIGQUIT
// Control-Z  suspend process signal, SIGINT

// Control-G  bell
// Control-S  suspense output
// Control-Q  resume output
//
// CR is '\r', code 13 (0x0d)
// LF is '\n', code 10 (0x0a)
//
// Define control codes
#define CONTROL_Q      128+'Q'
#define CONTROL_W      128+'W'
#define CONTROL_E      128+'E'
#define CONTROL_R      128+'R'
#define CONTROL_T      128+'T'
#define CONTROL_Y      128+'Y'
#define CONTROL_U      0x15
#define CONTROL_I      128+'I'
#define CONTROL_O      128+'O'
#define CONTROL_P      128+'P'
#define CONTROL_A      128+'A'
#define CONTROL_S      128+'S'
#define CONTROL_D      0x1A
#define CONTROL_F      128+'F'
#define CONTROL_G      128+'G'
#define CONTROL_H      '\b'
#define CONTROL_J      '\n'
#define CONTROL_K      128+'K'
#define CONTROL_L      128+'L'
#define CONTROL_Z      128+'Z'
#define CONTROL_X      128+'X'
#define CONTROL_C      128+'C'
#define CONTROL_V      128+'V'
#define CONTROL_B      128+'B'
#define CONTROL_N      128+'N'
#define CONTROL_M      128+'M'
#define CONTROL_BS     128+'\\'                 


/*********************************************************************
*   Terminal Structures
**********************************************************************/
typedef unsigned int tcflag_t;  // POSIX type tcflag_t
typedef unsigned char cc_t;     // POSIX type cc_t


// These define offsets of the control characters in the c_cc field of the
// terminal structure.  They will be filled with the default characters
// that are defined immediately following it
//
#define VEOF           0         // offset of EOF character
#define VEOL           1         //           newline
#define VERASE         2         //           backspace
#define VINTR          3         //           generates SIGINT
#define VKILL          4         //           kill char
#define VQUIT          5         //           generates SIGQUIT
#define VSUSP          6         //           generates SIGTSTP
#define VSTART         7         //           start char
#define VSTOP          8         //           stop char
#define NCCS           9         // Total number of these control characters

// Default control characters that are filled in the c_cc field of the 
// terminal structure
//
#define EOF_CHAR       CONTROL_D    // End of file
#define EOL_CHAR       CONTROL_J    // Carriage return, new line
#define ERASE_CHAR     CONTROL_H    // Erase Bakspace
#define KILL_CHAR      CONTROL_U    // Kill character erases the current line
#define INTR_CHAR      CONTROL_C    // Generates SIGINT effectively killing the process
#define QUIT_CHAR      CONTROL_K    // Generates SIGQUIT signal
#define SUSP_CHAR      CONTROL_Z    // Suspend process by sending SIGTSTP signal
#define START_CHAR     CONTROL_Q    // Resume output
#define STOP_CHAR      CONTROL_S    // Suspend output


// Bitfields in c_iflag and c_oflag
//
#define BRKINT         0x0001    // n/a
#define IGNBRK         0x0002    // n/a
#define IGNPAR         0x0004    // n/a
#define PARMRK         0x0008    // n/a
#define INPCK          0x0010    // n/a
#define ISTRIP         0x0020    // n/a
#define INLCR          0x0040    // LF is translated into CR
#define IGNCR          0x0080    // Ignore CR
#define ICRNL          0x0100    // CR is translated into LF
#define IXON           0x0200    // Perform flow control with STOP_CHAR and START_CHAR
#define IXOFF          0x0400    // n/a

// Bitfields in c_cflag, hardware related information
//
#define CLOCAL         0x0001    // n/a
#define CREAD          0x0002    // n/a
#define CSIZE          0x0004    // n/a
#define CS5            0x0008    // n/a
#define CS6            0x0010    // n/a
#define CS7            0x0020    // n/a
#define CS8            0x0040    // n/a
#define CSTOPB         0x0080    // n/a
#define HUPCL          0x0100    // n/a
#define PARENB         0x0200    // n/a
#define PARODD         0x0400    // n/a

// Bitfields in c_lflag, echoing and character processing
//
#define ECHO           0x0001    // Turn on echoing
#define ECHOE          0x0002    // Erase last character (if ICANON)
#define ECHOK          0x0004    // Erase last line (if ICANON)
#define ECHONL         0x0008    // '\n' is echoed even if ECHO is not set (if ICANON)
#define ICANON         0x0010    // Input processing, canonical (cooked) mode
#define ISIG           0x0020    // Enables signals
#define NOFLSH         0x0040    // Do not flush i/o queues after a signal
#define TOSTOP         0x0080    // Stop background process if it writes to tty
#define IEXTEN         0x0100    // n/a


// Queue structure defines input and output queue for the terminal;
// If head = tail, queue is empty.
//
typedef struct
{
    BYTE buf[TTY_BUFLEN];        // Buffer to store characters
    BYTE head;                   // characters are taken from this index
    BYTE tail;                   // characters enter at this index
    
} TTY_Queue; 


// The main array of terminal structures
//
typedef struct
{
    tcflag_t c_iflag;            // Controls processing of input data
    tcflag_t c_oflag;            // Controls processing of output data
    tcflag_t c_cflag;            // Hardware related information
    tcflag_t c_lflag;            // Echoing and character processing
    cc_t     c_cc[NCCS];         // Array of control characters

    WORD screen[TTY_LINES][80];  // Screen buffer
    BYTE x;                      // Current x output location
    BYTE y;                      // Current y output location
    BYTE stop;                   // Pauses writes (Control-S)
    BYTE tabs;                   // Number of spaces for Tabulator
    BYTE attrib;                 // Default screen attribute

    TTY_Queue in;                // Input queue
    TTY_Queue out;               // Output queue

    char (*in_fn)(BYTE);         // Output function
    void (*out_fn)(BYTE,char);   // Output function

} TTY_Struct;


/*********************************************************************
*   Externals
**********************************************************************/

extern BYTE ctty;                // Current terminal number
extern TTY_Struct TTY[ NR_TTYS ];// Terminal structure

extern interrupt ProcessKeyboard();
extern void display_out_char( BYTE tty_no, char c );
extern char keyboard_in_char( BYTE tty_no );
extern char dummy_in_char( BYTE tty_no );


#endif // _TTYS_H_
