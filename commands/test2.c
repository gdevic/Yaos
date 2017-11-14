
#include <stdio.h>
#include <stdlib.h>

extern char **environ;


int main( int argc, char *argp[], char **env )
{
    int i, pid;
    int handle;

    printf("Hello from a child process!\n");


#if 1
    printf("\nChild: Number of arguments: %d", argc );

    for( i=0; i<argc; i++ )
    {
        printf(" %08X %s\n", argp[i], argp[i] );
    }
#endif

#if 1
    printf("Child: Environment variables:\n");

    for( i=0; environ[i] != NULL; i++ )
    {
        printf(" %s\n", environ[i] );
    }

    printf("Child: Environment variables using `environ' variable:\n");

    for( i=0; env[i] != NULL; i++ )
    {
        printf(" %s\n", env[i] );
    }
#endif

    while(1);
    return 0;
}