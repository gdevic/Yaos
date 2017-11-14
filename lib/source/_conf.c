/******************************************************************************
*                                                                             *
*   Module:     _Conf.c                                                       *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       11/1/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This module contains the code for the configuration functions.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 11/1/97    Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include <unistd.h>                     // Include its own header

#include <sys/syscall.h>                // Include system call header file

#include <errno.h>                      // Include error number declaration

#include <limits.h>                     // Include constant limits

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
*   long sysconf( int name )                                                  *
*                                                                             *
*******************************************************************************
*
*   This function returns a value for a system limit or option.
*
*   Where:
*       name is one of the predefined limits
*
*   Returns:
*       -1 on error
#       value
*
******************************************************************************/
long sysconf( int name )
{
    switch( name )
    {
        case _SC_ARG_MAX:    return( ARG_MAX );
        case _SC_CHILD_MAX:  return( _POSIX_CHILD_MAX );
        case _SC_CLK_TCK:    return( CLK_TCK );
        case _SC_NGROUPS_MAX:return( _POSIX_NGROUPS_MAX );
        case _SC_STREAM_MAX: return( STREAM_MAX );
        case _SC_TZNAME_MAX: return( TZNAME_MAX );
        case _SC_OPEN_MAX:   return( _POSIX_OPEN_MAX );
        case _SC_JOB_CONTROL:return( _POSIX_JOB_CONTROL );
        case _SC_SAVED_IDS:  return( _POSIX_SAVED_IDS );
        case _SC_VERSION:    return( _POSIX_VERSION );
    }

    return( -1 );
}


/******************************************************************************
*                                                                             *
*   long fpathconf( int filedes, int name )                                   *
*                                                                             *
*******************************************************************************
*
*   This function returns a configuration limit for an open file.
*
*   Where:
#       filedes is an open file descriptor
*       name is one of the predefined limits
*
*   Returns:
*       -1 if filedes is invalid and errno is set
#       limit
*
******************************************************************************/
long fpathconf( int filedes, int name )
{
    int limit;

limit = -1;
//    limit = SYS_CALL2( SYS_CONF, filedes, name );

    if( limit >= 0 )
        return( limit );

    errno = limit;

    return( -1 );
}


/******************************************************************************
*                                                                             *
*   long pathconf( const char *path, int name )                               *
*                                                                             *
*******************************************************************************
*
*   This function returns a configuration limit.
*
*   Where:
*       name is one of the predefined limits
*
*   Returns:
*       -1 if name or path is invalid and errno is set
#       limit
*
******************************************************************************/
long pathconf( const char *path, int name )
{
#if 0
//    FILE *fp;

    // Open a temp directory on the path
#endif
    return( -1 );
}

