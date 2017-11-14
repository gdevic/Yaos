/******************************************************************************
*                                                                             *
*   Module:     _Stdio.c                                                      *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       10/14/97                                                      *
*                                                                             *
*   Author:     Minix                                                         *
*               Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This module contains the code for the standard input/output
        module.

        It is assumed that the process is started with 3 open file descriptors
        for which we build 3 file streams (stdin, stdout and stderr).

        Kernel `init' process actually opens those file descriptors and from
        then on they are inherited.

NOTE: Most of these functions are taken from the Minix stdio portion of the
      C Library

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 10/14/97   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include <sys/clib.h>                   // Include private library header file

#include <stdio.h>                      // Include standard I/O header file

#include <fcntl.h>                      // Include function io header file

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

FILE _stdin  = { NULL, NULL, 0, _IOREAD,           0, 0 };
FILE _stdout = { NULL, NULL, 0, _IOWRITE,          1, 0 };
FILE _stderr = { NULL, NULL, 0, _IOWRITE | _IOLBF, 2, 0 };


//-----------------------------------------------------------------------------
// Define the FILE structure - this is statically initialized, so if macro
// FOPEN_MAX changes, this needs to change
//-----------------------------------------------------------------------------

FILE *_io [FOPEN_MAX] =
{
    &_stdin,
    &_stdout,
    &_stderr,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};


/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

#define PMODE   0666

#if (SEEK_CUR != 1) || (SEEK_END != 2) || (SEEK_SET != 0)
#error SEEK_* values are wrong
#endif


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   fopen() function has been taken from the Minix C library                  *
*                                                                             *
******************************************************************************/
/******************************************************************************
*                                                                             *
*   FILE *fopen( const char *filename, const char *mode )                     *
*                                                                             *
*******************************************************************************
*
*   This function opens a buffered file stream.
*
*   Where:
*       filename is a pointer to a path/name of file to open
#       mode is a pointer to a character string:
#           "r" for read
#           "w" for write
#           "a" for append
#           "r+" for update (r/w, all existing data is preserved)
#           "w+" truncate to zero length and open for update
#           "a+" for append update (reads anywhere, but updates at the end)
*
*   Returns:
*       Pointer to a FILE structure
#       NULL if operation failed
*
******************************************************************************/
FILE *fopen(const char *name, const char *mode)
{
    register int i;
    int rwmode = 0, rwflags = 0;
    FILE *stream;
    int fd, flags = 0;

    for (i = 0; _io[i] != 0 ; i++)
        if ( i >= FOPEN_MAX-1 )
            return (FILE *)NULL;

    switch(*mode++) {
    case 'r':
        flags |= _IOREAD | _IOREADING;
        rwmode = O_RDONLY;
        break;
    case 'w':
        flags |= _IOWRITE | _IOWRITING;
        rwmode = O_WRONLY;
        rwflags = O_CREAT | O_TRUNC;
        break;
    case 'a':
        flags |= _IOWRITE | _IOWRITING | _IOAPPEND;
        rwmode = O_WRONLY;
        rwflags |= O_APPEND | O_CREAT;
        break;
    default:
        return (FILE *)NULL;
    }

    while (*mode) {
        switch(*mode++) {
        case 'b':
            continue;
        case '+':
            rwmode = O_RDWR;
            flags |= _IOREAD | _IOWRITE;
            continue;
        /* The sequence may be followed by additional characters */
        default:
            break;
        }
        break;
    }

    /* Perform a creat() when the file should be truncated or when
     * the file is opened for writing and the open() failed.
     */
    if ((rwflags & O_TRUNC)
        || (((fd = open(name, rwmode)) < 0)
            && (rwflags & O_CREAT))) {
        if (((fd = creat(name, PMODE)) > 0) && flags  | _IOREAD) {
            (void) close(fd);
            fd = open(name, rwmode);
        }

    }

    if (fd < 0) return (FILE *)NULL;

    if (( stream = (FILE *) malloc(sizeof(FILE))) == NULL ) {
        close(fd);
        return (FILE *)NULL;
    }

    if ((flags & (_IOREAD | _IOWRITE))  == (_IOREAD | _IOWRITE))
        flags &= ~(_IOREADING | _IOWRITING);

    stream->_count = 0;
    stream->_fd = fd;
    stream->_flags = flags;
    stream->_buf = NULL;
    _io[i] = stream;
    return stream;
}


/******************************************************************************
*                                                                             *
*   fclose() function has been taken from the Minix C library                 *
*                                                                             *
******************************************************************************/
/******************************************************************************
*                                                                             *
*   int fclose( FILE *fp )                                                    *
*                                                                             *
*******************************************************************************
*
*   This function flushes a stream and closes the file.
*
*   Where:
*       fp is the address of a FILE structure
*
*   Returns:
#       0 on success
#       EOF on error and error code is stored in errno
*
******************************************************************************/
int fclose(FILE *fp)
{
    int i, retval = 0;

    for (i=0; i<FOPEN_MAX; i++)
        if (fp == _io[i]) {
            _io[i] = 0;
            break;
        }
    if (i >= FOPEN_MAX)
        return EOF;
    if (fflush(fp)) retval = EOF;
    if (close(fileno(fp))) retval = EOF;
    if ( io_testflag(fp,_IOMYBUF) && fp->_buf )
        free((void *)fp->_buf);
    if (fp != stdin && fp != stdout && fp != stderr)
        free((void *)fp);
    return retval;
}


/******************************************************************************
*                                                                             *
*   setvbuf() function has been taken from the Minix C library                *
*                                                                             *
******************************************************************************/
/******************************************************************************
*                                                                             *
*   int setvbuf( FILE *fp, char *buf, int mode, size_t size )                 *
*                                                                             *
*******************************************************************************
*
*   Sets a stream buffering for a given stream.
*
*   Where:
*       fp is a stream to set the buffering
#       buf if NULL, uses default buffer
#           otherwise sets the user buffer
#       mode is buffering mode (_IONBF,_IOLBF,_IOFBF)
#       size is the size of a buffer
*
*   Returns:
*       0 on success
#       non zero on failure (bad arguments)
*
******************************************************************************/
int setvbuf(FILE *stream, char *buf, int mode, size_t size)
{
    int retval = 0;

//    _clean = __cleanup;
    if (mode != _IOFBF && mode != _IOLBF && mode != _IONBF)
        return EOF;

    if (stream->_buf && io_testflag(stream,_IOMYBUF) )
        free((void *)stream->_buf);

    stream->_flags &= ~(_IOMYBUF | _IONBF | _IOLBF);

    if (buf && size <= 0) retval = EOF;
    if (!buf && (mode != _IONBF)) {
        if (size <= 0 || (buf = (char *) malloc(size)) == NULL) {
            retval = EOF;
        } else {
            stream->_flags |= _IOMYBUF;
        }
    }

    stream->_buf = (unsigned char *) buf;

    stream->_count = 0;
    stream->_flags |= mode;
    stream->_ptr = stream->_buf;

    if (!buf) {
        stream->_bufsiz = 1;
    } else {
        stream->_bufsiz = size;
    }

    return retval;
}


/******************************************************************************
*                                                                             *
*   void setbuf( FILE *fp, char *buffer )                                     *
*                                                                             *
*******************************************************************************
*
*   Sets the user buffer address for a file stream fp.  If the buffer is NULL
#   stream will be unbuffered.
*
*   Where:
*       fp stream to set the buffer
#       buffer if NULL, non-buffered stream
#              address of a buffer at least BUFSIZ in length, fully buffered
*
*   Returns:
*       void
*
******************************************************************************/
void setbuf( FILE *fp, char *buffer )
{
    setvbuf( fp, buffer, (buffer==NULL)? _IONBF : _IOFBF, BUFSIZ );
}


/******************************************************************************
*                                                                             *
*   ungetc() function has been taken from the Minix C library                 *
*                                                                             *
******************************************************************************/
/******************************************************************************
*                                                                             *
*   int ungetc( int c, FILE *fp )                                             *
*                                                                             *
*******************************************************************************
*
*   Pushes back a character onto a stream.
*
*   Where:
*       c is the character to push back
*       fp is a pointer to file being read
*
*   Returns:
*       c on success
#       EOF on failure
*
******************************************************************************/
int ungetc(int ch, FILE *stream)
{
    unsigned char *p;

    if (ch == EOF  || !io_testflag(stream,_IOREADING))
        return EOF;
    if (stream->_ptr == stream->_buf) {
        if (stream->_count != 0) return EOF;
        stream->_ptr++;
    }
    stream->_count++;
    p = --(stream->_ptr);       /* ??? Bloody vax assembler !!! */
    /* ungetc() in sscanf() shouldn't write in rom */
    if (*p != (unsigned char) ch)
        *p = (unsigned char) ch;
    return ch;
}


/******************************************************************************
*                                                                             *
*   int fgetc( FILE *fp )                                                     *
*                                                                             *
******************************************************************************/
int fgetc( FILE *fp )
{
    return( getc(fp) );
}


/******************************************************************************
*                                                                             *
*   fgets() function has been taken from the Minix C library                  *
*                                                                             *
******************************************************************************/
char *fgets(char *s, int n, FILE *stream)
{
    int ch;
    char *ptr;

    ptr = s;
    while (--n > 0 && (ch = getc(stream)) != EOF) {
        *ptr++ = ch;
        if ( ch == '\n')
            break;
    }
    if (ch == EOF) {
        if (feof(stream)) {
            if (ptr == s) return NULL;
        } else return NULL;
    }
    *ptr = '\0';
    return s;
}


/******************************************************************************
*                                                                             *
*   gets() function has been taken from the Minix C library                   *
*                                                                             *
******************************************************************************/
char *gets(char *s)
{
    register FILE *stream = stdin;
    register int ch;
    register char *ptr;

    ptr = s;
    while ((ch = getc(stream)) != EOF && ch != '\n')
        *ptr++ = ch;

    if (ch == EOF) {
        if (feof(stream)) {
            if (ptr == s) return NULL;
        } else return NULL;
    }

    *ptr = '\0';
    return s;
}


/******************************************************************************
*                                                                             *
*   int fputc( int c, FILE *fp )                                              *
*                                                                             *
******************************************************************************/
int fputc( int c, FILE *fp )
{
    return( putc(c, fp) );
}


/******************************************************************************
*                                                                             *
*   fputs() function has been taken from the Minix C library                  *
*                                                                             *
******************************************************************************/
int fputs(const char *s, FILE *stream)
{
    int i = 0;

    while (*s)
        if (putc(*s++, stream) == EOF) return EOF;
        else i++;

    return i;
}


/******************************************************************************
*                                                                             *
*   puts() function has been taken from the Minix C library                   *
*                                                                             *
******************************************************************************/
int puts( const char *s)
{
    register FILE *file = stdout;
    register int i = 0;

    while (*s) {
        if (putc(*s++, file) == EOF) return EOF;
        else i++;
    }
    if (putc('\n', file) == EOF) return EOF;
    return i + 1;
}


/******************************************************************************
*                                                                             *
*   fread() function has been taken from the Minix C library                  *
*                                                                             *
******************************************************************************/
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    char *cp = ptr;
    int c;
    size_t ndone = 0;
    size_t s;

    if (size)
        while ( ndone < nmemb ) {
            s = size;
            do {
                if ((c = getc(stream)) != EOF)
                    *cp++ = c;
                else
                    return ndone;
            } while (--s);
            ndone++;
        }

    return ndone;
}


/******************************************************************************
*                                                                             *
*   fwrite() function has been taken from the Minix C library                 *
*                                                                             *
******************************************************************************/
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    register const unsigned char *cp = ptr;
    register size_t s;
    size_t ndone = 0;

    if (size)
        while ( ndone < nmemb ) {
            s = size;
            do {
                if (putc((int)*cp, stream)
                    == EOF)
                    return ndone;
                cp++;
            }
            while (--s);
            ndone++;
        }
    return ndone;
}


/******************************************************************************
*                                                                             *
*   freopen() function has been taken from the Minix C library                *
*                                                                             *
******************************************************************************/
FILE *freopen(const char *name, const char *mode, FILE *stream)
{
    int i;
    int rwmode = 0, rwflags = 0;
    int fd, flags = stream->_flags & (_IONBF | _IOFBF | _IOLBF | _IOMYBUF);

    (void) fflush(stream);              /* ignore errors */
    (void) close(fileno(stream));

    switch(*mode++) {
    case 'r':
        flags |= _IOREAD;
        rwmode = O_RDONLY;
        break;
    case 'w':
        flags |= _IOWRITE;
        rwmode = O_WRONLY;
        rwflags = O_CREAT | O_TRUNC;
        break;
    case 'a':
        flags |= _IOWRITE | _IOAPPEND;
        rwmode = O_WRONLY;
        rwflags |= O_APPEND | O_CREAT;
        break;
    default:
        return (FILE *)NULL;
    }

    while (*mode) {
        switch(*mode++) {
        case 'b':
            continue;
        case '+':
            rwmode = O_RDWR;
            flags |= _IOREAD | _IOWRITE;
            continue;
        /* The sequence may be followed by aditional characters */
        default:
            break;
        }
        break;
    }

    if ((rwflags & O_TRUNC)
        || (((fd = open(name, rwmode)) < 0)
            && (rwflags & O_CREAT))) {
        if (((fd = creat(name, PMODE)) < 0) && flags | _IOREAD) {
            (void) close(fd);
            fd = open(name, rwmode);
        }
    }

    if (fd < 0) {
        for( i = 0; i < FOPEN_MAX; i++) {
            if (stream == _io[i]) {
                _io[i] = 0;
                break;
            }
        }
        if (stream != stdin && stream != stdout && stream != stderr)
            free((void *)stream);
        return (FILE *)NULL;
    }

    stream->_count = 0;
    stream->_fd = fd;
    stream->_flags = flags;
    return stream;
}


/******************************************************************************
*                                                                             *
*   fseek() function has been taken from the Minix C library                  *
*                                                                             *
******************************************************************************/
int fseek(FILE *stream, long int offset, int whence)
{
    int adjust = 0;
    long pos;

    stream->_flags &= ~(_IOEOF | _IOERR);
    /* Clear both the end of file and error flags */

    if (io_testflag(stream, _IOREADING)) {
        if (whence == SEEK_CUR
            && stream->_buf
            && !io_testflag(stream,_IONBF))
            adjust = stream->_count;
        stream->_count = 0;
    } else if (io_testflag(stream,_IOWRITING)) {
        fflush(stream);
    } else  /* neither reading nor writing. The buffer must be empty */
        /* EMPTY */ ;

    pos = lseek(fileno(stream), offset - adjust, whence);
    if (io_testflag(stream, _IOREAD) && io_testflag(stream, _IOWRITE))
        stream->_flags &= ~(_IOREADING | _IOWRITING);

    stream->_ptr = stream->_buf;
    return ((pos == -1) ? -1 : 0);
}


/******************************************************************************
*                                                                             *
*   fsetpos() function has been taken from the Minix C library                *
*                                                                             *
******************************************************************************/
int fsetpos(FILE *stream, fpos_t *pos)
{
    return fseek(stream, *pos, SEEK_SET);
}


/******************************************************************************
*                                                                             *
*   ftell() function has been taken from the Minix C library                  *
*                                                                             *
******************************************************************************/
long ftell(FILE *stream)
{
    long result;
    int adjust = 0;

    if (io_testflag(stream,_IOREADING))
        adjust = -stream->_count;
    else if (io_testflag(stream,_IOWRITING)
            && stream->_buf
            && !io_testflag(stream,_IONBF))
        adjust = stream->_ptr - stream->_buf;
    else adjust = 0;

    result = lseek(fileno(stream), (off_t)0, SEEK_CUR);

    if ( result == -1 )
        return result;

    result += (long) adjust;
    return result;
}
