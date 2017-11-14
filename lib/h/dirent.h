/******************************************************************************
*                                                                             *
*   Module:     dirent.h                                                      *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       11/13/97                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This is a header file containing the directory defines.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 11/13/97   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _DIRENT_H_
#define _DIRENT_H_


/******************************************************************************
*                                                                             *
*   Include Files                                                             *
*                                                                             *
******************************************************************************/

#include <limits.h>                     // Include system limits

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

struct dirent
{
    unsigned short d_ino;
    int handle;                         // Directory descriptor
    char d_name[ NAME_MAX ];            // ASCIIZ directory name
    char base_dir[ PATHNAME_MAX + 1 ];  // Original directory name request
};


typedef struct
{
    struct dirent dir;                  // Directory name

} DIR;


/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern DIR *opendir( const char *dirname );
extern struct dirent *readdir( DIR *pDir );
extern int closedir( DIR *pDir );
extern void rewinddir( struct dirent *pdirent );


#endif //  _DIRENT_H_
