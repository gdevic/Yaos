/******************************************************************************
*                                                                             *
*   Module:     _unistd.c                                                     *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       11/15/97                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This module contains the code for the standard UNIX functions defined
        in `unistd.h' file.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 11/15/97   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include <sys/clib.h>                   // Include private library header file

#include <unistd.h>                     // Include its own header

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
*   int access( const char *path, int amode )                                 *
*                                                                             *
*******************************************************************************
*
*   Tests for file accessability.
*
*   Where:
#       path is a file path/name to be checked.
*       amode is bitwise OR of the access permissions to be checked:
#           R_OK for read, W_OK for write, X_OK for execute, F_OK for existence
*
*   Returns:
*       0 if access is allowed
#       -1 on error, if access is not allower, errno will be set to EACCESS.
*
******************************************************************************/
int access( const char *path, int amode )
{
    return( 0 );
}


