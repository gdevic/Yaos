/******************************************************************************
*                                                                             *
*   Module:     _Stats.c                                                      *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       11/7/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This module contains the code for the stat() and fstat() functions.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 11/7/97    Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include <sys/clib.h>                   // Include private library header file

#include <sys/stat.h>                   // Include its own header

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
*   int stat( const char *path, struct stat *buf )                            *
*                                                                             *
*******************************************************************************
*
*   Gets information about a file.
*
*   Where:
*       path is a name of file to research
#       buf is a pointer to a stat buffer that will get the information
*
*   Returns:
*       0 on success
#       -1 on error, errno set
*
******************************************************************************/
int stat( const char *path, struct stat *buf )
{
    return( -1 );
}


