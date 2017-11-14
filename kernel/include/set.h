/******************************************************************************
*                                                                             *
*   Module:     Set.h                                                         *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       9/9/97                                                        *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This is a header file for the Set.c module.  This module is a part of
        the debugger and it provides system variable services.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 9/9/97     1.00  Original                                       Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _SET_H_
#define _SET_H_

/******************************************************************************
*                                                                             *
*   Include Files                                                             *
*                                                                             *
******************************************************************************/

#include "types.h"                      // Include default data types

#include "queue.h"                      // Include queue header file

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

extern TQueue qSet;                     // Linked list of set keys and values


/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern BOOL SetValueAdd( char * sKey, char * sValue );
extern void SetPrintAll();
extern char * SetGetValue( char *sKey );
extern int SetGetInteger( char *sKey );


#endif //  _SET_H_
