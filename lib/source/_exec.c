/******************************************************************************
*                                                                             *
*   Module:     _Exec.c                                                       *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       10/20/97                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This module contains the code for the exec family of functions.

TO DO:

- where is `environ' declared ? in stdlib??
- how many arguments is allowed?   16 also in exec

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 10/20/97   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include <sys/syscall.h>                // Include system call header

#include <errno.h>                      // Include error header file

#include <unistd.h>                     // Include its own header

#include <stdarg.h>                     // Include variable argument list

#include <stdio.h>                      // Include standard io header fil

#include <stdlib.h>                     // Include standard library header

#include <string.h>                     // Include string header file

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

#define MAX_ARG  16

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
*   pid_t fork()                                                              *
*                                                                             *
*******************************************************************************
*
*   This function performs a fork() system call.
*
*   Where:
*       no args
*
*   Returns:
*       pid of a child process to child
#       0 to parent process
*
******************************************************************************/
pid_t fork()
{
    return SYS_CALL0( SYS_FORK );
}


/******************************************************************************
*                                                                             *
*   int execve(const char *path, char *const argv[], char *const *envp )      *
*                                                                             *
*******************************************************************************
*
*   This is the main exec* function.
*
*   Where:
*       path is the path/name of the program to execute
#       argv is the address of the array of pointers to argument list
#       envp is the address of the array of pointers to enviroment strings
*
*   Returns:
*       -1 if exec failed (errno is set)
*
******************************************************************************/
int execve(const char *path, char *const argv[], char *const *envp )
{
    char *tmp_argv[2] = { NULL, NULL };

    // At least one argument must be passed (by the convention, that is the
    // program name itself.  So, if the argv is NULL, assign it a path/name

    if( argv == NULL )  tmp_argv[0] = path;

    // If the envp is NULL, use the environment of the current process

    if( envp == NULL )  envp = environ;

    // Close all files that have FD_CLOEXEC flag set



    // This call should never return unless there was an error

    errno = SYS_CALL3( SYS_EXEC, path, (argv==NULL) ? tmp_argv : argv, envp );

    return( -1 );
}


/******************************************************************************
*                                                                             *
*   int execv( const char *path, char *const argv[] )                         *
*                                                                             *
*******************************************************************************
*
*   This is one of the exec* functions.
*
*   Where:
*       path is the path/name of the program to execute
#       argv is the address of the array of pointers to argument list
*
*   Returns:
*       -1 if exec failed (errno is set)
*
******************************************************************************/
int execv( const char *path, char *const argv[] )
{
    return( execve( path, argv, environ ) );
}


/******************************************************************************
*                                                                             *
*   int execl( const char *path, char const *arg0, ... )                      *
*                                                                             *
*******************************************************************************
*
*   This is one of the exec* functions.
*
*   Where:
*       path is the path/name of the program to execute
#       arg0 is the first argument to a process
#       ... followed by the successive arguments, last to be NULL
*
*   Returns:
*       -1 if exec failed (errno is set)
*
#   Note: The environment for the new process is taken from this process
#
******************************************************************************/
int execl( const char *path, char const *arg0, ... )
{
    va_list arg;
    char *Args[ MAX_ARG ];
    int i = 1;

    va_start( arg, arg0 );

    // Set up the first argument that we know of

    Args[0] = arg0;

    // Build the array of pointers to argument list

    do
    {
        Args[i] = va_arg( arg, char * );
    }
    while( (Args[i++] != NULL) && (i < MAX_ARG-1) );

    va_end( arg );

    return( execve( path, Args, environ ) );
}


/******************************************************************************
*                                                                             *
*   int execle( const char *path, char const *arg0, ... )                     *
*                                                                             *
*******************************************************************************
*
*   This is one of the exec* functions.
*
*   Where:
*       path is the path/name of the program to execute
#       arg0 is the first argument to a process
#       ... followed by the successive arguments, last to be the pointer to
#           the array of character pointers to the environment strings,
#           followed by NULL
*
*   Returns:
*       -1 if exec failed (errno is set)
*
******************************************************************************/
int execle( const char *path, char const *arg0, ... )
{
    va_list arg;
    char *Args[ MAX_ARG ], *pEnv;
    int i = 1;

    va_start( arg, arg0 );

    // Set up the first argument that we know of

    Args[0] = arg0;

    // Build the array of pointers to argument list

    do
    {
        Args[i] = va_arg( arg, char * );
    }
    while( (Args[i++] != NULL) && (i < MAX_ARG-1) );

    va_end( arg );

    // Last pointer was a pointer to the environment array

    if( i<=2 )
        pEnv = (char *) environ;
    else
    {
        pEnv = Args[i-2];
        Args[i-2] = NULL;
    }

    return( execve( path, Args, (char *const *) pEnv ) );
}


/******************************************************************************
*                                                                             *
*   int execvp( const char *file, char *const argv[] )                        *
*                                                                             *
*******************************************************************************
*
*   This is one of the exec* functions.
*
*   Where:
*       file is the name of the program to execute - this function will search
#           all directories in the PATH variable for the file, if the first
#           character of the file is not `/'
#       argv is the address of the array of pointers to argument list
*
*   Returns:
*       -1 if exec failed (errno is set)
#
#   Note: The environment for the new process is taken from this process
*
******************************************************************************/
int execvp( const char *file, char *const Args[] )
{
    char *pStart, *pEnd;
    char sPathName[ PATHNAME_MAX + 1 ];
    int fd;


    if( *file == '/' )
        return( execve( file, Args, environ ) );


    // Get the path variable that should exist

    if( (pStart = getenv("PATH")) == NULL)
    {
        errno = ENOENT;
        return( -1 );
    }

    // Loop for each path substring

    do
    {
        //--------------------------------------------------------------------
        // Form the path prefix based on the traversal of the PATH variable
        //--------------------------------------------------------------------

        // Skip the starting separators

        while( *pStart == ':' ) pStart++;

        // Exit if this is the end of path variable

        if( *pStart == '\0' ) break;

        // Find the separator

        pEnd = strchr( pStart + 1, ':' );

        // If the pEnd is NULL, there was no more separators

        if( pEnd == NULL )  pEnd = strchr( pStart + 1, '\0' );

        if( pStart==pEnd )  break;

        // Copy the substring to the temp pathname and form a name

        strncpy( sPathName, pStart, pEnd - pStart );
        sPathName[pEnd - pStart] = '\0';

        // Append a slash, but do not duplicate it

        if( *(pEnd-1) != '/' )  strcat( sPathName, "/" );

        // Append the file name

        strcat( sPathName, file );

//printf("* `%s'\n", sPathName );

        // Prepare for the next pass

        pStart = pEnd;

        //--------------------------------------------------------------------
        // Check if a file exists on a specified path
        //--------------------------------------------------------------------

        fd = open(sPathName, 0);

        // If there was an error opening the target file, loop again

        if( fd >= 0 )
        {
            // Close the file handle

            close(fd);

            // Try to execute that file

            execve( sPathName, Args, environ );

            // If we have come back, there was some error, but we can ignore
            // some of them and loop back

            if( (errno != ENOEXEC) && (errno != EACCESS) && (errno != ENOENT))
                return( -1 );
        }
    }
    while( 1 );

    // If we have break out of the loop, we couldnt find the program to run

    errno = ENOENT;

    return( -1 );
}


/******************************************************************************
*                                                                             *
*   int execlp( const char *file, char const *arg0, ... )                     *
*                                                                             *
*******************************************************************************
*
*   This is one of the exec* functions.
*
*   Where:
*       file is the name of the program to execute - this function will search
#           all directories in the PATH variable for the file, if the first
#           character of the file is not `/'
#       arg0 is the first argument to a process
#       ... followed by the successive arguments, last to be NULL
*
*   Returns:
*       -1 if exec failed (errno is set)
#
#   Note: The environment for the new process is taken from this process
*
******************************************************************************/
int execlp( const char *file, char const *arg0, ... )
{
    va_list arg;
    char *Args[ MAX_ARG ];
    int i = 1;


    va_start( arg, arg0 );

    // Set up the first argument that we know of

    Args[0] = arg0;

    // Build the array of pointers to argument list

    do
    {
        Args[i] = va_arg( arg, char * );
    }
    while( (Args[i++] != NULL) && (i < MAX_ARG-1) );

    va_end( arg );

    // Call the execvp function with the formed parameters

    return( execvp(file, Args) );
}

