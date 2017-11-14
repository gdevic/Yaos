/******************************************************************************
*                                                                             *
*   Module:     DosFS.h                                                       *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       11/1/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This is a header file for the DOS File System device driver.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 11/1/97    Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _DOSFS_H_
#define _DOSFS_H_


/******************************************************************************
*                                                                             *
*   Include Files                                                             *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   Device control block defines                                              *
*                                                                             *
******************************************************************************/

#define MAJOR_DOSFS         2           // Major number of a DOS device driver
#define MINOR_DOSFS         26          // Minor number of DOS letters


/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/


#endif //  _DOSFS_H_
