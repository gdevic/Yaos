/******************************************************************************
*                                                                             *
*   Module:     _Flsbuf.c                                                     *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       11/5/97                                                       *
*                                                                             *
*   Author:     This function has been taken from Minix C library             *
*                                                                             *
*******************************************************************************

    Module Description:

        This module contains the code for buffer flush on put, putc()

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
*   int do_write( int fd, char *buf, int count )                              *
*                                                                             *
*******************************************************************************
*
*   Since `write' may not write all at once in POSIX, we will loop here and
#   write whole buffer.
*
*   Where:
*       fd is the file descriptor
#       buf is the buffer to write
#       count is the buffer size in bytes
*
*   Returns:
*       0 if successful
#       EOF if failed
*
******************************************************************************/
static int do_write(int d, char *buf, int nbytes)
{
    int c;

    /* POSIX actually allows write() to return a positive value less
       than nbytes, so loop ...
    */
    while ((c = write(d, buf, nbytes)) > 0 && c < nbytes) {
        nbytes -= c;
        buf += c;
    }
    return c > 0;
}


/******************************************************************************
*                                                                             *
*   int flsbuf( int c, FILE *fp )                                             *
*                                                                             *
*******************************************************************************
*
*   This function flushes a buffer on an attempt to write to a buffer that
#   is full or at non-buffered stream.
*
*   Where:
#       c is the character that did not fit in the full buffer
*       fp is the address of a file stream
*
*   Returns:
*       c if successful
#       EOF on error
*
******************************************************************************/
int _flsbuf(int c, FILE * stream)
{
    if (fileno(stream) < 0) return EOF;
    if (!io_testflag(stream, _IOWRITE)) return EOF;
    if (io_testflag(stream, _IOREADING) && !feof(stream)) return EOF;

    stream->_flags &= ~_IOREADING;
    stream->_flags |= _IOWRITING;
    if (!io_testflag(stream, _IONBF))
    {
        if (!stream->_buf)
        {
            if (stream == stdout && isatty(fileno(stdout)))
            {
                if (!(stream->_buf =
                        (unsigned char *) malloc(BUFSIZ)))
                {
                    stream->_flags |= _IONBF;
                }
                else
                {
                    stream->_flags |= _IOLBF|_IOMYBUF;
                    stream->_bufsiz = BUFSIZ;
                    stream->_count = -1;
                }
            }
            else
            {
                if (!(stream->_buf =
                        (unsigned char *) malloc(BUFSIZ)))
                {
                    stream->_flags |= _IONBF;
                }
                else
                {
                    stream->_flags |= _IOMYBUF;
                    stream->_bufsiz = BUFSIZ;
                    if (!io_testflag(stream, _IOLBF))
                        stream->_count = BUFSIZ - 1;
                    else
                        stream->_count = -1;
                }
            }
            stream->_ptr = stream->_buf;
        }
    }

    if (io_testflag(stream, _IONBF))
    {
        char c1 = c;

        stream->_count = 0;
        if (io_testflag(stream, _IOAPPEND))
        {
            if (lseek(fileno(stream), 0L, SEEK_END) == -1)
            {
                stream->_flags |= _IOERR;
                return EOF;
            }
        }

        if (write(fileno(stream), &c1, 1) != 1)
        {
            stream->_flags |= _IOERR;
            return EOF;
        }
        return c;
    }
    else
    if (io_testflag(stream, _IOLBF))
    {
        *stream->_ptr++ = c;
        if (c == '\n' || stream->_count == -stream->_bufsiz)
        {
            if (io_testflag(stream, _IOAPPEND))
            {
                if (lseek(fileno(stream), 0L, SEEK_END) == -1)
                {
                    stream->_flags |= _IOERR;
                    return EOF;
                }
            }
            if (! do_write(fileno(stream), (char *)stream->_buf,
                    -stream->_count)) {
                stream->_flags |= _IOERR;
                return EOF;
            }
            else
            {
                stream->_ptr  = stream->_buf;
                stream->_count = 0;
            }
        }
    }
    else
    {
        int count = stream->_ptr - stream->_buf;

        stream->_count = stream->_bufsiz - 1;
        stream->_ptr = stream->_buf + 1;

        if (count > 0)
        {
            if (io_testflag(stream, _IOAPPEND))
            {
                if (lseek(fileno(stream), 0L, SEEK_END) == -1)
                {
                    stream->_flags |= _IOERR;
                    return EOF;
                }
            }
            if (! do_write(fileno(stream), (char *)stream->_buf, count))
            {
                *(stream->_buf) = c;
                stream->_flags |= _IOERR;
                return EOF;
            }
        }
        *(stream->_buf) = c;
    }
    return c;
}
