/******************************************************************************
*                                                                             *
*   Module:     DosFS.c                                                       *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       11/1/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This module contains the code for the DOS File System device driver.
        It extensively uses DOS extender module `DosX.c'.
-.
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

#include <errno.h>                      // Include error definitions

#include <string.h>                     // Include string function header file

#include <fcntl.h>                      // Include file operations header

#include <sys\ioctl.h>                  // Include io control header file

#include <sys\stat.h>                   // Include file information header

#include "device.h"                     // Include device driver header file

#include "intel.h"                      // Include Intel-specific defines

#include "mm.h"                         // Include memory management header

#include "DosFS.h"                      // Include its own header

#include "File.h"                       // Include file management header

#include "DosX.h"                       // Include DOS extender header file

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

extern BYTE fs_root_major;
extern BYTE fs_root_minor;


/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

//----------------------------------------------------------------------------
// Define a device control block for the DosFS device
//----------------------------------------------------------------------------

int DosFS_init();

int DosFS_creat( BYTE bMinor, TDevRq *pRq );
int DosFS_open( BYTE bMinor, TDevRq *pRq );
int DosFS_close( BYTE bMinor, TDevRq *pRq );
int DosFS_read( BYTE bMinor, TDevRq *pRq );
int DosFS_write( BYTE bMinor, TDevRq *pRq );
int DosFS_lseek( BYTE bMinor, TDevRq *pRq );
int DosFS_ioctl( BYTE bMinor, TDevRq *pRq );
int DosFS_stat( BYTE bMinor, TDevRq *pRq );

//----------------------------------------------------------------------------

TDev DevDosFS =                         // Device control block template
{
    MINOR_DOSFS,                        // Number of minor instances

    DosFS_init,                         // Initialize DOS file system

    DosFS_creat,                        //  Create a file
    DosFS_open,                         //  Open a file
    DosFS_close,                        //  Close a file
    DosFS_read,                         //  Read from a file
    DosFS_write,                        //  Write to a file
    DosFS_lseek,                        //  Seek into a file
    DosFS_ioctl,                        //  Control aspects of a device
    DosFS_stat                          //  Get stat of a node
};

//----------------------------------------------------------------------------

// Define the array of DOS letters for minor devices

char DOS_Letter[ MINOR_DOSFS ];

#define IS_DOS_VALID(minor)     (DOS_Letter[minor]!=' ')

//.-
//-----------------------------------------------------------------------------
// Define structure of a special file from the DOS FS
// Special files have DOS System flag set in the attribute field (that also
// makes them `invisible' from the DOS simple `dir' command.
//-----------------------------------------------------------------------------

#define FILE_CDEVICE        0x01        // File is a character device
#define FILE_BDEVICE        0x02        // File is a block device
#define FILE_FIFO           0x03        // File is a FIFO

#define FILE_SIG            'SOAY'      // Special file signature (`YAOS')


typedef struct
{
    DWORD sig;                          // Signature
    BYTE bFiller;                       // Usually contains 0x1A
    BYTE bType;                         // File type
    BYTE bMajor;                        // Major device number
    BYTE bMinor;                        // Minor device number

} TFileSpec;

//-.

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   int DosFS_init()                                                          *
*                                                                             *
*******************************************************************************
*
*   This function initializes the DOS file system device driver.
*
*   Where:
*       no args
*
*   Returns:
*       0 if successful
#       -1 if device cannot be initialized
*
******************************************************************************/
int DosFS_init()
{
    int i, found = 0, cur;

    // Clean up the array of DOS letters to no assignment

    memset( &DOS_Letter[0], ' ', MINOR_DOSFS );

    // Loop for all minor devices, validate and allocate the DOS letter

    dprintf("\nIntegrated DOS File System support - ");

    for( i=0; i<MINOR_DOSFS; i++ )
    {
        Reg.ebx = i + 1;
        if( DOS_ioctl( DOSIOCTL_GETLOCALDRVMAP, &Reg ) >= 0 )
        {
            // Drive is found

            DOS_Letter[i] = i + 'A';
            found++;
            dprintf("%c: ", i + 'A' );
        }
    }

    if( !found )
    {
        dprintf("No DOS drives found!");
        return( -1 );
    }

    // Find teh current drive letter so we can mount root file system
    // using that device

    cur = DOS_GetCurDrive();

    dprintf("\nCurrent drive is %c:", cur + 'A' );
    dprintf("\nMounting `/dev/dos%c' on root `/'", cur + 'a' );

    fs_root_major = MAJOR_DOSFS;
    fs_root_minor = cur;

    return( 0 );
}


/******************************************************************************
*                                                                             *
*   int DosFS_open( BYTE bMinor, TDevRq *pRq )                                *
*                                                                             *
*******************************************************************************
*
*   This function performs the `open' function on a device.
#   The file may be a regular file or a directory.
*
#   Note: For a file to be opened, it first has to be `stat' into a Stat
#         structure!
#
*   Where:
*       bMinor is the minor device number
#       pRq is the address of the request structure:
#           s->the address of the `struct stat' of a file to open
#           n->open access mode
*
*   Returns:
*       error if not successful
#       DOS file handle is successful
*
******************************************************************************/
int DosFS_open( BYTE bMinor, TDevRq *pRq )
{
    int DOS_handle;


    if( !IS_DOS_VALID(bMinor) )  return( ENODEV );

//dprintf("\nDOS open: `%s' %o", ((struct stat *)pRq->p)->st_name, pRq->n );

    // If the file is a directory, us different path

    if( ((struct stat *)pRq->s)->st_mode & S_IFDIR )
    {
        // File is a directory - open allways succeeds

        return( 0 );
    }
    else
    {
        // File is a regular file

        DOS_handle = DOS_open( &((struct stat *)pRq->s)->st_name, pRq->n );
    }

    ((struct stat *)pRq->s)->st_fd = DOS_handle;

//dprintf("\nDOS handle: %d", DOS_handle );

    return( DOS_handle );
}


/******************************************************************************
*                                                                             *
*   int DosFS_close( BYTE bMinor, TDevRq *pRq )                               *
*                                                                             *
*******************************************************************************
*
*   This function closes a file.
#   The file may be a regular file or a directory.
*
*   Where:
*       bMinor is the minor device number
#       pRq is the address of the request structure:
#           s->the address of the `struct stat' of a file to close
*
*   Returns:
*       error if not successful
#       0 is successful
*
******************************************************************************/
int DosFS_close( BYTE bMinor, TDevRq *pRq )
{
    if( !IS_DOS_VALID(bMinor) )  return( ENODEV );

    // If the file is a directory, us different path

    if( ((struct stat *)pRq->s)->st_mode & S_IFDIR )
    {
        // Free DOS directory structure (DTA)

        FREE( ((struct stat *)pRq->s)->st_dir );
        ((struct stat *)pRq->s)->st_dir = NULL;

        return( 0 );
    }
    else
    {
        // File is a regular file

        return( DOS_close( ((struct stat *)pRq->s)->st_fd) );
    }
}


/******************************************************************************
*                                                                             *
*   int DosFS_read( BYTE bMinor, TDevRq *pRq )                                *
*                                                                             *
*******************************************************************************
*
*   This function reads from an opened file.
#   The file may be a regular file or a directory.  If a file is a directory,
#   simulate reading i-node directory entry name.
*
*   Where:
*       bMinor is the minor device number
#       pRq is the address of the request structure:
#           s->the address of the `struct stat' of a file to read
#           p->the destination address to read into
#           n->the number of bytes to read
*
*   Returns:
*       error if not successful
#       number of bytes read is successful
*
******************************************************************************/
int DosFS_read( BYTE bMinor, TDevRq *pRq )
{
    int err;
    struct stat *pStat;
    struct DOS_DTA *pDTA;


    if( !IS_DOS_VALID(bMinor) )  return( ENODEV );

    // If the file is a directory, us different path

    pStat = (struct stat *)pRq->s;

    if( pStat->st_mode & S_IFDIR )
    {
        // File is a directory

        if( pStat->st_dir == NULL )
        {
            // This is a first directory read

            // Allocate DOS directory structure (DTA)

            if( (pStat->st_dir = pDTA = MALLOC( DTA_LEN ))==NULL)  return( ENOMEM );

            // Call findfirst to set up the directory search and return first
            // matching entry

            err = DOS_findfirst( &pStat->st_name, 63, pStat->st_dir );
        }
        else
        {
            // This is not a first directory read

            pDTA = (struct DOS_DTA *) pStat->st_dir;

            err = DOS_findnext( pDTA );
        }

        if( err < 0 )  return( err );

        // Copy the directory entry name into the process buffer

        memcpy( pRq->p, pDTA->sName, 13 );

        return( pRq->n );
    }
    else
    {
        // File is a regular file

        return( DOS_read( ((struct stat *)pRq->s)->st_fd, pRq->n, pRq->p) );
    }
}


/******************************************************************************
*                                                                             *
*   int DosFS_write( BYTE bMinor, TDevRq *pRq )                               *
*                                                                             *
*******************************************************************************
*
*   This function writes to an opened file.
*
*   Where:
*       bMinor is the minor device number
#       pRq is the address of the request structure:
#           s->the address of the `struct stat' of a file to write
#           p->the source address of the memory block to write
#           n->the number of bytes to write
*
*   Returns:
*       error if not successful
#       number of bytes written is successful
*
******************************************************************************/
int DosFS_write( BYTE bMinor, TDevRq *pRq )
{
    if( !IS_DOS_VALID(bMinor) )  return( ENODEV );

    return( DOS_write( ((struct stat *)pRq->s)->st_fd, pRq->n, pRq->p) );
}


/******************************************************************************
*                                                                             *
*   int DosFS_lseek( BYTE bMinor, TDevRq *pRq )                               *
*                                                                             *
*******************************************************************************
*
*   This function seeks into an open file.
*
*   Where:
*       bMinor is the minor device number
#       pRq is the address of the request structure:
#           s->the address of the `struct stat' of a file to seek
#           p->the offset to seek
#           n->whence
*
*   Returns:
*       error if not successful
#       new file position if successful
*
******************************************************************************/
int DosFS_lseek( BYTE bMinor, TDevRq *pRq )
{
    if( !IS_DOS_VALID(bMinor) )  return( ENODEV );

    return( DOS_lseek( ((struct stat *)pRq->s)->st_fd, (int)pRq->p, pRq->n) );
}


/******************************************************************************
*                                                                             *
*   int DosFS_ioctl( BYTE bMinor, TDevRq *pRq )                               *
*                                                                             *
*******************************************************************************
*
*   This function performs the io control call.
*
*   Where:
*       bMinor is the minor device number
#       pRq is the address of the request structure
*
*   Returns:
*
*
******************************************************************************/
int DosFS_ioctl( BYTE bMinor, TDevRq *pRq )
{
    int ret = 0;

    if( !IS_DOS_VALID(bMinor) )  return( ENODEV );

    switch( pRq->n )
    {
        case IOCTL_GETMODE:
        {
            // Returns a file attributes that needs to be transformed into
            // UNIX file mode

            ret = DOS_getattrib( pRq->p );

        } break;

        default:
            return( 0 );
    }

    return( ret );
}


/******************************************************************************
*                                                                             *
*   int DosFS_creat( BYTE bMinor, TDevRq *pRq )                               *
*                                                                             *
*******************************************************************************
*
*   This function performs the `creat' function on a device.
*
#   Note: For a file to be created, it first has to be `stat' into a Stat
#         structure!
#
*   Where:
*       bMinor is the minor device number
#       pRq is the address of the request structure:
#           p->the file path/name to create
#           s->the address of the `struct stat' of a file to create
#           n->create access mode
*
*   Returns:
*       error if not successful
#       DOS file handle is successful
*
******************************************************************************/
int DosFS_creat( BYTE bMinor, TDevRq *pRq )
{
    int DOS_handle;
    struct stat *pStat = (struct stat *) pRq->s;


    if( !IS_DOS_VALID(bMinor) )  return( ENODEV );

    // Form the absolute drive:/path/name from the minor device number

    pStat->st_name[0] = DOS_Letter[bMinor];
    pStat->st_name[1] = ':';
    strcpy( &pStat->st_name[2], pRq->p );

//dprintf("\nDOS creat: `%s' %X", pStat->st_name, pRq->n );

    DOS_handle = DOS_create( pStat->st_name, pRq->n );

    // Fill in the stat structure

    pStat->st_ino = 0;
    pStat->st_mode = pRq->n;
    pStat->st_nlink = 0;
    pStat->st_uid = 0;
    pStat->st_gid = 0;
    pStat->st_size = 0;

    // Set the DOS section of the file stat structure

    pStat->st_fd = DOS_handle;
    pStat->st_attr = 0;

//dprintf("\nCreate DOS handle: %d", DOS_handle );

    return( DOS_handle );
}


/******************************************************************************
*                                                                             *
*   int DosFS_stat( BYTE bMinor, TDevRq *pRq )                                *
*                                                                             *
*******************************************************************************
*
*   This function returns the file/node information structure
*
*   Where:
*       bMinor is the minor device number
#       pRq is the address of the request structure:
#           p->the address of a file's `/path/name' string
#           n->the address to fill in the `struct stat' of a file
*
*   Returns:
*       error if not successful
#       0 is successful
*
******************************************************************************/
int DosFS_stat( BYTE bMinor, TDevRq *pRq )
{
    TFileSpec Head;
    int Dos_Handle, read;
    struct stat *pStat = (struct stat *) pRq->n;
    mode_t mode = 0;
    int atr;

//dprintf("\nDOS Stat %c: at %08X", DOS_Letter[bMinor], pStat );
    if( !IS_DOS_VALID(bMinor) )  return( ENODEV );

    // Presume local regular file device number

    pStat->st_dev = (MAJOR_DOSFS << 8) | bMinor;

    // Form the absolute drive:/path/name from the minor device number

    pStat->st_name[0] = DOS_Letter[bMinor];
    pStat->st_name[1] = ':';
    strcpy( &pStat->st_name[2], pRq->p );
//dprintf("\nDOS Path: %s", pStat->st_name );

    // First, we need to get the file's attributes - that will tell what kind
    // of file that is (directory/file/system)

    if( (atr = DOS_getattrib( pStat->st_name )) < 0 )  return( atr );

    // If a file is a system file, in this FS implementation we need to read it

    if( atr & DOS_SYSTEM )
    {
        // A file is a special file.  Open it for reading and read it
        // to know what it actually is.

        if( (Dos_Handle = DOS_open( pStat->st_name, O_RDONLY )) < 0 )  return( Dos_Handle );
//dprintf("  hnd: %d", Dos_Handle);
        // Read a file into a header buffer

        memset( &Head, 0, sizeof(TFileSpec) );
        read = DOS_read( Dos_Handle, sizeof(TFileSpec), (char *) &Head );

        // Close a file

        DOS_close( Dos_Handle );

        // Return if there was an error reading it

        if( read <= 0 )  return( EIO );

        // Check the special file signature

        if( Head.sig != FILE_SIG )  return( EBADF );

        // Switch depending on the type of the special file

        switch( Head.bType )
        {
            case FILE_CDEVICE :  // File is a character device driver
//dprintf("\nCharacter device");
                    mode |= S_IFCHR;
                    pStat->st_dev = (Head.bMajor << 8) | Head.bMinor;

                break;

            case FILE_BDEVICE :  // File is a block device driver
//dprintf("\nBlock device");
                    mode |= S_IFBLK;
                    pStat->st_dev = (Head.bMajor << 8) | Head.bMinor;

                break;

            case FILE_FIFO    :  // File is a FIFO
//dprintf("\nFIFO");
                    mode |= S_IFIFO;
                    pStat->st_dev = (Head.bMajor << 8) | Head.bMinor;

                break;

            default           :  // Invalid special file
                return( EBADF );
        }
    }
    else
    if( atr & DOS_DIR )
    {
        // A file is a directory

        mode |= S_IFDIR;
    }
    else
    {
        // A file is a regular file

        mode |= S_IFREG;

        // Get a file size       ?????
    }

    // Adjust the file mode - use the closest match of DOS attributes

    mode |= S_IRWXU | S_IRWXG | S_IRWXO;

    if( atr & DOS_READONLY ) mode &= ~0333;

    // Fill in the stat structure

    pStat->st_ino = 0;
    pStat->st_mode = mode;
    pStat->st_nlink = 0;
    pStat->st_uid = 0;
    pStat->st_gid = 0;
    pStat->st_size = 0;             // ??????

    // Set the DOS section of the file stat structure

    pStat->st_fd = 0;
    pStat->st_attr = atr;

#if 0
    dprintf("\nDOS (%d.%d): mode %X  attrib: %04X",
        pStat->st_dev >> 8, pStat->st_dev & 0xFF,
        pStat->st_mode = mode, pStat->st_attr );
#endif

    return( 0 );
}

