/******************************************************************************
*                                                                             *
*   Module:     ufs.c                                                         *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       09/17/96                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        UNIX File System Interface code.

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
// DOS

#include "doscomp.h"                    // Include DOS test compatibility

// YAOS
#include "ufs.h"                        // Include its own header file


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

// Mounting node structure
typedef struct
{
    char    pPath[ MAX_PATH + 1];       // Mounting path

    // This describes the device that the path is mounted on
    BYTE    bHostMajor;                 // Major of the host device
    BYTE    bHostMinor;                 // Minor of the host device

    // This describes the device that is being mounted atop
    BYTE    bMajor;                     // Major device number
    BYTE    bMinor;                     // Minor of the device
} TMount;


//// Data structure defining an open file
//typedef struct
//{
//    char *sPathName;                    // File path/name
//    DWORD dwRWHead;                     // Read/Write position
//} TFile;


static TMount Mount[ NUM_MOUNT ];       // Mounting nodes array
static TDevice *pDev[ NUM_DEVICES ];    // Device list pointer

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

void RegisterDevice( BYTE bMajorNumber, TDevice * pDevReg )
{
    printf("Registering major: %d\n", bMajorNumber );

    pDev[ bMajorNumber ] = pDevReg;
}


/******************************************************************************
*                                                                             *
*   TMount * DevicePath( char * pPath )                                       *
*                                                                             *
*******************************************************************************
*
*   Returns the pointer to a mount structure that holds the device minor:major
*   for the specified path.  The path may be modified (shortened) so that
*   it can be sent to that device.
*
*   Where:
*       pPath is pointer to a path that can be modified
*       
*   Returns:
*       Pointer to a mounting structure that handles the path
*       
******************************************************************************/
TMount * DevicePath( char *pPath )
{
    int i, len;
    BYTE bCurMajor, bCurMinor;
    TMount *pMount;

    // Start with the default root device
    bCurMajor = Mount[ MOUNT_ROOT ].bHostMajor;
    bCurMinor = Mount[ MOUNT_ROOT ].bHostMinor;
    pMount = &Mount[0];

    // Find if any mounted path match with the prefix of our path and is
    // expanded on our current device number
    for( i=0; i<NUM_MOUNT; i++ )
    {
        if( Mount[i].pPath[0] != 0 &&
            Mount[i].bHostMajor == bCurMajor &&
            Mount[i].bHostMinor == bCurMinor )
        {
            len = strlen(Mount[i].pPath);
            if( !strnicmp(Mount[i].pPath, pPath, len) )
            {
                // Found a device with a matching prefix of the path
                // shorten the path and go again
                // For any node other than root, ignore possible '/' prefix
                if( (Mount[i].pPath[0] != '/') && (pPath[len]=='/') ) len++;
                memmove( pPath, pPath + len, MAX_PATH - len );
                bCurMajor = Mount[i].bMajor;
                bCurMinor = Mount[i].bMinor;
                pMount = &Mount[i];
                i = 0;
            }
        }
    }

    return( pMount );
}


/******************************************************************************
*                                                                             *
*   void Sys_mount( int bMajor, int bMinor, char *pPath )                     *
*                                                                             *
*******************************************************************************
*
*   System call to mount a device on the top of another 
*
*   Where:
*       bMajor is the major number of the device to be mounted
*       bMinot is the minor number of the device to be mounted
*       pPath is the path to be mounted atop
*       
******************************************************************************/
void Sys_mount( int bMajor, int bMinor, char *pPath )
{
    int i;
    char pLocal[ MAX_PATH + 1];
    TMount *pMount;

    printf("Mounting major %d minor %d on `%s`\n", bMajor, bMinor, pPath );

    // Check if major device is ok
    if( bMajor<0 || bMajor>=NUM_DEVICES )
        return;                         // INVALID MAJOR DEVICE

    // Check if major device is registered
    if( pDev[bMajor] == NULL )
        return;                         // INVALID MAJOR DEVICE

    // Copy the path locally
    memcpy( pLocal, pPath, MAX_PATH );
    pLocal[ MAX_PATH ] = 0;

    // Find the mounting device for the given path
    pMount = DevicePath( pLocal );

    for( i=0; i<NUM_MOUNT; i++ )
    {
        if( Mount[i].pPath[0] == 0 )
        {
            // Found an empty mounting node - mount the path
            strcpy( Mount[i].pPath, pLocal );
            Mount[i].bHostMajor = pMount->bMajor;
            Mount[i].bHostMinor = pMount->bMinor;
            Mount[i].bMajor = bMajor;
            Mount[i].bMinor = bMinor;

            return;                     // OK
        }
    }

    return;                             // CANNOT FIND EMPTY MOUNTING INDEX
}


/******************************************************************************
*                                                                             *
*   void Sys_umount( char *pPath )                                            *
*                                                                             *
*******************************************************************************
*
*   System call to unmount a path
*
*   Where:
*       pPath is the path to be unmounted
*       
******************************************************************************/
void Sys_umount( char *pPath )
{
    char pLocal[ MAX_PATH + 1];
    TMount *pMount;

    printf("Unmounting `%s`\n", pPath );

    // Copy the path locally
    memcpy( pLocal, pPath, MAX_PATH );
    pLocal[ MAX_PATH ] = 0;

    // Find the mounting device for the given path
    pMount = DevicePath( pLocal );

    // The remainder of the path has to be empty
    if( pLocal[0] != 0 )
        return;                         // INVALID UNMOUNT PATH

    pMount->pPath[0] = 0;
    pMount->bHostMajor = INV_DEVICE;
    pMount->bHostMinor = INV_DEVICE;
    pMount->bMajor = INV_DEVICE;
    pMount->bMinor = INV_DEVICE;
}



/******************************************************************************
*                                                                             *
*   void Sys_opendir( DIR * pDir, char * pPath )                              *
*                                                                             *
*******************************************************************************
*
*   System call for opendir()
*
*   Where:
*       pDir is the DIR structure defined in "dirent.h"
*       pPath is the path to be searched on successive readdir() system call
*       
*   Returns:
*       sets up the DIR structure
*       
******************************************************************************/
void Sys_opendir( DIR * pDir, char * pPath )
{
    TMount *pMount;

    // Copy the path to the directory structure
    memcpy( pDir->pPath, pPath, MAX_PATH );
    pDir->pPath[ MAX_PATH ] = 0;

    // Find the device responsible for the directory path
    pMount = DevicePath( pDir->pPath );

    // Call the opendir() device service to find the path
    if( pDev[ pMount->bMajor ] != NULL )
        (pDev[ pMount->bMajor ]->fnOpendir)( pMount->bMinor, pDir );
}


/******************************************************************************
*                                                                             *
*   void Sys_readdir( struct dirent *pDirEntry )                              *
*                                                                             *
*******************************************************************************
*
*   Returns the successive directory entry using the DIR structure
*
*   Where:
*       pDirEntry is the directory structure set up by a opendir() call
*       
*   Returns:
*       
*       
******************************************************************************/
void Sys_readdir( struct dirent *pDirEntry )
{
    TMount *pMount;

    // Find the device responsible for the directory path
    pMount = DevicePath( pDirEntry->pPath );

    // Call the readdir() device service to retrieve the name
    if( pDev[ pMount->bMajor ] != NULL )
        (pDev[ pMount->bMajor ]->fnReaddir)( pMount->bMinor, pDirEntry );
}


/******************************************************************************
*                                                                             *
*   void InitUFS()                                                            *
*                                                                             *
*******************************************************************************
*
*   Initialize UNIX File System
*
*   Where:
*       
*       
*   Returns:
*       
*       
******************************************************************************/
void InitUFS()
{
    // Clear the mounting node array with NULL
    kmemset( Mount, 0, sizeof(TMount) * NUM_MOUNT );

    // Clear device list pointers
    kmemset( pDev, 0, sizeof(pDev) * NUM_DEVICES );

    InitDOSFS();

    // Mount floppy device as the root by default
    strcpy( Mount[ MOUNT_ROOT ].pPath, "/" );
    Mount[ MOUNT_ROOT ].bHostMajor = MAJOR_FDD;
    Mount[ MOUNT_ROOT ].bHostMinor = MINOR0;
    Mount[ MOUNT_ROOT ].bMajor = MAJOR_FDD;
    Mount[ MOUNT_ROOT ].bMinor = MINOR0;
}



/******************************************************************************
*   The main DOS test driver                                                  *
******************************************************************************/
main()
{
    #define NUM_TOKEN       5
    char sCmd[80];
    char *sTok[ NUM_TOKEN ];
    int i, isinit = 0;
        
    setbuf( stdout, 0 );
    printf("\nUFS DOS-based Test Interface\n");

    do
    {
        printf(">");
        gets( sCmd );

        // Tokenize the input request
        for( i=0; i<NUM_TOKEN; i++)
            sTok[i] = (char *)strtok( i? NULL : sCmd, " ");

        if( !stricmp(sTok[0], "help") )
        {
            // Prints the implemented commands
            printf("dir init mount mt opendir umount q[uit]\n");
        }else

        if( !stricmp(sTok[0], "init") )
        {
            // Initialize fs
            InitUFS();
            isinit = 1;
        }else

        if( !stricmp(sTok[0], "dir") )
        {
            DIR Dir;
            struct dirent *pDirEntry;

            // directory list
            printf("Directory list of `%s`\n", sTok[1] );
            Sys_opendir( &Dir, sTok[1] );
            do
            {
                Sys_readdir( &Dir );
                printf(" `%s`\n", Dir.d_name );
            }
            while( Dir.d_name[0] != 0 );
        }else

        if( !stricmp(sTok[0], "mount") )
        {
            // mount a device
            sscanf(sTok[1],"%d", &i );
            Sys_mount( MAJOR_FDD, i, sTok[2] );
        }else

        if( !stricmp(sTok[0], "mt") )
        {
            // print mount table
            for( i=0; i<NUM_MOUNT; i++)
                if( Mount[i].pPath[0] != 0 )
                {
                    printf("Mount %d: at `%s`\n", i, Mount[i].pPath );
                    printf("    Host: %d/%d\n", Mount[i].bHostMajor, Mount[i].bHostMinor );
                    printf("  Target: %d/%d\n", Mount[i].bMajor, Mount[i].bMinor );
                }
        }else

        if( !stricmp(sTok[0], "opendir") )
        {
            // opendir system call
            //
            // Get the handle of the directory stream
//            Sys_opendir( &dirslen, sTok[1] );
//            printf("opendir for `%s` returned %d bytes\n", sTok[1], dirslen);
        }else

        if( !stricmp(sTok[0], "umount") )
        {
            // unmount a path
            Sys_umount( sTok[1] );
        }else

        if( !stricmp(sTok[0], "q") )
        {
            // Quits the program
            if( isinit )
            {
                FDC_Close();
                FDD_CacheClose();
            }

            return( 0 );
        }
    }
    while(1);
}


void KernelPanic( char *str, ... )
{
    printf("KERNEL PANIC: %s\n", str );

    // Release interrupt handler
    FDC_Close();
    FDD_CacheClose();

    exit(1);
}
