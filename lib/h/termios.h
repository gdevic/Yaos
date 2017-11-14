/******************************************************************************
*                                                                             *
*   Module:     termios.h                                                     *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       11/5/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This is a header file for terminal structure.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 11/5/97    Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _TERMIOS_H_
#define _TERMIOS_H_


/******************************************************************************
*                                                                             *
*   Include Files                                                             *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

typedef unsigned int tcflag_t;          // POSIX type tcflag_t
typedef unsigned char cc_t;             // POSIX type cc_t
typedef unsigned int speed_t;           // POSIX type speed_t


// Bitfields in c_iflag and c_oflag, control the processing of input and
// output data

#define BRKINT         0x0001           // n/a
#define IGNBRK         0x0002           // n/a
#define IGNPAR         0x0004           // n/a
#define PARMRK         0x0008           // n/a
#define INPCK          0x0010           // n/a
#define ISTRIP         0x0020           // n/a   - mask input params to 7 bits ???
#define INLCR          0x0040           // LF is translated into CR
#define IGNCR          0x0080           // Ignore CR
#define ICRNL          0x0100           // CR is translated into LF
#define IXON           0x0200           // n/a Perform flow control with STOP_CHAR and START_CHAR
#define IXOFF          0x0400           // n/a


// Bitfields in c_cflag, hardware related information

#define CLOCAL         0x0001           // n/a
#define CREAD          0x0002           // n/a
#define CSIZE          0x0004           // n/a
#define CS5            0x0008           // n/a
#define CS6            0x0010           // n/a
#define CS7            0x0020           // n/a
#define CS8            0x0040           // n/a
#define CSTOPB         0x0080           // n/a
#define HUPCL          0x0100           // n/a
#define PARENB         0x0200           // n/a
#define PARODD         0x0400           // n/a


// Bitfields in c_lflag, echoing and character processing

#define ECHO           0x0001           // Turn on echoing
#define ECHOE          0x0002           // Erase last character (if ICANON)
#define ECHOK          0x0004           // Erase last line (if ICANON)
#define ECHONL         0x0008           // '\n' is echoed even if ECHO is not set (if ICANON)
#define ICANON         0x0010           // Input processing, canonical (cooked) mode
#define ISIG           0x0020           // Enables signals
#define NOFLSH         0x0040           // Do not flush i/o queues after a signal
#define TOSTOP         0x0080           // Stop background process if it writes to controlling tty
#define IEXTEN         0x0100           // n/a


// These define offsets of the control characters in the c_cc field of the
// terminal structure.  They will be filled with the default characters

#define VEOF           0                // offset of EOF character
#define VEOL           1                //           newline
#define VERASE         2                //           backspace
#define VINTR          3                //           generates SIGINT
#define VKILL          4                //           kill char
#define VQUIT          5                //           generates SIGQUIT
#define VSUSP          6                //           generates SIGTSTP
#define VSTART         7                //           start char
#define VSTOP          8                //           stop char
#define NCCS           9                // Total number of these control characters


// Values for the baud rate

#define B0             0x0000000        // Hang up the line
#define B50            0x0000001        // 50 baud
#define B75            0x0000002        // 75 baud
#define B110           0x0000003        // 110 baud
#define B134           0x0000004        // 134 baud
#define B150           0x0000005        // 150 baud
#define B200           0x0000006        // 200 baud
#define B300           0x0000007        // 300 baud
#define B600           0x0000010        // 600 baud
#define B1200          0x0000011        // 1200 baud
#define B1800          0x0000012        // 1800 baud
#define B2400          0x0000013        // 2400 baud
#define B4800          0x0000014        // 4800 baud
#define B9600          0x0000015        // 9600 baud
#define B19200         0x0000016        // 19200 baud
#define B38400         0x0000017        // 38400 baud


struct termios
{
    tcflag_t c_iflag;                   // Controls processing of input data
    tcflag_t c_oflag;                   // Controls processing of output data
    tcflag_t c_cflag;                   // Hardware related information
    tcflag_t c_lflag;                   // Echoing and character processing
    cc_t     c_cc[NCCS];                // Array of control characters

    speed_t  c_ispeed;                  // Input speed
    speed_t  c_ospeed;                  // Output speed
};


#define TCSANOW         0               // The changes occur immediately
#define TCSADRAIN       1               // The changes will occur after transmittion
#define TCSAFLUSH       2               // All input read is discarded


struct winsize
{
    unsigned short ws_row;
    unsigned short ws_col;
    unsigned short ws_xpixel;
    unsigned short ws_ypixel;
};


/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern int tcsetattr( int fd, int option, struct termios *tp );
extern int tcgetattr( int fd, struct termios *tp );
extern int isatty( int fd );


#endif //  _TERMIOS_H_
