/******************************************************************************
*                                                                             *
*   Module:     File.h                                                        *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       10/17/97                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This is a header file for the file module.  The main file structure
        is defined here.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 10/17/97   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _FILE_H_
#define _FILE_H_


/******************************************************************************
*                                                                             *
*   Include Files                                                             *
*                                                                             *
******************************************************************************/

#include <sys\stat.h>                   // Include file information header

#include "types.h"                      // Include basic data types

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

#define MAX_FILES       32              // Number of open files in the kernel

//.-
//----------------------------------------------------------------------------
// Kernel file structure - all open files are stored in this kernel file array
//----------------------------------------------------------------------------

typedef struct
{
    struct stat Stat;                   // File's stat structure, this struct
                                        //  must be the first in TFile!
    DWORD Flags;                        // File flags
    BYTE  bMajor;                       // Duplicate major number from st_dev
    BYTE  bMinor;                       // Duplicate minor number from st_dev

} TFile;


extern TFile Files[ MAX_FILES ];

//-.
/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern int Init_FS();

extern char *PathResolve( char *pPath, BYTE *bMajor, BYTE *bMinor );


#endif //  _FILE_H_
