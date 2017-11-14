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

        This is a header file for the UNIX file system interface.

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

/******************************************************************************
*                                                                             *
*   Include Files                                                             *
*                                                                             *
******************************************************************************/

// DOS
#include "doscomp.h"                    // Include DOS test


// YAOS
#include "types.h"                      // Include basic types


#define NUM_MOUNT           10          // Max number of mounting nodes
#define MOUNT_ROOT          0           // Root node mount index
#define MAX_PATH            254         // Max path length
#define INV_DEVICE          0xff        // Invalid device number (major/minor)

#define NAME_MAX        32
// This should go into "dirent.h"
//
typedef struct dirent
{
    char    pPath[ MAX_PATH + 1];       // Base path
    char    d_name[ NAME_MAX + 1];      // File's name
    int     count;                      // First time call readdir()
} DIR;


/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

#define MAX_MINOR_FLOPPY    2           // Max number of minor floppy devices
#define FDC_READ            0           // Read operation request
#define FDC_WRITE           1           // Write operation request

#define MINOR0              0           // Minor 0
#define MINOR1              1           // Minor 1

#define FDD_CACHE_SIZE      60000       // Default floppy cache size

// Major device numbers
#define NUM_DEVICES         2           // Number of devices
#define MAJOR_MEM           0           //   Memory device
#define MAJOR_FDD           1           //   Floppy drives


// Data structure defining the device with the major number
typedef struct
{
    void    (*fnOpendir)( BYTE bMinor, struct dirent *pDirEntry );
    void    (*fnReaddir)( BYTE bMinor, struct dirent *pDirEntry );
    int     (*fnOpen)();
    int     (*fnClose)();
    int     (*fnReadWrite)();
} TDevice;



/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern void InitUFS();


#endif // _UFS_H_
