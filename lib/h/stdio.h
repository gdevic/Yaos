/******************************************************************************
*                                                                             *
*   Module:     Stdio.h                                                       *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       10/14/97                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        ANSI C / POSIX stdio header file

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 04/20/96   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _STDIO_H_
#define _STDIO_H_


/******************************************************************************
*                                                                             *
*   Include Files                                                             *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

#define EOF                 (-1)        // End-Of-File/Error return code
#define FOPEN_MAX           16          // Number of files that can be handled
#define BUFSIZ              128         // Default size of a stream file buffer
#define FILENAME_MAX        13          // ?
#define PATHNAME_MAX        80          // Maximum size of path and name
#define TMP_MAX             (26*26*26)  // Number of temp files
#define NULL                0L

typedef long int fpos_t;
typedef unsigned int size_t;

// Define flags for the file stream entry

#define _IOFBF              0x0000      // Full buffering
#define _IOREAD             0x0001      // File is opened for reading
#define _IOWRITE            0x0002      // File is opened for writing
#define _IONBF              0x0004      // No buffering
#define _IOMYBUF            0x0008      // Stdio malloc()'d buffer
#define _IOEOF              0x0010      // EOF reached on read
#define _IOERR              0x0020      // I/O error from the system
#define _IOLBF              0x0040      // Line buffering
#define _IOREADING          0x0080      // Currently reading
#define _IOWRITING          0x0100      // Currently writing
#define _IOAPPEND           0x0200      // File is opened for append


// Define seek constants

#define SEEK_SET            0           // Seek relative to start of file
#define SEEK_CUR            1           // Seek relative to current position
#define SEEK_END            2           // Seek relative to end of file


typedef struct __iobuf
{
    unsigned char   *_buf;              // Pointer to a stream buffer
    unsigned char   *_ptr;              // Pointer to next character position
    int             _count;             // Num of available chars in a buffer
    int             _flags;             // The state of the stream
    int             _fd;                // UNIX System file descriptor
    int             _bufsiz;            // Size of a buffer

} FILE;


// The actual array of file descriptors for a process

extern FILE *_io [FOPEN_MAX];
extern FILE _stdin;
extern FILE _stdout;
extern FILE _stderr;

// Define standard input, output and error stream pointers

#define stdin       (&_stdin)
#define stdout      (&_stdout)
#define stderr      (&_stderr)


//----------------------------------------------------------------------------
// These are private definitions to support function declaration
//----------------------------------------------------------------------------

typedef char *__va_list[1];

/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern int setvbuf( FILE *fp, char *buf, int mode, size_t size );
extern void setbuf( FILE *fp, char *buffer );

extern int printf( const char *format, ... );
extern int vprintf( const char *format, __va_list arg );
extern int fprintf( FILE *fp, const char *format, ... );
extern int vfprintf( FILE *fp, const char *format, __va_list arg );

extern int sprintf( char *str, const char *format, ... );
extern int vsprintf( char *dest, const char *format, __va_list arg );

extern int scanf( const char *fmt, ... );
extern int vscanf( const char *fmt, __va_list arg );
extern int fscanf( FILE *fp, const char *fmt, ... );
extern int vfscanf( FILE *fp, const char *fmt, __va_list arg );

extern int ungetc( int c, FILE *fp );
extern int fgetc( FILE *fp );
extern int fputc( int c, FILE *fp );


extern int _filbuf(FILE *p);
extern int _flsbuf(int c, FILE *p);

//-----------------------------------------------------------------------------
// Define some functions as macros
//-----------------------------------------------------------------------------

#define getc(p)         (--(p)->_count >= 0 ? (int)(*(p)->_ptr++) : _filbuf(p))
#define putc(c,p)       (--(p)->_count >= 0 ? (int)(*(p)->_ptr++ = (c)) : _flsbuf((c),(p)))

#define getchar()       getc(stdin)
#define putchar(c)      putc((c),stdout)

#define feof(p)         ((p)->_flags & _IOEOF)
#define ferror(p)       ((p)->_flags & _IOERR)
#define clearerr(p)     ((p)->_flags & ~(_IOEOF | _IOERR))
#define fileno(p)       (p)->_fd


#define vprintf(f,a)        vfprintf(stdout,f,a)


#endif //  _STDIO_H_

