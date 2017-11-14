/******************************************************************************
*                                                                             *
*   Module:     _Stdlib.c                                                     *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       10/24/97                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This module contains the code for the standard unix C services

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 10/24/97   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include <sys/syscall.h>                // Include system call header

#include <stdlib.h>                     // Inclue its own header

#include <string.h>                     // Include string header file

#include <sys/clib.h>                   // Include private library header file

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

// Pointer to a dynamic memory heap

void * _dwDynamic;

// Environment variable

char **environ;


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
*   char *getenv( const char *name )                                          *
*                                                                             *
*******************************************************************************
*
*   This function searches the environment for the string that matches the
#   string pointed by name.
*
*   Where:
*       name is a pointer to string to be found
*
*   Returns:
*       pointer to value where the environment has `name=value'
#       NULL is there was no match
*
******************************************************************************/
char *getenv( const char *name )
{
    int i, len;

    // Little safety checking... this should not happen

    if( (environ==NULL) || (name==NULL) )
        return( NULL );

    len = strlen(name);

    i = 0;
    while( environ[i] != NULL )
    {
        if( (strnicmp( environ[i], name, len ) == 0)
         && (*(environ[i]+len) == '=' ))
        {
            // Found it, return the value address

            return( environ[i]+len+1 );
        }

        i++;
    }

    return( NULL );
}


/******************************************************************************
*                                                                             *
*   void exit( int status )                                                   *
*                                                                             *
*******************************************************************************
*
*   This is the exit function.
*
*   Where:
*       Status is the exit status number.
*
*   Returns:
*       Never returns
*
******************************************************************************/
void exit( int status )
{


    SYS_CALL1( SYS_EXIT, status );
}


/******************************************************************************
*                                                                             *
*   void *malloc( size_t size )                                               *
*                                                                             *
*******************************************************************************
*
*   This is a malloc function.
*
*   Where:
*       size is the number of bytes to allocate
*
*   Returns:
*       A pointer to the allocated space or NULL if no space is available
*
******************************************************************************/
void *malloc( size_t size )
{
    return (void *) _kMalloc( _dwDynamic, size );
}


/******************************************************************************
*                                                                             *
*   void *calloc( size_t nmemb, size_t size )                                 *
*                                                                             *
*******************************************************************************
*
*   This function allocates and zeroes memory.
*
*   Where:
*       nmemb is the number of elements to allocate
#       size is the size of each element
*
*   Returns:
*       A pointer to the allocated space or NULL if no space is available
*
******************************************************************************/
void *calloc( size_t nmemb, size_t size )
{
    void *ptr;

    ptr = (void *) _kMalloc( _dwDynamic, nmemb * size );

    // Clear the memory if the call to malloc succeded

    if( ptr != NULL )
        memset( ptr, 0, nmemb * size );

    return( ptr );
}


/******************************************************************************
*                                                                             *
*   void *realloc( void *ptr, size_t size )                                   *
*                                                                             *
*******************************************************************************
*
*   This function changes the size of a memory object.
*
*   Where:
*       ptr is the pointer returned by a calloc(), malloc() or realloc() call
#       size is the new size
*
*   Returns:
*       A pointer to the (possibly moved) allocated space
*
******************************************************************************/
void *realloc( void *ptr, size_t size )
{
    void *pNew;

    // Try to allocate new block if needed (malloc will return NULL on size=0)

    pNew = malloc( size );

    // Free the old block (free will do nothing if ptr is NULL)

    free(ptr);

    return( pNew );
}


/******************************************************************************
*                                                                             *
*   void free( void *ptr )                                                    *
*                                                                             *
*******************************************************************************
*
*   This function deallocates dynamic memory.
*
*   Where:
*       ptr is the pointer returned by a calloc(), malloc() or realloc() call
*
*   Returns:
*       void
*
******************************************************************************/
void free( void *ptr )
{
    _kFree( _dwDynamic, ptr );
}


