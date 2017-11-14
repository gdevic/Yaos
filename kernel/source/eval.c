/******************************************************************************
*                                                                             *
*   Module:     ExpressionEval.c                                              *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       5/15/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This is an generic expresion evaluator.  No error checking is
        performed.

        Numbers that are acepted are integers written in decimal notation,
        octal (first digit is 0), hexadecimal (ending with a letter 'h', case
        insensitive) or binary (ending with a letter 'b', case insensitive).

        Variables (alphanumerical literals) are supported by registering an
        external function that will handle them.

*******************************************************************************

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 05/15/97   1.00  Original                                       Goran Devic *
* 05/18/97   1.01  Added bitwise, boolean operators               Goran Devic *
* 05/20/97   1.02  Literal handling                               Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/
#include <stdio.h>                      // Include standard I/O header file
#include <ctype.h>                      // Include character testing
#include <string.h>                     // Include string header


/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

int  nEvaluate( char *sExpr);
void RegisterLiteralHandler( int (nLiteralHandler)( char *sName ) );


/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

#define TEST            0

#define MAX_STACK       10              // Max depth of a stack structure

#define BOTTOM_STACK    0               // Stack empty code
#define OP_PAREN_START  1               // Priority codes and also indices
#define OP_PAREN_END    2               //   into the sTokens array
#define OP_BOOL_OR      3
#define OP_BOOL_AND     4
#define OP_OR           5
#define OP_XOR          6
#define OP_AND          7
#define OP_EQ           8
#define OP_NE           9
#define OP_L            10
#define OP_LE           11
#define OP_G            12
#define OP_GE           13
#define OP_SHL          14
#define OP_SHR          15
#define OP_PLUS         16
#define OP_MINUS        17
#define OP_TIMES        18
#define OP_DIV          19
#define OP_MOD          20
#define OP_NOT          21
#define OP_NEG          255             // Unary minus has highest priority

static char *sTokens[] =
{
    "(", ")",
    "||",
    "&&",
    "|", "^", "&",
    "==", "!=",
    "<", "<=", ">", ">=",
    "<<", ">>",
    "+", "-",
    "*", "/", "%",
    "!",
    NULL
};


static struct Stack                     // Defines stack structure(s)
{
    int Data[ MAX_STACK ];              // Stack data
    int Top;                            // Top of stack index
} Values, Operators;


static int (*pLiteralHandler)( char *sName ) = NULL;

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   void Push( struct *Stack, int Value )                                     *
*                                                                             *
*******************************************************************************
*
*   Pushes a value into a given stack structure.
*
*   Where:
*       Stack is a pointer to a stack structure
#       Value is a number to push
*
*   Returns:
*       void
*
******************************************************************************/
static void Push( struct Stack *Stack, int Value )
{
    if( Stack->Top < MAX_STACK )
        Stack->Data[ Stack->Top++ ] = Value;
}


/******************************************************************************
*                                                                             *
*   int Pop( struct *Stack )                                                  *
*                                                                             *
*******************************************************************************
*
*   Returns a value from the top of a given stack.
*
*   Where:
*       Stack is a pointer to a stack structure
*
*   Returns:
*       Value from the top of a stack.  The value is removed from the stack.
*
******************************************************************************/
static int Pop( struct Stack *Stack )
{
    if( Stack->Top == 0 )
        return( BOTTOM_STACK );
    else
        return( Stack->Data[ --Stack->Top ] );
}


/******************************************************************************
*                                                                             *
*   int GetValue( char **sExpr )                                              *
*                                                                             *
*******************************************************************************
*
*   Returns a numerical value of a string.  Advances the given pointer.
*
*   Where:
*       sExpr is an address of a pointer to a string containing expression.
*
*   Returns:
*       Numerical value that the string contains.
*
******************************************************************************/
static int GetValue( char **sExpr )
{
    int value, base;
    char *sStart = *sExpr;
    char *sEnd = sStart;
    char ch;

    // Check if the first character is alphabetical, that's a literal
    if( isalpha(*sStart) )
    {
        // Find the end of a literal
        while( isalnum(*sEnd) || (*sEnd=='_') )
            sEnd++;

        if( pLiteralHandler != NULL )
        {
            ch = *sEnd;
            *sEnd = '\0';

            value = pLiteralHandler( sStart );

            // Restore end of literal character and return
            *sEnd = ch;
            *sExpr = sEnd;

            return( value );
        }
        else
        {
            // Not handling literals, return 0
            *sExpr = sEnd;

            return( 0 );
        }
    }

    // Assume base 10 and clear the initial value
    base  = 10;
    value = 0;

    // If the first character is '0', the base is presumed octal
    if( *sEnd=='0' )
        base = 8;

    // Traverse and find the last character comprising a value
    do
    {
        // Break out if the current character is not alphanumeric
        if( !isalnum( *sEnd ) )
            break;

        // Last char can be 'h' to set base to 16 or b to set base to 2
        ch = tolower(*sEnd);
        if( ch=='h' )
        {
            base = 16;

            break;
        }
        else
            if( ch=='b' )
                // If the next character is also alphanumeric, then this
                // 'b' cannot be binary designator
                if( !isalnum( *(sEnd+1) ) )
                {
                    base = 2;

                    break;
                };

        sEnd++;

    }while( 1 );

    // Scan from the start of the numerics and multiply/add them
    while( sStart < sEnd )
    {
        value *= base;
        value += (*sStart > '9')? tolower(*sStart) - 'a' + 10 : *sStart - '0';
        sStart++;
    }

    // These bases had extra character that now needs to be skipped
    if( base==2 || base==16 )
        sEnd++;

    *sExpr = sEnd;

    return( value );
}


/******************************************************************************
*                                                                             *
*   int TableMatch( char *sTable, char **sToken )                             *
*                                                                             *
*******************************************************************************
*
*   Returns a matching string from a token table or 0 if there was no match.
#   sToken pointer is advanced accordingly.  Spaces are ignored.
*
*   Where:
*       sTable is an array of pointers pointing to tokens.  Last entry is NULL.
#       sToken is an address of a pointer to string to be examined.
*
*   Returns:
*       Token number from an array or 0 if failed to match a token.
*
******************************************************************************/
static int TableMatch( char **sTable, char **sToken )
{
    char *sRef = sTable[0];
    int index = 0;

    // Skip over spaces
    while( *(*sToken)==' ' )
        *sToken += 1;

    // Find the matching substring in a table
    while( sRef != NULL )
    {
        if( !strncmp( sRef, *sToken, strlen(sRef) ) )
        {
            *sToken += strlen( sRef );

            return( index + 1 );
        }

        sRef = sTable[ ++index ];
    }

    return( 0 );
}


/******************************************************************************
*                                                                             *
*   void Execute( int Operation )                                             *
*                                                                             *
*******************************************************************************
*
*   Evaluates numbers on the stack depending on the operation opcode.
*
*   Where:
*       Operation is a code of the operation to be performed on a stack values.
*
*   Returns:
*       Operates on stacks Values and Operators and updates them accordingly.
*
******************************************************************************/
static void Execute( int Operation )
{
    int top;

    // Perform the operation
    switch( Operation )
    {
        case OP_BOOL_OR:Push( &Values, Pop( &Values ) || Pop( &Values ) );
            break;

        case OP_BOOL_AND:Push(&Values, Pop( &Values ) && Pop( &Values ) );
            break;

        case OP_OR:     Push( &Values, Pop( &Values ) | Pop( &Values ) );
            break;

        case OP_XOR:    Push( &Values, Pop( &Values ) ^ Pop( &Values ) );
            break;

        case OP_AND:    Push( &Values, Pop( &Values ) & Pop( &Values ) );
            break;

        case OP_EQ:     Push( &Values, Pop( &Values ) == Pop( &Values ) );
            break;

        case OP_NE:     Push( &Values, Pop( &Values ) != Pop( &Values ) );
            break;

        case OP_L:      Push( &Values, Pop( &Values ) < Pop( &Values ) );
            break;

        case OP_LE:     Push( &Values, Pop( &Values ) <= Pop( &Values ) );
            break;

        case OP_G:      Push( &Values, Pop( &Values ) > Pop( &Values ) );
            break;

        case OP_GE:     Push( &Values, Pop( &Values ) >= Pop( &Values ) );
            break;

        case OP_SHL:    Push( &Values, Pop( &Values ) << Pop( &Values ) );
            break;

        case OP_SHR:    Push( &Values, Pop( &Values ) >> Pop( &Values ) );
            break;

        case OP_PLUS:   Push( &Values, Pop( &Values ) + Pop( &Values ) );
            break;

        case OP_MINUS:  top = Pop( &Values );
                        Push( &Values, Pop( &Values ) - top );
            break;

        case OP_TIMES:  Push( &Values, Pop( &Values ) * Pop( &Values ) );
            break;

        case OP_DIV:    top = Pop( &Values );
                        if( top != 0 )
                            Push( &Values, Pop( &Values ) / top );
            break;

        case OP_MOD:    top = Pop( &Values );
                        if( top != 0 )
                            Push( &Values, Pop( &Values ) % top );
            break;

        case OP_NOT:    Push( &Values, ! Pop( &Values ) );
            break;

        case OP_NEG:    Push( &Values, -Pop( &Values ) );
            break;
    }
}


/******************************************************************************
*                                                                             *
*   void RegisterLiteralHandler( int (*nLiteralHandler)( char *sName ) )      *
*                                                                             *
*******************************************************************************
*
*   Registers an external function to handle literal string-to-value
#   conversions during the expression evaluation.
*
*   Where:
*       nLiteralHandler is a pointer to function that will accept a string
#       and return its numerical value.  NULL will deallocate function.
*
******************************************************************************/
void RegisterLiteralHandler( int (*nLiteralHandler)( char *sName ) )
{
    pLiteralHandler = nLiteralHandler;
}


/******************************************************************************
*                                                                             *
*   int nEvaluate( char *sExpr)                                               *
*                                                                             *
*******************************************************************************
*
*   Evaluates a string expression and returns its numerical value.
*
*   Where:
*       sExpr is a pointer to a zero-terminated string containing the
#           expression.
*
*   Returns:
*       Result of evaluation
*
******************************************************************************/
int nEvaluate( char *sExpr)
{
    int NewOp, OldOp;


    Values.Top = Operators.Top = 0;

    // Just in the case that the argument was NULL;
    if( sExpr==NULL )
        return( 0 );

    while( *sExpr != '\0' )
    {
        NewOp = TableMatch( sTokens, &sExpr);

        // Special case is unary minus in front of a value
        if( NewOp==OP_MINUS )
        {
            Push( &Operators, OP_NEG );

            continue;
        }else

        // If any operator was in front of a value, push it
        if( NewOp )
        {
            Push( &Operators, NewOp );

            continue;
        }

        Push( &Values, GetValue( &sExpr ) );
        NewOp = TableMatch( sTokens, &sExpr);

        // If there are no more operators, break out and clean the stack
        if( ! NewOp )
            break;

        OldOp = Pop( &Operators );

        // If the new op priority is less than the one on the stack, we
        // need to go down and evaluate terms
        if( NewOp < OldOp )
            Execute( OldOp );
        else
            Push( &Operators, OldOp );

        // Push new operation
        Push( &Operators, NewOp );
    }

    // Clean the stack by the means of evaluating expression in RPN
    while( (NewOp = Pop(&Operators)) != BOTTOM_STACK )
    {
        Execute( NewOp );
    }

    return( Pop( &Values ) );
}


//=============================================================================
#if TEST

int LiteralHandler( char *sName )
{
    if( !strcmp("var", sName) )
    {
        return( 100 );
    }

    return( 0 );
}

void main( int argn, char *argp[] )
{
    int Value;

    RegisterLiteralHandler( LiteralHandler );

    Value = nEvaluate( argp[1] );

    printf("%s = %d\n", argp[1], Value );

    // Print stacks
    printf("Values:\n");
    while( Values.Top )
        printf("   %d\n", Pop( &Values ) );

    printf("Operators:\n");
    while( Operators.Top )
        printf("   %d\n", Pop( &Operators ) );
}
#endif
