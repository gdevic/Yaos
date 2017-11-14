/******************************************************************************
*                                                                             *
*   Module:     dosfs.c                                                       *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       09/17/96                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

          This module provides DOS 4.0 FAT file system interface.
                  

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 09/17/96   1.00  Original                                       Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/
#define TEST    1

#include "doscomp.h"                    // Include DOS test compatibility
#include "dosfs.h"                      // Include DOS File System header
#include "ufs.h"

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

// Forward declarations:
static void fnOpendir( BYTE bMinor, struct dirent *pDirEntry );
static void fnReaddir( BYTE bMinor, struct dirent *pDirEntry );
static void fnOpen();
static void fnClose();
static void fnReadWrite();


TDevice DevDOS =
{
    fnOpendir,
    fnReaddir,
    fnOpen,
    fnClose,
    fnReadWrite
};


/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

#define IS_LONG_NAME(pDir)    (*(BYTE *)&pDir->Attrib == 0x0f)


// Dos attribute bitfields
typedef struct
{
    BYTE    fReadOnly:1;                // Read only bit
    BYTE    fHidden:1;                  // Hidden file bit
    BYTE    fSystem:1;                  // System file bit
    BYTE    fLabel:1;                   // Volume label entry bit
    BYTE    fDirectory:1;               // Subdirectory bit
    BYTE    fArchive:1;                 // Archive bit
    BYTE    Res:2;                      // Reserved
} TAttrib;


// Dos directory entry, 32 bytes long
typedef struct
{
    char    cName[8];                   // Name field
    char    cExt[3];                    // Extension field
    TAttrib Attrib;                     // Attribute byte
    BYTE    bRes[10];                   // 10 Reserved bytes
    WORD    wTime;                      // Time stamp
    WORD    wDate;                      // Date stamp
    WORD    wCluster;                   // File's first cluster number
    DWORD   dwSize;                     // File size
} TDosEntry;


// DOS Boot sector structure
typedef struct
{
    BYTE    bJmp[3];                    // Jump instruction (E9/EB xx xx)
    char    cOEM[8];                    // OEM Name and Number
    WORD    wSectorLen;                 // Bytes per sector (512)
    BYTE    bCluster;                   // Sectors per cluster (1)
    WORD    wReserved;                  // Number of reserved sectors (1)
    BYTE    bFats;                      // Number of FATs (2)
    WORD    wRootEntries;               // Number of root dir entries (224)
    WORD    wSectors;                   // Number of sectors on meduim (2880)
    BYTE    bMedium;                    // Media descriptor byte (fdd: F0)
    WORD    wSectorsPerFat;             // Number of sectors per FAT (9)
    WORD    wSectorsPerTrack;           // Number of sectors per track (18)
    WORD    wHeads;                     // Number of heads (2)
    WORD    wHidden;                    // Number of hidden sectors (0)
    WORD    wRes1;                      // Reserved
    DWORD   dwTotalSectors;             // If wSectors=0, total num of sectors
    BYTE    bDrive;                     // Physical drive number (0)
    BYTE    bRes2;                      // Reserved
    BYTE    bSignature;                 // Extended signature byte (29h)
    DWORD   dwVolume;                   // Volume serial number
    char    cVolume[11];                // Volume label    

    WORD    wRootSector;                // Root sector number
    BYTE    *pFAT;                      // FAT area
    TDosEntry *pRoot;                   // Root directory area
    BYTE    fReady;                     // Device is ready flag
} TBootSector;


// Directory/File structure
typedef struct
{
    char sName[256];                    // Name
} TFsEntry;


// Minor devices boot sector data structure
TBootSector BootSector[ MAX_MINOR_FLOPPY ];


// Working sector
static BYTE Sector[ 512 ];

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   TFsEntry * MkFsEntry( TFsEntry *pFs, TDosEntry *pDir )                    *
*                                                                             *
*******************************************************************************
*
*   
*
*   Where:
*       
*       
*   Returns:
*       
*       
******************************************************************************/
TFsEntry * MkFsEntry( TFsEntry *pFs, TDosEntry *pDir )
{
    int dest, src;

    // Copy the name
    dest = 0;
    for( src=0; src<8; src++)
        if( pDir->cName[src] != ' ' )
            pFs->sName[dest++] = pDir->cName[src];
        else
            break;

    if( strncmp(pDir->cExt, "   ", 3) != 0 )
    {
        pFs->sName[dest++] = '.';
        for( src=0; src<3; src++)
            if( pDir->cExt[src] != ' ' )
                pFs->sName[dest++] = pDir->cExt[src];
            else
                break;
    }

    pFs->sName[dest] = (char) 0;

    return( pFs );
}




/******************************************************************************
*                                                                             *
*   TracePath                                                                 *
*                                                                             *
*******************************************************************************
*
*   
*
*   Where:
*       
*       
*   Returns:
*       0 if the path is invalid
*       1 if the path is valid
*       
******************************************************************************/
static int TracePath( TFsEntry *pFsEntry, BYTE bMinor, char *pPath )
{
    int i;
    TBootSector *pBootSector;
    TDosEntry *pDosEntry;

    pBootSector = &BootSector[bMinor];
    pDosEntry = pBootSector->pRoot;

    // Search the root directory first
    for( i=0; i<pBootSector->wRootEntries; i++ )
    {
        // Make a Fs structure
        MkFsEntry( pFsEntry, pDosEntry );

        // Compare the name to the our given path suffix
        if( !strncmp( pFsEntry->sName, pPath, strlen(pFsEntry->sName) ))
        {
            // Found the directory!!!


            return( 1 );
        }

        // Next entry in a root directory
        pDosEntry++;
    }

    // The path was invalid
    return( 0 );
}


/******************************************************************************
*                                                                             *
*   void fnOpendir( BYTE bMinor, struct dirent *pDirEntry )                   *
*                                                                             *
*******************************************************************************
*
*   A opendir() system call at the device level.  The path stored in the 
*   pPath field is examined and read is set up.
*
*   Where:
*       pDir->pPath is the path to be examined
*       
*   Returns:
*       
*       
******************************************************************************/
static void fnOpendir( BYTE bMinor, struct dirent *pDirEntry )
{
    TFsEntry DirNode;

    if( TracePath( &DirNode, bMinor, pDirEntry->pPath ) )
    {
        // Path was valid
        pDirEntry->count = 0;

        return;
    }

    // Path was invalid
    pDirEntry->count = -1;
}


static void fnReaddir( BYTE bMinor, struct dirent *pDirEntry )
{
    TFsEntry DirNode;

    if( TracePath( &DirNode, bMinor, pDirEntry->pPath ) )
    {
        // Path was valid
        

        return;
    }

    // Path was invalid
    pDirEntry->count = -1;
}

static void fnOpen()
{
    
}

static void fnClose()
{
    
}

static void fnReadWrite()
{
    
}



/******************************************************************************
*   TEST
******************************************************************************/

void InitDOSFS()
{
    int i;
    char sStr[20];
    TBootSector *pBootSector = (TBootSector *) Sector;

    // Register major floppy device
    RegisterDevice( MAJOR_FDD, &DevDOS );

    // Initialize floppy driver
    printf("Init fdd\n");
    FDDInit();

    // Initialize cache
    printf("Init cache\n");
    FDD_CacheInit( FDD_CACHE_SIZE );

    // Initialize each minor device
    //
    for( i=0; i<MAX_MINOR_FLOPPY; i++ )
    {
        printf("Init minor device %d\n", i );

        if( FDD_LoadSectors( Sector, i, BOOT_SECT, 1 ) != OK )
        {
            // Minor device not ready
            printf("Minor device %d not ready\n", i );
            BootSector[i].fReady = 0;

            continue;
        }

        // Copy the configuration into the drive boot structure
        memcpy( &BootSector[i], &Sector[0], sizeof(TBootSector) );

        BootSector[i].wRootSector = pBootSector->wReserved + pBootSector->bFats * pBootSector->wSectorsPerFat;

        // Allocate memory for the FAT area
        BootSector[i].pFAT = (BYTE *)kmalloc( pBootSector->wSectorsPerFat * 512);
        if( BootSector[i].pFAT==NULL )
            KernelPanic("Unable to kmalloc FAT\n");

        // Load a copy of the FAT table
        FDD_LoadSectors( BootSector[i].pFAT, i, pBootSector->wReserved, pBootSector->wSectorsPerFat );

        // Allocate memory for the root image
        BootSector[i].pRoot = (BYTE *)kmalloc( pBootSector->wSectorsPerFat * 512);
        if( BootSector[i].pFAT==NULL )
            KernelPanic("Unable to kmalloc FAT\n");

        // Load a copy of the FAT table
        FDD_LoadSectors( BootSector[i].pFAT, i, pBootSector->wReserved, pBootSector->wSectorsPerFat );

        // Make a device ready
        BootSector[i].fReady = 1;

        memcpy(sStr, pBootSector->cVolume, 11 );  sStr[11] = 0;
        printf("Volume: %s\n", sStr);
#if 1
        printf("%8d  Bytes per sector (512)\n",                  pBootSector->wSectorLen);
        printf("%8d  Sectors per cluster (1)\n",                 pBootSector->bCluster);
        printf("%8d  Number of reserved sectors (1)\n",          pBootSector->wReserved);
        printf("%8d  Number of FATs (2)\n",                      pBootSector->bFats);
        printf("%8d  Number of root dir entries (224)\n",        pBootSector->wRootEntries);
        printf("%8d  Number of sectors on meduim (2880)\n",      pBootSector->wSectors);
        printf("%8x  Media descriptor byte (fdd: F0)\n",         pBootSector->bMedium);
        printf("%8d  Number of sectors per FAT (9)\n",           pBootSector->wSectorsPerFat);
        printf("%8d  Number of sectors per track (18)\n",        pBootSector->wSectorsPerTrack);
        printf("%8d  Number of heads (2)\n",                     pBootSector->wHeads);
        printf("%8d  Number of hidden sectors (0)\n",            pBootSector->wHidden);
        printf("%8d  If wSectors=0, total num of sectors\n",     pBootSector->dwTotalSectors);
        printf("%8d  Physical drive number (0)\n",               pBootSector->bDrive);
        printf("%8x  Extended signature byte (29h)\n",           pBootSector->bSignature);
        printf("%8X  Volume serial number\n",                    pBootSector->dwVolume);
        memcpy(sStr, pBootSector->cOEM, 8 );  sStr[8] = 0;
        printf("OEM string: %s\n", sStr);
#endif
    }
}

