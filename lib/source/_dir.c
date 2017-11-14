/******************************************************************************
*                                                                             *
*   Module:     _dir.c                                                        *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       11/13/97                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This module contains the code for directory functions.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 11/13/97   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include <sys/clib.h>                   // Include library header file

#include <sys/syscall.h>                // Include system call header file

#include <dirent.h>                     // Include its own header file

#include <fcntl.h>                      // Include function header

#include <string.h>                     // Include string file header

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
*   DIR *opendir( const char *dirname )                                       *
*                                                                             *
*******************************************************************************
*
*   Opens a directory stream and returns a pointer to a directory stream
#   structure.
*
*   Where:
*       dirname is the directory name to open
*
*   Returns:
*       pointer to a dir stream structure
*
******************************************************************************/
DIR *opendir( const char *dirname )
{
    DIR *pDir;
    int handle;

    if( dirname == NULL )  return( NULL );

    // System call to set up the kernel directory structure and return the
    // kernel process directory handle - we use normal open system call

    handle = SYS_CALL2( SYS_OPEN, dirname, O_RDONLY );

    if( handle >= 0 )
    {
        // Allocate memory for the directory structure

        if( (pDir = (DIR *) malloc(sizeof(DIR)))==NULL )  return( NULL );

        // Assign a directory handle

        pDir->dir.handle = handle;

        // Set the base directory name

        strcpy( pDir->dir.base_dir, dirname );

        return( pDir );
    }

    // Could not ask for the directory

    errno = handle;

    return( NULL );
}


/******************************************************************************
*                                                                             *
*   struct dirent *readdir( DIR *pdir )                                       *
*                                                                             *
*******************************************************************************
*
*   Returns next directory entry from the stream openend by opendir.
*
*   Where:
*       pdir is the DIR pointer returned by the opendir
*
*   Returns:
*       Pointer to a dirent structure containing d_name
#       NULL if there are no more directory entries or on error
*
******************************************************************************/
struct dirent *readdir( DIR *pdir )
{
    int res;


    if( pdir==NULL )  return( NULL );

    res = SYS_CALL3( SYS_READ, pdir->dir.handle, pdir->dir.d_name, NAME_MAX );

    if( res == NAME_MAX )
    {
        // Return the pointer to dirent structure

        return( &pdir->dir );
    }
    else
        if( res > 0 )  res = EACCESS;

    // Error happened

    errno = res;

    return( NULL );
}


/******************************************************************************
*                                                                             *
*   int closedir( DIR *pdir )                                                 *
*                                                                             *
*******************************************************************************
*
*   Closes a directory stream opened by opendir call.
*
*   Where:
*       pdir is the pointer to directory stream
*
*   Returns:
*       0 on success
#       -1 on failure, errno is set
*
******************************************************************************/
int closedir( DIR *pdir )
{
    int res;


    if( pdir==NULL )
    {
        errno = EBADF;
        return( -1 );
    }

    res = SYS_CALL1( SYS_CLOSE, pdir->dir.handle );

    if( res >= 0 )
    {
        free( pdir );

        return( 0 );
    }

    // Error happened on kernel close

    errno = res;
    return( -1 );
}


/******************************************************************************
*                                                                             *
*   void rewinddir( struct dirent *pdirent )                                  *
*                                                                             *
*******************************************************************************
*
*   Resets the reading directory pointer.
*
*   Where:
*       pdirent is the pointer to directory entry structure
*
*   Returns:
*       void
*
******************************************************************************/
void rewinddir( struct dirent *pdirent )
{
    // We just close and reopen the directory stream

    if( pdirent != NULL )
    {
        SYS_CALL1( SYS_CLOSE, pdirent->handle );

        pdirent->handle = SYS_CALL2( SYS_OPEN, pdirent->base_dir, O_RDONLY );
    }
}

