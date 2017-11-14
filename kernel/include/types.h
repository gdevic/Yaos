/******************************************************************************
*                                                                             *
*   Module:     Types.h                                                       *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       8/5/97                                                        *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This is a header file for the basic data types.  Here are defined
        types such as DWORD, WORD and BYTE.  Also, TRUE and FALSE are here.

        Watcom-specific pragma to pack the structure is defined.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 8/5/97     1.00  Original                                       Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _KTYPES_H_
#define _KTYPES_H_


/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

// This affects the Watcom compiler to pack structures

#pragma pack(1)


typedef int BOOL;                       // Define boolean type as integer

#define TRUE        1                   // Define true as 1
#define FALSE       0                   // Define false as 0


typedef unsigned char BYTE;             // Define a byte
typedef unsigned short WORD;            // Define a word (2 bytes)
typedef unsigned long DWORD;            // Define a double word (4 bytes)

#ifndef NULL
#define NULL        0L                  // Define NULL integer
#endif


#pragma aux Int3 =  \
" int 3"            \
parm;


#endif // _KTYPES_H_
