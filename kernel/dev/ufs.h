/******************************************************************************
*                                                                             *
*   Module:     ufs.h                                                         *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       09/17/96                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This is a header file for the Unified File System interface.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 09/17/96   1.00  Original                                       Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _UFS_H_
#define _UFS_H_

#ifdef TEST
#define kmalloc malloc
#define kfree   free
#define printk  printf
#endif

/******************************************************************************
*                                                                             *
*   Include Files                                                             *
*                                                                             *
******************************************************************************/

#include "types.h"                      // Include extended data types


// IOCtl error codes

#define EINV_REQUEST            -2      // Invalid request code for a device
#define EMALLOC                 -3      // Cant malloc memory


/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

#define SECTOR_SIZE             512     // Sector always has 512 bytes

#define MAX_OPEN_DIR            8       // System-wide numer of open dirs

//=============================================================================
// Defines for Low-Level device communication
//=============================================================================

// Request codes for the I/O device driver

#define IOCTL_DEVICE_INIT       0x0000  // Initializes a device
#define IOCTL_READ_SECTORS      0x0001  // Read one or more logical sectors
#define IOCTL_WRITE_SECTORS     0x0002  // Write one or more logical sectors
#define IOCTL_FLUSH_CACHE       0x0003  // Flush the device caches
#define IOCTL_READ_TRACK        0x0004  // Read a track (low-level, non-cached)
#define IOCTL_WRITE_TRACK       0x0005  // Write a track (low-level, non-cached)

typedef struct TIOCtlMsg                // I/O Control Message structure
{
    int nRequest;                       // Request code
    int nData;                          // Additional data (usually start)
    int nLen;                           // Additional data (usually length)
    void * pBuffer;                     // Memory buffer
    DWORD dwFlags;                      // Device flags
    BYTE bMinor;                        // Minor device to operate on

} TIOCtlMsg;

//=============================================================================
// Defines for Hardware Device Drivers
//=============================================================================

// Device number is [major*16]+[minor]
#define MAX_DEVICES_MAJOR       5       // Number of major devices
#define MAX_DEVICES  \
     (MAX_DEVICES_MAJOR*16+15)          // Number of devices supported

#define DEVICE_FLOPPY            0      // Major number of a floppy device

// Device flags, may be read in IOCtlMsg->dwFlags after an operation

#define DEV_NON_CACHED      0x00000001  // Non-cached device
#define DEV_WRITE_THROUGH   0x00000002  // Write-through caching strategy
#define DEV_WRITE_BACK      0x00000004  // Write-back caching strategy
#define DEV_REMOVABLE       0x00000008  // Removable device
#define DEV_MEDIA_CHANGE    0x00000010  // Media changed since last access

typedef struct TDevice                  // Hardware Device structure
{
    int (*IOCtl)(TIOCtlMsg * IOCtlMsg); // I/O Control function

    int hFS;                            // File system handle
    struct TSysCall * pFS;              // Pointer to a FS function calls

    BYTE bMajor;                        // Its own major number
    BYTE bMinor;                        // Its own minor number
    int hDev;                           // Handle to own device
    
} TDevice;


//=============================================================================
// Directory structure
//=============================================================================

#define NAME_MAX        12              // Max file name len

typedef union TAttr                   // File attributes structure type
{
    BYTE bAttr;                         // Attribute byte
    struct
    {
    BYTE fReadOnly          : 1;        // [0] Read only bit
    BYTE fHidden            : 1;        // [1] Hidden file
    BYTE fSystem            : 1;        // [2] System file
    BYTE fVolume            : 1;        // [3] Volume label
    BYTE fSubdir            : 1;        // [4] Subdirectory entry
    BYTE fArchive           : 1;        // [5] Archive bit
    BYTE fRes               : 2;        // [6:7] Reserved
    };

} TAttr;

typedef struct dirent
{
    char d_name[ NAME_MAX+1 ];          // File name
    TAttr Attr;                         // Attributes
    WORD    wTime;                      // Time stamp
    WORD    wDate;                      // Date stamp
    DWORD   dwSize;                     // File size
    
} dirent;


//=============================================================================
// Defines for System calls
//=============================================================================

typedef struct TSysCall                 // System calls
{
    int (*fnSys_Mount)( BYTE bMajor, BYTE bMinor );
    int (*fnSys_OpenDir)( int hDev, char *sDirName );
    dirent * (*fnSys_ReadDir)( int hOpenDir, dirent *pDirEnt );
    
} TSysCall;


#if 0
#define SYS_FCNTL               0
#define SYS_OPENDIR             1
#define SYS_READDIR             2
#define SYS_REWINDDIR           3
#define SYS_CLOSEDIR            4
#define SYS_MKDIR               5
#define SYS_RMDIR               6
#define SYS_RENAME              7
#define SYS_OPEN                8
#define SYS_READ                9
#define SYS_WRITE               10
#define SYS_LSEEK               11
#define SYS_CLOSE               12
#define SYS_FSTAT               13
#define SYS_LINK                14
#define SYS_UNLINK              15
#define SYS_CHMOD               16
#define SYS_CHOWN               17
#define SYS_UTIME               18
#define SYS_SYNC                19
#endif


/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern TDevice Device[MAX_DEVICES];     // Array of device drivers structures

extern int FloppyIOCtl( TIOCtlMsg * IOCtlMsg );

extern struct TSysCall fnDosFS;           // Functions of DOS File System


#endif // _UFS_H_
