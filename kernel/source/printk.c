/*********************************************************************
*                                                                    *
*   Module:     printk.c                                             *
*                                                                    *
*   Revision:   1.00                                                 *
*                                                                    *
*   Date:       02/28/96                                             *
*                                                                    *
*   Author:     Goran Devic                                          *
*                                                                    *
**********************************************************************
*                                                                    *
*   Module Description:                                              *

        This module contains the printk function

**********************************************************************
*                                                                    *
*   Changes:                                                         *
*                                                                    *
*   DATE     REV   DESCRIPTION OF CHANGES                 AUTHOR     *
* --------   ----  -----------------------------------   -----------
  02/28/96   1.00  Original                              Goran Devic
  04/20/96   1.10  Changed to printk                     Goran Devic
  04/20/96   1.10  Added printf.c to clib                Goran Devic

* --------   ----  -----------------------------------   ----------- *
**********************************************************************
*   Important defines/undefines
**********************************************************************/

/*********************************************************************
*   Include Files
**********************************************************************/
#include "stdio.h"
#include "stdarg.h"

/*********************************************************************
*   Global Functions and Variables
**********************************************************************/

static int written;

/*********************************************************************
*   Local Functions
**********************************************************************/

void putchark( char c )
{
    tty_write( 0, c );

    written++;
}

int printk( const char *format, ... )
{
    va_list arg;

    written = 0;
    _putcharfn = putchark;

    va_start( arg, format );
    _print( format, arg );
    
    return( written );
}

