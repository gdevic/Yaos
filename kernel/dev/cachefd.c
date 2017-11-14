/******************************************************************************
*                                                                             *
*   Module:     CacheFS.c                                                     *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       5/28/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This module implements the caching for the floppy device driver.
        Device calls IOCTL_READ_SECTORS / IOCTL_WRITE_SECTORS for the
        floppy driver are implementer here.

        The main IOCtl() function entry for the floppy driver is also here.

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

#include "ufs.h"                        // Include File System defines


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

#define CACHE_LINES             5       // Suggested number of cache lines
#define SECTORS_PER_TRACK       18      // Default sectors per track
#define EMPTY                   -1      // nAge: Cache line not used


typedef struct TCacheLine               // Cache line control block
{
    void  * pTrack;                      // Pointer to a track buffer
    int     nSector;                     // Starting logical sector
    int     nAge;                        // Age number for LRU
    BOOL    fDirty;                      // Cache line had been accessed

} TCacheLine;

typedef struct TCache                   // Cache control structure
{
    DWORD dwFlags;                      // Device cache flags
    WORD wCacheLines;                   // Number of cache lines in an array
    TCacheLine * pCacheLine;            // Points to an array of cache lines

} TCache;

TCache Cache[16];                       // Cache structure for every minor dev
TCache * pCache;                        // Current cache pointer (minor)
int  nSector;                           // Starting logical sector
//int  nTrack;                            // Current track
//int  nHead;                             // Current head
int  nSec;                              // Current sector WITHIN a track
TCacheLine * pLastAccessedLine;         // After Load or Find


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   int InitCacheFD( TIOCtlMsg * pIOCtlMsg )                                  *
*                                                                             *
*******************************************************************************
*
*
*   Where:
*
*   Returns:
*       ESUCCESS
*       EFAIL on a critical failure
*
******************************************************************************/
static int InitCacheFD( TIOCtlMsg * pIOCtlMsg )
{
    static BOOL fInitialized = FALSE;
    TIOCtlMsg FlushMsg;
    TCacheLine *pCacheLine;
    int ret;
    int i;


    if( fInitialized==FALSE )
    {
        // Driver is initialized for the first time

        fInitialized = TRUE;

        // Alocate cache line control array

        pCache->pCacheLine = (TCacheLine *) kmalloc( CACHE_LINES * sizeof(TCacheLine) );

        if( pCache->pCacheLine==NULL )
            return( EMALLOC );

        // Allocate a number of lines - this allocation may fail, but at least 1
        // cache line must be allocated
    
        for( i=0, pCacheLine = pCache->pCacheLine; i<CACHE_LINES; i++, pCacheLine++ )
        {
            // Allocate a cache line
            pCacheLine->pTrack = (void *) kmalloc( SECTOR_SIZE * SECTORS_PER_TRACK );

            // If failed, accept the existing number of lines
            if( pCacheLine->pTrack==NULL )
                break;

            // Reset the LRU pointer to 'not used' and other slots
            pCacheLine->nAge = EMPTY;
            pCacheLine->nSector = EMPTY;
            pCacheLine->fDirty = FALSE;
        }

        if( i==0 )
            return( EMALLOC );
        else
            pCache->wCacheLines = i;
    }
    else
    {
        // If a driver had already been initialized, do an implicit flush
        // and reset its parameters

        FlushMsg.nRequest = IOCTL_FLUSH_CACHE;

        if( (ret=FloppyIOCtl( &FlushMsg ))!=ESUCCESS )
            return( ret );
    }

    // Store the device cache flags

    pCache->dwFlags = (pIOCtlMsg->dwFlags &
        (DEV_NON_CACHED | DEV_WRITE_THROUGH | DEV_WRITE_BACK))
        | DEV_REMOVABLE;


    return( ESUCCESS );
}


/******************************************************************************
*                                                                             *
*   void * FindSector()                                                       *
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
static void * FindSector()
{
    TCacheLine *pLine;
    int i;


    // If the caching is disabled, do not look for a sector

    if( pCache->dwFlags & DEV_NON_CACHED )
        return( NULL );

    pLine = pCache->pCacheLine;

    for( i=0; i<pCache->wCacheLines; i++ )
    {
        if( pLine->nAge >= 0 )
        {
            if( pLine->nSector == nSector )
            {
                // Increment the track usage count

                pLine->nAge++;

                pLastAccessedLine = pLine;

                // Cache line has been found, return the address of the
                // sector that is offset in that line

                return( (void *)((int)pLine->pTrack + nSec * SECTOR_SIZE) );
            }

            pLine += 1;
        }
    }

    return( NULL );
}


/******************************************************************************
*                                                                             *
*   void * LoadSector()                                                       *
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
static void * LoadSector()
{
    TCacheLine *pLine, *pLineLRU;
    TIOCtlMsg IOCtlMsg;
    int nAgeLRU;
    int i;

    pLine = pCache->pCacheLine;
    nAgeLRU = pLine->nAge + 1;

    // Find the Least Recently Used cache line or an empty cache line

    for( i=0; i<pCache->wCacheLines; i++ )
    {
        if( pLine->nAge < nAgeLRU )
        {
            // Found a cache line that is used less than the current one

            nAgeLRU = pLine->nAge;
            pLineLRU = pLine;

            if( pLine->nAge == EMPTY )
                break;
        }

        pLine += 1;
    }

    // Write-through does not need to save a cache line bacause it was saved
    // immediately upon the write

    if( pCache->dwFlags & DEV_WRITE_BACK )
    {
        // Need to save the cache line since it will be reused
        
    }
    
    // Load a track into the cache line

    IOCtlMsg.nData = nSector;
    IOCtlMsg.pBuffer = pLineLRU->pTrack;
    if( FloppyRead( &IOCtlMsg )!=ESUCCESS )
        return( NULL );

    // Set up a cache line

    pLineLRU->nAge = 0;
    pLineLRU->nSector = nSector;

    pLastAccessedLine = pLineLRU;

    return( (void *)((int) pLineLRU->pTrack + nSec * SECTOR_SIZE) );
}


/******************************************************************************
*                                                                             *
*   int SaveSector()                                                          *
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
static int SaveSector()
{
    TIOCtlMsg IOCtlMsg;

    // The last accessed line is stored in the pLastAccessedLine and can be
    // used to save the current track.  The line is marked clean after save.

    IOCtlMsg.nData = nSector;
    IOCtlMsg.pBuffer = pLastAccessedLine->pTrack;
    if( FloppyWrite( &IOCtlMsg )!=ESUCCESS )
        return( EFAIL );

    // Mark just saved cache line as clean

    pLastAccessedLine->fDirty = FALSE;

    return( ESUCCESS );
}


/******************************************************************************
*                                                                             *
*   int FloppyIOCtl( TIOCtlMsg * IOCtlMsg )                                   *
*                                                                             *
*******************************************************************************
*
*   Executes I/O Control command on a floppy device.
*
*   Where:
*       pIOCtlMsg is a pointer to an I/O control message containing
*           bMinor is a minor device number
*           nRequest is I/O request
*           nData is the sector number
*           pbBuffer is a memory buffer where to read in / which to write
*
*   Returns:
*
*
******************************************************************************/
int FloppyIOCtl( TIOCtlMsg * IOCtlMsg )
{
    void * pSector;
    int ret;
    int nCount;


    // Set up the right cache structure

    pCache = &Cache[IOCtlMsg->bMinor];

    // Switch on a message

    switch( IOCtlMsg->nRequest )
    {
        case IOCTL_DEVICE_INIT:

                // Initialize cache structures

                if( (ret=InitCacheFD(IOCtlMsg))!=ESUCCESS)
                    return( ret );

                // Initialize floppy drive

                if( (ret=FloppyInit())!=ESUCCESS )
                    return( ret );

                return( ESUCCESS );

            break;

        case IOCTL_READ_SECTORS:
        {
            for( nCount=1; nCount<=IOCtlMsg->nLen; nCount++)
            {
                // Find the desired track and sector within it

                nSector = (IOCtlMsg->nData + nCount - 1);
                nSec    = nSector % SECTORS_PER_TRACK;
                nSector = (nSector / 18) * 18;

                if( (pSector = FindSector()) != NULL )
                {
                    // The sector had been found, copy it to the client buffer

                    memcpy( (int)IOCtlMsg->pBuffer + (nCount-1)*SECTOR_SIZE,
                            pSector, SECTOR_SIZE );
                }
                else
                {
                    // Sector is not in the cache, so load it

                    pSector = LoadSector();

                    if( pSector != NULL )
                    {
                        // Copy the loaded sector into the client buffer

                        memcpy( (int)IOCtlMsg->pBuffer + (nCount-1)*SECTOR_SIZE,
                                pSector, SECTOR_SIZE );
                    }
                    else
                        return( EFAIL );
                }
            }

            return( ESUCCESS );
        }
        break;

        case IOCTL_WRITE_SECTORS:
        {
            for( nCount=1; nCount<=IOCtlMsg->nLen; nCount++)
            {
                // Find the desired track and sector within it

                nSector = (IOCtlMsg->nData + nCount - 1);
                nSec    = nSector % SECTORS_PER_TRACK;
                nSector = (nSector / 18) * 18;

                if( (pSector = FindSector()) != NULL )
                {
                    // Sector is in the cache, copy the client buffer into it
                    // and mark the line dirty

                    memcpy( pSector, (int)IOCtlMsg->pBuffer + (nCount-1)*SECTOR_SIZE,
                            SECTOR_SIZE );
                    pLastAccessedLine->fDirty = TRUE;
                }
                else
                {
                    // Sector is not in the cache, so load it

                    pSector = LoadSector();

                    if( pSector != NULL )
                    {
                        // Copy the the client buffer into the loaded sector
                        // and mark the line dirty

                        memcpy( pSector, (int)IOCtlMsg->pBuffer + (nCount-1)*SECTOR_SIZE,
                                SECTOR_SIZE );
                        pLastAccessedLine->fDirty = TRUE;
                    }
                    else
                        return( EFAIL );
                }
            }

            // Only if the cache policy is write-back, return and do not
            // proceed into IOCTL_FLUSH_CACHE that will write back all
            // dirty cache lines.

            if( pCache->dwFlags & DEV_WRITE_BACK )
                return( ESUCCESS );
        }
        break;

        case IOCTL_FLUSH_CACHE:

                // Flush all dirty cache lines

                pLastAccessedLine = pCache->pCacheLine;

                for( nCount=0; nCount<pCache->wCacheLines; nCount++ )
                {
                    if( pLastAccessedLine->fDirty )
                        SaveSector();

                    pLastAccessedLine += 1;
                }

                return( ESUCCESS );

            break;

        case IOCTL_READ_TRACK:

                // Read a track, non-cached

                return( FloppyRead( IOCtlMsg ) );

            break;

        case IOCTL_WRITE_TRACK:

                // Write a track, non-cached

                return( FloppyWrite( IOCtlMsg ) );

            break;

        default:
                return( EINV_REQUEST );

            break;
    }

    return( EFAIL );
}


