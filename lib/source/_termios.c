/******************************************************************************
*                                                                             *
*   Module:     _termios.c                                                    *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       11/5/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This module contains the code for the terminal support.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 11/5/97    Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include <sys/clib.h>                   // Include library header file

#include <sys/syscall.h>                // Include system call header file

#include <termios.h>                    // Include terminal structures

#include <sys/ioctl.h>                  // Include io control defines

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
*   int isatty( int fd )                                                      *
*                                                                             *
*******************************************************************************
*
*   Checks if a file descriptor is a terminal.
*
*   Where:
*       fd is a fle descriptor to test.
*
*   Returns:
*       1 if a fd refers to a terminal
#       0 if it doesnt
*
******************************************************************************/
int isatty( int fd )
{
    struct termios tmp;

    return( tcgetattr(fd, &tmp) == 0 );
}


/******************************************************************************
*                                                                             *
*   int tcsetattr( int fd, int option, struct termios *tp )                   *
*                                                                             *
*******************************************************************************
*
*   Sets terminal attributes.
*
*   Where:
*       fs is a file descriptor that must refer to a terminal.
#       option is one of TCSANOW, TCSADRAIN or TCSAFLUSH
#       tp is a pointer to structure from which information is to be set.
*
*   Returns:
*       0 on success
#       -1 on failure, errno contains error code
*
******************************************************************************/
int tcsetattr( int fd, int option, struct termios *tp )
{
    int err, fn = 0;

    // Option can be only one of 3 constants

    switch( option )
    {
        case TCSANOW:   fn = TCSETS;   break;
        case TCSADRAIN: fn = TCSETSD;  break;
        case TCSAFLUSH: fn = TCSETSF;  break;

        default:
            errno = EINVAL;
            return( -1 );
    }

    err = SYS_CALL3( SYS_IOCTL, fn, fd, tp );

    if( err < 0 )
    {
        errno = err;

        return( -1 );
    }

    return( 0 );
}


/******************************************************************************
*                                                                             *
*   int tcgetattr( int fd, struct termios *tp )                               *
*                                                                             *
*******************************************************************************
*
*   Gets terminal attributes.
*
*   Where:
*       fs is a file descriptor that must refer to a terminal.
#       tp is a pointer to structure where information is to be returned.
*
*   Returns:
*       0 on success
#       -1 on failure, errno contains error code
*
******************************************************************************/
int tcgetattr( int fd, struct termios *tp )
{
    int err;

    err = SYS_CALL3( SYS_IOCTL, TCGETS, fd, tp );

    if( err < 0 )
    {
        errno = err;

        return( -1 );
    }

    return( 0 );
}


