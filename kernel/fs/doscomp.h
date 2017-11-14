/******************************************************************************
*
*   Module:     doscomp.h
*
*   Revision:   1.00
*
*   Date:       08/24/96
*
*   Author:     Goran Devic
*
*******************************************************************************

    Module Description:

          This file is a DOS-Compatibility header file.  It should be included
          whenever testing a module under DOS.

*******************************************************************************
*
*   Changes:
*
*   DATE     REV   DESCRIPTION OF CHANGES                          AUTHOR
* --------   ----  ---------------------------------------------   -----------
* 08/24/96   1.00  Original                                        Goran Devic
* --------   ----  ---------------------------------------------   -----------
*******************************************************************************
*   Include Files
******************************************************************************/
#ifndef _DOSCOMP_H_
#define _DOSCOMP_H_

#include <stdio.h>                      // Include standard header file
#include <stdlib.h>                     // Include standard library header
#include <conio.h>                      // Include console i/o, inp(), outp()
#include <dos.h>                        // Include console I/O
#include <bios.h>                       // Include BIOS support
#include <i86.h>                        // Include Intel specifics


/******************************************************************************
*   Global Variables, Macros and Defines
******************************************************************************/
//typedef unsigned int DWORD;             // Define DWORD data type
//typedef unsigned short WORD;            // Define WORD data type
//typedef unsigned char BYTE;             // Define BYTE data type

#define printk      printf
#define kmalloc     malloc
#define kfree       free
#define kmemcpy     memcpy
#define kmemset     memset


#define OK                  0           // Operation successful
#define EINVAL              -1          // Invalid value
#define EIO                 -2          // General I/O error


/******************************************************************************
*   Functions
******************************************************************************/

void DisableInterrupts();
#pragma aux DisableInterrupts = "cli" parm;

void EnableInterrupts();
#pragma aux EnableInterrupts = "sti" parm;

#define KEY() { printf("Press a key...\n"); getch(); }


#endif //   _DOSCOMP_H_
