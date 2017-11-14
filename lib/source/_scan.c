/******************************************************************************
*                                                                             *
*   Module:     _Scan.c                                                       *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       10/28/97                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This module contains the code for the `scan' function.

        Supported formats are: (%?)
            `c'  for character and string, optional width
            `s'  for string, optional width
            `n'  for count
            `['  for a scanset: `[^' for inverse scanset,
                                form `A-B' for subset
            `d', o, b, i, u, x, X for integer numbers; decimal, octal, binary
                  any type, unsigned, hexadecimal, hexadecimal
                  with optional +/- sign
                  with optional decimal field width
            optional modifier `h' for unsigned short
            optional modifier `l' for unsigned long
            `*' optionally disallow writing back any result

        Note: floating point numbers are not supported.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 10/28/97   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include <stdio.h>                      // Include standard I/O header file

#include <stdarg.h>                     // Include variable arguments

#include <ctype.h>                      // Include character types

#include <string.h>                     // Include string header file

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

// Define flag specifiers

#define USHORTINT       0x0001          // Write as unsigned short int type
#define ULONGINT        0x0002          // Write as unsigned long int type

#define INITIAL_FLAGS   0               // Initial settings for flags

#define EOFX            EOF

#define DEFAULT_WIDTH   0x7FFF          // Width value if none specified

static char scanset[256];               // Scanset array for `[...]' formats

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

int _peekc(FILE *fp)
{
   int c;
   ungetc( c = getc(fp) ,fp );
   return c;
}

/******************************************************************************
*                                                                             *
*   int _scan( FILE *fp, const char *fmt, va_list arg )                       *
*                                                                             *
*******************************************************************************
*
*   This function does the scanning from the input stream fp using the format
#   string fmt and the variable number of destination addresses arg.
*
*   Where:
*       fp is the pointer to the input stream structure
#       fmt is the pointer to the format string
#       arg is the variable argument list containing the destination addresses
#           for the scanned values
*
*   Returns:
*       number of values read in the argument variables
#       EOF if scanning ended prematurely
*
******************************************************************************/
int _scan( FILE *fp, const char *fmt, va_list arg )
{
    int read;                           // Number or items read
    int count;                          // Number of characters scanned
    int width;                          // Field width
    char * dest;                        // Destination address for the string
    int in;                             // Current input character
    int flags;                          // Current state flags
    int allow;                          // Did not have '*' to avoid store
    int base;                           // Base of the scanning numbers
    int minus;                          // Negative sign is read
    unsigned long num;                  // Number that is being read
    int digit;

    read  = 0;
    count = 0;

    // Return if there is some nonsense in the argument list or a file pointer

    if( (fp==NULL) || (fmt==NULL) || (*fmt=='\0') )  return( 0 );

    // Loop for each character in the format string

    while( 1 )
    {
        // Skip all whitespaces in the format string

        if( isspace(*fmt) )
        {
            while( isspace(*++fmt) );

            // .. therefore skip possible spaces in the input stream

            while( isspace((in = getc(fp))) )
            {
                count++;

                // Exit the loop if we have reached end of the input stream

                if( in==EOFX )  break;
            }

            // Put back the non-white space character

            ungetc(in, fp);
            count--;
        }

        // Check if the format string is at the end or the input stream ended

        if( !*fmt || feof(fp) ) break;

        // If the character in the stream is not the formatting one, it has to
        // match the current format character

        if( *fmt != '%' )
        {
            if( (in = getc(fp)) != *fmt++ )  break;
            count++;
            continue;
        }

        // If the second character in the format is `%' we need to match
        // that character

        if( *++fmt == '%' )
        {
            if( (in = getc(fp)) != '%' )  break;
            count++;
            continue;
        }

        flags = INITIAL_FLAGS;          // Reset flags to default values
        allow = 1;                      // Allow the writing of result
        width = DEFAULT_WIDTH;          // Make default width very large

        // If the first character is `*', set a flag to ignore the assignment

        if( *fmt == '*' )
        {
            allow = 0;
            fmt++;
        }

        // The following optional field is the decimal number that is
        // a field width parameter

        if( isdigit(*fmt) )
        {
            // Width specifier is here - evaluate it
            // Read the width decimal number from the format string

            width = 0;

            while( isdigit(*fmt) )
            {
                width *= 10;
                width += *fmt++ - '0';
            }
        }

        // If the next character is `h' or `l', set the data type

        if( *fmt=='h' ) flags |= USHORTINT, fmt++;
        else
        if( *fmt=='l' ) flags |= ULONGINT, fmt++;

        // For conversions other than `[', `c' and `n', skip the leading
        // white space characters

        if( (*fmt != '[') && (*fmt != 'c') && (*fmt != 'n') )
        {
            while( isspace((in = getc(fp))) ) count++;
            count++;

            // Put back the non-space character

            if( in != EOFX ) count--, ungetc(in,fp);
            else
                break;
        }

        // Depending upon the format character, switch
        // We are enterning this switch statement with the first character
        // ready in the input stream

        switch( *fmt )
        {
            case 'c':   // Character or a character stream (width).  If no
                        // width was specified, a single character is read

                if( width==DEFAULT_WIDTH ) width = 1;

                if( allow ) dest = va_arg( arg, char *);

                // Get the first character

                in = getc(fp);
                count++;

                while( width && (in!=EOFX) )
                {
                    // Store the character if allowed

                    if( allow )  *dest++ = in;

                    // Get the new character in a string if width needs it

                    if( --width ) count++, in = getc(fp);
                }

                break;

            case 's':   // Non-blank character string with optional width
                        // that is terminated by the null character
                        // First non-white space character terminates read

                if( allow )  dest = va_arg( arg, char *);

                // Get the first character

                in = getc(fp);
                count++;

                while( width && (in!=EOFX) )
                {
                    // If the character breaks (non-space), put it back

                    if( isspace(in) )
                    {
                        ungetc(in, fp);
                        count--;
                        break;
                    }

                    // Store the character if allowed

                    if( allow )  *dest++ = in;

                    // Get a new character

                    if( --width ) count++, in = getc(fp);
                }

                // Terminate a string

                if( allow ) *dest = '\0';

                break;

            case 'n':   // The number of characters scanned so far is written
                        // to the destination buffer.  The type of the buffer
                        // is affected by the data type specifier

                if( allow )
                {
                    if( flags & USHORTINT )
                        *va_arg(arg, short *) = (unsigned short int) count;
                    else
                    if( flags & ULONGINT )
                        *va_arg(arg, long *) = (unsigned long int) count;
                    else
                        *va_arg(arg, int *) = count;

                    // Compensate for the increment at the bottom since this
                    // format should not increment this counter

                    read--;
                }

                break;

            case '[':   // The longest non-empty sequence of characters
                        // containing in the substring range.

                fmt++;

                // If the first character is `^', invert the meaning

                if( *fmt == '^' )
                    minus = 1, fmt++;
                else
                    minus = 0;

                // Clear the scanset array to the desired polarity

                memset(scanset, minus, 256);

                // Loop for each character in the format and set the scanset

                do
                {
                    // Special case is the assignment in the form `A-B' where
                    // A,B are not `[',`^',`]' and A<B

                    if( (*fmt=='-')
                     &&(strchr("[^]",*(fmt-1))==NULL)
                     &&(strchr("[^]",*(fmt+1))==NULL)
                     &&(*(fmt-1) < *(fmt+1)))
                    {
                        memset(scanset+*(fmt-1), !minus, *(fmt+1) - *(fmt-1) + 1);
                        fmt++;
                    }
                    else
                        scanset[*fmt] = !minus;

                    fmt++;
                }
                while( *fmt && (*fmt!=']'));

                // Now read the input stream and assign the matching subset

                if( allow )  dest = va_arg( arg, char *);

                do
                {
                    in = getc(fp);
                    if( in==EOFX ) break;
                    count++;

                    if( scanset[in] && allow ) *dest++ = in;

                } while(scanset[in]);

                // Put back the last, non-conforming character

                ungetc(in, fp);
                count--;

                // Terminate a string

                if( allow ) *dest = '\0';

                break;

            case 'd':   // Decimal number with the base of 10
            case 'o':   // Octal number with the base of 8
            case 'b':   // Binary number with the base of 2
            case 'i':   // A number with the variable base
            case 'u':   // Unsigned decimal number with the base of 10
            case 'x':   // Hex number with the base of 16
            case 'X':

                 // Assign the base

                 switch( *fmt )
                 {
                    case 'x':
                    case 'X': base = 16; break;
                    case 'd':
                    case 'u': base = 10; break;
                    case 'o': base = 8;  break;
                    case 'b': base = 2;  break;

                    default:  base = 0;  // For the format of `i'
                 }

                 num = 0;
                 minus = 0;

                 // Peek a character ahead

                 in = _peekc(fp);

                 // The first character may be a minus sign, but
                 // by default, plus is expected and ignored

                 if( (in=='-') || (in=='+') )
                 {
                    if( in=='-' ) minus = 1;

                    // Skip the plus/minus sign

                    (void)getc(fp);
                    count++, width--;

                    // Peek the next character

                    in = _peekc(fp);
                 }

                 // If the next char is '0', we may have to adjust the base

                 if( in=='0' )
                 {
                    // Getting rid of this zero does not affect a number

                    (void)getc(fp);
                    count++, width--;

                    // If base is not defined, it may be set now to 8 or 16

                    in = _peekc(fp);

                    if( ((base==0) || (base==16))
                     && ((in=='x') || (in=='X')))
                    {
                        base = 16;

                        // Skip the 'x' character

                        (void)getc(fp);
                        count++, width--;
                    }
                    else
                    if( base==0 ) base = 8;
                 }

                 // Scan the number now

                 while( (width-->0) && (in != EOFX) )
                 {
                    in = _peekc(fp);

                    // Depending on the base, check and assign a digit

                    if( (base==2)  && (in>='0') && (in<='1')) digit = in-'0';
                    else
                    if( (base==8)  && (in>='0') && (in<='7')) digit = in-'0';
                    else
                    if( (base==10) && isdigit(in))  digit = in-'0';
                    else
                    if( (base==16) && isxdigit(in)) digit = (in<='9')? in-'0':tolower(in)-'a'+10;
                    else
                        break;

                    // Get the character for real

                    (void)getc(fp);
                    count++;

                    num *= base;
                    num += digit;
                 }

                 // Negate a number if a minus was used

                 if( minus ) num = -num;

                 if( allow )
                 {
                    if( flags & USHORTINT )
                        *va_arg(arg, short *) = (unsigned short int) num;
                    else
                    if( flags & ULONGINT )
                        *va_arg(arg, long *) = (unsigned long int) num;
                    else
                        *va_arg(arg, int *) = num;
                 }

                 break;
        }

        // Increase a number of items that were actually read

        if( allow ) read++;

        // Skip the format character

        if( !*fmt++ ) break;
    }

    // If the breaking character was EOF, return EOF.  Otherwise, return the
    // number of items actually read

    if( in == EOFX )
        return( EOFX );

    return( read );
}


/******************************************************************************
*                                                                             *
*   int scanf( const char *fmt, ... )                                         *
*                                                                             *
*******************************************************************************
*
*   This function is a member of scanf family.  It uses `stdin' stream to get
#   the input.
*
*   Where:
#       fmt is the scanning format
#       ... pointers to destination storage variables
*
*   Returns:
*       number of items read
#       EOF if end of file has been reached
*
******************************************************************************/
int scanf( const char *fmt, ... )
{
    va_list arg;
    int n;

    va_start( arg, fmt );
    n = _scan(stdin, fmt, arg );
    va_end( arg );

    return( n );
}


/******************************************************************************
*                                                                             *
*   int vscanf( const char *fmt, va_list arg )                                *
*                                                                             *
*******************************************************************************
*
*   This function is a member of scanf family.  It uses `stdin' stream to get
#   the input.
*
*   Where:
#       fmt is the scanning format
#       arg is a list of arguments that are pointers to destination storage
#           variables
*
*   Returns:
*       number of items read
#       EOF if end of file has been reached
*
******************************************************************************/
int vscanf( const char *fmt, va_list arg )
{
    return _scan(stdin, fmt, arg );
}


/******************************************************************************
*                                                                             *
*   int fscanf( FILE *fp, const char *fmt, ... )                              *
*                                                                             *
*******************************************************************************
*
*   This function is a member of scanf family.
*
*   Where:
*       fp is a input stream file pointer
#       fmt is the scanning format
#       ... pointers to destination storage variables
*
*   Returns:
*       number of items read
#       EOF if end of file has been reached
*
******************************************************************************/
int fscanf( FILE *fp, const char *fmt, ... )
{
    va_list arg;
    int n;

    va_start( arg, fmt );
    n = _scan(fp, fmt, arg );
    va_end( arg );

    return( n );
}



/******************************************************************************
*                                                                             *
*   int vfscanf( FILE *fp, const char *fmt, va_list arg )                     *
*                                                                             *
*******************************************************************************
*
*   This function is a member of scanf family.
*
*   Where:
*       fp is a input stream file pointer
#       fmt is the scanning format
#       arg is a list of arguments that are pointers to destination storage
#           variables
*
*   Returns:
*       number of items read
#       EOF if end of file has been reached
*
******************************************************************************/
int vfscanf( FILE *fp, const char *fmt, va_list arg )
{
    return _scan(fp, fmt, arg );
}

