/******************************************************************************
*                                                                             *
*   Module:     Bios.h                                                        *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       10/3/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This is a header file defining some BIOS constants.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 10/3/97    Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _BIOS_H_
#define _BIOS_H_


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

#define BIOS_VIDEO_MODE         0x449       // Current video mode
#define BIOS_VIDEO_COLUMNS      0x44A       // Columns on the screen
#define BIOS_VIDEO_REGEN        0x44C       // Regen buffer size in bytes
#define BIOS_VIDEO_PGSTART      0x44E       // Current regen page start address
#define BIOS_VIDEO_CURSORS      0x450       // Cursor positions
#define BIOS_VIDEO_CURSORTYPE   0x460       // Cursor type
#define BIOS_VIDEO_PAGE         0x462       // Current page number
#define BIOS_VIDEO_PORT         0x463       // CRT port number
#define BIOS_VIDEO_MODESEL      0x465       // Mode select register
#define BIOS_VIDEO_CGAPAL       0x466       // Setting of CGA palette register

#define BIOS_VIDEO_ROWS         0x484       // Rows on screen minus one
#define BIOS_VIDEO_HEIGHT       0x485       // Character height in scan lines
#define BIOS_VIDEO_CTRL         0x487       // Control
#define BIOS_VIDEO_SWITCHES     0x488       // Switches

/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/


#endif //  _BIOS_H_
