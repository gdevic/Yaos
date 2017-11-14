/******************************************************************************
*                                                                             *
*   Module:     Ioctl.h                                                       *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       11/5/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This is a header file containing all device controlling operations

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 11/5/97    Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _IOCTL_H_
#define _IOCTL_H_


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
*   Terminal IO Control codes                                                 *
*                                                                             *
******************************************************************************/

#define TCGETS                  0       // tcgetattr()
#define TCSETS                  1       // tcsetattr() with TCSANOW
#define TCSETSD                 2       // tcsetattr() with TCSADRAIN
#define TCSETSF                 3       // tcsetattr() with TCSAFLUSH


/******************************************************************************
*                                                                             *
*   File System IO Control codes                                              *
*                                                                             *
******************************************************************************/

#define IOCTL_GETMODE           0       // Get file attributes/mode


/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/


#endif //  _IOCTL_H_