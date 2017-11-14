/******************************************************************************
*                                                                             *
*   Module:     cache.c                                                       *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       09/17/96                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This module implements the file system buffer cache.  The services
        that it provides are sector read and write and block read and write.

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
#include <stdio.h>                      // Include standard I/O header file

#include "doscomp.h"                    // Include DOS test compatibility
#include "ufs.h"
#include "dosfs.h"                      // Include DOS File System header
#include "fdd.h"                        // Include floppy driver header


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

//#define TARGET 0xA0000
#define TARGET_LEN  (9*512*2)
#define TARGET (640*1024 - TARGET_LEN)
void *pTargetCache;


// Track buffering information
typedef struct
{
    BYTE    bLRU;                       // Usage counter
    BYTE    bMinor;                     // Minor device number
    BYTE    bTrack;                     // Track number
    BYTE    *pTrack;                    // Pointer to memory
} TBuffer;

static DWORD dwNumBuffers;              // Number of buffers
static TBuffer *aBuf;                   // Track buffer


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   void FDD_CacheInit( DWORD dwBuffers )                                     *
*                                                                             *
*******************************************************************************
*
*   Initializes floppy cache management and allocates up to dwBuffers bytes
*   for the buffer cache.
*
*   Where:
*       dwBuffers is the maximum length of the buffer cache
*
*   NOTE: Two minor device may have different track size.  This is a known
*         bug.
*       
******************************************************************************/
void FDD_CacheInit( DWORD dwBuffers )
{
    int i;

    // Calculate number of buffers for minor 
    dwNumBuffers = dwBuffers / FDC_GetTrackLen( MINOR0 );

    if( dwNumBuffers == 0 )
        KernelPanic("Insufficient buffer size\n");

    aBuf = (TBuffer *)kmalloc( dwNumBuffers * FDC_GetTrackLen( MINOR0 ) );
    if( aBuf==NULL )
        KernelPanic("Cannot allocate fdd cache buffer\n");

    printf("Allocated %d buffers at %08X\n", dwNumBuffers, aBuf );

    // Allocate and clear the buffer cache
    for( i=0; i<dwNumBuffers; i++ )
    {
        aBuf[i].bLRU = 0;               // Not used
        aBuf[i].pTrack = (BYTE *)kmalloc( FDC_GetTrackLen( 0 ) );
        if( aBuf[i].pTrack==NULL )
            KernelPanic("Unable to allocate buffer %d\n", i );
    }

#if 1
    if( (pTargetCache=kmalloc(TARGET_LEN))==NULL )
        KernelPanic("Unable to allocate target cache\n");

    memcpy( pTargetCache, TARGET, TARGET_LEN );
#endif
}


FDD_CacheClose()
{
    memcpy( TARGET, pTargetCache, TARGET_LEN );
}

/******************************************************************************
*                                                                             *
*   BYTE *FDD_LoadSector                                                      *
*       ( BYTE *pSector, BYTE bMinor, DWORD dwSector, DWORD dwCount )         *
*                                                                             *
*******************************************************************************
*
*   Copies logical sectors to the given location.  The sectors may or may
*   not be in the cache already, if it were not, they will be loaded.
*   The I/O is synchronous.
*
*   Where:
*       pSector is the starting address to copy sectors
*       bMinor is the minor device number
*       dwSector is the starting logical sector number
*       dwCount is a nonzero number of sectors to copy
*       
*   Returns:
*       Address of the last sector loaded
*       
******************************************************************************/
BYTE *FDD_LoadSectors( BYTE *pSector, BYTE bMinor, DWORD dwSector, DWORD dwCount )
{
    int i, lru, error, sector;
    TBuffer *pBuf;
    BYTE bSecPerTrack;                  // Number of sectors per track
    BYTE bTrack;                        // Physical track
    BYTE bSector;                       // Physical sector within the track


    bSecPerTrack = FDC_GetTrackLen( bMinor )/512;

    for( sector=0; sector<dwCount; sector++ )
    {
        bTrack = dwSector / bSecPerTrack;
        bSector = dwSector % bSecPerTrack;

        printf("Cache search: track %d, sector: %d\n", bTrack, bSector );

        // Search for the requested track in the cache
        for( i=0; i<dwNumBuffers; i++ )
        {
            pBuf = &aBuf[i];
            if( pBuf->bMinor == bMinor && 
                pBuf->bTrack == bTrack &&
                pBuf->bLRU   != 0 )
                {
                    // Increment LRU
                    pBuf->bLRU++;

                    // Copy the sector and loop for the next one
                    goto next_sector;
                }
        }

        // Sector was not found... Load it in the cache
        lru = 255;

        // Find the cache to load into (lru), keep the best one in pBuf
        for( i=0; i<dwNumBuffers; i++ )
        {
            if( aBuf[i].bLRU < lru )
            {
                pBuf = &aBuf[i];
                lru = pBuf->bLRU;
            }
        }

        // Load the track in a temporary low-memory track buffer
        error = FDC_Operation( bMinor, FDC_READ, bTrack, (BYTE *)TARGET );
        KEY();
        printf("result = %d\n", error );

        // Copy the track into a buffer cache and set up the cache line
        memcpy( pBuf->pTrack, TARGET, bSecPerTrack * 512 );

        pBuf->bLRU = 1;                     // Live cache line, first access
        pBuf->bMinor = bMinor;              // Minor device's track
        pBuf->bTrack = bTrack;              // Stored track number

    next_sector:

        // Copy the desired sector back to the user buffer
        kmemcpy( pSector, pBuf->pTrack + bSector*512, 512 );

        // Advance sector number and the destination pointer
        dwSector++;
        pSector += 512;
    }

    return( pSector - 512 );
}


