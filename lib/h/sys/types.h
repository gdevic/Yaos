/******************************************************************************
*                                                                             *
*   Module:     Types.h                                                       *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       09/2/96                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This is a header file containing basic POSIX data types

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 11/7/97    Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _TYPES_H_
#define _TYPES_H_


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

typedef unsigned short dev_t;           // Major/minor device number
typedef unsigned short ino_t;           // File serial number
typedef unsigned short mode_t;          // File mode/protection bits etc.
typedef unsigned short nlink_t;         // Number of links
typedef unsigned short uid_t;           // User ID of the file owner
typedef unsigned short gid_t;           // Group ID of the file owner
typedef unsigned long  off_t;           // File size

typedef int            pid_t;           // Process ID type
typedef unsigned int   size_t;          // Size type
typedef unsigned int   ssize_t;         // Another size type


/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/


#endif // _TYPES_H_

