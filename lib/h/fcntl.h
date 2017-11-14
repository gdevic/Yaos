/******************************************************************************
*                                                                             *
*   Module:     Fcntl.h                                                       *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       11/4/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This is a header file for the IO control.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 11/4/97    Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _FCNTL_H_
#define _FCNTL_H_


/******************************************************************************
*                                                                             *
*   Include Files                                                             *
*                                                                             *
******************************************************************************/

#include <sys/types.h>                  // Include data types

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

//----------------------------------------------------------------------------
// Mode values for open()
//----------------------------------------------------------------------------

#define O_RDONLY            0x0000      // Opens a file read-only
#define O_WRONLY            0x0001      // Opens a file write-only
#define O_RDWR              0x0002      // Opens a file read-write
#define O_CREAT             0x0004      // Create a file if it doesnt exist
#define O_TRUNC             0x0008      // Truncates a file
#define O_APPEND            0x0010      // Appends to a file


/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern int open( const char *path, int access, ... );
extern int creat( const char *path, mode_t mode );


#endif //  _FCNTL_H_