/******************************************************************************
*                                                                             *
*   Module:     Init.c                                                        *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       11/8/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This module contains the code for The init process - the mother
        and father of all processes....

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 11/8/97    Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include <stdlib.h>

#include <fcntl.h>

#include <sys/types.h>

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


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

int main( int argc, char *argp[], char **env )
{
    int s_stdin, s_stdout, s_stderr;
    pid_t child;


    // Open standard input, output and error streams

    s_stdin  = open("/dev/tty0", O_RDONLY );
    s_stdout = open("/dev/tty0", O_WRONLY );
    s_stderr = open("/dev/tty0", O_WRONLY );



#if 1
    printf("\nNumber of arguments: %d\n", argc );

    for( i=0; i<argc; i++ )
    {
        printf("%d.  %08X %s\n", i, argp[i], argp[i] );
    }
#endif

#if 1
    printf("Environment variables using `environ' variable:\n");

    for( i=0; environ[i] != NULL; i++ )
    {
        printf(" %s\n", environ[i] );
    }

    printf("Using env parameter:\n");

    for( i=0; env[i] != NULL; i++ )
    {
        printf(" %s\n", env[i] );
    }
#endif
while( 1 )
{
}



    // Start the test process

    if( child = fork() )
    {
        execve("/tmp/test.exp", argp, environ );
    }

    // Loop forever...

    while( 1 );
}

