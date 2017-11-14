/*********************************************************************
*                                                                    *
*   Module:     print.c                                              *
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

        This module contains the kprintf function

**********************************************************************
*                                                                    *
*   Changes:                                                         *
*                                                                    *
*   DATE     REV   DESCRIPTION OF CHANGES                 AUTHOR     *
* --------   ----  -----------------------------------   -----------
  02/28/96   1.00  Original                              Goran Devic

* --------   ----  -----------------------------------   ----------- *
**********************************************************************
*   Important defines/undefines
**********************************************************************/
#define _TEST_                  0

/*********************************************************************
*   Include Files
**********************************************************************/
#if _TEST_

#include "kernel.h"
#include "stdarg.h"
#include "stdlib.h"
#include "types.h"

#else

#include "stdlib.h"
#include "stdarg.h"
#include "kernel.h"

#endif

/*********************************************************************
*   Global Functions
**********************************************************************/

/*********************************************************************
*   External Functions
**********************************************************************/

/*********************************************************************
*   Local Variables and defines
**********************************************************************/

#define LINES            50          // Number of lines

#define LEFT_JUSTIFY     0x0001
#define PRINT_SIGN       0x0002
#define PRINT_SPACE      0x0004
#define ZERO_PREFIX      0x0008
#define HEX_UPPERCASE    0x0010
#define NEGATIVE         0x0020


void (*putcharfn)( char );
char *str_buf;

/*********************************************************************
*   Local Functions
**********************************************************************/

extern void memcpyw( int, int, int );
#pragma aux xmemcpyw = \
"      cld"             \
"      mov  edi, eax"   \
"      mov  esi, ebx"   \
"      rep  movsw"      \
parm caller [eax] [ebx] [ecx] modify [edi esi];


extern void memsetw( int, int, int );
#pragma aux xmemsetw = \
"      cld"             \
"      mov  edi, ebx"   \
"      rep  stosw"      \
parm caller [ebx] [ecx] [eax] modify [edi];


void xscroll_up()
{
    memcpyw( 0xB8000-ABS_KERNEL_LIN, 0xB8000-ABS_KERNEL_LIN + 80*2, 80*2*(LINES-1) );
    memsetw( 0xB8000-ABS_KERNEL_LIN+80*2*(LINES-1), 80, 0x720 );
}

int written;

void XX_kputchar( char c )
{
    static int x = 0;
    static int y = 16;


    // Look for the different control characters in raw mode
    //
    switch( c )
    {
        // CR character - does LF *AND* CR !
        //
        case 0x0D :

             x = 0;

             y++;
             if( y == LINES )
             {
                y = LINES-1;
                scroll_up();
             }

        break;

        // LF character
        //
        case 0x0A :

             y++;
        break;

        // Backspace - move one position left, print a space and move
        // left again
        //
        case '\b' :

             if( x > 0 )
             {
                x--;
                kputchar(' ');
                x--;
             }

        break;

        // Tabulator - pad with zeroes to the multiple of 8 column
        //
        case '\t' :

             do
                 kputchar(' ');

             while( x & 7 );
        break;


        // All the printable characters
        //
        default:

            *(short int*)(0xb8000 + y*80*2 + x*2 - ABS_KERNEL_LIN ) = c | 0x700;

            if( ++x == 80 )
            {
                x = 0;

                if( ++y == LINES )
                {
                    y = LINES-1;
                    scroll_up();
                }
            }
        }

        written++;
}


void sputchar( char c )
{
    *str_buf++ = c;
    *str_buf = (char)0;

    written++;
}



/*********************************************************************
*
* Functions:
*
*   printf
*   vprintf
*   sprintf
*   vsprintf
*
**********************************************************************/
int Printf( const char *format, ... )
{
    va_list arg;

    putcharfn = kputchar;

    va_start( arg, format );
    return print( format, arg );
}


int vPrintf( const char *format, va_list arg )
{
    putcharfn = kputchar;

    return print( format, arg );
}


int sPrintf( char *buf, const char *format, ... )
{
    va_list arg;

    str_buf = buf;
    putcharfn = sputchar;

    va_start( arg, format );
    return print( format, arg );
}


int vsPrintf( char *buf, const char *format, va_list arg )
{
    str_buf = buf;
    putcharfn = sputchar;

    return print( format, arg );
}




/*********************************************************************
*
*   int print( const char *format, va_list arg )
*
*
*   Where:
*
*   Returns:
*
*   Functional Description:
*
**********************************************************************/
int Print( const char *format, va_list arg )
{
    static const char hex[16]="0123456789abcdef";
    static const unsigned int dec[10] = {
        1000000000,
        100000000,
        10000000,
        1000000,
        100000,
        10000,
        1000,
        100,
        10,
        1
    };

    char buf[10];
    char * fmt = format;
    char c;
    int i, j, k, l;
    int flags;
    int width;

    int arg_int;
    unsigned int arg_uint;
    char arg_chr;
    char *arg_str;
    int *arg_pint;


    written = 0;

    while( c=*fmt++ )
    {
        switch( c )
        {

            // New line - send LF and CR characters
            //
            case '\n' :

//               (putcharfn)( 0x0A );
                 (putcharfn)( 0x0D );

            break;


            // Format control characters
            //
            case '%' :
            {
                flags = 0;
                width = 0;

                c = *fmt++;

                // Sequence of flags
                //
                if( c == '-' )              // Left-justify
                {
                    flags |= LEFT_JUSTIFY;
                    c = *fmt++;
                }

                if( c == '+' )              // Force printing a sign
                {
                    flags |= PRINT_SIGN;
                    c = *fmt++;
                }

                if( c == ' ' )              // Use space instead of + sign
                {
                    if( !(flags & PRINT_SIGN) )
                    {
                        flags |= PRINT_SPACE;
                    }

                    c = *fmt++;
                }

                if( c == '0' )              // Prefix result with 0s
                {
                    flags |= ZERO_PREFIX;
                    c = *fmt++;
                }

                // Width field
                //
                while( (c>='0') && (c<='9') )
                {
                    width += width*10 + (c-'0');
                    c = *fmt++;
                }


                // Perform the conversion
                //
                switch( c )
                {

                    // Signed integer (32 bit)
                    //
                    case 'd':
                    case 'i':
                        arg_int = va_arg( arg, int );

                        if( arg_int < 0 )
                        {
                            flags |= NEGATIVE;
                            arg_uint = -arg_int;
                        }
                        else
                            arg_uint = arg_int;

                        goto UnsignedInt;

                    // Unsigned number (32 bit)
                    //
                    // Affecting flags: LEFT_JUSTIFY
                    //                  ZERO_PREFIX
                    //                  PRINT_SIGN
                    //                  PRINT_SPACE
                    //                  width
                    //                  NEGATIVE (from cases 'd','i')
                    //
                    case 'u':
                         arg_uint = va_arg( arg, unsigned int );

                         // Print the decimal number into a temp buffer
                         // and count the number of digits excluding the
                         // leading zeroes
                         //
UnsignedInt:

                         k = l = 0;

                         for( i=0; i<10; i++ )
                         {
                            j = (arg_uint / dec[i]) + '0';

                            buf[ l++ ] = j;

                            // Count the significant digits
                            //
                            if( (j!='0') || (k>0) )
                                k++,
                                arg_uint %= dec[i];
                         }

                         if( k==0 )
                            k = 1;

                         // If the left justify was not set, we look into
                         // zero prefix flag and width
                         //
                         if( !(flags&LEFT_JUSTIFY) )
                         {
                             while( width-k > 0 )
                             {
                                if( flags&ZERO_PREFIX )
                                    (putcharfn)('0');
                                else
                                    (putcharfn)(' ');

                                width--;
                             }
                         }

                         // Take care of the sign
                         //
                         if( flags&NEGATIVE )
                             (putcharfn)('-');
                         else
                             if( flags&PRINT_SIGN )
                                 (putcharfn)('+');
                             else
                                 if( flags&PRINT_SPACE )
                                     (putcharfn)(' ');

                         // If the left justify is set, print the number
                         // and pad it with spaces if width is greater
                         //
                         if( flags&LEFT_JUSTIFY )
                         {
                            for( i=10-k; i<10; i++ )
                                (putcharfn)( buf[i] );

                            while( width-k > 0 )
                            {
                                (putcharfn)(' ');
                                width--;
                            }
                            break;
                         }

                         // Finally, put out the digits
                         //
                         for( i=10-k; i<10; i++ )
                             (putcharfn)( buf[i] );

                    break;


                    // Hex number (unsigned)
                    //
                    // Affecting flags: LEFT_JUSTIFY
                    //                  ZERO_PREFIX
                    //                  HEX_UPPERCASE
                    //                  width
                    //
                    case 'X':
                         flags |= HEX_UPPERCASE;

                    case 'x':
                         arg_uint = va_arg( arg, unsigned int );


                         // Print the hex number into a temp buffer
                         // and count the number of digits excluding the
                         // leading zeroes
                         //

                         k = l = 0;

                         for( i=28; i>=0; i-=4 )
                         {
                            j = hex[ (arg_uint >> i) & 0x0f ];

                            // Uppercase hex if needed
                            //
                            if( (flags&HEX_UPPERCASE) && (j>'9') )
                                j += 'A' - 'a';

                            buf[ l++ ] = j;

                            // Count the significant digits
                            //
                            if( (j!='0') || (k>0) )
                                k++;
                         }

                         if( k==0 )
                            k = 1;

                         // If the left justify is set, print the hex number
                         // and pad with spaces if width is greater
                         //
                         if( flags&LEFT_JUSTIFY )
                         {
                            for( i=8-k; i<8; i++ )
                                (putcharfn)( buf[i] );

                            while( width-k > 0 )
                            {
                                (putcharfn)(' ');
                                width--;
                            }

                            break;
                         }


                         // If the left justify was not set, we look into
                         // zero prefix flag and width
                         //
                         while( width-k > 0 )
                         {
                            if( flags&ZERO_PREFIX )
                                (putcharfn)('0');
                            else
                                (putcharfn)(' ');

                            width--;
                         }

                         // Finally, put out the digits
                         //
                         for( i=8-k; i<8; i++ )
                             (putcharfn)( buf[i] );


                    break;


                    // Pointer to a zero-terminated character string
                    //
                    // Affecting flags: LEFT_JUSTIFY
                    //                  width
                    //
                    case 's':

                         arg_str = va_arg( arg, char * );

                         // Find the length of the string

                         i = 0;
                         while( *(arg_str + i) ) i++;

                         // If right justified and width is greater than the
                         // string length, pad with spaces

                         if( !(flags&LEFT_JUSTIFY) && (i<width) )
                             for( j=0; j< width-i; j++ )
                             {
                                 (putcharfn)(' ');
                             }

                         // Print the entire string

                         while( c = *arg_str++ )
                         {
                            (putcharfn)( c );
                         }

                         // If the string was left justified and the width
                         // was greater than the string len, pad with spaces

                         if( (flags&LEFT_JUSTIFY) && (i<width) )
                             for( j=0; j< width-i; j++ )
                             {
                                 (putcharfn)(' ');
                             }

                    break;


                    // Simple character
                    //
                    case 'c':

                         arg_chr = va_arg( arg, char );

                         (putcharfn)( arg_chr );

                    break;


                    // Pointer to integer that is written with the
                    // number of characters written so far.
                    // This does not affect output.
                    //
                    case 'n':

                         arg_pint = va_arg( arg, int * );

                         *arg_pint = written;

                    break;

                };

            };

            break;


            // Print every character that was not in the above
            // category
            //
            default:

                (putcharfn)( c );
        }
    }

    return( written );
}



#if _TEST_

int main()
{
    char str[80] = "........................";

    sPrintf(str,"123*%4s%d %x\n", "st", 456, 10);
    Printf("%s %+05d\n", str, -1);

    return 0;
}

#endif
