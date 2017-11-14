/******************************************************************************
*                                                                             *
*   Module:     TTY.h                                                         *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       10/12/97                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This is a header file for the terminal teletype interface
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 10/12/97   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _TTY_H_
#define _TTY_H_


/******************************************************************************
*                                                                             *
*   Include Files                                                             *
*                                                                             *
******************************************************************************/

#include <termios.h>                    // Include terminal header file

#include "types.h"                      // Include basis data types

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

#define TTY_BUFLEN              32      // Terminal input and output buffer len
#define TTY_LINES               50      // Number of lines in a terminal buffer


// Control-D  end-of-file character, break through cooked mode
// Control-J  CR character
// Control-H  erase character
// Control-U  erase (kill) whole line
// Control-C  stop signal, SIGQUIT
// Control-Z  suspend process signal, SIGINT

// Control-G  bell
// Control-S  suspense output
// Control-Q  resume output

// CR is '\r', code 13 (0x0d)
// LF is '\n', code 10 (0x0a)

#define CONTROL_Q     'Q' + 128
#define CONTROL_W     'W' + 128
#define CONTROL_E     'E' + 128
#define CONTROL_R     'R' + 128
#define CONTROL_T     'T' + 128
#define CONTROL_Y     'Y' + 128
#define CONTROL_U     0x15
#define CONTROL_I     'I' + 128
#define CONTROL_O     'O' + 128
#define CONTROL_P     'P' + 128
#define CONTROL_A     'A' + 128
#define CONTROL_S     'S' + 128
#define CONTROL_D     0x1A
#define CONTROL_F     'F' + 128
#define CONTROL_G     'G' + 128
#define CONTROL_H     '\b'
#define CONTROL_J     '\n'
#define CONTROL_K     'K' + 128
#define CONTROL_L     'L' + 128
#define CONTROL_Z     'Z' + 128
#define CONTROL_X     'X' + 128
#define CONTROL_C     'C' + 128
#define CONTROL_V     'V' + 128
#define CONTROL_B     'B' + 128
#define CONTROL_N     'N' + 128
#define CONTROL_M     'M' + 128


// Default control characters that are filled in the c_cc field of a
// terminal structure

#define EOF_CHAR       CONTROL_D        // End of file
#define EOL_CHAR       CONTROL_J        // Carriage return, new line
#define ERASE_CHAR     CONTROL_H        // Erase Bakspace
#define KILL_CHAR      CONTROL_U        // Kill character erases the current line
#define INTR_CHAR      CONTROL_C        // Generates SIGINT effectively killing the process
#define QUIT_CHAR      CONTROL_K        // Generates SIGQUIT signal
#define SUSP_CHAR      CONTROL_Z        // Suspend process by sending SIGTSTP signal
#define START_CHAR     CONTROL_Q        // Resume output
#define STOP_CHAR      CONTROL_S        // Suspend output



// Queue structure defines input and output queue for a terminal:
// If head = tail, queue is empty.

typedef struct
{
    BYTE buf[TTY_BUFLEN];               // Buffer to store characters
    BYTE head;                          // characters are taken from this index
    BYTE tail;                          // characters enter at this index

} TTY_Queue;


//-----------------------------------------------------------------------------
//  Terminal structure
//-----------------------------------------------------------------------------

typedef struct TTY_StructType TTY_pStruct;

typedef struct TTY_StructType
{
    tcflag_t c_iflag;                   // Controls processing of input data
    tcflag_t c_oflag;                   // Controls processing of output data
    tcflag_t c_cflag;                   // Hardware related information
    tcflag_t c_lflag;                   // Echoing and character processing
    cc_t     c_cc[NCCS];                // Array of control characters

    WORD screen[TTY_LINES][80];         // Screen buffer
    BYTE x;                             // Current x output location
    BYTE y;                             // Current y output location
    BYTE stop;                          // Pauses writes (Control-S)
    BYTE tabs;                          // Number of spaces for Tabulator
    BYTE attrib;                        // Default screen attribute

    TTY_Queue in;                       // Input queue
    TTY_Queue out;                      // Output queue

    BYTE bMinor;                        // Minor number or -1 if free

    char (*fnIn)(TTY_pStruct *, char);  // Input function
    void (*fnOut)(TTY_pStruct *, char); // Output function

} TTY_Struct;

extern TTY_Struct *TTY[];

extern TTY_Struct *pCurTTY;             // Current (controlling) terminal


/******************************************************************************
*                                                                             *
*   Device control block defines                                              *
*                                                                             *
******************************************************************************/

#define MAJOR_TTY           1           // Major number of a terminal device
#define MINOR_TTY           16          // Minor number of terminal devices


/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern TTY_Struct * CreateTTY();
extern void DestroyTTY( TTY_Struct * );



#endif //  _TTY_H_
