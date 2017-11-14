/******************************************************************************
*                                                                             *
*   Module:     DosFS.c                                                       *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       5/26/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This module implements DOS file system interface

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
*  5/26/97   1.00  Original                                       Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/
#include <stdio.h>                      // Include standard I/O header file

#include "ufs.h"                        // Include File System defines


/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

static int DOS_Mount( BYTE bMajor, BYTE bMinor );
static int DOS_OpenDir( int hDev, char *sDirName );
static dirent * DOS_ReadDir( int hOpenDir, dirent *pDirEnt );

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

static BYTE TempSectorBuf[SECTOR_SIZE];

//=============================================================================
// Defines for DOS File System
//=============================================================================

// Table of functions that perform the system services

#if 0
struct fnDosFS                          // Functions of DOS File System
{
    DOS_Fcntl,
    DOS_Opendir,
    DOS_Readdir,
    DOS_Rewinddir,
    DOS_Closedir,
    DOS_Mkdir,
    DOS_Rmdir,
    DOS_Rename,
    DOS_Open,
    DOS_Read,
    DOS_Write,
    DOS_LSeek,
    DOS_Close,
    DOS_FStat,
    DOS_Link,
    DOS_Unlink,
    DOS_ChMod,
    DOS_ChOwn,
    DOS_UTime,
    DOS_Sync,

} fnDosFS;

#endif

struct TSysCall fnDosFS =           // Functions of DOS File System
{
    DOS_Mount,
    DOS_OpenDir,
    DOS_ReadDir
};


typedef struct TBoot                 // Dos boot sector
{
    BYTE    sbJump[3];                  // Jump codes E9XX90
    BYTE    sbOEM[8];                   // OEM name and number
    WORD    wBytesPerSector;            // Bytes per sector
    BYTE    bSectorsPerCluster;         // SectorsPerCluster
    WORD    wReservedSectors;           // Reserved sectors
    BYTE    bFat;                       // Number of FATs
    WORD    wRoot;                      // Number of root directory entries
    WORD    wSectors;                   // Number of logical sectors
    BYTE    bMDB;                       // Medium descriptor byte
    WORD    wSectorsPerFAT;             // Number of sectors per FAT
    WORD    wSectorsPerTrack;           // Number of sectors per track
    WORD    wHeads;                     // Number of heads
    WORD    wHidden;                    // Number of hidden sectors

} TBoot;


typedef struct TDosFS                // Dos File System control block
{
    BYTE    bSectorsPerCluster;         // SectorsPerCluster
    WORD    wReservedSectors;           // Reserved sectors
    BYTE    bFAT;                       // Number of FATs
    WORD    wRoot;                      // Number of root directory entries
    WORD    wSectors;                   // Number of logical sectors
    BYTE    bMDB;                       // Medium descriptor byte
    WORD    wSectorsPerFAT;             // Number of sectors per FAT
    WORD    wSectorsPerTrack;           // Number of sectors per track
    WORD    wHeads;                     // Number of heads
    WORD    wHidden;                    // Number of hidden sectors

    WORD    wFATSect;                   // FAT Sector number
    WORD    wRootSect;                  // Root directory sector number

    char   *pFAT;                      // Pointer to a FAT table

} TDosFS;


#define DIRENT_UNUSED       0x00        // Unused directory entry
#define DIRENT_ERASED       0xE5        // Erased entry
#define DIRENT_DOT          0x2E        // '.'

typedef struct TDir                     // Directory entry structure
{
    BYTE    sName[8];                   // File name
    BYTE    sExt[3];                    // File name extension
    TAttr   Attr;                       // File attributes
    BYTE    sRes[10];                   // Reserved by DOS
    WORD    wTime;                      // Time stamp
    WORD    wDate;                      // Date stamp
    WORD    wCluster;                   // First cluster number of a file
    DWORD   dwSize;                     // File size

} TDir;



TDosFS Dos[MAX_DEVICES];                // Dos block structures for devices



typedef struct TOpenDir                 // Open dir structure
{
    TDevice *pDev;                      // Pointer to the device structure
    int nSect;                          // Starting sector of a directory
    int nEntries;                       // Number of directory entries
    int nCurrent;                       // Number of current directory entry

} TOpenDir;

static TOpenDir OpenDir[MAX_OPEN_DIR];

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

int InitDosFS()
{
    int i;

    for( i=0; i<MAX_OPEN_DIR; i++ )
    {
        OpenDir[i].nSect = 0;
    };

    return( ESUCCESS );
}

// hDev is a handle to a device
// returns a handle to an open dir stream

int DOS_OpenDir( int hDev, char *sDirName )
{
    TDosFS * pDos;
    int i;


    pDos = &Dos[hDev];

    // Find the free open directory entry
    for( i=0; i<MAX_OPEN_DIR && OpenDir[i].nSect!=0; i++ );
    if( i==MAX_OPEN_DIR )
         return( EFAIL );

    // Set up the dir structure for the root directory
    // So far only the root directory is supported

    OpenDir[i].pDev = &Device[hDev];
    OpenDir[i].nSect = pDos->wRootSect;
    OpenDir[i].nEntries = pDos->wRoot;
    OpenDir[i].nCurrent = 0;

    return( i );
}


dirent * DOS_ReadDir( int hOpenDir, dirent *pDirEnt )
{
    TOpenDir *pDir;
    TIOCtlMsg Msg;
    TDir * pDirEntry;
    BYTE b;


    // Sanity check

    if( hOpenDir<0 || hOpenDir>=MAX_OPEN_DIR )
        return( NULL );

    pDir = &OpenDir[hOpenDir];

    if( pDir->nSect==0 )
        return( NULL );

NextSector:

    // If the dir entry is the last one, return with NULL

    if( pDir->nEntries == pDir->nCurrent )
        return( NULL );

    // Load a sector that has the current directory

    Msg.nRequest = IOCTL_READ_SECTORS;
    Msg.bMinor = pDir->pDev->bMinor;
    Msg.nData = pDir->nSect + (pDir->nCurrent * 32)/512;
    Msg.nLen = 1;
    Msg.pBuffer = TempSectorBuf;

    // Error loading ?

    if( pDir->pDev->IOCtl( &Msg )!=ESUCCESS)
        return( NULL );

NextEntry:

    // Check that the dir entry is valid

    pDirEntry = (TDir *) (TempSectorBuf + ((pDir->nCurrent * 32) % 512));
    b = pDirEntry->sName[0];

    if( b==DIRENT_UNUSED || b==DIRENT_ERASED )
    {
        pDir->nCurrent++;

        // There can be 16 entries in one sector (512/32b).  Check if we need
        // another sector to load

        if( (pDir->nCurrent & 15)==0 )
            goto NextSector;

        goto NextEntry;
    }

    // Retrieve the name of the entry

    if( pDirEntry->sName[0]==DIRENT_DOT )
    {
        if( pDirEntry->sName[1]==DIRENT_DOT )
        {
            pDirEnt->d_name[1] = '.';
            pDirEnt->d_name[2] = '\0';
        }
        else
            pDirEnt->d_name[1] = '\0';

        pDirEnt->d_name[0] = '.';
    }
    else
    {
        memcpy( &pDirEnt->d_name[0], pDirEntry->sName, NAME_MAX );
    }

    // Copy the rest of the relevant parameters

    pDirEnt->Attr = pDirEntry->Attr;
    pDirEnt->wTime = pDirEntry->wTime;
    pDirEnt->wDate = pDirEntry->wDate;
    pDirEnt->dwSize = pDirEntry->dwSize;


    // Increment the current directory index

    pDir->nCurrent++;


    return( pDirEnt );
}


int DOS_Mount( BYTE bMajor, BYTE bMinor )
{
    TIOCtlMsg Msg;
    TBoot * pBoot;
    TDevice * pDev;
    TDosFS * pDos;
    int i;
    int ret;

    pDev = &Device[ bMajor ];
    pDos = &Dos[ bMajor * 16 + bMinor ];

    // Read the first sector to find the device characteristics regarding the
    // file system that is on it

    Msg.nRequest = IOCTL_READ_SECTORS;
    Msg.bMinor = bMinor;
    Msg.nData = 0;
    Msg.nLen = 1;
    Msg.pBuffer = TempSectorBuf;
    if( (ret=pDev->IOCtl( &Msg ))!=ESUCCESS)
        return( ret );

    pBoot = (TBoot *) TempSectorBuf;

    printf("------- Mounting %s DOS File System -------\n",
        pBoot->bMDB==0xF0? "FAT12":"FAT16");
    printf("%2X %2X %2X Jump signature\n",
        pBoot->sbJump[0], pBoot->sbJump[1], pBoot->sbJump[2] );
    for( i=0; i<8; i++)
        printf("%c", pBoot->sbOEM[i] );
    printf(" OEM Name\n");
    printf("%d\t Bytes per sector\n", pBoot->wBytesPerSector );
    printf("%d\t Sectors per cluster\n", pBoot->bSectorsPerCluster );
    printf("%d\t Reserved sectors\n", pBoot->wReservedSectors );
    printf("%d\t Number of FATs\n", pBoot->bFat );
    printf("%d\t Root directory entries\n", pBoot->wRoot );
    printf("%d\t Logical sectors\n", pBoot->wSectors );
    printf("%2Xh\t Media descriptor byte\n", pBoot->bMDB );
    printf("%d\t Sectors per FAT\n", pBoot->wSectorsPerFAT );
    printf("%d\t Sectors per track\n", pBoot->wSectorsPerTrack );
    printf("%d\t Number of heads\n", pBoot->wHeads );
    printf("%d\t Number of hidden sectors\n", pBoot->wHidden );


    // Copy relevant fields to the DOS File System control block
    // for the specified device number

    pDos->bSectorsPerCluster = pBoot->bSectorsPerCluster;
    pDos->wReservedSectors   = pBoot->wReservedSectors;
    pDos->bFAT               = pBoot->bFat;
    pDos->wRoot              = pBoot->wRoot;
    pDos->wSectors           = pBoot->wSectors;
    pDos->bMDB               = pBoot->bMDB;
    pDos->wSectorsPerFAT     = pBoot->wSectorsPerFAT;
    pDos->wSectorsPerTrack   = pBoot->wSectorsPerTrack;
    pDos->wHeads             = pBoot->wHeads;
    pDos->wHidden            = pBoot->wHidden;

    // Calculate some of more useful sectors

    pDos->wFATSect   = pDos->wHidden +
                       pDos->wReservedSectors;

    pDos->wRootSect  = pDos->wHidden +
                       pDos->wReservedSectors +
                       pDos->wSectorsPerFAT * pDos->bFAT;



    // Allocate memory for a copy of the FAT table and load a FAT table into it

    pDos->pFAT = (BYTE *) kmalloc( pDos->wSectorsPerFAT * SECTOR_SIZE );

    if( pDos->pFAT != NULL )
    {
        Msg.nRequest = IOCTL_READ_SECTORS;
        Msg.bMinor = bMinor;
        Msg.nData = pDos->wFATSect;
        Msg.nLen = pDos->wSectorsPerFAT;
        Msg.pBuffer = pDos->pFAT;

        if( (ret=pDev->IOCtl( &Msg )) != ESUCCESS)
        {
            // Could not read FAT table
            kfree( pDos->pFAT );
            return( ret );
        }
    }
    else
        // FAT table allocation failed
        return( EMALLOC );

    return( bMajor * 16 + bMinor );
}
