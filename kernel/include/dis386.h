/*********************************************************************
*                                                                    *
*   Module:     dis386.k                                             *
*                                                                    *
*   Revision:   1.00                                                 *
*                                                                    *
*   Date:       03/02/96                                             *
*                                                                    *
*   Author:     Goran Devic                                          *
*                                                                    *
**********************************************************************
*                                                                    *
*   Module Description:                                              *

    Defines and arrays for Intel 386 disassembler        
       
**********************************************************************
*                                                                    *
*   Changes:                                                         *
*                                                                    *
*   DATE     REV   DESCRIPTION OF CHANGES                 AUTHOR     *
* --------   ----  -----------------------------------   -----------
  03/02/96   1.00  Original                              Goran Devic

* --------   ----  -----------------------------------   ----------- *
**********************************************************************
*   Important defines/undefines
**********************************************************************/
#ifndef _DIS386_H_
#define _DIS386_H_

/*********************************************************************
*   Include Files
**********************************************************************/
#ifdef _TEST_

#include <stdlib.h>
#include <stdio.h>

#else

#include "types.h"

#endif // _TEST_
/*********************************************************************
*   Local Variables and defines
**********************************************************************/

/*********************************************************************
*
*   This structure is used to pass parameters and options to the
*   line disassembler.
*
**********************************************************************/
typedef struct
{
    BYTE *bpTarget;              /* Target pointer to disassemble    */
    BYTE bDataSize;              /* Data size 16/32 bits (0/1)       */
    BYTE bAddressSize;           /* Address size 16/32 bits (0/1)    */
    
    BYTE *szDisasm;              /* String to put ascii result       */
    BYTE bAsciiLen;              /* Length of the ascii result       */
    BYTE bInstrLen;              /* Instruction lenght in bytes      */
    BYTE fIllegalOp;             /* Illegal opcode flag              */

} TDis386;


#define DIS_16_BIT     0         /* Use for size definition          */
#define DIS_32_BIT     1         /* in bDataSize and bAddressSize    */


/*********************************************************************
*   Global Functions
**********************************************************************/

extern BYTE Dis386( TDis386 *pDis );



#endif  // _DIS386_H_
