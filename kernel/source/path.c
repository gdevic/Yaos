/******************************************************************************
*                                                                             *
*   Module:     Path.c                                                        *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       11/7/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This module contains the code for the file path evaluation.
        The mount table is here affecting the resulting path that is
        to be given to a device driver.

        (I am still clue-less if this is the right way to do it :-0 )
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 11/7/97    Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include <string.h>

#include "types.h"

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/


BYTE fs_root_major = 0;
BYTE fs_root_minor = 0;

#define MOUNT_MAX       16              // Number of mounting nodes in a system

typedef struct
{
    char *path;                         // Pointer to a mount name
    BYTE bMajor;                        // Major device handling the node
    BYTE bMinor;                        // Minor device handling the node

} TMount;

TMount Mount[ MOUNT_MAX ] =
{
    { 0, 0, 0 },{ 0, 0, 0 },{ 0, 0, 0 },{ 0, 0, 0 },
    { 0, 0, 0 },{ 0, 0, 0 },{ 0, 0, 0 },{ 0, 0, 0 },
    { 0, 0, 0 },{ 0, 0, 0 },{ 0, 0, 0 },{ 0, 0, 0 },
    { 0, 0, 0 },{ 0, 0, 0 },{ 0, 0, 0 },{ 0, 0, 0 }
};

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
*   char *PathResolve( char *pPath, BYTE *bMajor, BYTE *bMinor )              *
*                                                                             *
*******************************************************************************
*
*   Resolves a path using the directory tree with respect to the mounted drives
*
*   Where:
#       pPath is the original path
*       bMajor is the address to store major device number (if not NULL)
#       bMinor is the address to store minor device number (if not NULL)
*
*   Returns:
*       A pointer to the canonical pathname string representing the orignal
#           pPath but accessible from the given device
#
#       NULL if not successful
*
******************************************************************************/
char *PathResolve( char *pPath, BYTE *bMajor, BYTE *bMinor )
{
    BYTE major, minor;
    int i;
    char *s, *p = pPath;

    // Start with the root device

    major = fs_root_major;
    minor = fs_root_minor;

    // Path is always absolute, so we assume the first character is a slash
    // Look for the next slash that would announce a (sub)directory

    s = strchr(pPath + 1, '/');

    // If there were no more (sub)directories, we got a file name

    if( s == NULL )
    {
        if( bMajor != NULL )  *bMajor = major;
        if( bMinor != NULL )  *bMinor = minor;

        return( pPath );
    }

    // Check if the directory patch is mounted

    for( i=0; i<MOUNT_MAX; i++ )
    {
        if( Mount[i].path != NULL )
        {
            if( strncmp(p, Mount[i].path, s - p ) == 0 )
            {
                // We have found a mounting point
            }
        }

    }

    if( bMajor != NULL )  *bMajor = major;
    if( bMinor != NULL )  *bMinor = minor;

    return( pPath );
}


