/******************************************************************************
*                                                                             *
*   Module:     DosX.c                                                        *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       10/5/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This module contains the code for the DOS extender whose services are
        used by the operating system, mainly by the `dosfs' driver.

        This module also performs the file path conversion.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 10/5/97    Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include <errno.h>                      // Include UNIX error codes header file

#include <stdio.h>                      // File constants

#include "dosx.h"                       // Include its own header

#include "display.h"                    // Debugger display device

#include "inline.h"                     // Inline functions

#include "intel.h"                      // Include processor specific defines

#include "v86.h"                        // Include virtual 86 machine header

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

// Define DOS error codes as reported by the DOS interrupt 0x21

#define DOS_ENOERR              0       // No error
#define DOS_EINVFN              (-1)    // Function number invalid
#define DOS_EINVFILE            (-2)    // File not found
#define DOS_EINVPATH            (-3)    // Path not found
#define DOS_ENHANDLE            (-4)    // Too many open files, no more handles
#define DOS_EACCESS             (-5)    // Access denied
#define DOS_EINVH               (-6)    // Invalid handle
#define DOS_EMCB                (-7)    // Memory control block destroyed
#define DOS_EMEM                (-8)    // Insufficent memory
#define DOS_EINVMBA             (-9)    // Memory block address invalid
#define DOS_EINVENV             (-10)   // Invalid environment
#define DOS_EINVFMT             (-11)   // Format invalid
#define DOS_EACCESSC            (-12)   // Invalid access code
#define DOS_EINVDATA            (-13)   // Data invalid
#define DOS_EINVDRIVE           (-15)   // Invalid drive
#define DOS_EREMCUR             (-16)   // Cant remove current directory
#define DOS_EDEVICE             (-17)   // Not same device
#define DOS_EFILES              (-18)   // No more files

#define DOS_EWONLY              (-19)   // Write protected
#define DOS_EXUNIT              (-20)   // Unknown unit
#define DOS_ENREADY             (-21)   // Drive not ready
#define DOS_EXCMD               (-22)   // Unknown command
#define DOS_ECRC                (-23)   // Data error (CRC)
#define DOS_ERQLEN              (-24)   // Bad request structure len
#define DOS_ESEEK               (-25)   // Seek error
#define DOS_EMEDIA              (-26)   // Unknown media type
#define DOS_ESECTOR             (-27)   // Sector not found
#define DOS_EPRINTER            (-28)   // Printer out of paper
#define DOS_EWRITE              (-29)   // Write fault
#define DOS_EREAD               (-30)   // Read fault
#define DOS_EGENERAL            (-31)   // General failure
#define DOS_ESHARE              (-32)   // Sharing violation
#define DOS_ELOCK               (-33)   // Lock violation
#define DOS_EDISK               (-34)   // Disk change invalid
#define DOS_EFCB                (-35)   // FCB unavailable
#define DOS_ESBUFF              (-36)   // Sharing buffer overflow
#define DOS_ECODEPG             (-37)   // Code page mismatch
#define DOS_EXCOMPL             (-38)   // Cant complete file operation
#define DOS_ESPACE              (-39)   // Insufficient disk space

#define DOS_EEXIST              (-80)   // File exists
#define DOS_EMKDIR              (-81)   // Cant make directory
#define DOS_EINT24              (-82)   // Fail on INT 24



#define DOS_BUFFER_LEN      1024        // Length of a buffer in DOS memory

#define FLAGS               Reg.esp     // Alias for flags
#define PFLAGS              Reg->esp    // Alias for pointer to flags

//----------------------------------------------------------------------------
// Buffer in low DOS memory that is used to pass parameters and data
//----------------------------------------------------------------------------

static WORD wBufferSeg;
static DWORD dwBuffer;
static DWORD dwInDOS;

//----------------------------------------------------------------------------
// Macro that copies string and appends a byte of zero
//----------------------------------------------------------------------------

#define COPY_ASCII(dest,src,len)    \
abs_memcpy(dest,src,len);  abs_pokeb((dest)+(len),0)

//----------------------------------------------------------------------------
// This is a temp buffer that UnixToDos() uses to translate a file name
// It is also used with findfirst() as a temp buffer.
//----------------------------------------------------------------------------

static char sNameBuf[ PATHNAME_MAX ];   // Path/name buffer length

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   void Init_DosX()                                                          *
*                                                                             *
*******************************************************************************
*
*   Initializes DOS extender data staructures.
*
******************************************************************************/
void Init_DosX()
{
    dprintf("\nInit DOS Extender");

    // Clear the global register structure

    memset( &Reg, 0, sizeof(TReg) );
    memset( &Seg, 0, sizeof(TSeg) );

    // Get the address of the InDOS flag

    Reg.eax = 0x3400;
    V86_Int( 0x21, &Seg, &Reg, NULL );

    dwInDOS = Seg.es * 16 + Reg.ebx;

    // Allocate a buffer in the low memory for the use by the extender

    wBufferSeg = DOS_Alloc( DOS_BUFFER_LEN );

    if( wBufferSeg==0 ) KERNEL_DIE;

    dprintf("\nDOS Extender support, buffer at %04X:0000.", wBufferSeg );
    dprintf("\nInDOS flag at %08Xh.", dwInDOS );

    dwBuffer = wBufferSeg * 16;
}


/******************************************************************************
*                                                                             *
*   char *UnixToDos( char *pName )                                            *
*                                                                             *
*******************************************************************************
*
*   This function converts an unix-style path/name string to a Dos-style.
#   It returns a pointer to a static buffer.
*
*   Where:
*       pName is an unix-style path/name string (e.g. `/dev/tty0')
*
*   Returns:
*       pointer to a Dos-style path/name string (e.g. `\dev\tty0')
#           that is sNameBuf array
*
******************************************************************************/
char *UnixToDos( char *pName )
{
    char *pDest = sNameBuf;

    while( *pName )
    {
        if( *pName=='/' )
            *pDest = '\\';
        else
            *pDest = *pName;

        pName++, pDest++;
    }

    *pDest = '\0';

    return( sNameBuf );
}


/******************************************************************************
*                                                                             *
*   int Error( int EC )                                                       *
*                                                                             *
*******************************************************************************
*
*   Translates DOS error code into UNIX POSIX error code
*
*   Where:
*       EC is a DOS file system error code
*
*   Returns:
*       POSIX equivalent error code
*
******************************************************************************/
static int Error( int EC )
{
    // Switch on most common error codes

    switch( EC & 0xFF )
    {
        case 1:     return( ENOSYS );
        case 2:     return( ENOENT );
        case 3:     return( ENOENT );
        case 4:     return( ENFILE );
        case 5:     return( EPERM );
        case 6:     return( EBADF );
        case 7:     return( ENOMEM );
        case 8:     return( ENOMEM );
        case 9:     return( ENOMEM );
        case 12:    return( EPERM);
        case 15:    return( ENXIO );
        case 16:    return( EBUSY );
        case 17:    return( EXDEV );
    }

    // For all other errors, return ENODEV

    return( ENODEV );
}


/******************************************************************************
*                                                                             *
*   int DOS_GetCurDrive()                                                     #
*                                                                             *
*******************************************************************************
*
*   Returns a current default drive
*
*   Where:
#       no args
*
*   Returns:
*       current default drive; 0-A, 1-B,...
*
******************************************************************************/
int DOS_GetCurDrive()
{
    // Call the DOS interrupt to get the drive

    Reg.eax = 0x1900;
    V86_Int( 0x21, &Seg, &Reg, NULL );

    return( Reg.eax );
}


/******************************************************************************
*                                                                             *
*   int DOS_mkdir( char *sPathname )                                          *
*                                                                             *
*******************************************************************************
*
*   This function creates a directory at the given path with the given name.
*
*   Where:
*       sPathname path and name of the (sub) directory to create
*
*   Returns:
*       < 0 if error (DOS_E* error code)
#       0 if no error
*
******************************************************************************/
int DOS_mkdir( char *sPathname )
{
    // Copy the pathname to the buffer in the low memory

    COPY_ASCII( dwBuffer, (DWORD) sPathname + LIN_KERNEL, strlen(sPathname) );

    // Call the DOS interrupt to create a directory

    Reg.eax = 0x3900;
    Reg.edx = 0;
    Seg.ds  = wBufferSeg;
    V86_Int( 0x21, &Seg, &Reg, NULL );

    // Can return errors: 3,5

    if( FLAGS & CF_MASK )
        return( Error(Reg.eax) );
    else
        return( 0 );
}


/******************************************************************************
*                                                                             *
*   int DOS_rmdir( char *sPathname )                                          *
*                                                                             *
*******************************************************************************
*
*   This function removes a directory that is specified with the given name.
*
*   Where:
*       sPathname is the path and name of the (sub) directory to remove
*
*   Returns:
*       < 0 if error (DOS_E* error code)
*       0 if no error
*
******************************************************************************/
int DOS_rmdir( char *sPathname )
{
    // Copy the pathname to the buffer in the low memory

    COPY_ASCII( dwBuffer, (DWORD) sPathname + LIN_KERNEL, strlen(sPathname) );

    // Call the DOS interrupt to remove a directory

    Reg.eax = 0x3A00;
    Reg.edx = 0;
    Seg.ds  = wBufferSeg;
    V86_Int( 0x21, &Seg, &Reg, NULL );

    // Can return errors: 3,5,6,16

    if( FLAGS & CF_MASK )
        return( Error(Reg.eax) );
    else
        return( 0 );
}


/******************************************************************************
*                                                                             *
*   int DOS_chdir( char *sPathname )                                          *
*                                                                             *
*******************************************************************************
*
*   This function changes the current directory.
*
*   Where:
*       sPathname is the new current directory
*
*   Returns:
*       < 0 if error (DOS_E* error code)
*       0 if no error
*
******************************************************************************/
int DOS_chdir( char *sPathname )
{
    // Copy the pathname to the buffer in the low memory

    COPY_ASCII( dwBuffer, (DWORD) sPathname + LIN_KERNEL, strlen(sPathname) );

    // Call the DOS interrupt to change a directory

    Reg.eax = 0x3B00;
    Reg.edx = 0;
    Seg.ds  = wBufferSeg;
    V86_Int( 0x21, &Seg, &Reg, NULL );

    // Can return errors: 3

    if( FLAGS & CF_MASK )
        return( Error(Reg.eax) );
    else
        return( 0 );
}


/******************************************************************************
*                                                                             *
*   int DOS_create( char *sPathname, int mode )                               *
*                                                                             *
*******************************************************************************
*
*   This function creates a file and returns a file handle.
*
*   Where:
*       sPathname is the path and name of the file to create.
#       mode is a file creation mode from sys/stat.h
*
*   Returns:
*       < 0 if error (DOS_E* error code)
#       >= 0  File handle
*
******************************************************************************/
int DOS_create( char *sPathname, int mode )
{
    int DOS_mode = 0;

    // Copy the pathname to the buffer in the low memory

    COPY_ASCII( dwBuffer, (DWORD) sPathname + LIN_KERNEL, strlen(sPathname) );

#if 0
// What's the point in creating a read-only file ?
    if( mode & 0444  )
        DOS_mode |= DOS_READONLY;
#endif

    // Call the DOS interrupt to create a file
//dprintf("\nDOSX creat (%X)", DOS_mode );

    Reg.eax = 0x3C00;
    Reg.ecx = DOS_mode;
    Reg.edx = 0;
    Seg.ds  = wBufferSeg;
    V86_Int( 0x21, &Seg, &Reg, NULL );

    // Can return errors: 3,4,5

    if( FLAGS & CF_MASK )
        return( Error(Reg.eax) );
    else
        return( Reg.eax );
}


/******************************************************************************
*                                                                             *
*   int DOS_open( char *sPathname, int access )                               *
*                                                                             *
*******************************************************************************
*
*   Opens a file for reading/writing or both.
*
*   Where:
*       sPathname is the path and name of a file to open
#       access is one of O_RDONLY, O_WRONLY or O_RDWR
#                        (0)       (1)         (2)
*
*   Returns:
*       < 0 if error (DOS_E* error code)
#       >= 0  File handle
*
******************************************************************************/
int DOS_open( char *sPathname, int access )
{
    char *pName;

    // Convert the path name to a DOS-style

    pName = UnixToDos( sPathname );

    // Copy the pathname to a buffer in the low memory

    COPY_ASCII( dwBuffer, (DWORD) pName + LIN_KERNEL, strlen(pName) );

//dprintf("\nName: %s -> %s  access: %08X", sPathname, pName, access );

    // Call the DOS interrupt to open a file

    Reg.eax = 0x3D00 + access;
    Reg.ecx = 0;
    Reg.edx = 0;
    Seg.ds  = wBufferSeg;
    V86_Int( 0x21, &Seg, &Reg, NULL );

//dprintf("\nfl: %08X  eax: %08X", FLAGS, Reg.eax );

    // Can return errors: 1,2,3,4,5,12

    if( FLAGS & CF_MASK )
        return( Error(Reg.eax) );
    else
        return( Reg.eax );
}


/******************************************************************************
*                                                                             *
*   int DOS_close( int handle )                                               *
*                                                                             *
*******************************************************************************
*
*   This function closes a file with a given handle.
*
*   Where:
*       handle is the file handle to close
*
*   Returns:
*       < 0 if error (DOS_E* error code)
*       0 if no error
*
******************************************************************************/
int DOS_close( int handle )
{
    // Call the DOS interrupt to close a file

    Reg.eax = 0x3E00;
    Reg.ebx = handle;
    V86_Int( 0x21, &Seg, &Reg, NULL );
//dprintf("\nDOSX Close (%d) %04X", handle, Reg.eax );
    // Can return errors: 6

    if( FLAGS & CF_MASK )
        return( Error(Reg.eax) );
    else
        return( 0 );
}


/******************************************************************************
*                                                                             *
*   int DOS_read( int handle, int len, char *pDest)                           *
*                                                                             *
*******************************************************************************
*
*   This function reads a file and stores the content in a buffer.
*
*   Where:
*       handle is a file handle to read
#       len is the number of bytes to read
#       pDest is a kernel buffer address
*
*   Returns:
*       < 0 if error (DOS_E* error code)
*       number of bytes actually read
*
******************************************************************************/
int DOS_read( int handle, int len, char *pDest )
{
    int nRead = 0, nReq;

//dprintf("\nhandle:%d *len: %d, dest: %08X", handle, len, pDest );
    while( len > 0 )
    {
//dprintf("\nlen: %d, read:%d, eax:%d", len, nRead, Reg.eax );
        // Call the DOS interrupt to read a file into a low memory buffer

        Reg.eax = 0x3F00;
        Reg.ebx = handle;
        Reg.ecx = nReq = (len > DOS_BUFFER_LEN)? DOS_BUFFER_LEN : len;
        Reg.edx = 0;
        Seg.ds  = wBufferSeg;
        V86_Int( 0x21, &Seg, &Reg, NULL );

//dprintf(" fl: %04X", FLAGS );
//dprintf("\ndest:%08X src:%08X (%d)", pDest + nRead + LIN_KERNEL, dwBuffer, Reg.eax );
        if( FLAGS & CF_MASK )
            return( Error(Reg.eax) );

        // Copy block to a kernel destination buffer

        abs_memcpy( (DWORD)pDest + nRead + LIN_KERNEL, dwBuffer, Reg.eax );

        nRead += Reg.eax;

        // Return if the number of bytes read is not equal requested size,
        // that means we read past the end of a file

        // Can return errors: 5,6

        if( Reg.eax < nReq )
            return( nRead );

        len   -= Reg.eax;
    }

    return( nRead );
}


/******************************************************************************
*                                                                             *
*   int DOS_write( int handle, int len, char *pSrc )                          *
*                                                                             *
*******************************************************************************
*
*   This function writes to a file the content of a buffer.
*
*   Where:
*       handle is a file handle to write to
#       len is the number of bytes to write, if 0, file is truncated
#       pSrc is a kernel buffer address
*
*   Returns:
*       < 0 if error (DOS_E* error code)
*       number of bytes actually written
*
******************************************************************************/
int DOS_write( int handle, int len, char *pSrc )
{
    int nWrite = 0;
    int nLen;

//dprintf("\nwrite %d b from %08X hnd:%d", len, pSrc, handle );

    while( len > 0 )
    {
        if( len > DOS_BUFFER_LEN )
            nLen = DOS_BUFFER_LEN;
        else
            nLen = len;

        // Copy the data to a DOS destination buffer

        abs_memcpy( dwBuffer, (DWORD)pSrc + nWrite + LIN_KERNEL, nLen );

        // Call the DOS interrupt to write a file
//dprintf("\n  %08X (%d)", (DWORD)pSrc + nWrite + LIN_KERNEL, nLen );
        Reg.eax = 0x4000;
        Reg.ebx = handle;
        Reg.ecx = nLen;
        Reg.edx = 0;
        Seg.ds  = wBufferSeg;
        V86_Int( 0x21, &Seg, &Reg, NULL );

        // Can return errors: 5,6

        if( FLAGS & CF_MASK )
            return( Error(Reg.eax) );

        nWrite += nLen;
        len    -= nLen;
    }

    return( nWrite );
}


/******************************************************************************
*                                                                             *
*   int DOS_unlink( char *sPathname )                                         *
*                                                                             *
*******************************************************************************
*
*   This function deletes a file from the given directory.
*
*   Where:
*       sPathname is the path and file name of a file to delete
*
*   Returns:
*       < 0 if error (DOS_E* error code)
*       0 if no error
*
******************************************************************************/
int DOS_unlink( char *sPathname )
{
    // Copy the pathname to the buffer in the low memory

    COPY_ASCII( dwBuffer, (DWORD) sPathname + LIN_KERNEL, strlen(sPathname) );

    // Call the DOS interrupt to delete a file

    Reg.eax = 0x4100;
    Reg.edx = 0;
    Seg.ds  = wBufferSeg;
    V86_Int( 0x21, &Seg, &Reg, NULL );

    // Can return errors: 2,3,5

    if( FLAGS & CF_MASK )
        return( Error(Reg.eax) );
    else
        return( 0 );
}


/******************************************************************************
*                                                                             *
*   int DOS_lseek( int handle, int offset, int whence )                       *
*                                                                             *
*******************************************************************************
*
*   This function moves a file's read/write pointer.
*
*   Where:
*       handle is a file handle to operate
#       offset is a new offset
#       whence is one of the following: SEEK_SET, SEEK_CUR, SEEK_END
*
*   Returns:
*       new file pointer offset
#       error code if error occurred
*
******************************************************************************/
int DOS_lseek( int handle, int offset, int whence )
{
    // Call the DOS interrupt to seek an offset

    Reg.eax = 0x4200 + whence;
    Reg.ebx = handle;
    Reg.ecx = (offset >> 16) & 0xFFFF;
    Reg.edx = offset & 0xFFFF;
    V86_Int( 0x21, &Seg, &Reg, NULL );

    // Can return errors: 1,6

    if( FLAGS & CF_MASK )
        return( Error(Reg.eax) );
    else
        return( (Reg.edx << 16) + Reg.eax );
}


/******************************************************************************
*                                                                             *
*   int DOS_getattrib( char *sPathname )                                      *
*                                                                             *
*******************************************************************************
*
*   Returns the file attributes.
*
*   Where:
*       sPathname is the path and name of a file to retrieve attributes
*
*   Returns:
*       < 0 if error (DOS_E* error code)
#       >=0 File attributes
*
******************************************************************************/
int DOS_getattrib( char *sPathname )
{
    char *pName;

    // Convert the path name to a DOS-style

    pName = UnixToDos( sPathname );

    // Copy the pathname to a buffer in the low memory

    COPY_ASCII( dwBuffer, (DWORD) pName + LIN_KERNEL, strlen(pName) );

//dprintf("\nDOS_getattrib: %s -> %s", sPathname, pName );

    // Call the DOS interrupt to get the file attributes

    Reg.eax = 0x4300;
    Reg.ecx = 0;
    Reg.edx = 0;
    Seg.ds  = wBufferSeg;
    V86_Int( 0x21, &Seg, &Reg, NULL );

    // Can return errors: 1,2,3,5

    if( FLAGS & CF_MASK )
        return( Error(Reg.eax) );
    else
        return( Reg.ecx );
}


/******************************************************************************
*                                                                             *
*   int DOS_chmod( char *sPathname, int attrib )                              *
*                                                                             *
*******************************************************************************
*
*   Sets the file attributes.
*
*   Where:
*       sPathname is the path and name of a file to set attributes
*
*   Returns:
*       < 0 if error (DOS_E* error code)
#       0 if no error
*
******************************************************************************/
int DOS_chmod( char *sPathname, int attrib )
{
    char *pName;

    // Convert the path name to a DOS-style

    pName = UnixToDos( sPathname );

    // Copy the pathname to a buffer in the low memory

    COPY_ASCII( dwBuffer, (DWORD) pName + LIN_KERNEL, strlen(pName) );

//dprintf("\nDOS_chmod: %s -> %s (%d)", sPathname, pName, attrib );

    // Call the DOS interrupt to get the file attributes

    Reg.eax = 0x4301;
    Reg.ecx = attrib;
    Reg.edx = 0;
    Seg.ds  = wBufferSeg;
    V86_Int( 0x21, &Seg, &Reg, NULL );

    // Can return errors: 1,2,3,5

    if( FLAGS & CF_MASK )
        return( Error(Reg.eax) );
    else
        return( 0 );
}


/******************************************************************************
*                                                                             *
*   int DOS_ioctl( int ioctl, TReg *Reg )                                     *
*                                                                             *
*******************************************************************************
*
*   This function performs io control call
*
*   Where:
*       ioctl is the call number
#       Reg is the register structure on input and on output
*
*   Returns:
*       < 0 if error (DOS_E* error code)
#       Reg returns back all values
*
******************************************************************************/
int DOS_ioctl( int ioctl, TReg *Reg )
{
    // Call the DOS interrupt for io control

    Reg->eax = 0x4400 + ioctl;
    V86_Int( 0x21, &Seg, Reg, NULL );

    if( PFLAGS & CF_MASK )
        return( Error(Reg->eax) );
    else
        return( Reg->eax );
}


/******************************************************************************
*                                                                             *
*   int DOS_dup( int handle )                                                 *
*                                                                             *
*******************************************************************************
*
*   This function duplicates a file handle.
*
*   Where:
*       handle is a file handle to duplicate
*
*   Returns:
*       < 0 if error (DOS_E* error code)
#       duplicate file handle
*
******************************************************************************/
int DOS_dup( int handle )
{
    // Call the DOS interrupt to duplicate a file handle

    Reg.eax = 0x4500;
    Reg.ebx = handle;
    V86_Int( 0x21, &Seg, &Reg, NULL );

    // Can return errors: 4,6

    if( FLAGS & CF_MASK )
        return( Error(Reg.eax) );
    else
        return( Reg.eax );
}


/******************************************************************************
*                                                                             *
*   int DOS_dup2( int handle, int handle2 )                                   *
*                                                                             *
*******************************************************************************
*
*   This function forces a file with the handle2 to be closed and then it
#   duplicates it so that it refer to the file with the handle (1).
*
*   Where:
*       handle is the original file to duplicate
#       handle2 is the desired new file descriptor
*
*   Returns:
*       < 0 if error (DOS_E* error code)
*       0 if no error
*
******************************************************************************/
int DOS_dup2( int handle, int handle2 )
{
    // Call the DOS interrupt to duplicate a file handle

    Reg.eax = 0x4600;
    Reg.ebx = handle;
    Reg.ecx = handle2;
    V86_Int( 0x21, &Seg, &Reg, NULL );

    // Can return errors: 4,6

    if( FLAGS & CF_MASK )
        return( Error(Reg.eax) );
    else
        return( 0 );
}


/******************************************************************************
*                                                                             *
*   char * DOS_getcwd()                                                       *
*                                                                             *
*******************************************************************************
*
*   This function retrieves the current working directory.
*
*   Where:
#       no args
*
*   Returns:
*       NULL if error
*       pointer to a temp buffer containing the cwd.  The value must be copied
#         away before another call to any DOS extender function.
*
******************************************************************************/
char * DOS_getcwd()
{
    // Call the DOS interrupt to get the current working directory

    Reg.eax = 0x4700;
    Reg.edx = 0x0000;                   // Default drive
    Reg.esi = 0;
    Seg.ds  = wBufferSeg;
    V86_Int( 0x21, &Seg, &Reg, NULL );

    // Can return errors: 15

    if( FLAGS & CF_MASK )
        return( NULL );

    // Copy the pathname to a buffer

    abs_memcpy( (DWORD)sNameBuf + LIN_KERNEL, dwBuffer, PATHNAME_MAX );

    return( sNameBuf );
}


/******************************************************************************
*                                                                             *
*   int DOS_alloc( int nSize )                                                *
*                                                                             *
*******************************************************************************
*
*   Allocates a buffer of nSize bytes in low memory.
*
*   Where:
*       nSize is the size of a buffer to be allocated in bytes
*
*   Returns:
*       < 0 if error (DOS_E* error code)
#       segment address of a block (handle)
*
******************************************************************************/
int DOS_alloc( int nSize )
{
    // Call the DOS interrupt to allocate memory

    Reg.eax = 0x4800;
    Reg.ebx = (nSize + 15) / 16;        // Make it in paragraphs
    V86_Int( 0x21, &Seg, &Reg, NULL );

    // Can return errors: 7,8

    if( FLAGS & CF_MASK )
        return( Error(Reg.eax) );
    else
        return( Reg.eax );
}


/******************************************************************************
*                                                                             *
*   int DOS_free( int handle )                                                *
*                                                                             *
*******************************************************************************
*
*   This function frees a memory block that was allocated using the DOS_Alloc
#   call.
*
*   Where:
*       handle is the segment returned by the DOS_Alloc call
*
*   Returns:
*       < 0 if error (DOS_E* error code)
*
******************************************************************************/
int DOS_free( int handle )
{
    // Call the DOS interrupt to free memory

    Reg.eax = 0x4900;
    Seg.es  = handle;
    V86_Int( 0x21, &Seg, &Reg, NULL );

    // Can return errors: 7,9

    if( FLAGS & CF_MASK )
        return( Error(Reg.eax) );
    else
        return( Reg.eax );
}


/******************************************************************************
*                                                                             *
*   int * DOS_findfirst( char *sPathname, int mask, struct DOS_DTA *DTA )     *
*                                                                             *
*******************************************************************************
*
*   This function finds the first file that matches the search criterion.
*
*   Where:
*       sPathname is the search string with possible wildcards
#       mask is the search mask (attributes)
#       DTA buffer for the current directory search
*
*   Returns:
*       Error code if failed
#       0 on success
*
******************************************************************************/
int DOS_findfirst( char *sPathname, int mask, struct DOS_DTA *DTA )
{
    int Orig_DTA;

    // Convert the path name to a DOS-style

    UnixToDos( sPathname );
    strcat( sNameBuf, "*.*");

    // Copy the pathname to the buffer in the low memory

    COPY_ASCII( dwBuffer, (DWORD) sNameBuf + LIN_KERNEL, strlen(sNameBuf) );

    // Call the DOS interrupt to find a file

    Reg.eax = 0x4E00;
    Reg.ecx = mask;
    Reg.edx = 0;
    Seg.ds  = wBufferSeg;
    V86_Int( 0x21, &Seg, &Reg, NULL );

    // Can return errors: 2,3,18

    if( FLAGS & CF_MASK )
        return( Error(Reg.eax) );

    Orig_DTA = DOS_getPSP() + DTA_PSPOFFSET;

    // Copy the DTA to the destination buffer

    abs_memcpy( (DWORD) DTA + LIN_KERNEL, Orig_DTA, DTA_LEN );

    return( 0 );
}


/******************************************************************************
*                                                                             *
*   int DOS_findnext( struct DOS_DTA *DTA )                                   *
*                                                                             *
*******************************************************************************
*
*   This function returns next file buffer from the search.
*
*   Where:
*       DTA is the buffer initialized by DOS_findfirst()
*
*   Returns:
*       Error code if failed or end of directory stream
#       0 on success
*
******************************************************************************/
int DOS_findnext( struct DOS_DTA *DTA )
{
    int Orig_DTA;

    Orig_DTA = DOS_getPSP() + DTA_PSPOFFSET;

    // Copy the working buffer to the low memory DTA

    abs_memcpy( Orig_DTA, (DWORD) DTA + LIN_KERNEL, DTA_LEN );

    // Call the DOS interrupt to find next file

    Reg.eax = 0x4F00;
    V86_Int( 0x21, &Seg, &Reg, NULL );

    // Copy the DTA back to the kernel buffer

    abs_memcpy( (DWORD) DTA + LIN_KERNEL, Orig_DTA, DTA_LEN );

    // Can return errors: 2,3,18

    if( FLAGS & CF_MASK )
        return( Error(Reg.eax) );

    return( 0 );
}

/******************************************************************************
*                                                                             *
*   void DOS_setPSP( int newPSP )                                             *
*                                                                             *
*******************************************************************************
*
*   This function sets a current PSP for file operations.  The value is an
#   absolute address that will be transformed into a segment value.
*
*   Where:
*       newPSP is the new Program Segment Prefix to be set as abs. address
*
*   Returns:
*       void
*
******************************************************************************/
void DOS_setPSP( int newPSP )
{
    // Call the DOS interrupt to set PSP

    Reg.eax = 0x5000;
    Reg.ebx = newPSP >> 4;
    V86_Int( 0x21, &Seg, &Reg, NULL );
}


/******************************************************************************
*                                                                             *
*   int DOS_getPSP()                                                          *
*                                                                             *
*******************************************************************************
*
*   This function returns the current PSP
*
*   Returns:
*       The absolute address of a current PSP
*
******************************************************************************/
int DOS_getPSP()
{
    // Call the DOS interrupt to get PSP

    Reg.eax = 0x5100;
    V86_Int( 0x21, &Seg, &Reg, NULL );

    return( Reg.ebx << 4 );
}


/******************************************************************************
*                                                                             *
*   void DOS_childPSP( int parentPSP )                                        *
*                                                                             *
*******************************************************************************
*
*   This function creates a child PSP at the given segment.
*
*   Where:
*       parentPSP is the PSP of the parent process
*
*   Returns:
*       void
*
******************************************************************************/
void DOS_childPSP( int parentPSP )
{
    // Call the DOS interrupt to set up a child PSP

    Reg.eax = 0x5500;
    V86_Int( 0x21, &Seg, &Reg, NULL );
}


/******************************************************************************
*                                                                             *
*   int DOS_rename( char *sNewname, char *sOldname )                          *
*                                                                             *
*******************************************************************************
*
*   This function renames a file or a subdirectory.
*
*   Where:
*       sNewname is the new name to be set
#       sOldname is the name of the file or directory to be renamed
*
*   Returns:
*       < 0 if error (DOS_E* error code)
*       0 if no error
*
******************************************************************************/
int DOS_rename( char *sNewname, char *sOldname )
{
    int lenNewname, lenOldname;

    lenNewname = strlen(sNewname);
    lenOldname = strlen(sOldname);

    // Copy the pathnames to the buffer in the low memory - first the newname
    // followed by the oldname

    COPY_ASCII( dwBuffer, (DWORD) sNewname + LIN_KERNEL, lenNewname );
    COPY_ASCII( dwBuffer + lenNewname + 1, (DWORD) sOldname + LIN_KERNEL, lenOldname );

    // Call the DOS interrupt to rename a file

    Reg.eax = 0x5600;
    Reg.edi = 0;
    Seg.es  = wBufferSeg;
    Reg.edx = lenNewname + 1;  // Offset of the old pathname
    Seg.ds  = wBufferSeg;
    V86_Int( 0x21, &Seg, &Reg, NULL );

    // Can return errors: 2,3,5,17

    if( FLAGS & CF_MASK )
        return( Error(Reg.eax) );
    else
        return( 0 );
}


/******************************************************************************
*                                                                             *
*   int DOS_flush( int handle )                                               *
*                                                                             *
*******************************************************************************
*
*   This function flushes all buffered data for a specific file to disk.
*
*   Where:
*       handle is the file handle of a file to be flushed
*
*   Returns:
*       < 0 if error (DOS_E* error code)
*       0 if no error
*
******************************************************************************/
int DOS_flush( int handle )
{
    // Call the DOS interrupt to flush file

    Reg.eax = 0x6800;
    Reg.ebx = handle;
    V86_Int( 0x21, &Seg, &Reg, NULL );

    // Can return errors: ???

    if( FLAGS & CF_MASK )
        return( Error(Reg.eax) );
    else
        return( 0 );
}

