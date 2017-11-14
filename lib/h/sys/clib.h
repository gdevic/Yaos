/******************************************************************************
*                                                                             *
*   Module:     CLib.h                                                        *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       11/1/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This is a header file private to the C Library.  It includes some
        of the most used include files.

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
#ifndef _CLIB_H_
#define _CLIB_H_


/******************************************************************************
*                                                                             *
*   Include Files                                                             *
*                                                                             *
******************************************************************************/

#include "..\\kernel\\include\\types.h" // Include kernel data types

#include <sys/types.h>                  // Include system types definitions

#include <errno.h>                      // Include error definition header file

#include <limits.h>                     // Include system limits header file

#include <assert.h>                     // Include assert macro

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

#define io_testflag(p,x)        ((p)->_flags & (x))

#pragma aux Int3 = "int 3" parm;

extern void *_dwDynamic;

/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

//----------------------------------------------------------------------------
// MAlloc.c
//----------------------------------------------------------------------------

extern BYTE * _Init_Alloc( BYTE *pRamStart, DWORD dwRamSize );
extern int _Alloc_Check( BYTE *pHeap, DWORD dwInitSize );
extern void * _kMalloc( BYTE *pHeap, DWORD size );
extern void _kFree( BYTE *pHeap, void *mPtr );


#endif //  _CLIB_H_
