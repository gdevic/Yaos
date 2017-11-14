/******************************************************************************
*                                                                             *
*   Module:     Ufs.c                                                         *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       5/28/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        UFS - Unified File System

        This file implements the Unified File System for Yaos.  Different
        file systems can be mounted beneath it.

        The file system provides following functionality:

        OPENDIR     - Open directory stream
        READDIR     - Read a directory entry
        CLOSEDIR    - Close directory stream
        OPEN
        READ
        WRITE
        LSEEK
        CLOSE
        SYNC        - Flush all caches

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
*  5/28/97   1.00  Original                                       Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/
#include <stdio.h>                      // Include standard I/O header file
#include <stdlib.h>                     // Include standard library header

#include "ufs.h"                        // Include File System defines


TDevice Device[MAX_DEVICES];            // Array of device drivers structures

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

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   int InitUFS()                                                             *
*                                                                             *
*******************************************************************************
*
*   Initializes UFS
*
*   Where:
*
*
*   Returns:
*       ESUCCESS
*       EFAIL on a critical failure
*
******************************************************************************/
int InitUFS()
{
    return( ESUCCESS );
}


#ifdef TEST

int InvalidIOCtl( TIOCtlMsg * pIOCtlMsg )
{
    printk("Invalid I/O Control function called!\n");
    printk("  IOCtlMsg.dwFlags  = %08Xh\n", pIOCtlMsg->dwFlags );
    printk("  IOCtlMsg.nRequest = %d\n", pIOCtlMsg->nRequest );
    printk("  IOCtlMsg.nData    = %d\n", pIOCtlMsg->nData );
    printk("  IOCtlMsg.nLen     = %d\n", pIOCtlMsg->nLen );
    printk("  IOCtlMsg.pBuffer  = %08Xh\n", pIOCtlMsg->pBuffer );

    return( EFAIL );
}

int main()
{
    TIOCtlMsg IOCtlMsg;
    int ret;
    int i;

    // This is to be done in kernel.c on init

    // Reset the device array

    for( i=0; i<MAX_DEVICES; i++)
    {
        Device[i].IOCtl = InvalidIOCtl;
    }

    // Init default floppy device driver in the floppy slot

    Device[DEVICE_FLOPPY].IOCtl = FloppyIOCtl;

    // Initialize the floppy device

    IOCtlMsg.nRequest = IOCTL_DEVICE_INIT;
    IOCtlMsg.dwFlags  = DEV_WRITE_THROUGH;
    IOCtlMsg.bMinor   = 0;

    if( (ret=Device[DEVICE_FLOPPY].IOCtl(&IOCtlMsg))!=ESUCCESS )
        return( ret );

    Device[DEVICE_FLOPPY].bMajor = DEVICE_FLOPPY;
    Device[DEVICE_FLOPPY].bMinor = 0;
    Device[DEVICE_FLOPPY].hDev   = DEVICE_FLOPPY * 16 + 0;

    // Initialize DOS File System interface

    if( (ret=InitDosFS()) != ESUCCESS )
        return( ret );

    Device[DEVICE_FLOPPY].pFS = &fnDosFS;    // We know that there's DOS !!!

    // Mount DOS File System on a floppy device

    if( (ret=Device[DEVICE_FLOPPY].pFS->fnSys_Mount(DEVICE_FLOPPY, 0))!=ESUCCESS )
        return( ret );

    // Assgn a file system handle to a device

    Device[DEVICE_FLOPPY].hFS = ret;

#if 0
    if( InitUFS() != ESUCCESS )
    {
        printf("Critical error:\n");
        printf("File system cannot be initialized.\n");
    }
#endif

#if 1
    {
        dirent d, *pd;
        int hnd;
        char sName[ NAME_MAX + 1 ];


        printf("Directory:\n");
        printf(" Name       | Size  | Attributes\n");
        
        hnd = Device[DEVICE_FLOPPY].pFS->fnSys_OpenDir( Device[DEVICE_FLOPPY].hDev,"" );

        do
        {
            pd = Device[DEVICE_FLOPPY].pFS->fnSys_ReadDir( hnd, &d );

            if( pd != NULL )
            {
                memcpy( sName, d.d_name, NAME_MAX );
                sName[NAME_MAX] = '\0';

                printf("%s", sName );
                printf(" %6d ", d.dwSize );
                printf(" %s%s%s%s%s%s\n",
                    d.Attr.fReadOnly ?  "r":" ",
                    d.Attr.fHidden ?    "h":" ",
                    d.Attr.fSystem ?    "s":" ",
                    d.Attr.fVolume ?    "v":" ",
                    d.Attr.fSubdir ?    " Subdirectory ":" ",
                    d.Attr.fArchive ?   "a":" " );               
            }
            
        } while( pd != NULL );
    }
#endif
    

    CloseFloppy();

    return( 0 );
}

#endif
