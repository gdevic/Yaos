/******************************************************************************
*                                                                             *
*   Module:     assertk.h                                                     *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       11/18/97                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This is a header file containing the kernel assert macro that calls
        a debugger via INT 3.  Use that macro for all checks of values
        that may be `imposible' if the kernel is working perfectly.

        If the macro NODEBUG is defined, all the assertion code will be
        left out.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 11/18/97   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _ASSERTK_H_
#define _ASSERTK_H_


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

#ifdef NODEBUG

#define assertk(flag,msg)    ((void)0)

#else

#define assertk(flag,msg)    ((flag)? (void)0: _assertk(#flag"", msg, __FILE__, __LINE__))

#endif

/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern void _assertk(char *cond, char *msg, char *file, int line);


#endif //  _ASSERTK_H_
