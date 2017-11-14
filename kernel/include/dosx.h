/******************************************************************************
*                                                                             *
*   Module:     DosX.h                                                        *
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

        This is a header file for the DOS extender functionality.
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
*   Important Defines                                                         *
******************************************************************************/
#ifndef _DOSX_H_
#define _DOSX_H_


/******************************************************************************
*                                                                             *
*   Include Files                                                             *
*                                                                             *
******************************************************************************/

#include "types.h"                      // Include basic data types

#include "kernel.h"                     // Define register structure

#include "v86.h"                        // Include virtual mode header file

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

// File attribute bits

#define DOS_READONLY            0x01    // Read only file
#define DOS_HIDDEN              0x02    // Hidden file
#define DOS_SYSTEM              0x04    // System file
#define DOS_VOLUME              0x08    // Volume label
#define DOS_DIR                 0x10    // Directory entry
#define DOS_ARCHIVE             0x40    // Archive bit


// Control functions for DOS_ioctl (`ioctl' argument)

#define DOSIOCTL_GETDEVINFO             0x00    // Get device information
#define DOSIOCTL_SETDEVINFO             0x01    // Set device information
#define DOSIOCTL_READCH                 0x02    // Read from char dev control channel
#define DOSIOCTL_WRITECH                0x03    // Write to char dev control channel
#define DOSIOCTL_READBL                 0x04    // Read from block dev control channel
#define DOSIOCTL_WRITEBL                0x05    // Write to block dev control channel
#define DOSIOCTL_GETINPUTSTAT           0x06    // Get input status
#define DOSIOCTL_GETOUTPUTSTAT          0x07    // Get output status
#define DOSIOCTL_ISREMOVABLE            0x08    // Is block device removable
#define DOSIOCTL_ISREMOTE               0x09    // Is block device remote
#define DOSIOCTL_ISREMOTEHND            0x0A    // Is handle remote
#define DOSIOCTL_SETRETRY               0x0B    // Set sharing retry count
#define DOSIOCTL_CHARRQ                 0x0C    // Generic char device request
#define DOSIOCTL_BLOCKRQ                0x0D    // Generic block device request
#define DOSIOCTL_GETLOCALDRVMAP         0x0E    // Get local drive map
#define DOSIOCTL_SETLOCALDRVMAP         0x0F    // Set local drive map


//----------------------------------------------------------------------------
// Define Disk Transfer Area used by DOS_findfirst() and DOS_findnext()
//----------------------------------------------------------------------------

struct DOS_DTA
{
    BYTE    bDrive;                     // Drive number 0=A,1=B,...
    char    sSearchTpl[11];             // Search template with wildcards
    BYTE    bSearchAttr;                // Search matching attribute
    WORD    wEntryNumber;               // Enumerated entry number
    BYTE    Reserved[7];                // Reserved fields
    WORD    wTime;                      // File creation time
    WORD    wDate;                      // File creation date
    DWORD   dwSize;                     // File size
    char    sName[13];                  // File name
};

#define DTA_PSPOFFSET           0x80    // Offset of DTA from the PSP
#define DTA_LEN                 0x80    // Length of DTA in bytes

/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern void Init_DosX();

extern int    DOS_GetCurDrive();
extern int    DOS_mkdir( char *sPathname );
extern int    DOS_rmdir( char *sPathname );
extern int    DOS_chdir( char *sPathname );
extern int    DOS_create( char *sPathname, int access );
extern int    DOS_open( char *sPathname, int access );
extern int    DOS_close( int handle );
extern int    DOS_read( int handle, int len, char *pDest );
extern int    DOS_write( int handle, int len, char *pSrc );
extern int    DOS_unlink( char *sPathname );
extern int    DOS_lseek( int handle, int offset, int whence );
extern int    DOS_getattrib( char *sPathname );
extern int    DOS_chmod( char *sPathname, int attrib );
extern int    DOS_ioctl( int ioctl, TReg *Reg );
extern int    DOS_dup( int handle );
extern int    DOS_dup2( int handle, int handle2 );
extern char * DOS_getcwd();
extern int    DOS_alloc( int nSize );
extern int    DOS_free( int handle );
extern int    DOS_findfirst( char *sPathname, int mask, struct DOS_DTA *DTA );
extern int    DOS_findnext( struct DOS_DTA *DTA );
extern void   DOS_setPSP( int newPSP );
extern int    DOS_getPSP();
extern void   DOS_childPSP( int parentPSP );
extern int    DOS_rename( char *sNewname, char *sOldname );
extern int    DOS_flush( int handle );


#endif //  _DOSX_H_
