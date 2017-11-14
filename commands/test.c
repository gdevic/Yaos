
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <setjmp.h>
#include <dirent.h>
#include <assert.h>


#pragma aux Int3 = "int 3";

//char *en1[] = { "SET=NULL", "PATH=::.:/:/Yaos:/tmp", NULL };
char *en1[] = { "SET=NULL", "PATH=/", NULL };

jmp_buf jmp1;

void fn()
{
static i=0;
    printf("Function\n");
    if( i++==0 )
        longjmp(jmp1, 1);
}

int main( int argc, char *argp[], char **env )
//int main( )
{
    int i=0, pid;
    char sBuf[80];
    char sName[20];
    int handle_in, handle_out;
    char *buffer=":";
    char *tok;
    int s_stdin, s_stdout, s_stderr;
    pid_t child;
    FILE *fp, *fp2;
    DIR *pDir;
    struct dirent *direntp;


    // Open standard input, output and error streams

    s_stdin  = open("/dev/tty0", O_RDONLY );
    s_stdout = open("/dev/tty0", O_WRONLY );
    s_stderr = open("/dev/tty0", O_WRONLY );

    printf("Init process: stdin stdout stderr -> /dev/tty0\n\n");

#if 1
    printf("Environment variables:\n");

    for( i=0; environ[i] != NULL; i++ )
    {
        printf(" %s\n", environ[i] );
    }
#endif

#if 1
    printf("\nNumber of arguments: %d\n", argc );

    for( i=0; i<argc; i++ )
    {
        printf("%d.  %08X %s\n", i, argp[i], argp[i] );
    }
#endif

#if 0
    // Test directory functions

    printf("Opening a directory...\n");

    pDir = opendir("/");
    if( pDir != NULL )
    {
        printf("Dir opened\n");

        for( ; ; i++ )
        {
            if( i==4 ) rewinddir( direntp );

            direntp = readdir( pDir );
            if( direntp == NULL )  break;
            printf(" %s\n", direntp->d_name);
        }

    }
    else
        printf("Cannot open dir\n");

    while( 1 );

#endif

#if 0
    // Test jumps

    printf("A\n");
    i = setjmp(jmp1);
    printf("Jump=%d\n", i );

    if( i==0 )
        fn();

    printf("B\n");

    while( 1 );

#endif

#if 0
    // Test stdio-h-ish stuff

    i = 1;
    fp = fopen("test.txt", "r");
    printf("fp=%08X\n", fp);

    if( fp!=NULL )
    {
        do
        {
            putchar( getc(fp) );
            i++;

        } while( !feof(fp) );
    }
    printf("\nCount: %d\n", i );

    while( 1 );

#endif

#if 0
    // Test stdio-h-ish stuff

    fp = fopen("test.out", "w");

    if( fp!=NULL )
    {
        fprintf(fp, "AAA");
    }
    else
        printf("\n\n\n\nCant open\n");

    fclose(fp);
    while( 1 );

#endif

#if 1

    // This is a little shell...
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n");

    if( i = fork() )
    {
        while( 1 )
        {
            printf("HOME>");

            gets(sBuf);
            tok = strchr(sBuf, ' ');
            *tok = '\0';
            strcpy(sName, sBuf);
            strcat(sName, ".exp");

            if( fork() )
            {
                execlp(sName, sBuf, tok+1, NULL );
                exit(1);
            }
            else
                wait(NULL);
        }
    }

    while( 1 );
#endif

#if 1
    printf("\nNumber of arguments: %d\n", argc );

    for( i=0; i<argc; i++ )
    {
        printf("%d.  %08X %s\n", i, argp[i], argp[i] );
    }
#endif

#if 0
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

#if 0
    if( fork() )
    {
//        i = execve("/tmp/test2.exp", NULL, NULL );
//        i = execve("/tmp/test2.exp", argp, environ );
//        i = execl("/tmp/test2.exp", "X1" );
//        i = execle("/tmp/test2.exp", "X1", "X2", en1, NULL );
//        i = execlp("test2.exp", "X1", NULL );
        i = execlp("banner.exp", "banner", "YAOS 2.0", NULL );

        printf("Error: Exec returned: %d  errno=%d\n", i, errno);
    }

    while( 1 );
#endif

#if 0
    if( pid = fork() )
    {
//        printf("Child pid:%d...\n", pid );
//        i = execlp("banner.exp", "banner", "YAOS 2.0", NULL );

        printf("Type something: ");
while(1)
{
//Int3();
        i = read(0, buffer, 1);

        printf(" read %d --> %s <--\n", i, buffer );
}

while(1);

//        i = execlp("banner.exp", "banner", "YAOS 2.0", NULL );
//        i = execle("/tmp/test2.exp", "X1", "X2", en1, NULL );

//        printf("Exec returned: %d\n", i);
        exit(0);
    }
    else
    {
        printf("Parent...\n");
while(1);

        i = wait( NULL );

        printf("wait() returned %d\n", i );
    }


    printf("Back\n");

#endif
    // Loop forever...

    while( 1 );

    return( 0 );
}

