/******************************************************************************
*
*   Module:     snoop.c
*
*   Revision:   1.00
*
*   Date:       09/05/96
*
*   Author:     Goran Devic
*
*******************************************************************************

    Module Description:

          This program will print the information about an EXP file.

*******************************************************************************
*
*   Changes:
*
*   DATE     REV   DESCRIPTION OF CHANGES                          AUTHOR
* --------   ----  ---------------------------------------------   -----------
* 09/05/96   1.00  Original                                        Goran Devic
* --------   ----  ---------------------------------------------   -----------
*******************************************************************************
*   Include Files
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/******************************************************************************
*   Global Variables
******************************************************************************/

/******************************************************************************
*   Local Defines, Variables and Macros
******************************************************************************/

typedef unsigned short WORD;
typedef unsigned int DWORD;


typedef struct
{
    char    cSig[2];
    WORD    wSzRem;
    WORD    wSize;
    WORD    wReloc;
    WORD    wHeaderSize;
    WORD    wMinPages;
    WORD    wMaxPages;
    DWORD   dwESP;
    WORD    wChecksum;
    DWORD   dwEIP;
    WORD    wRelocItem;
    WORD    wOverlay;
    WORD    wReserved;

} TEXP;

/******************************************************************************
*   Functions
******************************************************************************/

long int filesize( FILE *fp )
{
    long int save_pos, size_of_file;

    save_pos = ftell( fp );
    fseek( fp, 0L, SEEK_END );
    size_of_file = ftell( fp );
    fseek( fp, save_pos, SEEK_SET );

    return( size_of_file );
}


int main( int argn, char *argv[], char *envp[] )
{
    FILE *fp;
    int size, i;
    char *ptr;
    TEXP *pHead;

    printf("argn: %d\n", argn );
    for( i=0; i<=argn; i++ )
    {
        printf("argv[%d]: %08Xh-> %s\n", i, argv[i], argv[i] );
    }

    fp = fopen( "hello.exp", "r+b" );
    if( fp == NULL )
    {
        printf("Cannot open\n");
        exit(1);
    }

    size = filesize( fp );
    printf("File size: \t\t\t\t%d bytes\n", size );
    ptr = malloc( size );
    if( ptr==NULL )
    {
        printf("Cannot malloc\n");
        exit(1);
    }

    fread( ptr, size, 1, fp );
    fclose(fp);

    pHead = (TEXP *) ptr;

    printf("Signature: \t\t\t\t""%c%c""\n", pHead->cSig[0], pHead->cSig[1] );
    printf("Remainder of the size: \t\t\t%d\n", pHead->wSzRem );
    printf("Size in pages: \t\t\t\t%d\n", pHead->wSize );
    printf("Number of relocation entries: \t\t%d\n", pHead->wReloc );
    printf("Header size in paragraphs: \t\t%d\n", pHead->wHeaderSize );
    printf("Minimum number of extra pages: \t\t%d\n", pHead->wMinPages );
    printf("Maximum number of extra pages: \t\t%d\n", pHead->wMaxPages );
    printf("Initial ESP: \t\t\t\t%08X\n", pHead->dwESP );
    printf("Checksum: \t\t\t\t%04X\n", pHead->wChecksum );
    printf("Initial EIP: \t\t\t\t%08X\n", pHead->dwEIP );
    printf("Offset of the first relocation item: \t%04X\n", pHead->wRelocItem );
    printf("Overlay number: \t\t\t%d\n", pHead->wOverlay );
    printf("Reserved word: \t\t\t\t%04X\n", pHead->wReserved );


    free(ptr);
}
