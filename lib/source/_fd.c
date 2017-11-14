/******************************************************************************
*                                                                             *
*   Module:     _Fd.c                                                         *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       10/17/97                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This module contains the code for operations on file descriptors.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 10/17/97   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include <sys/clib.h>                   // Include library header file

#include <string.h>                     // Include string header file

#include <sys/syscall.h>                // Include system call header file

#include <fcntl.h>                      // Include file io header file

#include <stdarg.h>                     // Include variable argument list

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
*   int open( const char *path, int access )                                  *
*                                                                             *
*******************************************************************************
*
*   This call is a low-level open file function.
*
*   Where:
*       path to a device to open
*
*   Returns:
*       open mode
*
******************************************************************************/
int open( const char *path, int access, ... )
{
    va_list args;
    int fd;

    va_start(args,access);

    // If O_CREAT is specified, we have additional parameter which we will
    // merge together with the first one

    if( access & O_CREAT )
    {
        access |= va_arg(args, mode_t );
    }

    va_end(args);

    fd = SYS_CALL2( SYS_OPEN, path, access );

    if( fd >= 0 ) return( fd );

    errno = fd;

    return( -1 );
}


/******************************************************************************
*                                                                             *
*   int creat( const char *path, mode_t mode )                                *
*                                                                             *
*******************************************************************************
*
*   Creates a new file or rewrites an existing one.
*
*   Where:
*       path to a file to be created
#       permition bits for a new file
*
*   Returns:
*       A file descriptor or -1 on error
*
******************************************************************************/
int creat( const char *path, mode_t mode )
{
    int fd;

    fd = SYS_CALL2( SYS_CREAT, path, mode );

    if( fd >= 0 ) return( fd );

    errno = fd;

    return( -1 );
}


/******************************************************************************
*                                                                             *
*   int close( int filedes )                                                  *
*                                                                             *
*******************************************************************************
*
*   This call is a low-level close file function.
*
*   Where:
*       filedes is the file descriptor to be closed
*
*   Returns:
*       0 if ok
#       -1 on failure and errno is set
*
******************************************************************************/
int close( int filedes )
{
    int err;

    err = SYS_CALL1( SYS_CLOSE, filedes );

    if( err == 0 )
        return( 0 );

    errno = err;

    return( -1 );
}


/******************************************************************************
*                                                                             *
*   int read( int filedes, void *buf, unsigned int nbyte )                    *
*                                                                             *
*******************************************************************************
*
*   Reads from a file.
*
*   Where:
*       filedes is the file descriptor of the file for reading
#       buf is the pointer to the place the data should be read
#       nbyte is the number of bytes to read
*
*   Returns:
*       The number of bytes actually read
#       -1 on error
*
******************************************************************************/
int read( int filedes, void *buf, unsigned int nbyte )
{
    int read;

    read = SYS_CALL3( SYS_READ, filedes, buf, nbyte );

    if( read >= 0 )
        return( read );

    errno = read;

    return( -1 );
}


/******************************************************************************
*                                                                             *
*   int write( int filedes, void *buf, unsigned int nbyte )                   *
*                                                                             *
*******************************************************************************
*
*   Writes to a file.
*
*   Where:
*       filedes is the file descriptor of the file for writing
#       buf is the pointer to the source buffer
#       nbyte is the number of bytes to write
*
*   Returns:
*       The number of bytes actually written
#       -1 on error
*
******************************************************************************/
int write( int filedes, const void *buf, unsigned int nbyte )
{
    int written;

    written = SYS_CALL3( SYS_WRITE, filedes, buf, nbyte );

    if( written >= 0 )
        return( written );

    errno = written;

    return( -1 );
}



/******************************************************************************
*                                                                             *
*   off_t lseek( int fd, off_t offset, int whence )                           *
*                                                                             *
*******************************************************************************
*
*   Repositions read/write file offset
*
*   Where:
*       fd is the file descriptor of the file for writing
#       offset if the new offset
#       whence is the seek origin
*
*   Returns:
*       The new offset
#       In the case of error, (off_t-1) is returned
*
******************************************************************************/
off_t lseek( int fd, off_t offset, int whence )
{
    int pos;

    pos = SYS_CALL3( SYS_LSEEK, fd, offset, whence);

    if( pos >= 0 )
        return( pos );

    errno = pos;

    return( -1 );
}


