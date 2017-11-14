/******************************************************************************
*                                                                             *
*   Module:     mem.c                                                         *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       02/26/96                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This module contains the functions for memory allocation, 
        namely kmalloc() and kfree().
        
        Parent caller should have linear memory reserved and passed
        to the init_kmem(), that will set up structures.
        
        After initialization, successive calls to kmalloc() and 
        kfree() will do their job as defined by POSIX standard for 
        the functions malloc() and free().

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 02/26/96   1.00  Original                                       Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/
#include "stdlib.h"                     // Include standard library
#include "types.h"                      // Include basic types


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

#define MALLOC_COOKIE  0xABCD9876          

// Free node structure
typedef struct
{
    DWORD size;                         // Size of the free block
    struct Tmalloc *next;               // Pointer to next free block
} Tmalloc;


static char *pFree;                     // Free list of allocation blocks


// Access macros
#define TM   (Tmalloc *)
#define STM  (struct Tmalloc *)


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/



/******************************************************************************
*                                                                             *
*   init_kmem( BYTE *pRamStart, DWORD dwRamSize )                                                                          *
*                                                                             *
*******************************************************************************
*
*   This function initializes the buffer for the memory allocation
*   using kmalloc() and kfree().
*
*   Where:
*       pRamStart - beginning address of the linear buffer memory
*       dwRamTop  - size in bytes of the buffer memory
*
*   Returns:
*       nothing
*
******************************************************************************/
void init_kmem( BYTE *pRamStart, DWORD dwRamSize )
{
    Tmalloc *pMalloc;


    // Set the dummy free structure at the beginning of the free block to 
    // easily traverse the linked list (sentinel)
    //
    pMalloc = TM(pRamStart);            // Get the buffer start
    pFree = (char*)pMalloc;             // Set the free list beginning

    pMalloc->size = 0;                  // No one can request that much!
    pMalloc->next = STM(pMalloc + 1);   // Next structure immediately follows

    pMalloc = TM(pMalloc->next);        // Next free block header
    pMalloc->size = dwRamSize - 8;      // That's how much is really free
    pMalloc->next = NULL;               // Last block in the list
}


/*********************************************************************
*
*   void *kmalloc( size_t size )
*
*   Where:
*       size - size in bytes of the memory block to be allocated.
*               must be a positive, nonzero number
*
*   Returns:
*       pointer to the first byte of the allocated memory block.
*       NULL if block can not be allocated.
*
*   Functional Description:
*       Allocates memory block of the given size.
*
**********************************************************************/
void *kmalloc( size_t size )
{
    Tmalloc *pLast;
    Tmalloc *pNew;


    /* If the requested size is 0, do nothing.  Check for sanity range.        */
    if( (size == 0) || (size > 100000) )
        return NULL;

    /* Set the size to be a multiple of 4 to keep the allignemnt               */
    /* Also, add the size of the header to be allocated (8 bytes)              */
    size = ((size+4) & 0xFFFFFFFC) + 8;

    /* Traverse the free list and find the first block large enough for the    */
    /* block of the requested size                                             */
    pLast = TM(pFree);
    pNew  = TM(pLast->next);

    while( (pNew != NULL) && (pNew->size < size ))
    {
        pLast = pNew;
        pNew  = TM(pNew->next);
    }


    /* Check if we could not find the block large enough and are at the end of */
    /* the list                                                                */
    if( pNew==NULL )
        return( NULL );


    /* Two thinkg can happen now: either we will link another free block       */
    /* at the end of the allocated one, or not.  Linking another block will    */
    /* increase fragmentation so we will do it only if the space that remains  */
    /* is larger or equal to 16 bytes (arbitrary value)                        */
    if( pNew->size >= size+16+8 )
    {
         /* Link in another free block                                         */
         pLast->next = (struct Tmalloc*)((int)pNew + size);
         pLast = TM(pLast->next);      /* Point to the new free block          */
         
         pLast->next = pNew->next;     /* Link in the next free block          */
         pLast->size = pNew->size - size;

         pNew->size = size;            /* Set the allocated block size         */
         pNew->next = STM(MALLOC_COOKIE);/* And the debug cookie                 */

         return (void*)((int)pNew + 8);
    }
    else
    {
         /* There was not enough free space to link in new free node, so just  */
         /* allocate the whole block.                                          */
         pLast->next = pNew->next;     /* Skip the newly found space           */

         /* pNew->size is all the size of a block. No need to change.          */
         pNew->next = STM(MALLOC_COOKIE);/* Set the debug cookie                 */

         return (void*)((int)pNew + 8);
    }
}


/*********************************************************************
*
*   kfree( void *mPtr )
*
*   Where:
*       mPtr - pointer to a memory block to be freed.  It has to be
*               an address that was provided by the kmalloc().
*
*   Returns:
*       nothing
*
*   Functional Description:
*
*       The memory block is freed.  It should not be used any more.
*
**********************************************************************/
void kfree( void *mPtr )
{
    Tmalloc *pLast;
    Tmalloc *pMem;
    Tmalloc *pMalloc;

        
    /* Get the allocation structure */
    pMalloc = (Tmalloc*)((int)mPtr - 8);

    
    /* Check for the magic number to ensure that the right block was passed    */
    if( (int)pMalloc->next != MALLOC_COOKIE )
    {
        /* Should print some error message in the future                       */
        //printf(" *** ERROR - Magic Number Wrong: %08X ***\n",(int)pMalloc->next );
        return;
    }
    
    /* Now we have to return the block to the list of free blocks, so find the */
    /* place in the list to insert it.  The free list is ordered by the address*/
    /* of the blocks that it holds                                             */
    pLast = TM(pFree);
    pMem  = TM(pLast->next);

    /* Traverse the free list and find where the new block should be inserted  */
    while( (pMem != NULL) && (pMem < pMalloc) )
    {
        pLast = pMem;
        pMem  = TM(pMem->next);
    }


    /* If pMem is NULL, the block to be freed lies after the last node in the  */
    /* free list, so link it at the end.                                       */
    if( pMem == NULL )
    {
        pLast->next = STM(pMalloc); /* Last node in the free list              */
        pMalloc->next = NULL;       /* Terminate it                            */

        /* The new, last free block may be merged with the preceeding one      */
        if( (int)pLast + pLast->size == (int)pMalloc )
        {
            pLast->size += pMalloc->size;
            pLast->next = NULL;
        }
        
        return;
    }


    /* Now pLast points to the last node before pMalloc, and pMem points to    */
    /* the next node.  They just have to be linked now.                        */
    pLast->next = STM(pMalloc);
    pMalloc->next = STM(pMem);


    /* If pMem node is immediately after pMalloc, they will be merged.         */
    if( (int)pMalloc + pMalloc->size == (int)pMem )
    {
        /* Merge new node and the successor pointed by pMem                    */
        pMalloc->size += pMem->size;
        pMalloc->next = pMem->next;
    }


    /* If the newly freed node is after another free node, merge them          */
    if( (int)pLast + pLast->size == (int)pMalloc )
    {
        pLast->size += pMalloc->size;
        pLast->next = pMalloc->next;
    }

    return;
}

