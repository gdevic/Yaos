/*********************************************************************
*                                                                    *
*   Module:     printf.c                                             *
*                                                                    *
*   Revision:   1.00                                                 *
*                                                                    *
*   Date:       04/20/96                                             *
*                                                                    *
*   Author:     Goran Devic                                          *
*                                                                    *
**********************************************************************
*                                                                    *
*   Module Description:                                              *

        POSIX / ANSI C  printf-class functions:
        printf, sprintf
       
**********************************************************************
*                                                                    *
*   Changes:                                                         *
*                                                                    *
*   DATE     REV   DESCRIPTION OF CHANGES                 AUTHOR     *
* --------   ----  -----------------------------------   -----------
  04/20/96   1.00  Original                              Goran Devic

* --------   ----  -----------------------------------   ----------- *
**********************************************************************
*   Important defines/undefines
**********************************************************************/
#define _POSIX_SOURCE  1         /* POSIX - compliant library        */

/*********************************************************************
*   Include Files
**********************************************************************/
#include "ctype.h"
#include "stdlib.h"
#include "stdarg.h"
#include "stdio.h"
#include "sys\syscall.h"

/*********************************************************************
*   Local Variables and defines
**********************************************************************/

#define LEFT_JUSTIFY     0x0001
#define PRINT_SIGN       0x0002
#define PRINT_SPACE      0x0004
#define ZERO_PREFIX      0x0008
#define HEX_UPPERCASE    0x0010
#define NEGATIVE         0x0020


// Function that is called to output a character - it may be set by the
// user function and called _print directly (in that case character count
// has to be incremented by the user function)
//
void (*_putcharfn)( char );

static char *str_buf;
static int written;
static char buf[10];
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

/*********************************************************************
*   Local Functions
**********************************************************************/

/*********************************************************************
*
*   int _print( const char *format, va_list arg )
*
**********************************************************************/
int _print( const char *format, va_list arg )
{
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


    // So far characters written is 0
    written = 0;

    while( c=*fmt++ )
    {
        switch( c )
        {

            // New line - send LF and CR characters
            //
            case '\n' :

                 (_putcharfn)( 0x0A );
                 (_putcharfn)( 0x0D );

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
                                    (_putcharfn)('0');
                                else
                                    (_putcharfn)(' ');

                                width--;
                             }
                         }

                         // Take care of the sign
                         //
                         if( flags&NEGATIVE )
                             (_putcharfn)('-');
                         else    
                             if( flags&PRINT_SIGN )
                                 (_putcharfn)('+');
                             else    
                                 if( flags&PRINT_SPACE )
                                     (_putcharfn)(' ');
                     
                         // If the left justify is set, print the number
                         // and pad it with spaces if width is greater
                         //
                         if( flags&LEFT_JUSTIFY )
                         {
                            for( i=10-k; i<10; i++ )
                                (_putcharfn)( buf[i] );

                            while( width-k > 0 )
                            {
                                (_putcharfn)(' ');
                                width--;
                            }
                            break;
                         }

                         // Finally, put out the digits
                         //
                         for( i=10-k; i<10; i++ )
                             (_putcharfn)( buf[i] );

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
                            if( flags&HEX_UPPERCASE )
                                j = toupper(j);

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
                                (_putcharfn)( buf[i] );

                            while( width-k > 0 )
                            {
                                (_putcharfn)(' ');
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
                                (_putcharfn)('0');
                            else
                                (_putcharfn)(' ');

                            width--;
                         }

                         // Finally, put out the digits
                         //
                         for( i=8-k; i<8; i++ )
                             (_putcharfn)( buf[i] );


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
                                 (_putcharfn)(' ');
                             }

                         // Print the entire string

                         while( c = *arg_str++ )
                         {
                            (_putcharfn)( c );
                         }

                         // If the string was left justified and the width
                         // was greater than the string len, pad with spaces

                         if( (flags&LEFT_JUSTIFY) && (i<width) )
                             for( j=0; j< width-i; j++ )
                             {
                                 (_putcharfn)(' ');
                             }

                    break;


                    // Simple character
                    //
                    case 'c':

                         arg_chr = va_arg( arg, char );

                         (_putcharfn)( arg_chr );

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

                (_putcharfn)( c );
        }
    }

    return( written );
}


/*********************************************************************
*   Output function for printf()
**********************************************************************/
void kputchar( char c )
{
    sys_call( SYS_PUTCHAR, c );

    written++;
}

/*********************************************************************
*   Output function for sprintf()
**********************************************************************/
static void sputchar( char c )
{
    *str_buf++ = c;
    *str_buf = (char)0;

    written++;
}


/*********************************************************************
*
*   int printf( const char *format, ... )
*
**********************************************************************/
int printf( const char *format, ... )
{
    va_list arg;

    _putcharfn = kputchar;

    va_start( arg, format );
    return _print( format, arg );
}



/*********************************************************************
*
*   int sprintf( char *str, const char *format, ... )
*
**********************************************************************/
int sprintf( char *str, const char *format, ... )
{
    va_list arg;

    str_buf = str;
    _putcharfn = sputchar;

    va_start( arg, format );
    return _print( format, arg );
}

