/******************************************************************************
*                                                                             *
*   Module:     _Setup.c                                                      *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       10/12/97                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This module contains the "C" portion of the initialization of a
        run-time module (startup).

        Function called by __assert macro is also here.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 10/12/97   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include <sys/clib.h>                   // Include private library header file

#include <stdlib.h>                     // Include standard library header

#include <errno.h>                      // Include error defines

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

int errno = 0;                          // Global error number

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   void _Init_Environ( int argc, char *argp[], char *envp[], int envc )      *
*                                                                             *
*******************************************************************************
*
*   This function sets up the array of pointers to environment strings
#   in dynamic memory.
#
#   The parameter envp holds the pointer to a first environment string and
#   envc contains the number of environ strings.
#
*   Where:
*       argc - number of arguments, unused here
#       argp - address of an array of pointers to argument strings (unused here)
#       envp - address of the first environment string
#       envc - number of environment strings that follow
*
*   Returns:
*       void
*
******************************************************************************/
void _Init_Environ( int argc, char *argp[], char *envp, int envc )
{
    int i = 0;


    // Allocate memory for environment array

    environ = (char**) _kMalloc( _dwDynamic, (envc + 1) * sizeof(char*) );

    // This really should not happen... we are probably toasted anyway

    if( environ == NULL )
    {
        environ = &envp;
        return;
    }

    // Traverse through the environment strings and store their location

    while( envc-- > 0 )
    {
        environ[i++] = envp;

        // Find the start of the next environment string

        while( *envp++ );
    }

    // Store the terminating NULL

    environ[i] = NULL;

    // Make the third parameter point to the environment array so that
    // the main may be used as `main(int argc, char *argp[], char *environ[])'

    envp = (char *) environ;
}


/******************************************************************************
*                                                                             *
*   void __assert( char * file, int line )                                    *
*                                                                             *
*******************************************************************************
*
*   This is bad assert function.  It gets called when an assert fails.
*
*   Where:
*       file is the file name
#       line is the line number
*
*   Returns:
*       void
*
******************************************************************************/
void __assert( char * file, int line )
{
    fprintf(stderr, "ASSERT FAILED: FILE `%s' LINE %d\n", file, line );

    exit(-1);
}

