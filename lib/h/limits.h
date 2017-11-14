/******************************************************************************
*                                                                             *
*   Module:     Limits.h                                                      *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       11/1/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

          Unix Standard File Header

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
#ifndef _LIMITS_H_
#define _LIMITS_H_


/******************************************************************************
*                                                                             *
*   Include Files                                                             *
*                                                                             *
******************************************************************************/

#include <stdio.h>                      // Include standard io file header


/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

#define OPEN_MAX                8


#define CHAR_BIT                8
#define CHAR_MIN                0
#define CHAR_MAX                255
#define INT_MIN                 (signed)0x80000000
#define INT_MAX                 (signed)0x7FFFFFFF
#define LONG_MIN                0x80000000
#define LONG_MAX                0x7FFFFFFF
//#define MB_LEN_MAX
//#define SCHAR_MIN               (-128)
//#define SCHAR_MAX               (128)
//#define SHRT_MIN
//#define SHRT_MAX
//#define UCHAR_MAX
//#define UINT_MAX
//#define USHRT_MAX
#define ULONG_MAX               0xFFFFFFFF

//#define LINK_MAX
#define MAX_CANON               255     // Size of the canonical input buffer
#define MAX_INPUT               32      // Size of the type-ahead buffer
#define NAME_MAX                13      // Characters in a file name

//----------------------------------------------------------------------------
// Macros returned by the sysconf() function
//----------------------------------------------------------------------------

#define ARG_MAX                 3072    // Length of arguments for exec()
#define CHILD_MAX               256     // Number of user processes
#define NGROUPS_MAX             8       // Number of supplementary group IDs
#define CLK_TCK                 18      // Number of clock ticks per second
#define STREAM_MAX              FOPEN_MAX // Equals FOPEN_MAX from stdio.h
#define TZNAME_MAX              -1

#define _POSIX_CHILD_MAX        CHILD_MAX
#define _POSIX_NGROUPS_MAX      NGROUPS_MAX
#define _POSIX_OPEN_MAX         OPEN_MAX
#define _POSIX_JOB_CONTROL      0
#define _POSIX_SAVED_IDS        0
#define _POSIX_VERSION          199009L // Posix standard date

/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/


#endif //  _LIMITS_H_

