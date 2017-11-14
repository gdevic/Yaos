/******************************************************************************
*                                                                             *
*   Module:     Keyboard.h                                                    *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       9/6/97                                                        *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This is a header file for the keyboard module.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 9/6/97     1.00  Original                                       Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

/******************************************************************************
*                                                                             *
*   Include Files                                                             *
*                                                                             *
******************************************************************************/

#include "types.h"                      // Include basic data types

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

//-----------------------------------------------------------------------------
//  Character Codes
//-----------------------------------------------------------------------------

#define SC_CONTROL          29         // Control key key scan code
#define SC_LEFT_SHIFT       42         // Left shift key scan code
#define SC_RIGHT_SHIFT      54         // Right shift key scan code
#define SC_CAPS_LOCK        58         // Caps lock key scan code

#define ESC           27
#define ENTER         '\n'

#define F1            0x80
#define F2            0x81
#define F3            0x82
#define F4            0x83
#define F5            0x84
#define F6            0x85
#define F7            0x86
#define F8            0x87
#define F9            0x88
#define F10           0x89
#define F11           0x8A
#define F12           0x8B

#define SF1           0x8C
#define SF2           0x8D
#define SF3           0x8E
#define SF4           0x8F
#define SF5           0x90
#define SF6           0x91
#define SF7           0x92
#define SF8           0x93
#define SF9           0x94
#define SF10          0x95
#define SF11          0x96
#define SF12          0x97

#define CF1           0x98
#define CF2           0x99
#define CF3           0x9A
#define CF4           0x9B
#define CF5           0x9C
#define CF6           0x9D
#define CF7           0x9E
#define CF8           0x9F
#define CF9           0xA0
#define CF10          0xA1
#define CF11          0xA2
#define CF12          0xA3

#define NUMLOCK       18
#define SCROLL        19
#define HOME          20
#define UP            21
#define PGUP          22
#define LEFT          23
#define RIGHT         24
#define END           25
#define DOWN          26
#define PGDN          28
#define INS           29
#define DEL           30


/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern char GetKey( BOOL fBlock );

extern void Deb_Keyboard_Handler();
extern void Int_Keyboard_Handler();


#endif // _KEYBOARD_H_
