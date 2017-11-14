/*********************************************************************
*                                                                    *
*   Module:     ports.h
*                                                                    *
*   Revision:   1.00                                                 *
*                                                                    *
*   Date:       06/24/95                                             *
*                                                                    *
*   Author:     Goran Devic                                          *
*                                                                    *
**********************************************************************
*                                                                    *
*   Module Description:                                              *

          This header file contains the macros for basic port I/O
          
**********************************************************************
*                                                                    *
*   Changes:                                                         *
*                                                                    *
*   DATE     REV   DESCRIPTION OF CHANGES                 AUTHOR     *
* --------   ----  -----------------------------------   -----------
  06/24/95   1.00  Original                              Goran Devic

* --------   ----  -----------------------------------   ----------- *
**********************************************************************
*   Important defines/undefines
**********************************************************************/
#ifndef _PORTS_H_
#define _PORTS_H_

/*********************************************************************
*   Local Macros
**********************************************************************/
#pragma aux Inpb =  \
"in  al, dx"        \
parm caller [dx] value [al];

#pragma aux Outpb = \
"out  dx, al"       \
parm caller [dx] [al];

#pragma aux Inpw =  \
"in  ax, dx"        \
parm caller [dx] value [ax];

#pragma aux Outpw = \
"out  dx, ax"       \
parm caller [dx] [ax];


/*********************************************************************
*   Function Prototypes
**********************************************************************/
extern BYTE  Inpb(WORD);
extern void  Outpb(WORD, BYTE);
extern WORD  Inpw(WORD);
extern void  Outpw(WORD, WORD);


#endif // _PORTS_H_
