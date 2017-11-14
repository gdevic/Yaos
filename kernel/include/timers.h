/******************************************************************************
*
*   Module:     timers.h
*
*   Revision:   1.00
*
*   Date:       08/26/96
*
*   Author:     Goran Devic
*
*******************************************************************************

    Module Description:

          This is a header file for the timers.c module

*******************************************************************************
*
*   Changes:
*
*   DATE     REV   DESCRIPTION OF CHANGES                          AUTHOR
* --------   ----  ---------------------------------------------   -----------
* 08/26/96   1.00  Original                                        Goran Devic
* --------   ----  ---------------------------------------------   -----------
*******************************************************************************
*   Include Files
******************************************************************************/

/******************************************************************************
*   Global Variables, Macros and Defines
******************************************************************************/
extern DWORD dwDelay;                   // 1 ms in terms of inner delay loops

/******************************************************************************
*   Functions
******************************************************************************/
extern void InitTimers();
extern void kdelay( DWORD dwMilisec );

