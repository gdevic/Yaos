/******************************************************************************
*                                                                             *
*   Module:     setjmp.h                                                      *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       11/12/97                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This is a header file for the POSIX jumps.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 11/12/97   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _SETJMP_H_
#define _SETJMP_H_


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

typedef struct
{
    int state[8];

} jmp_buf[1];


//----------------------------------------------------------------------------

extern int _setjmp( jmp_buf env );

#pragma aux setjmp =        \
"   call  _setjmp"          \
parm caller [ebx] value [eax];

#pragma aux longjmp =       \
"   or   eax, eax"          \
"   jnz  skip"              \
"   or   eax, 1"            \
"skip:"                     \
"   mov  ecx, ds:[ebx+ 0]"  \
"   mov  edx, ds:[ebx+ 4]"  \
"   mov  esi, ds:[ebx+ 8]"  \
"   mov  edi, ds:[ebx+12]"  \
"   mov  esp, ds:[ebx+16]"  \
"   mov  ebp, ds:[ebx+20]"  \
"   push ds:[ebx+24]"       \
"   popfd"                  \
"   push ds:[ebx+28]"       \
"   ret"                    \
parm caller [ebx][eax];


/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern int setjmp( jmp_buf env );
extern void longjmp( jmp_buf env, int val );


#endif //  _SETJMP_H_
