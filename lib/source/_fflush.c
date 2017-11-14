/******************************************************************************
*                                                                             *
*   Module:     _fflush.c                                                     *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       11/5/97                                                       *
*                                                                             *
*   Author:     This function has been taken from Minix C library             *
*                                                                             *
*******************************************************************************

    Module Description:

        This module contains the code for the fflush() call.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include <sys/clib.h>                   // Include private library header file

#include <fcntl.h>                      // Include function io header file

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
*   int fflush( FILE *stream )                                                *
*                                                                             *
*******************************************************************************
*
*   Flushes the file stream.
*
*   Where:
*       fp is a pointer to a file stream to flush; if NULL, all open files
#           are flushed
*
*   Returns:
#       0 on success
#       EOF on error and error code is stored in errno
*
******************************************************************************/
int fflush( FILE *stream )
{
    int count, c1, i, retval = 0;

    if (!stream) {
        for(i= 0; i < FOPEN_MAX; i++)
        if (_io[i] && fflush(_io[i]))
            retval = EOF;
        return retval;
    }

    if (!stream->_buf
        || (!io_testflag(stream, _IOREADING)
        && !io_testflag(stream, _IOWRITING)))
        return 0;
    if (io_testflag(stream, _IOREADING)) {
        /* (void) fseek(stream, 0L, SEEK_CUR); */
        int adjust = 0;
        if (stream->_buf && !io_testflag(stream,_IONBF))
            adjust = stream->_count;
        stream->_count = 0;
        lseek(fileno(stream), (off_t) adjust, SEEK_CUR);
        if (io_testflag(stream, _IOWRITE))
            stream->_flags &= ~(_IOREADING | _IOWRITING);
        stream->_ptr = stream->_buf;
        return 0;
    } else if (io_testflag(stream, _IONBF)) return 0;

    if (io_testflag(stream, _IOREAD))       /* "a" or "+" mode */
        stream->_flags &= ~_IOWRITING;

    count = stream->_ptr - stream->_buf;
    stream->_ptr = stream->_buf;

    if ( count <= 0 )
        return 0;

    if (io_testflag(stream, _IOAPPEND)) {
        if (lseek(fileno(stream), 0L, SEEK_END) == -1) {
            stream->_flags |= _IOERR;
            return EOF;
        }
    }
    c1 = write(stream->_fd, (char *)stream->_buf, count);

    stream->_count = 0;

    if ( count == c1 )
        return 0;

    stream->_flags |= _IOERR;
    return EOF;
}

