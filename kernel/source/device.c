/******************************************************************************
*                                                                             *
*   Module:     Device.c                                                      *
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

        This module contains the code for the device management.
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
*   Include Files                                                             *
******************************************************************************/

#include <string.h>                     // Include strings header file

#include "device.h"                     // Include its own header file

#include "tty.h"                        // Include tty device to init

#include "dosfs.h"                      // Include dos fs device to init

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

extern TDev DevTTY;                     // Terminal device control block
extern TDev DevDosFS;                   // DOS File System device control block

//-------------------------------
// Array of device driver blocks
//-------------------------------

TDev Dev[ MAX_DEVMAJOR ] = { 0, 0, 0, 0, 0, 0, 0, 0 };

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
*   void Init_Dev()                                                           *
*                                                                             *
*******************************************************************************
*
*   This function initializes the device subsystem.
*
*   Returns:
*       void
*
******************************************************************************/
void Init_Dev()
{
    dprintf("\nInit device drivers:");

    //------------------------------------------------------------------------
    // Register a terminal device (tty) and call its init function
    //------------------------------------------------------------------------

    dprintf("\nTerminal");
    memcpy( &Dev[MAJOR_TTY], &DevTTY, sizeof(TDev) );
    (Dev[MAJOR_TTY].dev_init)();
    dprintf(".");

    //------------------------------------------------------------------------
    // Register a DOS File System and call its init function
    //------------------------------------------------------------------------

    dprintf("\nDos File System");
    memcpy( &Dev[MAJOR_DOSFS], &DevDosFS, sizeof(TDev) );
    (Dev[MAJOR_DOSFS].dev_init)();
    dprintf(".");
}


/******************************************************************************
*                                                                             *
*   int Dev_null()                                                            *
*                                                                             *
*******************************************************************************
*
*   This is a stub call for all unsupported/not applicable functions
#   for a device (such is dev_lseek() call for a terminal device).
*
*   Where:
*       no args
*
*   Returns:
*       0
*
******************************************************************************/
int Dev_null()
{
    return( 0 );
}

